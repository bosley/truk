#include "truk/kit/kit.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

namespace fs = std::filesystem;

TEST_GROUP(KitParserTests) {
  fs::path test_dir;

  void setup() override {
    test_dir = fs::temp_directory_path() / "truk_kit_test";
    fs::create_directories(test_dir);
  }

  void teardown() override {
    if (fs::exists(test_dir)) {
      fs::remove_all(test_dir);
    }
  }

  void write_kit_file(const std::string &content) {
    std::ofstream file(test_dir / "truk.kit");
    file << content;
    file.flush();
    file.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
};

TEST(KitParserTests, ParseSimpleProject) {
  write_kit_file("project testproject\n");

  auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

  STRCMP_EQUAL("testproject", config.project_name.c_str());
  CHECK_EQUAL(0, config.libraries.size());
  CHECK_EQUAL(0, config.applications.size());
}

TEST(KitParserTests, ParseSimpleApplication) {
  write_kit_file(R"(
project myapp

application main {
    source = main.truk
    output = build/main
}
)");

  auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

  STRCMP_EQUAL("myapp", config.project_name.c_str());
  CHECK_EQUAL(0, config.libraries.size());
  CHECK_EQUAL(1, config.applications.size());

  auto &[name, app] = config.applications[0];
  STRCMP_EQUAL("main", name.c_str());
  CHECK(app.source_entry_file_path.find("main.truk") != std::string::npos);
  CHECK(app.output_file_path.find("build/main") != std::string::npos);
}

TEST(KitParserTests, ParseApplicationWithLibraries) {
  write_kit_file(R"(
application server {
    source = apps/server.truk
    output = build/server
    libraries = http json database
}
)");

  auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

  CHECK_EQUAL(1, config.applications.size());
  auto &[name, app] = config.applications[0];

  CHECK_TRUE(app.libraries.has_value());
  CHECK_EQUAL(3, app.libraries.value().size());
  STRCMP_EQUAL("http", app.libraries.value()[0].c_str());
  STRCMP_EQUAL("json", app.libraries.value()[1].c_str());
  STRCMP_EQUAL("database", app.libraries.value()[2].c_str());
}

TEST(KitParserTests, ParseApplicationWithPaths) {
  write_kit_file("application app {\n"
                 "    source = main.truk\n"
                 "    output = build/app\n"
                 "}\n");

  try {
    auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");
    CHECK_EQUAL(1, config.applications.size());
  } catch (const truk::kit::kit_exception_c &e) {
    FAIL(e.what());
  }
}

TEST(KitParserTests, ParseSimpleLibrary) {
  write_kit_file(R"(
library math {
    source = libs/math/lib.truk
    output = build/libmath.c
}
)");

  auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

  CHECK_EQUAL(1, config.libraries.size());
  auto &[name, lib] = config.libraries[0];

  STRCMP_EQUAL("math", name.c_str());
  CHECK(lib.source_entry_file_path.find("libs/math/lib.truk") !=
        std::string::npos);
  CHECK(lib.output_file_path.find("build/libmath.c") != std::string::npos);
  CHECK_FALSE(lib.depends.has_value());
  CHECK_FALSE(lib.test_file_path.has_value());
}

TEST(KitParserTests, ParseLibraryWithDependencies) {
  write_kit_file(R"(
library database {
    source = libs/db/lib.truk
    output = build/libdb.c
    depends = json logger
}
)");

  auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

  auto &[name, lib] = config.libraries[0];

  CHECK_TRUE(lib.depends.has_value());
  CHECK_EQUAL(2, lib.depends.value().size());
  STRCMP_EQUAL("json", lib.depends.value()[0].c_str());
  STRCMP_EQUAL("logger", lib.depends.value()[1].c_str());
}

TEST(KitParserTests, ParseLibraryWithTest) {
  write_kit_file(R"(
library math {
    source = libs/math/lib.truk
    output = build/libmath.c
    test = libs/math/test.truk
}
)");

  auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

  auto &[name, lib] = config.libraries[0];

  CHECK_TRUE(lib.test_file_path.has_value());
  CHECK(lib.test_file_path.value().find("libs/math/test.truk") !=
        std::string::npos);
}

TEST(KitParserTests, ParseMultipleLibrariesAndApps) {
  write_kit_file("project webserver\n"
                 "\n"
                 "library json {\n"
                 "    source = libs/json/lib.truk\n"
                 "    output = build/libjson.c\n"
                 "}\n"
                 "\n"
                 "library http {\n"
                 "    source = libs/http/lib.truk\n"
                 "    output = build/libhttp.c\n"
                 "    depends = json\n"
                 "}\n"
                 "\n"
                 "application server {\n"
                 "    source = apps/server/main.truk\n"
                 "    output = build/server\n"
                 "    libraries = http json\n"
                 "}\n");

  try {
    auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

    STRCMP_EQUAL("webserver", config.project_name.c_str());
    CHECK_EQUAL(2, config.libraries.size());
    CHECK_EQUAL(1, config.applications.size());

    STRCMP_EQUAL("json", config.libraries[0].first.c_str());
    STRCMP_EQUAL("http", config.libraries[1].first.c_str());

    STRCMP_EQUAL("server", config.applications[0].first.c_str());
  } catch (const truk::kit::kit_exception_c &e) {
    FAIL(e.what());
  }
}

TEST(KitParserTests, ParsePathsWithSlashes) {
  write_kit_file(R"(
application test {
    source = apps/nested/deep/main.truk
    output = build/output/test
}
)");

  auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

  auto &[name, app] = config.applications[0];
  CHECK(app.source_entry_file_path.find("apps/nested/deep/main.truk") !=
        std::string::npos);
  CHECK(app.output_file_path.find("build/output/test") != std::string::npos);
}

TEST(KitParserTests, ParseWithComments) {
  write_kit_file("# This is a comment\n"
                 "project myproject\n"
                 "\n"
                 "# Another comment\n"
                 "library math {\n"
                 "    source = lib.truk\n"
                 "    output = build/lib.c\n"
                 "}\n");

  auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

  STRCMP_EQUAL("myproject", config.project_name.c_str());
  CHECK_EQUAL(1, config.libraries.size());
}

TEST(KitParserTests, ParseQuotedStrings) {
  write_kit_file("application test {\n"
                 "    source = \"path with spaces/main.truk\"\n"
                 "    output = \"output path/test\"\n"
                 "}\n");

  try {
    auto config = truk::kit::parse_kit_file(test_dir / "truk.kit");

    auto &[name, app] = config.applications[0];
    CHECK(app.source_entry_file_path.find("path with spaces/main.truk") !=
          std::string::npos);
    CHECK(app.output_file_path.find("output path/test") != std::string::npos);
  } catch (const truk::kit::kit_exception_c &e) {
    FAIL(e.what());
  }
}

TEST(KitParserTests, ErrorOnMissingRequiredField_Library) {
  write_kit_file("library math {\n    source = lib.truk\n}\n");

  try {
    truk::kit::parse_kit_file(test_dir / "truk.kit");
    FAIL("Expected exception for missing 'output' field");
  } catch (const truk::kit::kit_exception_c &e) {
    CHECK(std::string(e.what()).find("missing required field 'output'") !=
          std::string::npos);
  }
}

TEST(KitParserTests, ErrorOnMissingRequiredField_Application) {
  write_kit_file("application main {\n    output = build/main\n}\n");

  try {
    truk::kit::parse_kit_file(test_dir / "truk.kit");
    FAIL("Expected exception for missing 'source' field");
  } catch (const truk::kit::kit_exception_c &e) {
    CHECK(std::string(e.what()).find("missing required field 'source'") !=
          std::string::npos);
  }
}

TEST(KitParserTests, ErrorOnDuplicateLibraryName) {
  write_kit_file("library math {\n"
                 "    source = lib1.truk\n"
                 "    output = build/lib1.c\n"
                 "}\n"
                 "library math {\n"
                 "    source = lib2.truk\n"
                 "    output = build/lib2.c\n"
                 "}\n");

  try {
    truk::kit::parse_kit_file(test_dir / "truk.kit");
    FAIL("Expected exception for duplicate library name");
  } catch (const truk::kit::kit_exception_c &e) {
    CHECK(e.type() == truk::kit::exception_e::PARSE_ERROR);
  }
}

TEST(KitParserTests, ErrorOnDuplicateApplicationName) {
  write_kit_file("application main {\n"
                 "    source = main1.truk\n"
                 "    output = build/main1\n"
                 "}\n"
                 "application main {\n"
                 "    source = main2.truk\n"
                 "    output = build/main2\n"
                 "}\n");

  try {
    truk::kit::parse_kit_file(test_dir / "truk.kit");
    FAIL("Expected exception for duplicate application name");
  } catch (const truk::kit::kit_exception_c &e) {
    CHECK(e.type() == truk::kit::exception_e::PARSE_ERROR);
  }
}

TEST(KitParserTests, ErrorOnUnknownLibraryField) {
  write_kit_file("library math {\n"
                 "    source = lib.truk\n"
                 "    output = build/lib.c\n"
                 "    invalid_field = value\n"
                 "}\n");

  try {
    truk::kit::parse_kit_file(test_dir / "truk.kit");
    FAIL("Expected exception for unknown field");
  } catch (const truk::kit::kit_exception_c &e) {
    CHECK(e.type() == truk::kit::exception_e::PARSE_ERROR);
  }
}

TEST(KitParserTests, ErrorOnUnknownApplicationField) {
  write_kit_file("application main {\n"
                 "    source = main.truk\n"
                 "    output = build/main\n"
                 "    bad_field = value\n"
                 "}\n");

  try {
    truk::kit::parse_kit_file(test_dir / "truk.kit");
    FAIL("Expected exception for unknown field");
  } catch (const truk::kit::kit_exception_c &e) {
    CHECK(e.type() == truk::kit::exception_e::PARSE_ERROR);
  }
}

TEST(KitParserTests, ErrorOnMissingClosingBrace) {
  write_kit_file("library math {\n"
                 "    source = lib.truk\n"
                 "    output = build/lib.c\n");

  try {
    truk::kit::parse_kit_file(test_dir / "truk.kit");
    FAIL("Expected exception for missing closing brace");
  } catch (const truk::kit::kit_exception_c &e) {
    CHECK(e.type() == truk::kit::exception_e::PARSE_ERROR);
  } catch (...) {
    FAIL("Unexpected exception type");
  }
}

TEST_GROUP(KitResolverTests) {
  truk::kit::kit_config_s config;

  void setup() override {
    config.project_name = "test";
    config.kit_file_directory = "/tmp";
  }
};

TEST(KitResolverTests, ResolveNoDependencies) {
  config.libraries.emplace_back(
      "lib1", truk::kit::target_library_c("lib1.truk", "lib1.c"));
  config.libraries.emplace_back(
      "lib2", truk::kit::target_library_c("lib2.truk", "lib2.c"));

  auto order = truk::kit::resolve_build_order(config);

  CHECK_EQUAL(2, order.libraries.size());
}

TEST(KitResolverTests, ResolveSimpleDependency) {
  config.libraries.emplace_back(
      "json", truk::kit::target_library_c("json.truk", "json.c"));

  std::vector<std::string> deps = {"json"};
  config.libraries.emplace_back(
      "database", truk::kit::target_library_c("db.truk", "db.c", deps));

  auto order = truk::kit::resolve_build_order(config);

  CHECK_EQUAL(2, order.libraries.size());
  STRCMP_EQUAL("json", order.libraries[0].first.c_str());
  STRCMP_EQUAL("database", order.libraries[1].first.c_str());
}

TEST(KitResolverTests, ResolveComplexDependencies) {
  config.libraries.emplace_back(
      "json", truk::kit::target_library_c("json.truk", "json.c"));

  std::vector<std::string> db_deps = {"json"};
  config.libraries.emplace_back(
      "database", truk::kit::target_library_c("db.truk", "db.c", db_deps));

  std::vector<std::string> http_deps = {"json"};
  config.libraries.emplace_back(
      "http", truk::kit::target_library_c("http.truk", "http.c", http_deps));

  std::vector<std::string> api_deps = {"http", "database"};
  config.libraries.emplace_back(
      "api", truk::kit::target_library_c("api.truk", "api.c", api_deps));

  auto order = truk::kit::resolve_build_order(config);

  CHECK_EQUAL(4, order.libraries.size());
  STRCMP_EQUAL("json", order.libraries[0].first.c_str());

  bool db_before_api = false;
  bool http_before_api = false;
  for (size_t i = 0; i < order.libraries.size(); ++i) {
    if (order.libraries[i].first == "database") {
      for (size_t j = i + 1; j < order.libraries.size(); ++j) {
        if (order.libraries[j].first == "api") {
          db_before_api = true;
        }
      }
    }
    if (order.libraries[i].first == "http") {
      for (size_t j = i + 1; j < order.libraries.size(); ++j) {
        if (order.libraries[j].first == "api") {
          http_before_api = true;
        }
      }
    }
  }
  CHECK_TRUE(db_before_api);
  CHECK_TRUE(http_before_api);
}

TEST(KitResolverTests, ErrorOnCircularDependency) {
  std::vector<std::string> deps_a = {"lib_b"};
  config.libraries.emplace_back(
      "lib_a", truk::kit::target_library_c("a.truk", "a.c", deps_a));

  std::vector<std::string> deps_b = {"lib_a"};
  config.libraries.emplace_back(
      "lib_b", truk::kit::target_library_c("b.truk", "b.c", deps_b));

  try {
    truk::kit::resolve_build_order(config);
    FAIL("Expected exception for circular dependency");
  } catch (const truk::kit::kit_exception_c &e) {
    CHECK(std::string(e.what()).find("Circular dependency") !=
          std::string::npos);
  }
}

TEST(KitResolverTests, ErrorOnUnknownDependency) {
  std::vector<std::string> deps = {"nonexistent"};
  config.libraries.emplace_back(
      "lib", truk::kit::target_library_c("lib.truk", "lib.c", deps));

  try {
    truk::kit::resolve_build_order(config);
    FAIL("Expected exception for unknown dependency");
  } catch (const truk::kit::kit_exception_c &e) {
    CHECK(std::string(e.what()).find("unknown library") != std::string::npos);
  }
}

TEST(KitResolverTests, ApplicationsComeLast) {
  config.libraries.emplace_back(
      "lib", truk::kit::target_library_c("lib.truk", "lib.c"));

  config.applications.emplace_back(
      "app", truk::kit::target_application_c("app.truk", "app"));

  auto order = truk::kit::resolve_build_order(config);

  CHECK_EQUAL(1, order.libraries.size());
  CHECK_EQUAL(1, order.applications.size());
}

TEST_GROUP(KitUtilsTests) {
  fs::path test_dir;

  void setup() override {
    test_dir = fs::temp_directory_path() / "truk_utils_test";
    fs::create_directories(test_dir);
  }

  void teardown() override {
    if (fs::exists(test_dir)) {
      fs::remove_all(test_dir);
    }
  }
};

TEST(KitUtilsTests, FindKitFileInCurrentDir) {
  std::ofstream file(test_dir / "truk.kit");
  file << "project test\n";
  file.close();

  auto found = truk::kit::find_kit_file(test_dir);

  CHECK_TRUE(found.has_value());
  STRCMP_EQUAL("truk.kit", found.value().filename().string().c_str());
}

TEST(KitUtilsTests, FindKitFileInParentDir) {
  fs::path subdir = test_dir / "subdir" / "nested";
  fs::create_directories(subdir);

  std::ofstream file(test_dir / "truk.kit");
  file << "project test\n";
  file.close();

  auto found = truk::kit::find_kit_file(subdir);

  CHECK_TRUE(found.has_value());
  STRCMP_EQUAL("truk.kit", found.value().filename().string().c_str());
}

TEST(KitUtilsTests, FindKitFileNotFound) {
  auto found = truk::kit::find_kit_file(test_dir);

  CHECK_FALSE(found.has_value());
}

TEST(KitUtilsTests, ResolveRelativePath) {
  fs::path base = "/home/user/project";
  std::string relative = "libs/math/lib.truk";

  auto resolved = truk::kit::resolve_path(base, relative);

  CHECK(resolved.string().find("libs/math/lib.truk") != std::string::npos);
}

TEST(KitUtilsTests, ResolveAbsolutePath) {
  fs::path base = "/home/user/project";
  std::string absolute = "/usr/local/lib/test.truk";

  auto resolved = truk::kit::resolve_path(base, absolute);

  STRCMP_EQUAL("/usr/local/lib/test.truk", resolved.string().c_str());
}

TEST(KitUtilsTests, ResolvePathWithDotDot) {
  fs::path base = "/home/user/project/subdir";
  std::string relative = "../other/file.truk";

  auto resolved = truk::kit::resolve_path(base, relative);

  CHECK(resolved.string().find("other/file.truk") != std::string::npos);
  CHECK(resolved.string().find("subdir") == std::string::npos);
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
