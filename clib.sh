#!/usr/bin/env bash

set -e

# Common variables
HOST="dray-nuklear"
LIB_NAME="raylib_nuklear"
SOURCETREE_URL="https://github.com/redthing1/raylib-nuklear.git"
SOURCETREE_DIR="raylib_nuklear_source"
SOURCETREE_BRANCH="master"
LIB_FILE_1="libraylib_nuklear.a"
PACKAGE_DIR=$(dirname "$0")

# Utility variables
LN="ln"
if [[ "$OSTYPE" == "darwin"* ]]; then
    LN="gln"
fi

# Function to check if a command is available
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to ensure a command is available
ensure_command() {
    if ! command_exists "$1"; then
        echo "Error: $1 is not installed or not in PATH"
        echo "Please install $1 and try again"
        exit 1
    fi
    echo "$1 is available"
}

# Ensure all required commands are available
ensure_command "git"
ensure_command "make"
ensure_command "cmake"

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

    # set up build args
    cmake_args=()
    if [ -z "$RAYLIB_DIR" ]; then
        echo "RAYLIB_DIR is not set.."
        exit 1
    else
        cmake_args+=("-DRAYLIB_DIR=$RAYLIB_DIR")
    fi
    if [ "$RELEASE" = "1" ]; then
        cmake_args+=("-DCMAKE_BUILD_TYPE=Release")
    else
        cmake_args+=("-DCMAKE_BUILD_TYPE=Debug")
    fi

    cmake_args_string="${cmake_args[*]}"
    
    # START BUILD
    rm -rf build
    cmake -B build -S . $cmake_args
    cmake --build build --config Release
    # END BUILD

    echo "[$HOST] finished build of $LIB_NAME"
    echo "[$HOST] copying $LIB_NAME binary ($LIB_FILE_1) to $PACKAGE_DIR"
    $LN -vrfs $(pwd)/$LIB_FILE_1 $PACKAGE_DIR/$LIB_FILE_1
}

# Main execution
main() {
    # export all other KEY=VALUE pairs as environment variables
    for arg in "${@:2}"; do
        export $arg
    done
    if [ "$1" = "prepare" ]; then
        prepare
    elif [ "$1" = "build" ]; then
        build $@
    else
        echo "Usage: $0 [prepare|build] [build arguments...]"
        exit 1
    fi
}

main "$@"