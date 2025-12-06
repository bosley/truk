#!/usr/bin/env bash

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

NPROC=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

build_config() {
    local config_name="$1"
    local build_dir="$2"
    local cmake_build_type="$3"
    local extra_flags="$4"
    
    echo ""
    echo "=========================================="
    echo "Building ${config_name} configuration"
    echo "=========================================="
    
    echo "==> Configuring CMake for ${config_name}..."
    if ! cmake -S "${PROJECT_ROOT}" -B "${build_dir}" \
        -DCMAKE_BUILD_TYPE="${cmake_build_type}" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        ${extra_flags}; then
        echo "❌ ${config_name} configuration failed"
        exit 1
    fi
    
    echo "==> Building ${config_name} with ${NPROC} parallel jobs..."
    if ! cmake --build "${build_dir}" --parallel ${NPROC}; then
        echo "❌ ${config_name} build failed"
        exit 1
    fi
    
    echo "==> Running ${config_name} tests..."
    if ! (cd "${build_dir}" && ctest --output-on-failure --parallel ${NPROC}); then
        echo "❌ ${config_name} tests failed"
        exit 1
    fi
    
    echo "✅ ${config_name} build and tests completed successfully"
}

build_asan() {
    build_config "ASAN" "${PROJECT_ROOT}/build-asan" "Debug" "-DENABLE_ASAN=ON"
}

build_dev() {
    build_config "DEV" "${PROJECT_ROOT}/build-dev" "Debug" ""
}

build_rel() {
    build_config "RELEASE" "${PROJECT_ROOT}/build-rel" "Release" ""
}

build_all() {
    echo "Building all configurations..."
    build_asan
    build_dev
    build_rel
    echo ""
    echo "=========================================="
    echo "✅ All configurations built successfully"
    echo "=========================================="
}

clean() {
    echo "==> Cleaning all build directories..."
    rm -rf "${PROJECT_ROOT}/build-asan"
    rm -rf "${PROJECT_ROOT}/build-dev"
    rm -rf "${PROJECT_ROOT}/build-rel"
    rm -rf "${PROJECT_ROOT}/build"
    echo "✅ Clean completed"
}

run_test_scripts() {
    local test_dir="${PROJECT_ROOT}/tests"
    local found_tests=0
    
    if [ -d "${test_dir}" ]; then
        echo ""
        echo "==> Running shell test scripts..."
        
        for script in "${test_dir}"/*.sh; do
            if [ -f "${script}" ] && [ -x "${script}" ]; then
                found_tests=1
                local script_name=$(basename "${script}")
                echo "  Running ${script_name}..."
                
                if ! "${script}"; then
                    echo "❌ Test script ${script_name} failed"
                    exit 1
                fi
                
                echo "  ✅ ${script_name} passed"
            fi
        done
        
        if [ ${found_tests} -eq 0 ]; then
            echo "  No executable test scripts found in ${test_dir}"
        fi
    fi
}

test_suite() {
    echo "Running full test suite with ASAN build..."
    
    build_asan
    
    run_test_scripts
    
    echo ""
    echo "=========================================="
    echo "✅ All tests passed"
    echo "=========================================="
}

usage() {
    cat << EOF
Usage: $0 {build|clean|test} [config]

Commands:
  build [config]  - Build project
                    asan  - Build with AddressSanitizer (build-asan/)
                    dev   - Build debug without optimizations (build-dev/)
                    rel   - Build release with optimizations (build-rel/)
                    (none) - Build all configurations
  
  test            - Build ASAN config and run all tests (CTest + .sh scripts)
  
  clean           - Remove all build directories

Examples:
  ./bar.sh build asan    # Build only ASAN configuration
  ./bar.sh build         # Build all configurations
  ./bar.sh test          # Run full test suite with ASAN
  ./bar.sh clean         # Clean all builds
EOF
}

case "${1:-build}" in
    build)
        case "${2}" in
            asan)
                build_asan
                ;;
            dev)
                build_dev
                ;;
            rel)
                build_rel
                ;;
            "")
                build_all
                ;;
            *)
                echo "❌ Unknown build configuration: ${2}"
                usage
                exit 1
                ;;
        esac
        ;;
    clean)
        clean
        ;;
    test)
        test_suite
        ;;
    help|--help|-h)
        usage
        ;;
    *)
        echo "❌ Unknown command: ${1}"
        usage
        exit 1
        ;;
esac
