#!/bin/bash

BUILD_DIR="build"

if [ -d "$BUILD_DIR" ]; then
    echo "Suppression de : $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

mkdir "$BUILD_DIR"
cp Makefile "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Début de la compilation..."

make all

cd ..

