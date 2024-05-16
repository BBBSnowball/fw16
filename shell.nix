{ pkgs ? import <nixpkgs> {} }:
with pkgs;
mkShell {
  packages = [
    platformio
    (python3.withPackages (p: [ p.pyserial p.pillow ]))
    picotool
  ];
}
