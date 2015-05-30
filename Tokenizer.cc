// XPath tokenizer.
// Based on: http://www.w3.org/TR/xpath/#exprlex

#include "Tokenizer.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

#define RETURN_TOKEN(Size, Type) do { *token_size = Size; return Type; } while(0)
#define RETURN_ERROR() RETURN_TOKEN(1, T_None)

namespace xpath {
  
namespace {

bool IsIdentifierStartChar(char c) {
  // TODO: support Unicode chars too. Requires UTF-8 decoding?
  // Note: unlike XPath we don't support ':' in identifiers.
  return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool IsIdentifierChar(char c) {
  // TODO: support Unicode chars too. Requires UTF-8 decoding?
  return IsIdentifierStartChar(c) || c == '-' || c == '.' || (c >= '0' && c <= '9');
}

// Returns the length of the longest prefix of the string described by `data`
// and `size` that is a valid identifier. If the input does not start with an
// identifier, returns 0 instead.
size_t ScanIdentifier(const char* data, size_t size) {
  if (size == 0 || !IsIdentifierStartChar(data[0])) return 0;
  size_t n = 1;
  while (n != size && IsIdentifierChar(data[n])) ++n;
  return n;
}

}  // namespace

template<int N> bool Equals(const char (&buf)[N], const char* data, size_t size) {
  return N > 0 && size == N - 1 && memcmp(data, &buf[0], N - 1) == 0;
}

NodeType ParseNodeType(const char* data, size_t size) {
  if (Equals("comment", data, size)) return N_Comment;
  if (Equals("text", data, size)) return N_Text;
  if (Equals("processing-instruction", data, size)) return N_ProcessingInstruction;
  if (Equals("node", data, size)) return N_Node;
  return N_None;
}

OperatorName ParseOperatorName(const char* data, size_t size) {
  if (Equals("and", data, size)) return O_And;
  if (Equals("or", data, size)) return O_Or;
  if (Equals("mod", data, size)) return O_Mod;
  if (Equals("div", data, size)) return O_Div;
  return O_None;
}

AxisName ParseAxisName(const char* data, size_t size) {
  if (Equals("ancestor", data, size)) return A_Ancestor;
  if (Equals("ancestor-or-self", data, size)) return A_AncestorOrSelf;
  if (Equals("attribute", data, size)) return A_Attribute;
  if (Equals("child", data, size)) return A_Child;
  if (Equals("descendant", data, size)) return A_Descendant;
  if (Equals("descendant-or-self", data, size)) return A_DescendantOrSelf;
  if (Equals("following", data, size)) return A_Following;
  if (Equals("following-sibling", data, size)) return A_FollowingSibling;
  if (Equals("namespace", data, size)) return A_Namespace;
  if (Equals("parent", data, size)) return A_Parent;
  if (Equals("preceding", data, size)) return A_Preceding;
  if (Equals("preceding-sibling", data, size)) return A_PrecedingSibling;
  if (Equals("self", data, size)) return A_Self;
  return A_None;
}

TokenType ScanToken(const char* data, size_t size,
                    const char** token_data, size_t* token_size) {
  // Skip leading whitespace                       
  while (size != 0 && isspace(*data)) ++data, --size;
  *token_data = data;
  if (size == 0) RETURN_TOKEN(0, T_None);  // End of input.

  switch (data[0]) {
    case '(':
      RETURN_TOKEN(1, T_LeftParen);
    case ')':
      RETURN_TOKEN(1, T_RightParen);
    case '[':
      RETURN_TOKEN(1, T_LeftBracket);
    case ']':
      RETURN_TOKEN(1, T_RightBracket);
    case '.':
      if (size > 1 && data[1] == '.') RETURN_TOKEN(2, T_DoubleDot);
      if (size > 1 && isdigit(data[1])) {
        // Recognize a number of the form: '.' Digits
        size_t n = 2;
        while (n != size && isdigit(data[n])) ++n;
        RETURN_TOKEN(n, T_Number);
      }
      RETURN_TOKEN(1, T_Dot);
    case '@':
      RETURN_TOKEN(1, T_At);
    case ',':
      RETURN_TOKEN(1, T_Comma);
    case ':':
      if (size > 1 && data[1] == ':') RETURN_TOKEN(2, T_DoubleColon);
      break;
    case '\'':
    case '\"': {
      // Recognize a literal. Seach for delimiter matching data[0].
      size_t n = 1;
      while (n != size && data[n] != data[0]) ++n;
      if (n == size) RETURN_ERROR();  // Matching delimiter not found.
      RETURN_TOKEN(n + 1, T_Literal);
    }
    case '/':
      if (size > 1 && data[1] == '/') RETURN_TOKEN(2, T_DoubleSlash);
      RETURN_TOKEN(1, T_Slash);
    case '|':
      RETURN_TOKEN(1, T_Pipe);
    case '+':
      RETURN_TOKEN(1, T_Plus);
    case '-':
      RETURN_TOKEN(1, T_Minus);
    case '=':
      RETURN_TOKEN(1, T_Equal);
    case '!':
      if (size > 1 && data[1] == '=') RETURN_TOKEN(2, T_NotEqual);
      RETURN_ERROR();
    case '<':
      if (size > 1 && data[1] == '=') RETURN_TOKEN(2, T_LessEqual);
      RETURN_TOKEN(1, T_LessThan);
    case '>':
      if (size > 1 && data[1] == '=') RETURN_TOKEN(2, T_GreaterEqual);
      RETURN_TOKEN(1, T_GreaterThan);
    case '*':
      RETURN_TOKEN(1, T_Multiply);
    case '$': {
      size_t n = ScanIdentifier(data + 1, size - 1);
      if (n > 0) RETURN_TOKEN(n + 1, T_VariableReference);
      RETURN_ERROR();  // Invalid variable reference.
    }
  }

  if (isdigit(data[0])) {
    // Recognize a number of the form: Digits ('.' Digits?)?
    size_t n = 1;
    while (n != size && isdigit(data[n])) ++n;
    if (n != size && data[n] == '.') {
      ++n;
      while (n != size && isdigit(data[n])) ++n;
    }
    RETURN_TOKEN(n, T_Number);
  }

  // Must be an identifier.
  size_t i = ScanIdentifier(data, size);
  if (i > 0) RETURN_TOKEN(i, T_NameTest);
  RETURN_ERROR();
}

bool IsOperator(TokenType type) {
  return (type >= T_Slash && type <= T_Multiply) || type == T_OperatorName;
}

TokenType DisambiguateToken(TokenType previous,
                            TokenType current,
                            TokenType next) {
  // If there is a preceding token and the preceding token is not one of @, ::,
  // (, [, , or an Operator, then a * must be recognized as a MultiplyOperator
  // and an NCName must be recognized as an OperatorName.
  if (previous != T_None && previous != T_At && previous != T_DoubleColon &&
      previous != T_LeftParen && previous != T_LeftBracket && previous != T_Comma &&
      !IsOperator(previous)) {
    if (current == T_Multiply) return T_Multiply;
    if (current == T_NameTest) return T_OperatorName;
  }
  // If the character following an NCName (possibly after intervening
  // ExprWhitespace) is (, then the token must be recognized as a NodeType or a
  // FunctionName.
  if (current == T_NameTest && next == T_LeftParen) return T_FunctionName;
  // If the two characters following an NCName (possibly after intervening
  // ExprWhitespace) are ::, then the token must be recognized as an AxisName.
  if (current == T_NameTest && next == T_DoubleColon) return T_AxisName;
  // Otherwise, the token must not be recognized as a MultiplyOperator, an
  // OperatorName, a NodeType, a FunctionName, or an AxisName.
  if (current == T_Multiply) return T_NameTest;
  return current;
}

size_t Tokenize(const std::string& input,
                std::function<bool(const std::string&)> is_node_type,
                std::vector<std::pair<TokenType, std::string>> *tokens_ptr) {
  auto& tokens = *tokens_ptr;
  tokens.clear();
  const char* data = input.data();
  const char* data_end = data + input.size();
  const char* token_data = nullptr;
  size_t token_size = 0;
  TokenType token_type;
  while ((token_type = ScanToken(data, data_end - data, &token_data, &token_size)) != T_None) {
    tokens.push_back({token_type, std::string(token_data, token_size)});
    data = token_data + token_size;
  }
  for (size_t i = 0; i < tokens.size(); ++i) {
    tokens[i].first = DisambiguateToken(
        i > 0 ? tokens[i - 1].first : T_None,
        tokens[i].first,
        i + 1 < tokens.size() ? tokens[i + 1].first : T_None);
    // Disambiguate function names and node types.
    if (tokens[i].first == T_FunctionName && is_node_type(tokens[i].second)) {
      tokens[i].first = T_NodeType;
    }
  }
  if (token_size != 0) {
    assert(token_data >= data && token_data < data_end);
    return token_data - input.data();
  }
  return input.size();
}

}  // namespace xpath
