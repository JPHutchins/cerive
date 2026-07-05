{
  description = "X-macro derive experiments for embedded C23 — ARM32 codegen evidence harness";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

    # Task runner + MCP server: tasks.py is the SSOT for validation (devs, CI, agents).
    camas.url = "github:JPHutchins/camas/0.1.22";

    # Zero-config C formatter (Rust, no flake of its own) for the agent autofix node.
    jphfmt = {
      url = "github:JPHutchins/jphfmt/v0.1.3";
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
          version = "0.1.3";
          src = jphfmt;
          cargoLock.lockFile = jphfmt + "/Cargo.lock";
        };
        camas = camas.packages.${system}.with-mcp;
      });

      devShells = forAllSystems (system: pkgs: {
        default = pkgs.mkShellNoCC {
          packages = [
            pkgs.gcc-arm-embedded # arm-none-eabi-gcc 15.2 + newlib + semihosting specs
            pkgs.qemu # qemu-system-arm (semihosting test runner)
            pkgs.cmake
            pkgs.ninja
            pkgs.astyle # pretty-prints preprocessed macro expansions
            pkgs.uv # manages the `cstructs` python tooling (cmake/python/)
            pkgs.python314 # interpreter uv resolves against
            pkgs.clang # scan-build for clang static analyzer
            camas.packages.${system}.with-mcp # task runner + MCP server (tasks.py SSOT)
            self.packages.${system}.jphfmt # C formatter driven by the autofix node
          ];

          # Make uv use the nix-provided interpreter instead of downloading one.
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
