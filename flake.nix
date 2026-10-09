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

          shellHook = "unset PYTHONPATH";
        };
      });
    };
}
