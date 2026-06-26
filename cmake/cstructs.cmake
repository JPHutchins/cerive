# Integrate the `cstructs` python tooling. The venv is materialised from the
# committed lockfile by a build step that re-runs only when uv.lock changes, so
# configures stay side-effect-free and the environment is reproducible.

find_program(UV uv REQUIRED)

set(PYDIR ${CMAKE_SOURCE_DIR}/cmake/python)
set(CSTRUCTS ${PYDIR}/.venv/bin/cstructs)
set(CSTRUCTS_STAMP ${CMAKE_BINARY_DIR}/cstructs-venv.stamp)

file(GLOB CSTRUCTS_SRCS CONFIGURE_DEPENDS ${PYDIR}/src/cstructs/*.py)

add_custom_command(
	OUTPUT ${CSTRUCTS_STAMP}
	COMMAND ${UV} sync --frozen --project ${PYDIR}
	COMMAND ${CMAKE_COMMAND} -E touch ${CSTRUCTS_STAMP}
	DEPENDS ${PYDIR}/uv.lock
	COMMENT "cstructs: uv sync --frozen (uv.lock changed)"
	VERBATIM)

add_custom_target(cstructs_env DEPENDS ${CSTRUCTS_STAMP})
