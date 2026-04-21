#include <OptionParser.hpp>
#include <iostream>

enum Options { kHelp, kVerbose, kFoo, kBar };

static const CommandLine::Option options[] = {
    {
        .ID = Options::kHelp,
        .Names = {"help", "h"},
        .Help = "Print this help message",
    },
    {
        .ID = Options::kVerbose,
        .Names = {"verbose", "v"},
        .Help = "Enable verbose output",
    },
    {
        .ID = Options::kFoo,
        .Names = {"foo", "f"},
        .RequiresValue = true,
        .ValueName = "bar",
        .Help = "Do something",
    },
    {
        .ID = Options::kBar,
        .Names = {"bar", "b"},
        .RequiresValue = true,
        .AllowsMultiple = true,
        .Help = "Do something else",
    },
};

int main(int argc, const char **argv) {
  auto cmdLine = CommandLine::Parse(argc, argv, options);
  if (!cmdLine)
    return 1;

  if (cmdLine->Options.contains(Options::kHelp)) {
    CommandLine::PrintUsage(argv[0], options);
    return 0;
  }

  if (cmdLine->Options.contains(Options::kVerbose)) {
    std::cout << "Verbose mode enabled\n";
  }
  if (cmdLine->Options.contains(Options::kFoo)) {
    std::cout << "Foo value: " << cmdLine->Options[Options::kFoo] << "\n";
  }
  if (cmdLine->MultiOptions.contains(Options::kBar)) {
    for (const auto &value : cmdLine->MultiOptions[Options::kBar]) {
      std::cout << "Bar value: " << value << "\n";
    }
  }

  for (const auto &arg : cmdLine->Arguments) {
    std::cout << "Argument: " << arg << "\n";
  }

  return 0;
}
