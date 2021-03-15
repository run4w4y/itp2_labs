{ pkgs ? import <nixos-unstable> {}, ... }:

pkgs.mkShell {
    name = "itp2-assignment-2";

    buildInputs = with pkgs; [
        git
        gcc10
        binutils-unwrapped
    ];
    
    shellHook = ''
        export TERM=xterm-256color
    '';
}