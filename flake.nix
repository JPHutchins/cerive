{
  description = "X-macro derive experiments for embedded C23 — ARM32 codegen evidence harness";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in {
      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShellNoCC {
          packages = [
            pkgs.gcc-arm-embedded # arm-none-eabi-gcc 15.2 + newlib + semihosting specs
            pkgs.qemu # qemu-system-arm (semihosting test runner)
            pkgs.cmake
            pkgs.ninja
            pkgs.astyle # pretty-prints preprocessed macro expansions
            pkgs.uv # manages the `cstructs` python tooling (cmake/python/)
            pkgs.python314 # interpreter uv resolves against
          ];

          # Make uv use the nix-provided interpreter instead of downloading one.
          UV_PYTHON_PREFERENCE = "only-system";
          UV_PYTHON = "${pkgs.python314}/bin/python3.14";
        };
      });
    };
}
