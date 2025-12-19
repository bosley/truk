#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
BOLD='\033[1m'
RESET='\033[0m'

print_header() {
    echo -e "${BOLD}${CYAN}"
    echo "╔════════════════════════════════════════════════════════════╗"
    echo "║                   TRUK PROJECT STATS                       ║"
    echo "╚════════════════════════════════════════════════════════════╝"
    echo -e "${RESET}"
}

print_usage() {
    echo -e "${YELLOW}Usage:${RESET} $0 [truk|c|shell|tests]"
    echo ""
    echo -e "${CYAN}Options:${RESET}"
    echo -e "  ${GREEN}truk${RESET}   - Count non-empty lines in .truk files"
    echo -e "  ${GREEN}c${RESET}      - Count non-empty lines in C/C++ files (.c, .cpp, .h, .hpp)"
    echo -e "  ${GREEN}shell${RESET}  - Count non-empty lines in shell scripts (.sh)"
    echo -e "  ${GREEN}tests${RESET}  - Show test suite statistics"
    echo -e "  ${GREEN}(none)${RESET} - Show all statistics"
    echo ""
}

count_non_empty_lines() {
    local extensions="$1"
    local total=0
    local file_count=0
    
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    
    local find_cmd="find \"$script_dir\" -type d -name build -prune -o -type f"
    
    IFS='|' read -ra exts <<< "$extensions"
    local first=true
    for ext in "${exts[@]}"; do
        if [ "$first" = true ]; then
            find_cmd="$find_cmd \\( -name \"*.$ext\""
            first=false
        else
            find_cmd="$find_cmd -o -name \"*.$ext\""
        fi
    done
    find_cmd="$find_cmd \\) -print"
    
    while IFS= read -r file; do
        if [ -f "$file" ]; then
            lines=$(grep -c -v '^\s*$' "$file" 2>/dev/null || echo 0)
            total=$((total + lines))
            file_count=$((file_count + 1))
        fi
    done < <(eval "$find_cmd" 2>/dev/null)
    
    echo "$total:$file_count"
}

show_truk_stats() {
    echo -e "${BOLD}${MAGENTA}🚚 TRUK Language Files${RESET}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
    
    result=$(count_non_empty_lines "truk")
    lines=$(echo $result | cut -d: -f1)
    files=$(echo $result | cut -d: -f2)
    
    printf "  ${WHITE}%-20s${RESET} ${GREEN}%'d${RESET}\n" "Files:" "$files"
    printf "  ${WHITE}%-20s${RESET} ${GREEN}%'d${RESET}\n" "Lines (non-empty):" "$lines"
    
    if [ $files -gt 0 ]; then
        avg=$((lines / files))
        printf "  ${WHITE}%-20s${RESET} ${YELLOW}%'d${RESET}\n" "Avg lines/file:" "$avg"
    fi
    echo ""
}

show_c_stats() {
    echo -e "${BOLD}${BLUE}⚙️  C/C++ Source Files${RESET}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
    
    result=$(count_non_empty_lines "c|cpp|h|hpp")
    lines=$(echo $result | cut -d: -f1)
    files=$(echo $result | cut -d: -f2)
    
    printf "  ${WHITE}%-20s${RESET} ${GREEN}%'d${RESET}\n" "Files:" "$files"
    printf "  ${WHITE}%-20s${RESET} ${GREEN}%'d${RESET}\n" "Lines (non-empty):" "$lines"
    
    if [ $files -gt 0 ]; then
        avg=$((lines / files))
        printf "  ${WHITE}%-20s${RESET} ${YELLOW}%'d${RESET}\n" "Avg lines/file:" "$avg"
    fi
    echo ""
}

show_shell_stats() {
    echo -e "${BOLD}${GREEN}📜 Shell Scripts${RESET}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
    
    result=$(count_non_empty_lines "sh")
    lines=$(echo $result | cut -d: -f1)
    files=$(echo $result | cut -d: -f2)
    
    printf "  ${WHITE}%-20s${RESET} ${GREEN}%'d${RESET}\n" "Files:" "$files"
    printf "  ${WHITE}%-20s${RESET} ${GREEN}%'d${RESET}\n" "Lines (non-empty):" "$lines"
    
    if [ $files -gt 0 ]; then
        avg=$((lines / files))
        printf "  ${WHITE}%-20s${RESET} ${YELLOW}%'d${RESET}\n" "Avg lines/file:" "$avg"
    fi
    echo ""
}

show_test_stats() {
    echo -e "${BOLD}${YELLOW}🧪 Test Suite Statistics${RESET}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
    
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    
    rca_dirs=$(find "$script_dir/tests/return_code_assertions" -mindepth 1 -maxdepth 1 -type d 2>/dev/null | wc -l | tr -d ' ')
    rca_truk=$(find "$script_dir/tests/return_code_assertions" -name '*.truk' 2>/dev/null | wc -l | tr -d ' ')
    
    echo -e "  ${BOLD}${WHITE}Return Code Assertions:${RESET}"
    printf "    ${WHITE}%-18s${RESET} ${GREEN}%'d${RESET}\n" "Categories:" "$rca_dirs"
    printf "    ${WHITE}%-18s${RESET} ${GREEN}%'d${RESET}\n" ".truk files:" "$rca_truk"
    echo ""
    
    exp_dirs=$(find "$script_dir/tests/expects" -mindepth 1 -maxdepth 1 -type d 2>/dev/null | wc -l | tr -d ' ')
    exp_truk=$(find "$script_dir/tests/expects" -name '*.truk' 2>/dev/null | wc -l | tr -d ' ')
    exp_expect=$(find "$script_dir/tests/expects" -name '*.expect' 2>/dev/null | wc -l | tr -d ' ')
    
    echo -e "  ${BOLD}${WHITE}Expected Failures:${RESET}"
    printf "    ${WHITE}%-18s${RESET} ${GREEN}%'d${RESET}\n" "Categories:" "$exp_dirs"
    printf "    ${WHITE}%-18s${RESET} ${GREEN}%'d${RESET}\n" ".truk files:" "$exp_truk"
    printf "    ${WHITE}%-18s${RESET} ${GREEN}%'d${RESET}\n" ".expect files:" "$exp_expect"
    echo ""
    
    meta_dirs=$(find "$script_dir/tests/meta_test_testing_fw" -mindepth 1 -maxdepth 1 -type d 2>/dev/null | wc -l | tr -d ' ')
    meta_truk=$(find "$script_dir/tests/meta_test_testing_fw" -name '*.truk' 2>/dev/null | wc -l | tr -d ' ')
    meta_expect=$(find "$script_dir/tests/meta_test_testing_fw" -name '*.expect' 2>/dev/null | wc -l | tr -d ' ')
    
    echo -e "  ${BOLD}${WHITE}Meta Test Framework:${RESET}"
    printf "    ${WHITE}%-18s${RESET} ${GREEN}%'d${RESET}\n" "Categories:" "$meta_dirs"
    printf "    ${WHITE}%-18s${RESET} ${GREEN}%'d${RESET}\n" ".truk files:" "$meta_truk"
    printf "    ${WHITE}%-18s${RESET} ${GREEN}%'d${RESET}\n" ".expect files:" "$meta_expect"
    echo ""
    
    total_tests=$((rca_truk + exp_truk + meta_truk))
    printf "  ${BOLD}${MAGENTA}%-20s${RESET} ${BOLD}${GREEN}%'d${RESET}\n" "Total Test Files:" "$total_tests"
    echo ""
}

print_header

if [ $# -eq 0 ]; then
    show_truk_stats
    show_c_stats
    show_shell_stats
    show_test_stats
else
    case "$1" in
        truk)
            show_truk_stats
            ;;
        c)
            show_c_stats
            ;;
        shell)
            show_shell_stats
            ;;
        tests)
            show_test_stats
            ;;
        *)
            echo -e "${RED}Error: Invalid argument '$1'${RESET}"
            echo ""
            print_usage
            exit 1
            ;;
    esac
fi

echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
