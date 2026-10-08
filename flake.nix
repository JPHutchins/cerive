{
  description = "X-macro derive experiments for embedded C23 — ARM32 codegen evidence harness";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

    camas.url = "github:JPHutchins/camas/0.1.29";

    jphfmt = {
      url = "github:JPHutchins/jphfmt/v0.3.0";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, camas, jphfmt }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f system nixpkgs.legacyPackages.${system});
    in {
      packages = forAllSystems (system: pkgs: {
        jphfmt = pkgs.rustPlatform.buildRustPackage {
          pname = "jphfmt";
          version = "0.3.0";
          src = jphfmt;
          cargoLock.lockFile = jphfmt + "/Cargo.lock";
        };
        camas = camas.packages.${system}.with-mcp;
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

          # camas (a Python app in `packages`) drags its whole python3.13 closure onto
          # PYTHONPATH via mkShell; that leaks into uv/uvx subprocesses and breaks
          # `camas mcp` (a stale pydantic_core shadows the isolated one). camas's own
          # flake unsets it for the same reason.
          shellHook = "unset PYTHONPATH";
        };
      });
    };
}
