#include "Tokenizer.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <utility>

using std::string;
using std::vector;
using std::pair;
using testing::ContainerEq;

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
  vector<pair<TokenType, string>> received;
  const char* input_data = input.data();
  const char* input_end = input_data + input.size();
  TokenType token_type;
  const char* token_data;
  size_t token_size;
  while ((token_type = ScanToken(
      input_data, input_end - input_data,
      &token_data, &token_size)) != T_None) {
    received.push_back({token_type, string(token_data, token_data + token_size)});
    input_data = token_data + token_size;
  }
  EXPECT_THAT(received, ContainerEq(expected));
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

TEST(DisambiguateToken, Multiply) {
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_None, T_Multiply, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_At, T_Multiply, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_DoubleColon, T_Multiply, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_LeftParen, T_Multiply, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_LeftBracket, T_Multiply, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_Comma, T_Multiply, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_Slash, T_Multiply, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_Multiply, T_Multiply, T_None));
  EXPECT_EQ(T_Multiply, DisambiguateToken(T_RightParen, T_Multiply, T_None));
  EXPECT_EQ(T_Multiply, DisambiguateToken(T_RightBracket, T_Multiply, T_None));
  EXPECT_EQ(T_Multiply, DisambiguateToken(T_Number, T_Multiply, T_None));
}

TEST(DisambiguateToken, OperatorName) {
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_None, T_NameTest, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_At, T_NameTest, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_DoubleColon, T_NameTest, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_LeftParen, T_NameTest, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_LeftBracket, T_NameTest, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_Comma, T_NameTest, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_Multiply, T_NameTest, T_None));
  EXPECT_EQ(T_NameTest, DisambiguateToken(T_Slash, T_NameTest, T_None));
  EXPECT_EQ(T_OperatorName, DisambiguateToken(T_RightParen, T_NameTest, T_None));
  EXPECT_EQ(T_OperatorName, DisambiguateToken(T_RightBracket, T_NameTest, T_None));
  EXPECT_EQ(T_OperatorName, DisambiguateToken(T_Number, T_NameTest, T_None));
}

TEST(DisambiguateToken, FunctionName) {
  EXPECT_EQ(T_FunctionName, DisambiguateToken(T_None, T_NameTest, T_LeftParen));
}

TEST(DisambiguateToken, AxisName) {
  EXPECT_EQ(T_AxisName, DisambiguateToken(T_None, T_NameTest, T_DoubleColon));
}

TEST(ParseNodeType, ValidNodeTypes) {
  EXPECT_EQ(N_Comment, ParseNodeType("comment"));
  EXPECT_EQ(N_Text, ParseNodeType("text"));
  EXPECT_EQ(N_ProcessingInstruction, ParseNodeType("processing-instruction"));
  EXPECT_EQ(N_Node, ParseNodeType("node"));
}

TEST(ParseNodeType, InvalidNodeTypes) {
  EXPECT_EQ(N_None, ParseNodeType(""));
  EXPECT_EQ(N_None, ParseNodeType("NODE"));
  EXPECT_EQ(N_None, ParseNodeType("nod"));
  EXPECT_EQ(N_None, ParseNodeType("nodex"));
  EXPECT_EQ(N_None, ParseNodeType("processing_instruction"));
  EXPECT_EQ(N_None, ParseNodeType("and"));
}

TEST(ParseOperatorName, ValidOperatorNames) {
  EXPECT_EQ(O_And, ParseOperatorName("and"));
  EXPECT_EQ(O_Or, ParseOperatorName("or"));
  EXPECT_EQ(O_Mod, ParseOperatorName("mod"));
  EXPECT_EQ(O_Div, ParseOperatorName("div"));
}

TEST(ParseOperatorName, InvalidOperatorNames) {
  EXPECT_EQ(O_None, ParseOperatorName(""));
  EXPECT_EQ(O_None, ParseOperatorName("AND"));
  EXPECT_EQ(O_None, ParseOperatorName("an"));
  EXPECT_EQ(O_None, ParseOperatorName("andy"));
  EXPECT_EQ(O_None, ParseOperatorName("node"));
}

TEST(ParseAxisName, ValidAxisNames) {
  EXPECT_EQ(A_Ancestor, ParseAxisName("ancestor"));
  EXPECT_EQ(A_AncestorOrSelf, ParseAxisName("ancestor-or-self"));
  EXPECT_EQ(A_Attribute, ParseAxisName("attribute"));
  EXPECT_EQ(A_Child, ParseAxisName("child"));
  EXPECT_EQ(A_Descendant, ParseAxisName("descendant"));
  EXPECT_EQ(A_DescendantOrSelf, ParseAxisName("descendant-or-self"));
  EXPECT_EQ(A_Following, ParseAxisName("following"));
  EXPECT_EQ(A_FollowingSibling, ParseAxisName("following-sibling"));
  EXPECT_EQ(A_Namespace, ParseAxisName("namespace"));
  EXPECT_EQ(A_Parent, ParseAxisName("parent"));
  EXPECT_EQ(A_Preceding, ParseAxisName("preceding"));
  EXPECT_EQ(A_PrecedingSibling, ParseAxisName("preceding-sibling"));
  EXPECT_EQ(A_Self, ParseAxisName("self"));
}

TEST(ParseAxisName, InvalidAxisNames) {
  EXPECT_EQ(A_None, ParseAxisName(""));
  EXPECT_EQ(A_None, ParseAxisName("ancestor-"));
  EXPECT_EQ(A_None, ParseAxisName("ancestor_or_self"));
  EXPECT_EQ(A_None, ParseAxisName("and"));
  EXPECT_EQ(A_None, ParseAxisName("node"));
}

void TestTokenize(const string& input, size_t parsed_prefix_length,
    const std::vector<std::pair<TokenType, std::string>>& expected) {
  std::vector<std::pair<TokenType, std::string>> received;
  EXPECT_EQ(parsed_prefix_length, Tokenize(input, &received));
  EXPECT_THAT(received, ContainerEq(expected));
}

void TestTokenize(
    const string& input,
    const std::vector<std::pair<TokenType, std::string>>& expected) {
  return TestTokenize(input, input.size(), expected);
}

TEST(Tokenize, ValidExpressions) {
  TestTokenize("", {});
  TestTokenize("foo", {{T_NameTest, "foo"}});
  TestTokenize("/foo/sibling::bar[index()<7]",
     {{T_Slash, "/"},
      {T_NameTest, "foo"},
      {T_Slash, "/"},
      {T_AxisName, "sibling"},
      {T_DoubleColon, "::"},
      {T_NameTest, "bar"},
      {T_LeftBracket, "["},
      {T_FunctionName, "index"},
      {T_LeftParen, "("},
      {T_RightParen, ")"},
      {T_LessThan, "<"},
      {T_Number, "7"},
      {T_RightBracket, "]"}});
}

TEST(Tokenize, ClearsOutput) {
  std::vector<std::pair<TokenType, std::string>> expected, received;
  expected = {{T_NameTest, "foo"}};
  Tokenize("foo", &received);
  EXPECT_THAT(received, ContainerEq(expected));
  expected = {{T_NameTest, "bar"}};
  Tokenize("bar", &received);
  EXPECT_THAT(received, ContainerEq(expected));
  expected.clear();
  EXPECT_EQ(0, Tokenize("~", &received));
  EXPECT_THAT(received, ContainerEq(expected));
}

TEST(Tokenize, InvalidInput) {
  TestTokenize("~", 0, {});
  TestTokenize(" ~", 1, {});
  TestTokenize("foo ~", 4, {{T_NameTest, "foo"}});
}

TEST(Tokenize, IdentifiesNodeType) {
  TestTokenize("/foo()",
      {{T_Slash, "/"},
       {T_FunctionName, "foo"},
       {T_LeftParen, "("},
       {T_RightParen, ")"}});
   TestTokenize("/text()",
       {{T_Slash, "/"},
        {T_NodeType, "text"},
        {T_LeftParen, "("},
        {T_RightParen, ")"}});
}

TEST(Tokenize, InvalidAxisName) {
  // The tokenizer does not currently enforce the validity of axis names.
  TestTokenize("foo::bar",
      {{T_AxisName, "foo"},
       {T_DoubleColon, "::"},
       {T_NameTest, "bar"}});
}

TEST(Tokenize, InvalidOperatorName) {
  // The tokenizer does not currently enforce the validity of operator names.
  TestTokenize("1 plus 2",
      {{T_Number, "1"},
       {T_OperatorName, "plus"},
       {T_Number, "2"}});
}

}  // namespace
}  // namespace xpath
