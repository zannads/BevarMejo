#!/bin/bash
set -e

# This is mandatory software
cmake --version
git --version

# Parse arguments: two things can be done at this moment:
# 1. dev mode [--dev flag]
# 2. reproducibility mode (builds all previous releases) [--reproducibility flag]
BRANCH="master"
REPRODUCIBILITY=false
BUILD_EPANET=false
while [[ $# -gt 0 ]]; do
    case "$1" in
        --dev)
            BRANCH="dev/main"
            shift
            ;;
        --reproducibility)
            REPRODUCIBILITY=true
            shift
            ;;
        --build-epanet)
            BUILD_EPANET=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

cd .. # Move out of the scripts folder

git checkout $BRANCH

# Prepare the submodule and add my remote to have the extra branches
git submodule init
git submodule update
cd bemelib/extern/EPANET
# if my remote has not been added yet, add it (necessary for the branches)
if ! git remote | grep -q '^worktree-src$'; then
    git remote add --tags worktree-src git@github.com:zannads/EPANET.git
fi
# If the worktree folder has not been made ready, prepare it
if [ ! -d ../EPANET.beme ]; then
    mkdir ../EPANET.beme
fi

# Function to create the EPANET worktrees with my naming convention
create_EPANET_worktree() {
    local ver=$1
    local dir="../EPANET.beme/${ver}"

    if [ -d "$dir" ]; then
       git worktree remove "$dir"
    fi
    git worktree add "$dir" "beme/en-v$ver"

    if [ BUILD_EPANET ]; then
        cmake -B "$dir/build" -S "$dir"
        cmake --build "$dir/build"
    fi
}

# Actually checkout the default worktree 
create_EPANET_worktree "25.9.30"

# If reproducibility mode on, prepare also the other worktrees
if [ "$REPRODUCIBILITY" = true ]; then
    echo "Reproducibility mode enabled. Preparing also older EPANET versions on their own worktrees"
    create_EPANET_worktree "24.12.21"
    create_EPANET_worktree "24.6.18"
fi

# EPANET setup completed.
# ----------------------------

cd ../../../ # Move back to main folder

# Function to build bemelib with CMake (with my naming convention)
build_beme_cmake() {
    local ver=$1
    local dir="./releases/${ver}"

    # Clean out the folder before building (if exists)
    if [ -d "$dir" ]; then
        rm -r "$dir"
    fi

    # Configure and build the projet
    cmake -B "$dir" -S ./bemelib "-DPROJECT_VERSION:STRING=${ver}"
    cmake --build "$dir"
}

# Actually build the default (latest) version of bemelib
build_beme_cmake "latest"

# If reproducibility mode on, build also the other versions
if [ "$REPRODUCIBILITY" = true ]; then
    versions=("24.4.0" "24.6.0" "24.10.0" "24.11.0" "24.12.0" "25.2.0" "25.4.1" "25.6.1" "25.6.2")
    for ver in "${versions[@]}"; do
        build_beme_cmake "$ver"
    done
fi

exit 0