#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUCKET_FILE="$ROOT/bucket"
BUILD_DIR="$ROOT/build"

COLOR_RESET='\033[0m'
COLOR_BOLD='\033[1m'
COLOR_GREEN='\033[0;32m'
COLOR_BLUE='\033[0;34m'
COLOR_CYAN='\033[0;36m'
COLOR_YELLOW='\033[0;33m'
COLOR_MAGENTA='\033[0;35m'

parse_bucket() {
    local key="$1"
    if [[ ! -f "$BUCKET_FILE" ]]; then
        echo "Error: bucket file not found at $BUCKET_FILE" >&2
        exit 1
    fi
    
    while IFS= read -r line; do
        line="${line%%;*}"
        
        line="${line#"${line%%[![:space:]]*}"}"
        line="${line%"${line##*[![:space:]]}"}"
        
        if [[ -z "$line" ]]; then
            continue
        fi
        
        if [[ "$line" == *"="* ]]; then
            local k="${line%%=*}"
            local v="${line#*=}"
            
            k="${k#"${k%%[![:space:]]*}"}"
            k="${k%"${k##*[![:space:]]}"}"
            v="${v#"${v%%[![:space:]]*}"}"
            v="${v%"${v##*[![:space:]]}"}"
            
            if [[ "$k" == "$key" ]]; then
                echo "$v"
                return 0
            fi
        fi
    done < "$BUCKET_FILE"
    
    echo ""
}

get_compiler() {
    parse_bucket "compiler"
}

get_mode() {
    parse_bucket "mode"
}

announce_config() {
    local compiler="$1"
    local mode="$2"

    echo -e "${COLOR_BOLD}${COLOR_CYAN}=== TRUK BUILD CONFIGURATION ===${COLOR_RESET}"
    echo -e "${COLOR_BOLD}${COLOR_CYAN}-------------------------------${COLOR_RESET}"
    echo -e "${COLOR_YELLOW}Compiler:${COLOR_RESET} ${COLOR_GREEN}${compiler}${COLOR_RESET}"
    echo -e "${COLOR_YELLOW}Mode:    ${COLOR_RESET} ${COLOR_MAGENTA}${mode}${COLOR_RESET}"
    echo ""
}


validate_config() {
    local compiler="$1"
    local mode="$2"
    
    if [[ "$compiler" != "clang" ]]; then
        echo "Error: Invalid compiler '$compiler'. Must be 'clang'" >&2
        exit 1
    fi
    
    if [[ "$mode" != "dev" && "$mode" != "rel" && "$mode" != "asan" ]]; then
        echo "Error: Invalid mode '$mode'. Must be 'dev', 'rel', or 'asan'" >&2
        exit 1
    fi
}

clean_build() {
    echo "Cleaning build directory..."
    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
        echo "Build directory cleaned."
    else
        echo "Build directory does not exist."
    fi
}

configure_cmake() {
    local compiler="$1"
    local mode="$2"
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    local cmake_build_type=""
    case "$mode" in
        dev)
            cmake_build_type="Debug"
            ;;
        rel)
            cmake_build_type="Release"
            ;;
        asan)
            cmake_build_type="Asan"
            ;;
    esac
    
    local cmake_args=(-DCMAKE_BUILD_TYPE="$cmake_build_type" -DCMAKE_C_COMPILER="clang")
    
    echo "Configuring with CMake: ${cmake_args[*]}"
    cmake "${cmake_args[@]}" "$ROOT"
}

build_project() {
    local compiler mode
    compiler="$(get_compiler)"
    mode="$(get_mode)"
    
    validate_config "$compiler" "$mode"
    announce_config "$compiler" "$mode"
    
    if [[ ! -d "$BUILD_DIR" ]] || [[ ! -f "$BUILD_DIR/Makefile" ]]; then
        configure_cmake "$compiler" "$mode"
    fi
    
    cd "$BUILD_DIR"
    echo "Building project..."
    make -j"$(detect_cpus)"
    echo "Build complete."
}

build_package() {
    local pkg="$1"
    local compiler mode
    compiler="$(get_compiler)"
    mode="$(get_mode)"
    
    validate_config "$compiler" "$mode"
    announce_config "$compiler" "$mode"
    
    if [[ ! -d "$BUILD_DIR" ]] || [[ ! -f "$BUILD_DIR/Makefile" ]]; then
        configure_cmake "$compiler" "$mode"
    fi
    
    cd "$BUILD_DIR"
    echo "Building package: $pkg"
    make -j"$(detect_cpus)" "$pkg"
    echo "Package $pkg built."
}

test_package() {
    local pkg="$1"
    
    if [[ -z "$pkg" ]]; then
        echo "Error: Package name required for test" >&2
        exit 1
    fi
    
    local compiler mode
    compiler="$(get_compiler)"
    mode="$(get_mode)"
    
    validate_config "$compiler" "$mode"
    announce_config "$compiler" "$mode"
    
    local pkg_build_dir="$BUILD_DIR/pkg/$pkg"
    if [[ -d "$pkg_build_dir" ]]; then
        echo "Cleaning package $pkg from build..."
        rm -rf "$pkg_build_dir"
    fi
    
    if [[ ! -d "$BUILD_DIR" ]] || [[ ! -f "$BUILD_DIR/Makefile" ]]; then
        configure_cmake "$compiler" "$mode"
    fi
    
    cd "$BUILD_DIR"
    echo "Rebuilding package $pkg (includes buildtime tests)..."
    make -j"$(detect_cpus)" "$pkg"
    echo "Building tests for $pkg..."
    make -j"$(detect_cpus)" "test_${pkg}_buildtime" 2>/dev/null || true
    
    echo "Running buildtime tests..."
    ctest --output-on-failure -R "${pkg}_buildtime_test" || true
    
    local runtime_tests_dir="$ROOT/pkg/$pkg/tests/runtime"
    if [[ -d "$runtime_tests_dir" ]]; then
        echo "Running runtime tests for $pkg..."
        local test_count=0
        local passed_count=0
        
        while IFS= read -r -d '' test_script; do
            ((test_count++)) || true
            echo "Running test: $(basename "$test_script")"
            if bash "$test_script"; then
                echo "  PASSED"
                ((passed_count++)) || true
            else
                echo "  FAILED"
            fi
        done < <(find "$runtime_tests_dir" -name "*.sh" -type f -print0)
        
        if [[ $test_count -eq 0 ]]; then
            echo "No runtime tests found for $pkg"
        else
            echo "Runtime tests: $passed_count/$test_count passed"
            if [[ $passed_count -ne $test_count ]]; then
                exit 1
            fi
        fi
    else
        echo "No runtime tests directory for $pkg"
    fi
    
    echo "All tests passed for $pkg"
}

detect_cpus() {
    case "$(uname -s)" in
        Linux) nproc ;;
        Darwin) sysctl -n hw.ncpu ;;
        *) echo 1 ;;
    esac
}

show_usage() {
    cat <<EOF
Usage: $0 [OPTIONS]

Options:
  -c, --clean              Clean the build directory
  -b, --build [pkg]        Build the project or specific package
  -t, --test <pkg>         Test a specific package (clean, rebuild, run tests)
  -h, --help               Show this help message

Configuration is read from the 'bucket' file in the project root.
EOF
}

main() {
    if [[ $# -eq 0 ]]; then
        show_usage
        exit 1
    fi
    
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -c|--clean)
                clean_build
                shift
                ;;
            -b|--build)
                if [[ $# -gt 1 && ! "$2" =~ ^- ]]; then
                    build_package "$2"
                    shift 2
                else
                    build_project
                    shift
                fi
                ;;
            -t|--test)
                if [[ $# -lt 2 ]]; then
                    echo "Error: --test requires a package name" >&2
                    show_usage
                    exit 1
                fi
                test_package "$2"
                shift 2
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            *)
                echo "Error: Unknown option '$1'" >&2
                show_usage
                exit 1
                ;;
        esac
    done
}

main "$@"
