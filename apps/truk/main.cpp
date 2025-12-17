#include "commands/compile.hpp"
#include "commands/run.hpp"
#include "commands/tcc.hpp"
#include "commands/test.hpp"
#include "commands/toc.hpp"
#include "common/args.hpp"

int main(int argc, char **argv) {
  auto args = truk::common::parse_args(argc, argv);

  if (args.command == "toc") {
    return truk::commands::toc(
        {args.input_file, args.output_file, args.include_paths});
  } else if (args.command == "tcc") {
    return truk::commands::tcc({args.input_file, args.output_file,
                                args.include_paths, args.library_paths,
                                args.libraries, args.rpaths});
  } else if (args.command == "run") {
    return truk::commands::run({args.input_file, args.include_paths,
                                args.library_paths, args.libraries, args.rpaths,
                                args.program_args});
  } else if (args.command == "test") {
    return truk::commands::test({args.input_file, args.include_paths,
                                 args.library_paths, args.libraries,
                                 args.rpaths, args.program_args});
  } else {
    return truk::commands::compile({args.input_file, args.output_file,
                                    args.include_paths, args.library_paths,
                                    args.libraries, args.rpaths});
  }
}
