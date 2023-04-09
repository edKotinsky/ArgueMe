# ArgueMe

Simple and lightweight command line parsing library.

Library is under active development, so feel free to create issues and open pull
requests, if you have found a bug or have an idea. I'll be glad to receive your
help!

## Usage

```sh
git clone https://github.com/edKotinsky/ArgueMe.git
cd ArgueMe/
```

### Make and run tests

It will fetch and compile Catch2 testing framework first.

```sh
cmake -S . -B build -D ARGUEME_TEST=ON
```

### Make examples

```sh
cmake -S . -B build -D ARGUEME_EXAMPLES=ON
```

### Make documentation

Documentation is not written yet.

### Building

After configuring cmake run:

```sh
cmake --build build
```

Or `cd` to the `build` directory and build with your preffered building tool.

### Setup with FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
	ArgueMe
	GIT_REPOSITORY git clone https://github.com/edKotinsky/ArgueMe.git
)

FetchContent_MakeAvailable(ArgueMe)

target_link_libraries(${YOUR_TARGET} ArgueMe)
```

## License

Licensed under [MIT License](./LICENSE)
