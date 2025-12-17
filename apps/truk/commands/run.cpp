#include "run.hpp"
#include <fmt/core.h>
#include <truk/core/error_reporter.hpp>
#include <truk/emitc/emitter.hpp>
#include <truk/ingestion/file_utils.hpp>
#include <truk/ingestion/parser.hpp>
#include <truk/tcc/tcc.hpp>
#include <truk/validation/typecheck.hpp>

namespace truk::commands {

int run(const run_options_s &opts) {
  core::error_reporter_c reporter;

  for (const auto &path : opts.include_paths) {
    fmt::print("Include path: {}\n", path);
  }
  for (const auto &path : opts.library_paths) {
    fmt::print("Library path: {}\n", path);
  }
  for (const auto &lib : opts.libraries) {
    fmt::print("Library: {}\n", lib);
  }
  for (const auto &path : opts.rpaths) {
    fmt::print("Rpath: {}\n", path);
  }

  std::string source = ingestion::read_file(opts.input_file);

  ingestion::parser_c parser(source.c_str(), source.size());
  auto parse_result = parser.parse();

  if (!parse_result.success) {
    if (!parse_result.error_message.empty() && parse_result.source_data) {
      std::string source_str(parse_result.source_data, parse_result.source_len);
      reporter.report_parse_error(
          opts.input_file, source_str, parse_result.error_line,
          parse_result.error_column, parse_result.error_message);
    } else {
      reporter.report_generic_error(core::error_phase_e::PARSING,
                                    "Parse failed");
    }
    reporter.print_summary();
    return 1;
  }

  std::unordered_map<const truk::language::nodes::base_c *, std::string>
      decl_to_file;
  std::unordered_map<std::string, std::vector<std::string>> file_to_shards;
  for (const auto &decl : parse_result.declarations) {
    decl_to_file[decl.get()] = opts.input_file;
    if (auto *shard_node = decl.get()->as_shard()) {
      file_to_shards[opts.input_file].push_back(shard_node->name());
    }
  }

  validation::type_checker_c type_checker;
  type_checker.set_declaration_file_map(decl_to_file);
  type_checker.set_file_to_shards_map(file_to_shards);
  for (auto &decl : parse_result.declarations) {
    type_checker.check(decl.get());
  }

  if (type_checker.has_errors()) {
    for (const auto &err : type_checker.errors()) {
      std::string error_file =
          err.file_path.empty() ? opts.input_file : err.file_path;
      std::string error_source = source;
      if (!err.file_path.empty() && err.file_path != opts.input_file) {
        try {
          error_source = ingestion::read_file(err.file_path);
        } catch (...) {
        }
      }
      reporter.report_typecheck_error(error_file, error_source,
                                      err.source_index, err.message);
    }
    reporter.print_summary();
    return 1;
  }

  emitc::emitter_c emitter;
  auto emit_result = emitter.add_declarations(parse_result.declarations)
                         .set_declaration_file_map(decl_to_file)
                         .set_file_to_shards_map(file_to_shards)
                         .set_c_imports(parse_result.c_imports)
                         .finalize();

  if (emit_result.has_errors()) {
    for (const auto &err : emit_result.errors) {
      std::string phase_context =
          fmt::format("phase: {}, context: {}",
                      emitc::emission_phase_name(err.phase), err.node_context);
      reporter.report_emission_error(opts.input_file, source, err.source_index,
                                     err.message, phase_context);
    }
    reporter.print_summary();
    return 1;
  }

  if (!emit_result.metadata.has_main_function) {
    reporter.report_generic_error(core::error_phase_e::CODE_EMISSION,
                                  "No main function found. Cannot run program");
    reporter.print_summary();
    return 1;
  }

  if (emit_result.metadata.has_multiple_mains()) {
    fmt::print(stderr,
               "Warning: Multiple main functions detected. Using first one.\n");
  }

  auto assembly_result =
      emit_result.assemble(emitc::assembly_type_e::APPLICATION);
  std::string c_source = assembly_result.source;

  truk::tcc::tcc_compiler_c compiler;

  for (const auto &path : opts.include_paths) {
    compiler.add_include_path(path);
  }
  for (const auto &path : opts.library_paths) {
    compiler.add_library_path(path);
  }
  for (const auto &lib : opts.libraries) {
    compiler.add_library(lib);
  }
  for (const auto &path : opts.rpaths) {
    compiler.set_rpath(path);
  }

  int argc = static_cast<int>(opts.program_args.size()) + 1;
  std::vector<char *> argv_ptrs;
  argv_ptrs.reserve(argc + 1);

  std::string program_name = opts.input_file;
  argv_ptrs.push_back(const_cast<char *>(program_name.c_str()));

  for (const auto &arg : opts.program_args) {
    argv_ptrs.push_back(const_cast<char *>(arg.c_str()));
  }
  argv_ptrs.push_back(nullptr);

  auto run_result = compiler.compile_and_run(c_source, argc, argv_ptrs.data());

  if (!run_result.success) {
    reporter.report_compilation_error(run_result.error_message);
    reporter.print_summary();
    return 1;
  }

  return run_result.exit_code;
}

} // namespace truk::commands
