{ pkgs ? import <nixpkgs> {}, ... }:

with import ./deps.nix { inherit pkgs; };
pkgs.stdenv.mkDerivation {
    name = "itp2-assignment-4";
    src = ./.;

    nativeBuildInputs = with pkgs; [ cmake ];
    buildInputs = with pkgs; [
        sqlite
        gcc
        binutils-unwrapped
        sqlite_orm
        cpp-httplib
        cryptopp
        nlohmann_json
    ];

    cmakeFlags = [];

    patchPhase = ''
        cp -r ${sqlite_orm}/include/* .
        cp -r ${cpp-httplib}/include/* .
    '';

    configurePhase = ''
        cmake .
    '';

    installPhase = ''
        make
        mkdir -p $out/bin
        cp -r assignment4 * $out/bin
    '';
}