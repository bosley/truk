# Example 

Libraries here always have a "src" that contains the src files, with headers
being up-top here. The SRC may have subdirs that split the library into "implementations" for os-specific concerns, but they also might not.

Each package can have its buildtime and runtime tests defined under `tests` directory.

builtime tests happen at compile time, and optional builtime tests are .sh scripts. The way runtime tests work is this: the `truk.sh` file at the root of the repo has commands to build/run the project and run tests. one of the flags will be to iterate over each of the `pkg/name/tests/runtime` files for `.sh` scripts to run. it will run the script and expect exit success from the script to consider it a pass.
