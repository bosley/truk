# truk

C project build system using CMake with configurable compiler, mode, and target.

## Configuration

Edit `bucket` file:

```
compiler = clang
mode = dev | rel | asan
target = local | docker
```

## Commands

### Build
- `./truk.sh -b` - build entire project
- `./truk.sh -b <pkg>` - build specific package
- `./truk.sh -c` - clean build directory

### Test
- `./truk.sh -t <pkg>` - test package (clean, rebuild, run buildtime + runtime tests)

### Docker
- `./truk.sh --docker-start` - build image, start container
- `./truk.sh --docker-stop` - stop and remove container
- `./truk.sh --docker-status` - check container status
- `./truk.sh --docker-shell` - open shell in container

## Targets

### local
Builds on macOS using `build/` directory

### docker
Builds in Alpine Linux container using `build-docker/` directory with valgrind available
