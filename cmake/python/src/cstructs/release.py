"""Release tooling."""

import re
from typing import NamedTuple, assert_never

_VERSION_PART = re.compile(
    r"^#define CERIVE_VERSION_(?P<part>MAJOR|MINOR|PATCH) (?P<value>\d+)$", re.MULTILINE
)


class Version(NamedTuple):
    """A release version."""

    major: int
    minor: int
    patch: int

    @property
    def tag(self) -> str:
        """The git tag of this version.

        >>> Version(0, 1, 0).tag
        'v0.1.0'
        """
        return f"v{self.major}.{self.minor}.{self.patch}"


def header_version(header: str) -> Version | None:
    r"""The version that cerive.h declares.

    >>> header_version(
    ...     "#define CERIVE_VERSION_MAJOR 0\n"
    ...     "#define CERIVE_VERSION_MINOR 1\n"
    ...     "#define CERIVE_VERSION_PATCH 2\n"
    ... )
    Version(major=0, minor=1, patch=2)
    >>> header_version("#define CERIVE_VERSION_MAJOR 0\n") is None
    True
    """
    match {m["part"]: int(m["value"]) for m in _VERSION_PART.finditer(header)}:
        case {"MAJOR": int(major), "MINOR": int(minor), "PATCH": int(patch)}:
            return Version(major, minor, patch)
        case _:
            return None


class Released(NamedTuple):
    """A version gate verdict."""

    version: Version


class Mismatched(NamedTuple):
    """A version gate verdict."""

    version: Version
    tag: str


class Unversioned(NamedTuple):
    """A version gate verdict."""


type Verdict = Released | Mismatched | Unversioned


def gate(header: str, tag: str) -> Verdict:
    r"""Judge a release tag against the version that cerive.h declares.

    >>> header = (
    ...     "#define CERIVE_VERSION_MAJOR 0\n"
    ...     "#define CERIVE_VERSION_MINOR 1\n"
    ...     "#define CERIVE_VERSION_PATCH 0\n"
    ... )
    >>> gate(header, "v0.1.0")
    Released(version=Version(major=0, minor=1, patch=0))
    >>> gate(header, "v0.1.1")
    Mismatched(version=Version(major=0, minor=1, patch=0), tag='v0.1.1')
    >>> gate("", "v0.1.0")
    Unversioned()
    """
    match header_version(header):
        case None:
            return Unversioned()
        case version if version.tag == tag:
            return Released(version)
        case version:
            return Mismatched(version, tag)


def problem(verdict: Verdict) -> str | None:
    """Why a verdict blocks the release.

    >>> problem(Released(Version(0, 1, 0))) is None
    True
    >>> problem(Mismatched(Version(0, 1, 0), "v0.2.0"))
    'cerive.h is v0.1.0, the tag is v0.2.0'
    >>> problem(Unversioned())
    'cerive.h declares no complete CERIVE_VERSION_MAJOR/MINOR/PATCH'
    """
    match verdict:
        case Released():
            return None
        case Mismatched(version=version, tag=tag):
            return f"cerive.h is {version.tag}, the tag is {tag}"
        case Unversioned():
            return "cerive.h declares no complete CERIVE_VERSION_MAJOR/MINOR/PATCH"
        case _:
            assert_never(verdict)


class Assets(NamedTuple):
    """The files a release attaches."""

    report: str
    matrix: str


def assets(tag: str) -> Assets:
    """The release asset names for a tag.

    >>> assets("v0.1.0")
    Assets(report='cerive-v0.1.0-evidence.md', matrix='cerive-v0.1.0-evidence-matrix.tar.gz')
    """
    return Assets(f"cerive-{tag}-evidence.md", f"cerive-{tag}-evidence-matrix.tar.gz")
