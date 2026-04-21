#include <OptionParser.hpp>

enum Options { kHelp, kVerbose, kFoo, kBar, kShort, kLong };

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
        .Help =
            "A long help message that should be wrapped to multiple lines to "
            "demonstrate the line wrapping functionality of the usage printer",
    },
    {
        .ID = Options::kShort,
        .Names = {"xxxxxxxxxxxxxxxxxxx"},
        .Help = "<- Short enough to fit on one line",
    },
    {
        .ID = Options::kLong,
        .Names = {"xxxxxxxxxxxxxxxxxxxx"},
        .Help = "^ Meets or exceeds HelpTextStartColumn, so this help message "
                "is on the next line",
    },
};

int main(int argc, const char **argv) {
  // These are the default settings.
  CommandLine::PrintUsageSettings settings = {
      .OptionIndent = 2,
      .HelpTextStartColumn = 24,
      .MaxLineLength = 80,
  };

  CommandLine::PrintUsage(argv[0],
                          "A long description of the program that should be "
                          "wrapped to multiple lines to demonstrate the line "
                          "wrapping functionality of the usage printer",
                          options, settings);
  return 0;
}
