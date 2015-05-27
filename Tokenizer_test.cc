#include "Tokenizer.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <utility>

using std::string;
using std::vector;
using std::pair;

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
    
TEST(ScanToken, EmptyInput) {
  TestEndOfInput("");
  TestEndOfInput(" ");
  TestEndOfInput("   \r\n\t ");
}

void TestScanToken(const string& input, TokenType expected) {
  const char *token_data = nullptr;
  size_t token_size = 1234567;
  TokenType received = ScanToken(input.data(), input.size(), &token_data, &token_size);
  EXPECT_EQ(expected, received) << input;
  EXPECT_EQ(input.data(), token_data) << input;
  if (expected != T_None) EXPECT_EQ(input.size(), token_size) << input;
}

TEST(ScanToken, SingleToken) {
	TestScanToken("(", T_LeftParen);
	TestScanToken(")", T_RightParen);
	TestScanToken("[", T_LeftBracket);
	TestScanToken("]", T_RightBracket);
	TestScanToken(".", T_Dot);
	TestScanToken("..", T_DoubleDot);
	TestScanToken("@", T_At);
	TestScanToken(",", T_Comma);
	TestScanToken(":", T_None);
	TestScanToken("::", T_DoubleColon);
	TestScanToken("'x'", T_Literal);
	TestScanToken("'foo'", T_Literal);
	TestScanToken("'\"'", T_Literal);
	TestScanToken("\"y\"", T_Literal);
	TestScanToken("\"bar\"", T_Literal);
	TestScanToken("\"'\"", T_Literal);
	TestScanToken("7", T_Number);
	TestScanToken("123", T_Number);
	TestScanToken("4.2", T_Number);
	TestScanToken(".7", T_Number);
	TestScanToken(".123", T_Number);
	TestScanToken("/", T_Slash);
	TestScanToken("//", T_DoubleSlash);
	TestScanToken("|", T_Pipe);
	TestScanToken("+", T_Plus);
	TestScanToken("-", T_Minus);
	TestScanToken("=", T_Equal);
	TestScanToken("!", T_None);
	TestScanToken("!=", T_NotEqual);
	TestScanToken("<", T_LessThan);
	TestScanToken("<=", T_LessEqual);
	TestScanToken(">", T_GreaterThan);
	TestScanToken(">=", T_GreaterEqual);
	TestScanToken("*", T_Multiply);
  TestScanToken("$", T_None);
  TestScanToken("$foo", T_VariableReference);
  TestScanToken("or", T_NameTest);
  TestScanToken("child", T_NameTest);
  TestScanToken("f", T_NameTest);
}

void TestScanToken(
    const string& input,
    const vector<pair<TokenType, string>>& expected) {
  int num_tokens = 0;
  const char* input_data = input.data();
  const char* input_end = input_data + input.size();
  TokenType type;
  const char* token_data;
  size_t token_size;
  while ((type = ScanToken(
      input_data, input_end - input_data,
      &token_data, &token_size)) != T_None) {
    string token(token_data, token_data + token_size);
    EXPECT_EQ(num_tokens < expected.size() ? expected[num_tokens].first : T_None, type);
    EXPECT_EQ(num_tokens < expected.size() ? expected[num_tokens].second : string(), token);
    ++num_tokens;
    input_data = token_data + token_size;
  }
  EXPECT_EQ(0, token_size);
  EXPECT_EQ(expected.size(), num_tokens);
}

TEST(TokenizerTest, MultipleTokens) {
  TestScanToken("concat('foo', 'bar', 'baz')",
    {{T_NameTest, "concat"},
     {T_LeftParen, "("},
     {T_Literal, "'foo'"},
     {T_Comma, ","},
     {T_Literal, "'bar'"},
     {T_Comma, ","},
     {T_Literal, "'baz'"},
     {T_RightParen, ")"}});
  TestScanToken(" test with \t some\nwhitespace\r",
    {{T_NameTest, "test"},
     {T_NameTest, "with"},
     {T_NameTest, "some"},
     {T_NameTest, "whitespace"}});
  TestScanToken("-123", {{T_Minus, "-"}, {T_Number, "123"}});
  TestScanToken("/child::*[position()>=1])",
     {{T_Slash, "/"},
      {T_NameTest, "child"},
      {T_DoubleColon, "::"},
      {T_Multiply, "*"},
      {T_LeftBracket, "["},
      {T_NameTest, "position"},
      {T_LeftParen, "("},
      {T_RightParen, ")"},
      {T_GreaterEqual, ">="},
      {T_Number, "1"},
      {T_RightBracket, "]"},
      {T_RightParen, ")"}});
}

// TODO: unit tests for DisambiguateToken

}  // namespace xpath
}  // namespace
