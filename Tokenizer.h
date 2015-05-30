#ifndef XPATH_TOKENIZER_INCLUDED
#define XPATH_TOKENIZER_INCLUDED

#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>
#include <functional>

namespace xpath {

enum TokenType {
  T_None = 0,

  T_LeftParen,
  T_RightParen,
  T_LeftBracket,
  T_RightBracket,
  T_Dot,
  T_DoubleDot,
  T_At,
  T_Comma,
  T_DoubleColon,

  // String literal, enclosed in single or double quotes
  T_Literal,

  // Number in decimal notation.
  T_Number,

  // Operators
  T_Slash,
  T_DoubleSlash,
  T_Pipe,
  T_Plus,
  T_Minus,
  T_Equal,
  T_NotEqual,
  T_LessThan,
  T_LessEqual,
  T_GreaterThan,
  T_GreaterEqual,
  T_Multiply,

  T_VariableReference,

  // Identifiers
  T_NodeType,
  T_OperatorName,
  T_FunctionName,
  T_AxisName,
  T_NameTest };

enum NodeType {
  N_None = 0,

  N_Comment,
  N_Text,
  N_ProcessingInstruction,
  N_Node };

enum OperatorName {
  O_None = 0,

  O_And,
  O_Or,
  O_Mod,
  O_Div };

enum AxisName {
  A_None = 0,

  A_Ancestor,
  A_AncestorOrSelf,
  A_Attribute,
  A_Child,
  A_Descendant,
  A_DescendantOrSelf,
  A_Following,
  A_FollowingSibling,
  A_Namespace,
  A_Parent,
  A_Preceding,
  A_PrecedingSibling,
  A_Self };

// Converts the string described by `data` and `size` to a NodeType or returns
// N_None if it does not match any node type. Matching is case sensitive.
NodeType ParseNodeType(const char* data, size_t size);

inline NodeType ParseNodeType(const std::string& s) {
  return ParseNodeType(s.data(), s.size());
}

// Converts the string described by `data` and `size` to an OperatorName or
// returns O_None if it does not match any operator name. Matching is case
// sensitive.
OperatorName ParseOperatorName(const char* data, size_t size);

inline OperatorName ParseOperatorName(const std::string& s) {
  return ParseOperatorName(s.data(), s.size());
}

// Converts the string described by `data` and `size` to an AxisType or returns
// A_None if it does not match any axis type. Matching is case sensitive.
AxisName ParseAxisName(const char* data, size_t size);

inline AxisName ParseAxisName(const std::string& s) {
  return ParseAxisName(s.data(), s.size());
}

// Scans the next token (skipping whitespace, if any) from the string described
// by `data` and `size`.
//
// If a token is found, its type is returned, and *token_data and *token_size
// are set to the occurrence of the token in the input string.
//
// If the input contains no more tokens (i.e. the string contains whitespace
// characters only) then T_NONE is returned, *token_data points to the end of
// the string, and *token_size is zero.
//
// If the next non-whitespace characters don't make up a valid token, T_NONE is
// returned, *token_data points to the first problematic character in the input,
// and *token_size is nonzero.
//
// This function performs context-free tokenization. It wil never return tokens
// of type T_OperatorName, T_FunctionName, T_NodeType or T_AxisName. Instead,
// all identifiers are returned as T_NameTest, and an asterisk is returned as
// T_Multiply. Use DisambiguateToken() or DisambiguateTokens() below to perform
// context-sensitive disambiguation of tokens.
TokenType ScanToken(const char* data, size_t size,
                    const char** token_data, size_t* token_size);

// Returns the disambiguated token type for token `current`, based on the
// types of the surrounding tokens `previous` and `next` (either of which may
// be T_None, if the current token is the first or last token in sequence,
// respectively).
//
// This function does not distinguish between function names and node types.
// Both are returned as T_FunctionName.
TokenType DisambiguateToken(TokenType previous,
                            TokenType current,
                            TokenType next);

// Tokenizes  the `input` string, disambiguates tokens, and writes the result
// to *tokens. If the whole input can be succesfully tokenized, this function
// returns input.size(). Otherwise, it returns a value less than input.size():
// the index where the scanning error occurred.
size_t Tokenize(const std::string& input,
                std::function<bool(const std::string&)> is_node_type,
                std::vector<std::pair<TokenType, std::string>> *tokens);

inline size_t Tokenize(const std::string& input,
    std::vector<std::pair<TokenType, std::string>> *tokens) {
  return Tokenize(
      input,
      [](const std::string& s) {
        return ParseOperatorName(s) != O_None;
      },
      tokens);
}

}  // namespace xpath

#endif /* ndef XPATH_TOKENIZER_INCLUDED */
