#!/bin/sh

n="resvg-0.45.1"
p="$1/.."

tar -xJf "$p"/packagefiles/"$n".tar.xz -C "$p"

if [ "$4" = "debug" ]; then
    cargo build --manifest-path="$p"/"$n"/Cargo.toml --target-dir="$2" --workspace --all-features
    cp "$2"/debug/libresvg.a "$3"
elif [ "$4" = "release" ]; then
    cargo build --manifest-path="$p"/"$n"/Cargo.toml --target-dir="$2" --workspace --all-features --release
    cp "$2"/release/libresvg.a "$3"
fi

cp "$p"/"$n"/crates/c-api/resvg.h "$3"
