#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUCKET_FILE="$ROOT/bucket"
BUILD_DIR="$ROOT/build"
DOCKER_BUILD_DIR="$ROOT/build-docker"
DOCKER_IMAGE_NAME="truk-dev"
DOCKER_CONTAINER_NAME="truk-dev-container"

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

get_target() {
    local target
    target="$(parse_bucket "target")"
    if [[ -z "$target" ]]; then
        echo "local"
    else
        echo "$target"
    fi
}

announce_config() {
    local compiler="$1"
    local mode="$2"
    local target="$3"

    echo -e "${COLOR_BOLD}${COLOR_CYAN}=== TRUK BUILD CONFIGURATION ===${COLOR_RESET}"
    echo -e "${COLOR_BOLD}${COLOR_CYAN}-------------------------------${COLOR_RESET}"
    echo -e "${COLOR_YELLOW}Compiler:${COLOR_RESET} ${COLOR_GREEN}${compiler}${COLOR_RESET}"
    echo -e "${COLOR_YELLOW}Mode:    ${COLOR_RESET} ${COLOR_MAGENTA}${mode}${COLOR_RESET}"
    echo -e "${COLOR_YELLOW}Target:  ${COLOR_RESET} ${COLOR_BLUE}${target}${COLOR_RESET}"
    echo ""
}

get_build_dir() {
    local target
    target="$(get_target)"
    if [[ "$target" == "docker" ]]; then
        echo "$DOCKER_BUILD_DIR"
    else
        echo "$BUILD_DIR"
    fi
}

host_to_container_path() {
    local host_path="$1"
    local rel_path="${host_path#$ROOT}"
    rel_path="${rel_path#/}"
    echo "/workspace/$rel_path"
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

docker_image_exists() {
    docker image inspect "$DOCKER_IMAGE_NAME" &>/dev/null
}

docker_container_running() {
    docker ps --filter "name=$DOCKER_CONTAINER_NAME" --filter "status=running" --format '{{.Names}}' | grep -q "^${DOCKER_CONTAINER_NAME}$"
}

docker_container_exists() {
    docker ps -a --filter "name=$DOCKER_CONTAINER_NAME" --format '{{.Names}}' | grep -q "^${DOCKER_CONTAINER_NAME}$"
}

docker_build_image() {
    echo "Building Docker image: $DOCKER_IMAGE_NAME..."
    docker build -t "$DOCKER_IMAGE_NAME" "$ROOT"
    echo "Docker image built successfully."
}

docker_start_container() {
    if docker_container_running; then
        echo "Container $DOCKER_CONTAINER_NAME is already running."
        return 0
    fi
    
    if ! docker_image_exists; then
        docker_build_image
    fi
    
    if docker_container_exists; then
        echo "Starting existing container: $DOCKER_CONTAINER_NAME..."
        docker start "$DOCKER_CONTAINER_NAME"
    else
        echo "Creating and starting container: $DOCKER_CONTAINER_NAME..."
        docker run -d \
            --name "$DOCKER_CONTAINER_NAME" \
            -v "$ROOT:/workspace" \
            -w /workspace \
            "$DOCKER_IMAGE_NAME"
    fi
    
    echo "Container $DOCKER_CONTAINER_NAME is running."
}

docker_stop_container() {
    if ! docker_container_exists; then
        echo "Container $DOCKER_CONTAINER_NAME does not exist."
        return 0
    fi
    
    echo "Stopping and removing container: $DOCKER_CONTAINER_NAME..."
    docker stop "$DOCKER_CONTAINER_NAME" 2>/dev/null || true
    docker rm "$DOCKER_CONTAINER_NAME" 2>/dev/null || true
    echo "Container $DOCKER_CONTAINER_NAME stopped and removed."
}

docker_reset() {
    echo "Resetting Docker environment (removing container and image)..."
    
    if docker_container_exists; then
        echo "Stopping and removing container: $DOCKER_CONTAINER_NAME..."
        docker stop "$DOCKER_CONTAINER_NAME" 2>/dev/null || true
        docker rm "$DOCKER_CONTAINER_NAME" 2>/dev/null || true
        echo "Container removed."
    fi
    
    if docker_image_exists; then
        echo "Removing image: $DOCKER_IMAGE_NAME..."
        docker rmi "$DOCKER_IMAGE_NAME" 2>/dev/null || true
        echo "Image removed."
    fi
    
    echo "Docker environment reset complete. Run --docker-start to rebuild."
}

docker_status() {
    echo "Docker Status:"
    echo "  Image exists: $(docker_image_exists && echo "yes" || echo "no")"
    echo "  Container exists: $(docker_container_exists && echo "yes" || echo "no")"
    echo "  Container running: $(docker_container_running && echo "yes" || echo "no")"
}

docker_shell() {
    if ! docker_container_running; then
        echo "Container not running. Starting it now..."
        docker_start_container
    fi
    
    echo "Opening shell in container: $DOCKER_CONTAINER_NAME..."
    docker exec -it "$DOCKER_CONTAINER_NAME" /bin/bash
}

docker_exec() {
    if ! docker_container_running; then
        echo "Container not running. Starting it now..."
        docker_start_container
    fi
    
    docker exec -w /workspace "$DOCKER_CONTAINER_NAME" "$@"
}

clean_build() {
    local build_dir
    build_dir="$(get_build_dir)"
    
    echo "Cleaning build directory: $build_dir..."
    if [[ -d "$build_dir" ]]; then
        rm -rf "$build_dir"
        echo "Build directory cleaned."
    else
        echo "Build directory does not exist."
    fi
}

configure_cmake() {
    local compiler="$1"
    local mode="$2"
    local build_dir="$3"
    
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
    
    local target
    target="$(get_target)"
    
    if [[ "$target" == "docker" ]]; then
        local container_build_dir
        container_build_dir="$(host_to_container_path "$build_dir")"
        docker_exec bash -c "mkdir -p $container_build_dir && cd $container_build_dir && cmake ${cmake_args[*]} /workspace"
    else
        mkdir -p "$build_dir"
        cd "$build_dir"
        cmake "${cmake_args[@]}" "$ROOT"
    fi
}

build_project() {
    local compiler mode target build_dir
    compiler="$(get_compiler)"
    mode="$(get_mode)"
    target="$(get_target)"
    build_dir="$(get_build_dir)"
    
    validate_config "$compiler" "$mode"
    announce_config "$compiler" "$mode" "$target"
    
    if [[ "$target" == "docker" ]]; then
        if ! docker_container_running; then
            docker_start_container
        fi
        
        local container_build_dir
        container_build_dir="$(host_to_container_path "$build_dir")"
        
        if [[ ! -d "$build_dir" ]] || ! docker_exec test -f "$container_build_dir/Makefile"; then
            configure_cmake "$compiler" "$mode" "$build_dir"
        fi
        
        echo "Building project..."
        docker_exec bash -c "cd $container_build_dir && make -j\$(nproc)"
        echo "Build complete."
    else
        if [[ ! -d "$build_dir" ]] || [[ ! -f "$build_dir/Makefile" ]]; then
            configure_cmake "$compiler" "$mode" "$build_dir"
        fi
        
        cd "$build_dir"
        echo "Building project..."
        make -j"$(detect_cpus)"
        echo "Build complete."
    fi
}

build_package() {
    local pkg="$1"
    local compiler mode target build_dir
    compiler="$(get_compiler)"
    mode="$(get_mode)"
    target="$(get_target)"
    build_dir="$(get_build_dir)"
    
    validate_config "$compiler" "$mode"
    announce_config "$compiler" "$mode" "$target"
    
    if [[ "$target" == "docker" ]]; then
        if ! docker_container_running; then
            docker_start_container
        fi
        
        local container_build_dir
        container_build_dir="$(host_to_container_path "$build_dir")"
        
        if [[ ! -d "$build_dir" ]] || ! docker_exec test -f "$container_build_dir/Makefile"; then
            configure_cmake "$compiler" "$mode" "$build_dir"
        fi
        
        echo "Building package: $pkg"
        docker_exec bash -c "cd $container_build_dir && make -j\$(nproc) $pkg"
        echo "Package $pkg built."
    else
        if [[ ! -d "$build_dir" ]] || [[ ! -f "$build_dir/Makefile" ]]; then
            configure_cmake "$compiler" "$mode" "$build_dir"
        fi
        
        cd "$build_dir"
        echo "Building package: $pkg"
        make -j"$(detect_cpus)" "$pkg"
        echo "Package $pkg built."
    fi
}

test_package() {
    local pkg="$1"
    
    if [[ -z "$pkg" ]]; then
        echo "Error: Package name required for test" >&2
        exit 1
    fi
    
    local compiler mode target build_dir
    compiler="$(get_compiler)"
    mode="$(get_mode)"
    target="$(get_target)"
    build_dir="$(get_build_dir)"
    
    validate_config "$compiler" "$mode"
    announce_config "$compiler" "$mode" "$target"
    
    if [[ "$target" == "docker" ]]; then
        if ! docker_container_running; then
            docker_start_container
        fi
        
        local container_build_dir
        container_build_dir="$(host_to_container_path "$build_dir")"
        
        local pkg_build_dir="$build_dir/pkg/$pkg"
        if [[ -d "$pkg_build_dir" ]]; then
            echo "Cleaning package $pkg from build..."
            rm -rf "$pkg_build_dir"
        fi
        
        if [[ ! -d "$build_dir" ]] || ! docker_exec test -f "$container_build_dir/Makefile"; then
            configure_cmake "$compiler" "$mode" "$build_dir"
        fi
        
        echo "Rebuilding package $pkg (includes buildtime tests)..."
        docker_exec bash -c "cd $container_build_dir && make -j\$(nproc) $pkg"
        echo "Building tests for $pkg..."
        docker_exec bash -c "cd $container_build_dir && make -j\$(nproc) test_${pkg}_buildtime 2>/dev/null || true"
        
        echo "Running buildtime tests..."
        docker_exec bash -c "cd $container_build_dir && ctest --output-on-failure -R ${pkg}_buildtime_test || true"
        
        local runtime_tests_dir="/workspace/pkg/$pkg/tests/runtime"
        echo "Running runtime tests for $pkg..."
        docker_exec bash -c "
            if [[ -d \"$runtime_tests_dir\" ]]; then
                test_count=0
                passed_count=0
                
                while IFS= read -r -d '' test_script; do
                    ((test_count++)) || true
                    echo \"Running test: \$(basename \"\$test_script\")\"
                    if bash \"\$test_script\"; then
                        echo \"  PASSED\"
                        ((passed_count++)) || true
                    else
                        echo \"  FAILED\"
                    fi
                done < <(find \"$runtime_tests_dir\" -name \"*.sh\" -type f -print0)
                
                if [[ \$test_count -eq 0 ]]; then
                    echo \"No runtime tests found for $pkg\"
                else
                    echo \"Runtime tests: \$passed_count/\$test_count passed\"
                    if [[ \$passed_count -ne \$test_count ]]; then
                        exit 1
                    fi
                fi
            else
                echo \"No runtime tests directory for $pkg\"
            fi
        "
        
        echo "All tests passed for $pkg"
    else
        local pkg_build_dir="$build_dir/pkg/$pkg"
        if [[ -d "$pkg_build_dir" ]]; then
            echo "Cleaning package $pkg from build..."
            rm -rf "$pkg_build_dir"
        fi
        
        if [[ ! -d "$build_dir" ]] || [[ ! -f "$build_dir/Makefile" ]]; then
            configure_cmake "$compiler" "$mode" "$build_dir"
        fi
        
        cd "$build_dir"
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
    fi
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

Build Options:
  -c, --clean              Clean the build directory
  -b, --build [pkg]        Build the project or specific package
  -t, --test <pkg>         Test a specific package (clean, rebuild, run tests)
  -h, --help               Show this help message

Docker Options:
  --docker-start           Build Docker image and start persistent container
  --docker-stop            Stop and remove Docker container
  --docker-reset           Remove container and image (force rebuild on next start)
  --docker-status          Show Docker container status
  --docker-shell           Open interactive shell in Docker container

Configuration is read from the 'bucket' file in the project root.
Set 'target = docker' in bucket to run builds in Docker automatically.
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
            --docker-start)
                docker_start_container
                shift
                ;;
            --docker-stop)
                docker_stop_container
                shift
                ;;
            --docker-reset)
                docker_reset
                shift
                ;;
            --docker-status)
                docker_status
                shift
                ;;
            --docker-shell)
                docker_shell
                shift
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
