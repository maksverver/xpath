#include "Tokenizer.h"
#include <gtest/gtest.h>
#include <string>

using std::string;

namespace xpath {
namespace {
  
void TestEndOfInput(const string& input) {
  const char *token_data = nullptr;
  size_t token_size = 1234567;
  TokenType received = ScanToken(input.data(), input.size(), &token_data, &token_size);
  EXPECT_EQ(T_None, received);
  EXPECT_EQ(input.data() + input.size(), token_data);
  EXPECT_EQ(0, token_size);

}
    
TEST(TokenizerTest, EmptyInput) {
  TestEndOfInput("");
  TestEndOfInput(" ");
  TestEndOfInput("   \r\n\t ");
}


void TestScanBasicToken(const string& input, TokenType expected) {
  const char *token_data = nullptr;
  size_t token_size = 1234567;
  TokenType received = ScanToken(input.data(), input.size(), &token_data, &token_size);
  EXPECT_EQ(expected, received) << input;
  EXPECT_EQ(input.data(), token_data) << input;
  if (expected != T_None) EXPECT_EQ(input.size(), token_size) << input;
}

TEST(TokenizerTest, ParseTokens) {
	TestScanBasicToken("(", T_LeftParen);
	TestScanBasicToken(")", T_RightParen);
	TestScanBasicToken("[", T_LeftBracket);
	TestScanBasicToken("]", T_RightBracket);
	TestScanBasicToken(".", T_Dot);
	TestScanBasicToken("..", T_DoubleDot);
	TestScanBasicToken("@", T_At);
	TestScanBasicToken(",", T_Comma);
	TestScanBasicToken(":", T_None);
	TestScanBasicToken("::", T_DoubleColon);
	TestScanBasicToken("'x'", T_Literal);
	TestScanBasicToken("'foo'", T_Literal);
	TestScanBasicToken("'\"'", T_Literal);
	TestScanBasicToken("\"y\"", T_Literal);
	TestScanBasicToken("\"bar\"", T_Literal);
	TestScanBasicToken("\"'\"", T_Literal);
	TestScanBasicToken("7", T_Number);
	TestScanBasicToken("123", T_Number);
	TestScanBasicToken("4.2", T_Number);
	TestScanBasicToken(".7", T_Number);
	TestScanBasicToken(".123", T_Number);
	TestScanBasicToken("/", T_Slash);
	TestScanBasicToken("//", T_DoubleSlash);
	TestScanBasicToken("|", T_Pipe);
	TestScanBasicToken("+", T_Plus);
	TestScanBasicToken("-", T_Minus);
	TestScanBasicToken("=", T_Equal);
	TestScanBasicToken("!", T_None);
	TestScanBasicToken("!=", T_NotEqual);
	TestScanBasicToken("<", T_LessThan);
	TestScanBasicToken("<=", T_LessEqual);
	TestScanBasicToken(">", T_GreaterThan);
	TestScanBasicToken(">=", T_GreaterEqual);
	TestScanBasicToken("*", T_Multiply);
  TestScanBasicToken("$", T_None);
  TestScanBasicToken("$foo", T_VariableReference);
  TestScanBasicToken("or", T_NameTest);
  TestScanBasicToken("child", T_NameTest);
  TestScanBasicToken("f", T_NameTest);
}


// TODO: extended literal test
// TODO: whitespace
// TODO: several tokens in a row
// TODO: testcase with -123 (T_MINUS, T_NUMBER)
// TODO: testcase with more complicated sequence of tokens; check that substrings match
// TODO: unit tests for DisambiguateToken

}  // namespace xpath
}  // namespace
