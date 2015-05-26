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

void TestTokenizer(
    const string& input,
    const vector<pair<TokenType,string>>& expected) {
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
  TestTokenizer("concat('foo', 'bar', 'baz')",
    {{T_NameTest, "concat"},
     {T_LeftParen, "("},
     {T_Literal, "'foo'"},
     {T_Comma, ","},
     {T_Literal, "'bar'"},
     {T_Comma, ","},
     {T_Literal, "'baz'"},
     {T_RightParen, ")"}});
  TestTokenizer(" test with \t some\nwhitespace\r",
    {{T_NameTest, "test"},
     {T_NameTest, "with"},
     {T_NameTest, "some"},
     {T_NameTest, "whitespace"}});
  TestTokenizer("-123", {{T_Minus, "-"}, {T_Number, "123"}});
  TestTokenizer("/child::*[position()>=1])",
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
