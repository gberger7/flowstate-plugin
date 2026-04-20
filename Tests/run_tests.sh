#!/bin/bash

# run_tests.sh - Build and run DelayEngine property tests
# Usage: ./run_tests.sh [cmake|make|help]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

show_help() {
    echo "DelayEngine Property Tests - Build and Run Script"
    echo ""
    echo "Usage: ./run_tests.sh [option]"
    echo ""
    echo "Options:"
    echo "  cmake    - Build using CMake (default)"
    echo "  make     - Build using Makefile"
    echo "  clean    - Clean build artifacts"
    echo "  help     - Show this help message"
    echo ""
    echo "Prerequisites:"
    echo "  - JUCE framework installed"
    echo "  - CMake 3.15+ (for cmake option)"
    echo "  - C++17 compatible compiler"
    echo ""
    echo "Examples:"
    echo "  ./run_tests.sh           # Build with CMake and run tests"
    echo "  ./run_tests.sh make      # Build with Makefile and run tests"
    echo "  ./run_tests.sh clean     # Clean all build artifacts"
}

build_cmake() {
    echo "Building with CMake..."
    
    if ! command -v cmake &> /dev/null; then
        echo "Error: CMake not found. Please install CMake or use 'make' option."
        exit 1
    fi
    
    mkdir -p build
    cd build
    
    echo "Configuring..."
    cmake .. || {
        echo "Error: CMake configuration failed."
        echo "Make sure JUCE is installed and CMake can find it."
        exit 1
    }
    
    echo "Building..."
    cmake --build . || {
        echo "Error: Build failed."
        exit 1
    }
    
    echo ""
    echo "Running tests..."
    echo "==============================================================================="
    ./DelayEnginePropertyTests || {
        echo "==============================================================================="
        echo "Tests failed!"
        exit 1
    }
    
    echo "==============================================================================="
    echo "All tests passed!"
}

build_make() {
    echo "Building with Makefile..."
    
    if [ ! -f "Makefile" ]; then
        echo "Error: Makefile not found."
        exit 1
    fi
    
    # Check if JUCE_PATH is set in Makefile or environment
    if [ -z "$JUCE_PATH" ]; then
        echo "Warning: JUCE_PATH not set. Using default from Makefile."
        echo "If build fails, set JUCE_PATH environment variable or edit Makefile."
    fi
    
    echo "Building..."
    make clean 2>/dev/null || true
    make || {
        echo "Error: Build failed."
        echo "Make sure JUCE_PATH is set correctly in Makefile."
        exit 1
    }
    
    echo ""
    echo "Running tests..."
    echo "==============================================================================="
    ./DelayEnginePropertyTests || {
        echo "==============================================================================="
        echo "Tests failed!"
        exit 1
    }
    
    echo "==============================================================================="
    echo "All tests passed!"
}

clean_build() {
    echo "Cleaning build artifacts..."
    
    # Clean CMake build
    if [ -d "build" ]; then
        echo "Removing build/ directory..."
        rm -rf build
    fi
    
    # Clean Makefile build
    if [ -f "DelayEnginePropertyTests" ]; then
        echo "Removing test executable..."
        rm -f DelayEnginePropertyTests
    fi
    
    echo "Clean complete."
}

# Main script
case "${1:-cmake}" in
    cmake)
        build_cmake
        ;;
    make)
        build_make
        ;;
    clean)
        clean_build
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        echo "Error: Unknown option '$1'"
        echo ""
        show_help
        exit 1
        ;;
esac
