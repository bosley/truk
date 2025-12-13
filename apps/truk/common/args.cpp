#include "args.hpp"
#include <cstring>
#include <fmt/core.h>

namespace truk::common {

void print_usage(const char *program_name) {
  fmt::print(stderr, "Usage:\n");
  fmt::print(stderr, "  {} <file.truk> [-o output] [-I path]... [-L path]... "
                     "[-l lib]... [-rpath path]...\n",
             program_name);
  fmt::print(stderr, "    Compile Truk source to executable (default)\n\n");
  fmt::print(stderr, "  {} toc <file.truk> -o output.c [-I path]...\n",
             program_name);
  fmt::print(stderr, "    Compile Truk source to C\n\n");
  fmt::print(stderr, "  {} tcc <file.c> -o output [-I path]... [-L path]... "
                     "[-l lib]... [-rpath path]...\n",
             program_name);
  fmt::print(stderr, "    Compile C source to executable using TCC\n\n");
  fmt::print(stderr, "Options:\n");
  fmt::print(stderr, "  -o <file>   Output file path\n");
  fmt::print(stderr, "  -I <path>   Include directory (multiple allowed)\n");
  fmt::print(stderr, "  -L <path>   Library search path (multiple allowed)\n");
  fmt::print(stderr, "  -l <name>   Link library (multiple allowed)\n");
  fmt::print(stderr, "  -rpath <p>  Runtime library search path (multiple "
                     "allowed)\n");
}

parsed_args_s parse_args(int argc, char **argv) {
  parsed_args_s args;

  if (argc < 2) {
    print_usage(argv[0]);
    std::exit(1);
  }

  int idx = 1;

  if (std::strcmp(argv[1], "toc") == 0 || std::strcmp(argv[1], "tcc") == 0) {
    args.command = argv[1];
    idx = 2;
  }

  if (idx >= argc) {
    print_usage(argv[0]);
    std::exit(1);
  }

  args.input_file = argv[idx++];

  while (idx < argc) {
    if (std::strcmp(argv[idx], "-o") == 0 && idx + 1 < argc) {
      args.output_file = argv[idx + 1];
      idx += 2;
    } else if (std::strcmp(argv[idx], "-I") == 0 && idx + 1 < argc) {
      args.include_paths.push_back(argv[idx + 1]);
      idx += 2;
    } else if (std::strcmp(argv[idx], "-L") == 0 && idx + 1 < argc) {
      args.library_paths.push_back(argv[idx + 1]);
      idx += 2;
    } else if (std::strcmp(argv[idx], "-l") == 0 && idx + 1 < argc) {
      args.libraries.push_back(argv[idx + 1]);
      idx += 2;
    } else if (std::strcmp(argv[idx], "-rpath") == 0 && idx + 1 < argc) {
      args.rpaths.push_back(argv[idx + 1]);
      idx += 2;
    } else {
      fmt::print(stderr, "Unknown option: {}\n", argv[idx]);
      print_usage(argv[0]);
      std::exit(1);
    }
  }

  if (args.output_file.empty()) {
    if (args.command == "toc") {
      args.output_file = "output.c";
    } else {
      args.output_file = "a.out";
    }
  }

  return args;
}

} // namespace truk::common
