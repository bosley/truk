FROM alpine:latest

RUN apk update && apk add --no-cache \
    cmake \
    make \
    clang \
    clang-dev \
    gcc \
    g++ \
    musl-dev \
    git \
    bash \
    valgrind \
    build-base \
    compiler-rt \
    sdl2-dev \
    pkgconf

WORKDIR /workspace

CMD ["tail", "-f", "/dev/null"]
