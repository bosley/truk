# SHOP

This is where I'm building the core libraries in Truk to wrap C things and define idiomatic Truk.
The stdlib will eventually be installed and added to the include path.

We can access stdlib from this level simply because it's there, so includes match the expected use-case on target systems: `stdlib/lib`.

If we import .truk libraries with no file, but at the directory level, it looks for `lib.truk`, which will be considered the public API to the library.
It will likely be a `shard` of one or more categories to fulfill the public API.

Each stdlib will do its own C wrapping, even if that means duplicating code, to ensure that there is no interdependency in the libraries.