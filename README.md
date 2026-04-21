# C++ Option Parser

A small and ultra efficient C++ 20 command-line option parser.

This library makes heavy use of `std::string_view` to point directly into original command-line argument strings, so no heap allocations are made during parsing except for the `std::unordered_map` and `std::vector` used to store the parsed options and arguments respectively.

This library consists of only a header and a single source file, so you can just copy them into your project and start using it right away, or build it as a CMake submodule and link against it.

## Examples 

See the [`examples`](./examples) directory for some example programs.

### Basic Example

This example declares a few options. If the `help` option is provided, the program will print a usage message and exit. Option `foo` requires a value

```cpp
#include <OptionParser.hpp>
#include <print>

enum Options { kHelp, kVerbose, kFoo };

static const CommandLine::Option options[] = {
    {
        .ID = Options::kHelp,
        .Names = { "help", "h" },
        .Help = "Print this help message",
    },
    {
        .ID = Options::kVerbose,
        .Names = { "verbose", "v"},
        .Help = "Enable verbose output",
    },
    {
        .ID = Options::kFoo,
        .Names = { "foo", "f" },
        .RequiresValue = true,
        .ValueName = "bar",
        .Help = "Do something",
    },
};

int main(int argc, const char** argv) {
    auto cmdLine = CommandLine::Parse(argc, argv, options);
    if (!cmdLine)
        return 1;
    
    if (cmdLine->Options.contains(Options::kHelp)) {
        CommandLine::PrintUsage(argv[0], options);
        return 0;
    }
    
    if (cmdLine->Options.contains(Options::kVerbose)) {
        std::println("Verbose mode enabled");
    }
    if (cmdLine->Options.contains(Options::kFoo)) {
        std::println("Foo value: {}", cmdLine->Options[Options::kFoo]); 
    }
    
    for (const auto& arg : cmdLine->Arguments) {
        std::println("Argument: {}", arg);
    }
}
```

<details>
  <summary>Usage message</summary>

  > ```
  > Usage: program-name [options] [arguments]
  >
  > Options:
  >   --help, -h            Print this help message
  >   --verbose, -v         Enable verbose output
  >   --foo, -f <bar>       Do something
  > ```

</details>

### Manual Error Handling

Instead of relying on the library to print error messages, you can handle errors yourself by simply passing a pointer to a `CommandLine::Error` struct to the `Parse` function, like so:

```cpp
int main(int argc, const char** argv) {
    CommandLine::Error error;
    auto cmdLine = CommandLine::Parse(argc, argv, options, &error);
    if (!cmdLine) {
        HandleError(error.type, error.argument);       
        return 1;
    }
    // ...
}
```

### Windows

Starting from `WinMain` instead? No problem:

```cpp
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    auto cmdLine = CommandLine::Parse(GetCommandLineW(), options);
    // ...
}
```

## License

Released under the MIT License. See [LICENSE](./LICENSE) for details.
