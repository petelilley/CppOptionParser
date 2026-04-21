#include <OptionParser.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

TEST(OptionParserTest, Arguments) {
  enum {
    kOptionFoo,
    kOptionBar,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"foo", "f"},
      },
      {
          .ID = kOptionBar,
          .Names = {"bar", "b"},
      },
  };

  std::string_view programName = "program";
  {
    std::vector<std::string_view> args = {"arg1", "arg2"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    ASSERT_THAT(cmdLine->Options, IsEmpty());
    ASSERT_THAT(cmdLine->MultiOptions, IsEmpty());
    ASSERT_THAT(cmdLine->Arguments, ElementsAre("arg1", "arg2"));
  }

  {
    std::vector<std::string_view> args = {"foo", "bar", "foo"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    ASSERT_THAT(cmdLine->Options, IsEmpty());
    ASSERT_THAT(cmdLine->MultiOptions, IsEmpty());
    ASSERT_THAT(cmdLine->Arguments, ElementsAre("foo", "bar", "foo"));
  }
}

TEST(OptionParserTest, ShortOption) {
  enum {
    kOptionFoo,
    kOptionBar,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"f"},
      },
      {
          .ID = kOptionBar,
          .Names = {"b"},
          .RequiresValue = true,
      },
  };

  std::string_view programName = "program";

  // No value
  {
    std::vector<std::string_view> args = {"-f"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, IsEmpty());
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionFoo, "")));
  }

  // Separated value
  {
    std::vector<std::string_view> args = {"-b", "value"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, IsEmpty());
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionBar, "value")));
  }

  // Connected value
  {
    std::vector<std::string_view> args = {"-b=value"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, IsEmpty());
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionBar, "value")));
  }

  // Connected value
  {
    std::vector<std::string_view> args = {"-bvalue"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, IsEmpty());
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionBar, "value")));
  }
}

TEST(OptionParserTest, LongOption) {
  enum {
    kOptionFoo,
    kOptionBar,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"foo"},
      },
      {
          .ID = kOptionBar,
          .Names = {"bar"},
          .RequiresValue = true,
      },
  };

  std::string_view programName = "program";

  // No value
  {
    std::vector<std::string_view> args = {"--foo"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, IsEmpty());
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionFoo, "")));
  }

  // Separated value
  {
    std::vector<std::string_view> args = {"--bar", "value"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, IsEmpty());
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionBar, "value")));
  }

  // Connected value
  {
    std::vector<std::string_view> args = {"--bar=value"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, IsEmpty());
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionBar, "value")));
  }
}

TEST(OptionParserTest, EmptyArgument) {
  enum {
    kOptionFoo,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"foo"},
      },
  };

  std::string_view programName = "program";

  {
    std::vector<std::string_view> args = {"", "--foo"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, IsEmpty());
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionFoo, "")));
  }
}

TEST(OptionParserTest, AllowMultipleOptions) {
  enum {
    kOptionFoo,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"foo"},
          .RequiresValue = true,
          .AllowsMultiple = true,
      },
  };

  std::string_view programName = "program";

  {
    std::vector<std::string_view> args = {"--foo=hello", "--foo=what"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, IsEmpty());
    EXPECT_THAT(cmdLine->Options, IsEmpty());
    EXPECT_THAT(
        cmdLine->MultiOptions,
        UnorderedElementsAre(std::make_pair(
            kOptionFoo, std::vector<std::string_view>{"hello", "what"})));
  }
}

TEST(OptionParserTest, ErrorInvalidOption) {
  enum {
    kOptionFoo,
    kOptionBar,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"foo"},
          .RequiresValue = true,
          .AllowsMultiple = true,
      },
      {
          .ID = kOptionFoo,
          .Names = {"b"},
      },
  };

  std::string_view programName = "program";

  // Empty
  {
    std::vector<std::string_view> args = {"-"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kInvalidOption);
    ASSERT_EQ(error.argument, "-");
  }

  // Empty
  {
    std::vector<std::string_view> args = {"--"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kInvalidOption);
    ASSERT_EQ(error.argument, "--");
  }

  // Unknown
  {
    std::vector<std::string_view> args = {"--bar"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kInvalidOption);
    ASSERT_EQ(error.argument, "--bar");
  }

  // Unknown, but matches prefix of valid option
  {
    std::vector<std::string_view> args = {"--foobar"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kInvalidOption);
    ASSERT_EQ(error.argument, "--foobar");
  }

  // Single dash, but it's a long option so no match
  {
    std::vector<std::string_view> args = {"-foo"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kInvalidOption);
    ASSERT_EQ(error.argument, "-foo");
  }

  // Double dash, but it's a short option so no match
  {
    std::vector<std::string_view> args = {"--b"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kInvalidOption);
    ASSERT_EQ(error.argument, "--b");
  }
}

TEST(OptionParserTest, ErrorDuplicateOption) {
  enum {
    kOptionFoo,
    kOptionBar,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"foo"},
          .RequiresValue = true,
      },
      {
          .ID = kOptionBar,
          .Names = {"bar", "b"},
      },
  };

  std::string_view programName = "program";

  {
    std::vector<std::string_view> args = {"--foo=hello", "--foo=what"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kDuplicateOption);
    ASSERT_EQ(error.argument, "--foo=what");
  }

  {
    std::vector<std::string_view> args = {"--bar", "--bar"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kDuplicateOption);
    ASSERT_EQ(error.argument, "--bar");
  }

  {
    std::vector<std::string_view> args = {"--bar", "-b"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kDuplicateOption);
    ASSERT_EQ(error.argument, "-b");
  }
}

TEST(OptionParserTest, ErrorMissingValue) {
  enum {
    kOptionFoo,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"foo", "f"},
          .RequiresValue = true,
      },
  };

  std::string_view programName = "program";

  {
    std::vector<std::string_view> args = {"--foo"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kMissingValue);
    ASSERT_EQ(error.argument, "--foo");
  }

  {
    std::vector<std::string_view> args = {"-f"};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kMissingValue);
    ASSERT_EQ(error.argument, "-f");
  }

  {
    std::vector<std::string_view> args = {"--foo="};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kMissingValue);
    ASSERT_EQ(error.argument, "--foo=");
  }

  {
    std::vector<std::string_view> args = {"-f="};
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(programName, args, options, &error);
    ASSERT_FALSE(cmdLine.has_value());
    ASSERT_EQ(error.type, CommandLine::ErrorType::kMissingValue);
    ASSERT_EQ(error.argument, "-f=");
  }
}

TEST(OptionParserTest, ArgcArgv) {
  enum {
    kOptionFoo,
    kOptionBar,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"foo", "f"},
          .RequiresValue = true,
      },
      {
          .ID = kOptionBar,
          .Names = {"bar", "b"},
      },
  };

  std::string_view programName = "program";

  {
    const char *argv[] = {"program", "--foo=hello", "arg1", "-b"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(argc, argv, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, ElementsAre("arg1"));
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionFoo, "hello"),
                                     std::make_pair(kOptionBar, "")));
  }
}

#ifdef _WIN32

TEST(OptionParserTest, WindowsCommandLine) {
  enum {
    kOptionFoo,
    kOptionBar,
  };

  static const CommandLine::Option options[] = {
      {
          .ID = kOptionFoo,
          .Names = {"foo", "f"},
          .RequiresValue = true,
      },
      {
          .ID = kOptionBar,
          .Names = {"bar", "b"},
      },
  };

  std::string_view programName = "program";

  {
    LPCWSTR cmdLineString = L"--foo=hello arg1 -b";
    CommandLine::Error error;
    std::optional<CommandLine> cmdLine =
        CommandLine::Parse(cmdLineString, options, &error);
    ASSERT_TRUE(cmdLine.has_value());

    EXPECT_THAT(cmdLine->Arguments, ElementsAre("arg1"));
    EXPECT_THAT(cmdLine->MultiOptions, IsEmpty());
    EXPECT_THAT(cmdLine->Options,
                UnorderedElementsAre(std::make_pair(kOptionFoo, "hello"),
                                     std::make_pair(kOptionBar, "")));
  }
}

#endif
