{ pkgs, ... }:

{
    sqlite_orm = pkgs.stdenv.mkDerivation rec {
        name = "sqlite_orm-${version}";
        src = pkgs.fetchurl {
            url = "https://github.com/fnc12/sqlite_orm/releases/download/${version}/sqlite_orm.h";
            sha256 = "1bzb7z1fw2321zzrmnfjzwpbihrvmgbcljhhzn6hnx76nzs6rz6c";
        };

        version = "1.6";

        unpackCmd = "mkdir tmp";

        installPhase = ''
            mkdir -p $out/include/sqlite_orm
            cp ${src} $out/include/sqlite_orm/sqlite_orm.h
        '';
    };

    cpp-httplib = pkgs.stdenv.mkDerivation rec {
        name = "cpp-httplib-${version}";
        src = pkgs.fetchFromGitHub {
            owner = "yhirose";
            repo = "cpp-httplib";
            rev = "v${version}";
            sha256 = "19hm3zrqz507kw6pq76i9z9awl22zjgnzqr1yaad5g4yg14iy8w6";
        };

        version = "0.8.6";

        installPhase = ''
            mkdir -p $out/include/httplib
            cp ${src}/httplib.h $out/include/httplib/httplib.h
        '';
    };
}