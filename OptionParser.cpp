/*
 * Copyright (C) 2026 Peter Lilley
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <OptionParser.hpp>
#include <cassert>

CommandLine::~CommandLine() {
  if (m_argv) {
    FreeArgv(m_argc, m_argv);
  }
}

void CommandLine::FreeArgv(int argc, char **argv) {
  for (int i = 0; i < argc; ++i) {
    delete[] argv[i];
  }
  delete[] argv;
}

void CommandLine::PrintLineWrapped(FILE *stream, std::string_view text,
                                   size_t maxLineLength, size_t indent,
                                   size_t currentIndent) {
  while (!text.empty()) {
    if (currentIndent < indent) {
      std::fprintf(stream, "%*s", static_cast<int>(indent - currentIndent), "");
    }

    size_t remainingTextLength = text.length();

    // No need to wrap
    if (remainingTextLength <= maxLineLength - indent) {
      std::fprintf(stream, "%.*s\n", static_cast<int>(remainingTextLength),
                   text.data());
      return;
    }

    size_t lineLength = maxLineLength - indent;

    bool replaceSpace;

    // Find the last space in the line
    size_t lastSpacePosition = text.rfind(' ', lineLength);
    replaceSpace = lastSpacePosition != std::string_view::npos;
    if (replaceSpace) {
      lineLength = lastSpacePosition;
    }

    // Print up to the last space
    std::fprintf(stream, "%.*s\n", static_cast<int>(lineLength), text.data());
    currentIndent = 0;

    // Move past the printed text
    text.remove_prefix(lineLength + replaceSpace);
  }
}

void CommandLine::PrintUsage(FILE *stream, std::string_view programName,
                             std::string_view preamble,
                             std::span<const Option> options,
                             const PrintUsageSettings &settings) noexcept {
  if (!stream) {
    stream = stdout;
  }
  if (programName.empty()) {
    programName = "program";
  }

  std::fprintf(stream, "Usage: %.*s [options] [arguments]\n\n",
               static_cast<int>(programName.size()), programName.data());

  if (!preamble.empty()) {
    PrintLineWrapped(stream, preamble, settings.MaxLineLength, 0);
    std::fprintf(stream, "\n");
  }

  if ((settings.OptionIndent >= settings.HelpTextStartColumn) ||
      (settings.HelpTextStartColumn >= settings.MaxLineLength)) {
    return;
  }

  std::fprintf(stream, "Options:\n");

  for (const Option &option : options) {
    if (option.Names.empty())
      continue;

    size_t lineLength = settings.OptionIndent;
    std::fprintf(stream, "%*s", static_cast<int>(settings.OptionIndent), "");

    // Print option names
    size_t i = 0;
    for (std::string_view name : option.Names) {
      if (name.empty())
        continue;

      const size_t optionNameLength = name.length();
      lineLength += optionNameLength;
      if (optionNameLength == 1) {
        lineLength++;
        std::fprintf(stream, "-");
      } else {
        lineLength += 2;
        std::fprintf(stream, "--");
      }
      std::fprintf(stream, "%.*s", static_cast<int>(optionNameLength),
                   name.data());

      const bool isLast = (++i == option.Names.size());
      if (!isLast) {
        lineLength++;
        std::fprintf(stream, ",");
      }

      lineLength++;
      std::fprintf(stream, " ");
    }

    // Value name
    if (option.RequiresValue) {
      std::string_view valueName = "arg";
      if (!option.ValueName.empty()) {
        valueName = option.ValueName;
      }

      lineLength += 3 + valueName.size();
      std::fprintf(stream, "<%.*s> ", static_cast<int>(valueName.size()),
                   valueName.data());
    }

    // Help text
    if (!option.Help.empty()) {
      if (lineLength > settings.HelpTextStartColumn) {
        std::fprintf(stream, "\n");
        lineLength = 0;
      }

      PrintLineWrapped(stream, option.Help, settings.MaxLineLength,
                       settings.HelpTextStartColumn, lineLength);
    } else {
      std::fprintf(stream, "\n");
    }
  }
}

std::optional<CommandLine> CommandLine::Parse(int argc, const char *argv[],
                                              std::span<const Option> options,
                                              const ParseSettings &settings,
                                              Error *error) noexcept {
  if (argc < 1 || !argv) {
    return {};
  }
  std::string_view programName = argv[0];
  std::vector<std::string_view> args(argv + 1, argv + argc);
  return Parse(programName, args, options, settings, error);
}

size_t CommandLine::MatchOption(const Option &option, std::string_view arg,
                                bool isLongOption) {
  assert(!arg.empty());
  const size_t argLength = arg.length();

  for (size_t i = 0; i < option.Names.size(); ++i) {
    std::string_view name = option.Names[i];
    size_t nameLength = name.length();
    if (name.empty()) {
      continue;
    }

    bool matches = false;
    if (!isLongOption) {
      matches = (nameLength == 1) && (arg[0] == name[0]);
    } else if (nameLength > 1) {
      matches = arg.starts_with(name);
      if (argLength > nameLength) {
        matches &= option.RequiresValue && arg[nameLength] == '=';
      }
    }

    if (matches) {
      return nameLength;
    }
  }

  return 0;
}

CommandLine::ErrorType
CommandLine::ParseOption(size_t *index, std::span<std::string_view> args,
                         std::span<const CommandLine::Option> options) {
  assert(index);
  assert(*index < args.size());

  std::string_view arg = args[*index];
  assert(!arg.empty());

  // Not an option
  if (arg[0] != '-') {
    Arguments.push_back(arg);
    return ErrorType::kNone;
  }
  arg.remove_prefix(1);
  // Empty option
  if (arg.empty()) {
    return ErrorType::kInvalidOption;
  }
  // Second '-'
  const bool isLongOption = arg[0] == '-';
  if (isLongOption) {
    arg.remove_prefix(1);
    if (arg.empty()) {
      return ErrorType::kInvalidOption;
    }
  }

  size_t matchedNameLength = 0;
  const Option *option = &options[0];
  for (size_t i = 0; i < options.size(); ++i, ++option) {
    if ((matchedNameLength = MatchOption(*option, arg, isLongOption))) {
      break;
    }
  }

  if (!matchedNameLength) {
    return ErrorType::kInvalidOption;
  }

  assert(matchedNameLength <= arg.size());

  // No value required
  if (!option->RequiresValue) {
    if (option->AllowsMultiple) {
      MultiOptions[option->ID].push_back("");
    } else if (Options.contains(option->ID)) {
      return ErrorType::kDuplicateOption;
    }

    Options[option->ID] = "";
    return ErrorType::kNone;
  }

  // Separated value
  if (arg.length() == matchedNameLength) {
    if (*index + 1 >= args.size()) {
      return ErrorType::kMissingValue;
    }

    if (option->AllowsMultiple) {
      MultiOptions[option->ID].push_back(args[++(*index)]);
    } else {
      if (Options.contains(option->ID)) {
        return ErrorType::kDuplicateOption;
      }
      Options[option->ID] = args[++(*index)];
    }

    return ErrorType::kNone;
  }

  // Connected value
  arg.remove_prefix(matchedNameLength);
  assert(!arg.empty());

  if (arg[0] == '=') {
    arg.remove_prefix(1);

    if (arg.empty()) {
      return ErrorType::kMissingValue;
    }
  }

  if (option->AllowsMultiple) {
    MultiOptions[option->ID].push_back(arg);
  } else {
    if (Options.contains(option->ID)) {
      return ErrorType::kDuplicateOption;
    }
    Options[option->ID] = arg;
  }

  return ErrorType::kNone;
}

std::optional<CommandLine> CommandLine::Parse(std::string_view programName,
                                              std::span<std::string_view> args,
                                              std::span<const Option> options,
                                              const ParseSettings &settings,
                                              Error *error) noexcept {
  CommandLine cmdLine;

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view argument = args[i];

    if (argument.empty()) {
      continue;
    }

    ErrorType errorType = cmdLine.ParseOption(&i, args, options);
    if (errorType != ErrorType::kNone) {
      if (error) {
        error->type = errorType;
        error->argument = argument;
      } else {
        if (!programName.empty()) {
          std::fprintf(stderr, "%.*s: ", static_cast<int>(programName.size()),
                       programName.data());
        }
        switch (errorType) {
          using enum ErrorType;
        case kInvalidOption:
          std::fprintf(stderr, "invalid option:");
          break;
        case kMissingValue:
          std::fprintf(stderr, "missing value for option:");
          break;
        case kDuplicateOption:
          std::fprintf(stderr, "duplicate option:");
          break;
        case kNone:
          assert(false);
          break;
        }
        std::fprintf(stderr, " '%.*s'\n", static_cast<int>(argument.size()),
                     argument.data());
      }
      return std::nullopt;
    }
  }

  return cmdLine;
}

#if defined(_WIN32)

std::optional<CommandLine> CommandLine::Parse(LPCWSTR cmdLineString,
                                              std::span<const Option> options,
                                              const ParseSettings &settings,
                                              Error *error) noexcept {
  LPWSTR *wcArgv;
  int argc;
  wcArgv = CommandLineToArgvW(cmdLineString, &argc);
  if (!wcArgv || !argc) {
    return {};
  }

  char **argv = new char *[argc];
  for (int i = 0; i < argc; ++i) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wcArgv[i], -1, nullptr, 0,
                                  nullptr, nullptr);
    argv[i] = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, wcArgv[i], -1, argv[i], len, nullptr,
                        nullptr);
  }

  LocalFree(wcArgv);

  std::optional<CommandLine> cmdLine =
      Parse(argc, (const char **)argv, options, settings, error);

  if (!cmdLine) {
    FreeArgv(argc, argv);
  } else {
    cmdLine->m_argc = argc;
    cmdLine->m_argv = argv;
  }

  return cmdLine;
}

#endif
