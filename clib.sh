#!/usr/bin/env bash

set -e

# Common variables
HOST="dray-nuklear"
LIB_NAME="raylib_nuklear"
SOURCETREE_URL="https://github.com/redthing1/raylib-nuklear.git"
SOURCETREE_DIR="raylib_nuklear_source"
SOURCETREE_BRANCH="v0.5.0-r1"
LIB_FILE_1="libraylib_nuklear.a"
PACKAGE_DIR=$(dirname "$0")

# Utility variables
LN="ln"
if [[ "$OSTYPE" == "darwin"* ]]; then
    LN="gln"
fi

# Function to prepare the source
prepare() {
    echo "[$HOST] preparing $LIB_NAME source..."
    cd "$PACKAGE_DIR"
    
    if [ -d $SOURCETREE_DIR ]; then
        echo "[$HOST] source folder already exists, using it."
    else
        echo "[$HOST] getting source to build $LIB_NAME"
        git clone --depth 1 --branch $SOURCETREE_BRANCH $SOURCETREE_URL $SOURCETREE_DIR
    fi

    cd $SOURCETREE_DIR
    git submodule update --init --recursive
    echo "[$HOST] finished preparing $LIB_NAME source"
}

# Function to build the library
build() {
    echo "[$HOST] starting build of $LIB_NAME"
    cd "$PACKAGE_DIR/$SOURCETREE_DIR"
    
    # START BUILD
    make clean
    mkdir -p build # create build directory
    make $@
    # END BUILD

    echo "[$HOST] finished build of $LIB_NAME"
    echo "[$HOST] copying $LIB_NAME binary ($LIB_FILE_1) to $PACKAGE_DIR"
    $LN -vrfs $(pwd)/$LIB_FILE_1 $PACKAGE_DIR/$LIB_FILE_1
}

# Main execution
main() {
    if [ "$1" = "prepare" ]; then
        prepare
    elif [ "$1" = "build" ]; then
        shift # Remove the "build" argument
        prepare # Always call prepare before build
        build $@
    else
        echo "Usage: $0 [prepare|build] [build arguments...]"
        exit 1
    fi
}

main "$@"