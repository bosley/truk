#!/usr/bin/env bash

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

NPROC=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

build() {
    echo "==> Configuring CMake..."
    cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    
    echo "==> Building with ${NPROC} parallel jobs..."
    if ! cmake --build "${BUILD_DIR}" --parallel ${NPROC}; then
        echo "❌ Build failed"
        exit 1
    fi
    
    echo "==> Running tests..."
    if ! (cd "${BUILD_DIR}" && ctest --output-on-failure --parallel ${NPROC}); then
        echo "❌ Tests failed"
        exit 1
    fi
    
    echo "✅ Build and tests completed successfully"
}

clean() {
    echo "==> Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
    echo "✅ Clean completed"
}

test_only() {
    if [ ! -d "${BUILD_DIR}" ]; then
        echo "❌ Build directory not found. Run './bar.sh build' first."
        exit 1
    fi
    
    echo "==> Running tests..."
    if ! (cd "${BUILD_DIR}" && ctest --output-on-failure --parallel ${NPROC}); then
        echo "❌ Tests failed"
        exit 1
    fi
    
    echo "✅ Tests completed successfully"
}

case "${1:-build}" in
    build)
        build
        ;;
    clean)
        clean
        ;;
    test)
        test_only
        ;;
    *)
        echo "Usage: $0 {build|clean|test}"
        echo "  build - Configure, build, and test (default)"
        echo "  clean - Remove build directory"
        echo "  test  - Run tests only"
        exit 1
        ;;
esac
