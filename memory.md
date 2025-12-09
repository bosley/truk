# Valgrind Memory Check Commands

**IMPORTANT**: Valgrind conflicts with ASAN. Set `mode = dev` or `mode = rel` in bucket before running valgrind.
Cannot use `mode = asan` with valgrind - they both instrument memory operations.

Run from docker shell: `./truk.sh --docker-shell`

## Find all test binaries
```bash
cd /workspace/build-docker
find /workspace/build-docker/pkg -name "*_tests" -type f -executable
```

## Run tests with valgrind

### Scanner tests
```bash
cd /workspace/build-docker
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
  ./pkg/scanner/tests/scanner_tests

valgrind --leak-check=full ./pkg/scanner/tests/scanner_find_group_tests
valgrind --leak-check=full ./pkg/scanner/tests/scanner_stress_tests
```

### Buffer tests
```bash
valgrind --leak-check=full ./pkg/buffer/tests/buffer_tests
valgrind --leak-check=full ./pkg/buffer/tests/buffer_manipulation_tests
valgrind --leak-check=full ./pkg/buffer/tests/split_buffer_tests
valgrind --leak-check=full ./pkg/buffer/tests/sub_buffer_tests
```

### Ctx tests
```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
  ./pkg/ctx/tests/ctx_tests
```

### Map tests
```bash
valgrind --leak-check=full ./pkg/map/tests/map_tests
```

## Valgrind flags
- `--leak-check=full` - detailed leak info
- `--show-leak-kinds=all` - show all leak types (definite, indirect, possible, reachable)
- `--track-origins=yes` - track uninitialized values origin
- `--error-exitcode=1` - exit non-zero on errors
- `--verbose` - more detailed output
- `--suppressions=file` - suppress known false positives