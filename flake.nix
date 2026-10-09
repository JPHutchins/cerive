{
  description = "X-macro derive experiments for embedded C23 — ARM32 codegen evidence harness";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

    camas.url = "github:JPHutchins/camas/0.1.30";

    jphfmt = {
      url = "github:JPHutchins/jphfmt/v0.3.0";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, camas, jphfmt }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f system nixpkgs.legacyPackages.${system});
      headerLines = builtins.filter builtins.isString (builtins.split "\n" (builtins.readFile ./include/cerive/cerive.h));
      versionParts = builtins.listToAttrs (builtins.concatMap
        (line:
          let m = builtins.match "#define CERIVE_VERSION_(MAJOR|MINOR|PATCH) ([0-9]+)" line;
          in if m == null then [ ] else [{ name = builtins.elemAt m 0; value = builtins.elemAt m 1; }])
        headerLines);
      version =
        if versionParts ? MAJOR && versionParts ? MINOR && versionParts ? PATCH
        then "${versionParts.MAJOR}.${versionParts.MINOR}.${versionParts.PATCH}"
        else throw "cerive.h declares no complete CERIVE_VERSION_MAJOR/MINOR/PATCH";
    in {
      packages = forAllSystems (system: pkgs: {
        cerive = pkgs.stdenvNoCC.mkDerivation {
          pname = "cerive";
          inherit version;
          src = pkgs.lib.fileset.toSource {
            root = ./.;
            fileset = pkgs.lib.fileset.unions [
              ./CMakeLists.txt
              ./Kconfig
              ./LICENSE
              ./include
              ./src
              ./zephyr/module.yml
            ];
          };
          installPhase = "cp -r . $out";
        };
        default = self.packages.${system}.cerive;
        jphfmt = pkgs.rustPlatform.buildRustPackage {
          pname = "jphfmt";
          version = "0.3.0";
          src = jphfmt;
          cargoLock.lockFile = jphfmt + "/Cargo.lock";
        };
        camas = camas.packages.${system}.with-mcp;
      });

      checks = forAllSystems (system: pkgs: {
        consumer = pkgs.stdenv.mkDerivation {
          pname = "cerive-consumer";
          inherit version;
          src = pkgs.lib.fileset.toSource {
            root = ./.;
            fileset = pkgs.lib.fileset.unions [
              ./tests/consumer
              ./tests/test_shapes.c
              ./tests/check.h
              ./variants/cerive
            ];
          };
          nativeBuildInputs = [ pkgs.cmake pkgs.ninja ];
          cmakeDir = "../tests/consumer";
          cmakeFlags = [ "-DCERIVE_DIR=${self.packages.${system}.cerive}" ];
          doCheck = true;
        };
      });

      devShells = forAllSystems (system: pkgs: {
        default = pkgs.mkShellNoCC {
          packages = [
            pkgs.gcc-arm-embedded
            pkgs.qemu
            pkgs.cmake
            pkgs.ninja
            pkgs.astyle
            pkgs.uv
            pkgs.python314
            pkgs.clang
            camas.packages.${system}.with-mcp
            self.packages.${system}.jphfmt
          ];

          UV_PYTHON_PREFERENCE = "only-system";
          UV_PYTHON = "${pkgs.python314}/bin/python3.14";

          shellHook = "unset PYTHONPATH";
        };
      });
    };
}
