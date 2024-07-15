# Common variables
$PROJECT = "dray-nuklear"
$LIB_NAME = "raylib_nuklear"
$SOURCETREE_URL = "https://github.com/redthing1/raylib-nuklear.git"
$SOURCETREE_DIR = "raylib_nuklear_source"
$SOURCETREE_BRANCH = "master"
$LIB_FILE_NAME = "rlnuklear.lib"
$LIB_FILE_BUILD_NAME = "build\Release\$LIB_FILE_NAME"
$PACKAGE_DIR = $PSScriptRoot

# Function to check if a command is available
function Test-Command($cmdname) {
    return [bool](Get-Command -Name $cmdname -ErrorAction SilentlyContinue)
}

# Function to ensure a command is available
function Ensure-Command($cmdname) {
    if (-not (Test-Command $cmdname)) {
        Write-Error "Error: $cmdname is not installed or not in PATH"
        Write-Error "Please install $cmdname and try again"
        exit 1
    }
    Write-Host "$cmdname is available"
}

# Ensure all required commands are available
Ensure-Command "git"
Ensure-Command "cmake"

# Function to prepare the source
function Prepare-Source {
    Write-Host "[$PROJECT] preparing $LIB_NAME source..."
    Set-Location $PACKAGE_DIR
    
    if (Test-Path $SOURCETREE_DIR) {
        Write-Host "[$PROJECT] source folder already exists, using it."
    } else {
        Write-Host "[$PROJECT] getting source to build $LIB_NAME"
        git clone --depth 1 --branch $SOURCETREE_BRANCH $SOURCETREE_URL $SOURCETREE_DIR
    }

    Set-Location $SOURCETREE_DIR
    git submodule update --init --recursive
    Write-Host "[$PROJECT] finished preparing $LIB_NAME source"
}

# Function to build the library
function Build-Library {
    Write-Host "[$PROJECT] starting build of $LIB_NAME"
    Set-Location "$PACKAGE_DIR\$SOURCETREE_DIR"

    # list RAYLIB_DIR
    Write-Host "RAYLIB_DIR: $env:RAYLIB_DIR"
    Get-ChildItem $env:RAYLIB_DIR

    # set up build args
    $cmake_args = @()
    if (-not $env:RAYLIB_DIR) {
        Write-Error "RAYLIB_DIR is not set.."
        exit 1
    } else {
        $cmake_args += "-DRAYLIB_DIR=$env:RAYLIB_DIR"
    }
    if ($env:RELEASE -eq "1") {
        $cmake_args += "-DCMAKE_BUILD_TYPE=Release"
    } else {
        $cmake_args += "-DCMAKE_BUILD_TYPE=Debug"
    }

    $cmake_args_string = $cmake_args -join " "
    
    # START BUILD
    cmake -B build -S . $cmake_args_string
    cmake --build build --config Release
    # END BUILD

    Write-Host "[$PROJECT] finished build of $LIB_NAME"
    Write-Host "[$PROJECT] copying $LIB_NAME binary ($LIB_FILE_NAME) to $PACKAGE_DIR"
    Copy-Item -Path $LIB_FILE_BUILD_NAME -Destination $PACKAGE_DIR -Force
    # ensure the library is available
    if (-not (Test-Path "$PACKAGE_DIR\$LIB_FILE_NAME")) {
        Write-Error "Error: $LIB_NAME library not found at $PACKAGE_DIR\$LIB_FILE_NAME"
        exit 1
    }
}

# Main execution
function Main {
    param (
        [Parameter(Mandatory=$true)]
        [string]$Action,
        
        [Parameter(ValueFromRemainingArguments=$true)]
        $RemainingArgs
    )

    # export all other KEY=VALUE pairs as environment variables
    foreach ($arg in $RemainingArgs) {
        if ($arg -match '^(\w+)=(.*)$') {
            $key = $matches[1]
            $value = $matches[2]
            Set-Item -Path "env:$key" -Value $value
        }
    }

    switch ($Action) {
        "prepare" { Prepare-Source }
        "build" { 
            Prepare-Source  # Always call prepare before build
            Build-Library 
        }
        default {
            Write-Host "Usage: .\script.ps1 [prepare|build] [build arguments...]"
            exit 1
        }
    }
}

# Run the script
Main @args