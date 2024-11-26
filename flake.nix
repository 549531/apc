{
  description = "Description for the project";

  inputs = {
    devenv-root = {
      url = "file+file:///dev/null";
      flake = false;
    };
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:cachix/devenv-nixpkgs/rolling";
    devenv.url = "github:cachix/devenv";
    nix2container.url = "github:nlewo/nix2container";
    nix2container.inputs.nixpkgs.follows = "nixpkgs";
    mk-shell-bin.url = "github:rrbutani/nix-mk-shell-bin";

    cpm-cmake = {
      url = "https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake";
      flake = false;
    };
    incbin = {
      url = "github:graphitemaster/incbin";
      flake = false;
    };
  };

  nixConfig = {
    extra-trusted-public-keys = "devenv.cachix.org-1:w1cLUi8dv3hnoSPGAuibQv+f9TZLr6cv/Hm9XgU50cw=";
    extra-substituters = "https://devenv.cachix.org";
  };

  outputs = inputs@{ flake-parts, devenv-root, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [
        inputs.devenv.flakeModule
      ];
      systems = [ "x86_64-linux" "i686-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ];

      perSystem = { config, self', inputs', pkgs, system, ... }: {

        packages = {
          default = self'.packages.apc;

          apc = pkgs.stdenv.mkDerivation {
            pname = "apc";
            version = "1.0";
            src = ./.;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
            ];

            buildInputs = [
              pkgs.SDL2
              pkgs.glew
            ] ++ (if pkgs.stdenv.hostPlatform.isDarwin then [
              pkgs.OpenGL
            ] else [
              pkgs.libGL
              pkgs.xorg.libICE
              pkgs.xorg.libSM
              pkgs.xorg.libXext
            ]);

            patchPhase = ''
              cp -f ${inputs.cpm-cmake} cmake/CPM.cmake
            '';

            cmakeFlags = [
              "-DCPM_USE_LOCAL_PACKAGES=ON"
              "-DCPM_incbin_SOURCE=${config.packages.incbin}"
            ];
          };

          incbin = pkgs.linkFarm "incbin" {
            "incbin.h" = "${inputs.incbin}/incbin.h";
          };

          ci = pkgs.linkFarm "ci" {
            inherit (self'.packages)
              apc
              ;
          };

        };

        devenv.shells.default = {
          devenv.root =
            let
              devenvRootFileContent = builtins.readFile devenv-root.outPath;
            in
            pkgs.lib.mkIf (devenvRootFileContent != "") devenvRootFileContent;

          name = "apc";

          packages = [
            pkgs.clang-tools
            pkgs.clang
            pkgs.clinfo
            pkgs.cmake-format
            pkgs.devenv
            pkgs.glslls
            pkgs.nixpkgs-fmt
            pkgs.treefmt
          ] ++ config.packages.apc.buildInputs ++ config.packages.apc.nativeBuildInputs;
        };

      };
    };
}
