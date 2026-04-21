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

#pragma once

#include <cstddef>
#include <cstdio>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

struct CommandLine {
  std::unordered_map<int, std::string_view> Options;
  std::unordered_map<int, std::vector<std::string_view>> MultiOptions;
  std::vector<std::string_view> Arguments;

  ~CommandLine();

  struct Option {
    int ID;
    std::vector<std::string_view> Names;
    bool RequiresValue = false;
    bool AllowsMultiple = false;
    std::string_view ValueName = "";
    std::string_view Help = "";
  };

  struct PrintUsageSettings {
    size_t OptionIndent = 2;
    size_t HelpTextStartColumn = 24;
    size_t MaxLineLength = 80;
  };

  static void PrintUsage(std::string_view programName,
                         std::span<const Option> options) noexcept {
    PrintUsage(programName, "", options);
  }

  static void PrintUsage(std::string_view programName,
                         std::span<const Option> options,
                         const PrintUsageSettings &settings) noexcept {
    PrintUsage(programName, "", options, settings);
  }

  static void PrintUsage(std::string_view programName,
                         std::string_view preamble,
                         std::span<const Option> options) noexcept {
    PrintUsage(stdout, programName, preamble, options);
  }

  static void PrintUsage(std::string_view programName,
                         std::string_view preamble,
                         std::span<const Option> options,
                         const PrintUsageSettings &settings) noexcept {
    PrintUsage(stdout, programName, preamble, options, settings);
  }

  static void PrintUsage(FILE *stream, std::string_view programName,
                         std::string_view preamble,
                         std::span<const Option> options) noexcept {
    PrintUsage(stream, programName, preamble, options, PrintUsageSettings{});
  }

  static void PrintUsage(FILE *stream, std::string_view programName,
                         std::string_view preamble,
                         std::span<const Option> options,
                         const PrintUsageSettings &settings) noexcept;

  struct ParseSettings {};

  enum class ErrorType {
    kNone,
    kInvalidOption,
    kMissingValue,
    kDuplicateOption,
  };

  struct Error {
    ErrorType type;
    std::string argument;
  };

  static std::optional<CommandLine> Parse(int argc, const char *argv[],
                                          std::span<const Option> options,
                                          Error *error = nullptr) noexcept {
    return Parse(argc, argv, options, ParseSettings{}, error);
  }

  static std::optional<CommandLine> Parse(int argc, const char *argv[],
                                          std::span<const Option> options,
                                          const ParseSettings &settings,
                                          Error *error = nullptr) noexcept;

  static std::optional<CommandLine> Parse(std::string_view programName,
                                          std::span<std::string_view> args,
                                          std::span<const Option> options,
                                          Error *error = nullptr) noexcept {
    return Parse(programName, args, options, ParseSettings{}, error);
  }

  static std::optional<CommandLine> Parse(std::string_view programName,
                                          std::span<std::string_view> args,
                                          std::span<const Option> options,
                                          const ParseSettings &settings,
                                          Error *error = nullptr) noexcept;

#ifdef _WIN32
  static std::optional<CommandLine> Parse(LPCWSTR cmdLine,
                                          std::span<const Option> options,
                                          Error *error = nullptr) noexcept {
    return Parse(cmdLine, options, ParseSettings{}, error);
  }

  static std::optional<CommandLine> Parse(LPCWSTR cmdLine,
                                          std::span<const Option> options,
                                          const ParseSettings &settings,
                                          Error *error = nullptr) noexcept;
#endif

private:
  static void PrintLineWrapped(FILE *stream, std::string_view text,
                               size_t maxLineLength, size_t indent,
                               size_t currentIndent = 0);

  static size_t MatchOption(const Option &option, std::string_view arg,
                            bool isLongOption);

  ErrorType ParseOption(size_t *index, std::span<std::string_view> args,
                        std::span<const CommandLine::Option> options);

private:
  int m_argc = 0;
  char **m_argv = nullptr;

  static void FreeArgv(int argc, char **argv);
};
