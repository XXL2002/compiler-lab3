#include "front/lexical.h"

#include <map>
#include <cassert>
#include <string>

#define TODO assert(0 && "todo")

// #define DEBUG_DFA
// #define DEBUG_SCANNER

std::string frontend::toString(State s)
{
    switch (s)
    {
    case State::Empty:
        return "Empty";
    case State::Ident:
        return "Ident";
    case State::IntLiteral:
        return "IntLiteral";
    case State::FloatLiteral:
        return "FloatLiteral";
    case State::op:
        return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}

std::set<std::string> frontend::keywords = {
    "const", "int", "float", "if", "else", "while", "continue", "break", "return", "void"};

frontend::DFA::DFA() : cur_state(frontend::State::Empty), cur_str() {}

frontend::DFA::~DFA() {}

bool isoperator(char c)
{ // 判断是否是符号
    if (c == '+' || c == '-' || c == '*' || c == '/' ||
        c == '(' || c == ')' || c == '{' || c == '}' ||
        c == '[' || c == ']' || c == ',' || // c == '.' ||
        c == '%' || c == '<' || c == '>' || c == ':' ||
        c == '=' || c == ';' || c == '[' || c == '!' ||
        c == '|' || c == '&')
    {
        return true;
    }
    else
    {
        return false;
    }
}

frontend::TokenType get_Ident_type(std::string s)
{ // 获取标识符的类型
    if (frontend::keywords.count(s))
    {
        if (s == "const")
        {
            return frontend::TokenType::CONSTTK;
        }
        else if (s == "void")
        {
            return frontend::TokenType::VOIDTK;
        }
        else if (s == "float")
        {
            return frontend::TokenType::FLOATTK;
        }
        else if (s == "if")
        {
            return frontend::TokenType::IFTK;
        }
        else if (s == "else")
        {
            return frontend::TokenType::ELSETK;
        }
        else if (s == "while")
        {
            return frontend::TokenType::WHILETK;
        }
        else if (s == "continue")
        {
            return frontend::TokenType::CONTINUETK;
        }
        else if (s == "break")
        {
            return frontend::TokenType::BREAKTK;
        }
        else if (s == "return")
        {
            return frontend::TokenType::RETURNTK;
        }
        else // if (s == "int")
        {
            return frontend::TokenType::INTTK;
        }
    }
    else
    {
        return frontend::TokenType::IDENFR;
    }
}

frontend::TokenType get_op_type(std::string s)
{ // 获取符号的类型
    if (s == "+")
    {
        return frontend::TokenType::PLUS;
    }
    else if (s == "-")
    {
        return frontend::TokenType::MINU;
    }
    else if (s == "*")
    {
        return frontend::TokenType::MULT;
    }
    else if (s == "/")
    {
        return frontend::TokenType::DIV;
    }
    else if (s == "%")
    {
        return frontend::TokenType::MOD;
    }
    else if (s == "<")
    {
        return frontend::TokenType::LSS;
    }
    else if (s == ">")
    {
        return frontend::TokenType::GTR;
    }
    else if (s == ":")
    {
        return frontend::TokenType::COLON;
    }
    else if (s == "=")
    {
        return frontend::TokenType::ASSIGN;
    }
    else if (s == ";")
    {
        return frontend::TokenType::SEMICN;
    }
    else if (s == ",")
    {
        return frontend::TokenType::COMMA;
    }
    else if (s == "(")
    {
        return frontend::TokenType::LPARENT;
    }
    else if (s == ")")
    {
        return frontend::TokenType::RPARENT;
    }
    else if (s == "[")
    {
        return frontend::TokenType::LBRACK;
    }
    else if (s == "]")
    {
        return frontend::TokenType::RBRACK;
    }
    else if (s == "{")
    {
        return frontend::TokenType::LBRACE;
    }
    else if (s == "}")
    {
        return frontend::TokenType::RBRACE;
    }
    else if (s == "!")
    {
        return frontend::TokenType::NOT;
    }
    else if (s == "<=")
    {
        return frontend::TokenType::LEQ;
    }
    else if (s == ">=")
    {
        return frontend::TokenType::GEQ;
    }
    else if (s == "==")
    {
        return frontend::TokenType::EQL;
    }
    else if (s == "!=")
    {
        return frontend::TokenType::NEQ;
    }
    else if (s == "&&")
    {
        return frontend::TokenType::AND;
    }
    else // if (s == "||")
    {
        return frontend::TokenType::OR;
    }
}

bool frontend::DFA::next(char input, Token &buf)
{ // DFA核心
#ifdef DEBUG_DFA
#include <iostream>
    std::cout << "in state [" << toString(cur_state) << "], input = \'" << input << "\', str = " << cur_str << "\t";
#endif
    //---------------------------------------------------------------
    if (cur_state == State::Empty)
    { // Empty状态的状态转移过程
        if ((int(input) >= int('0') && int(input) <= int('9')))
        { // 转移至Int状态
            cur_str += input;
            cur_state = State::IntLiteral;
            return false;
        }
        else if (input == '.')
        { // 转移至Float状态
            cur_str += input;
            cur_state = State::FloatLiteral;
            return false;
        }
        else if (isoperator(input))
        { // 转移至OP状态
            cur_str += input;
            cur_state = State::op;
            return false;
        }
        else if ((int(input) >= int('a') && int(input) <= int('z')) || (int(input) >= int('A') && int(input) <= int('Z')) || input == '_')
        { // 转移至Ident状态
            cur_str += input;
            cur_state = State::Ident;
            return false;
        }
        else
        { // 保持Empty状态
            return false;
        }
    }
    //-------------------------------------------------------------------
    else if (cur_state == State::FloatLiteral)
    { // 浮点数状态的状态转移过程
        if ((int(input) >= int('0') && int(input) <= int('9')))
        { // 保持Float状态
            cur_str += input;
            return false;
        }
        else if (isoperator(input))
        { // 转移至OP状态
            buf.type = TokenType::FLOATLTR;
            buf.value = cur_str;

            cur_str = input;
            cur_state = State::op;
            return true;
        }
        else if ((int(input) >= int('a') && int(input) <= int('z')) || (int(input) >= int('A') && int(input) <= int('Z')) || input == '_')
        { // 转移至Ident状态
            buf.type = TokenType::FLOATLTR;
            buf.value = cur_str;

            cur_str = input;
            cur_state = State::Ident;
            return true;
        }
        else
        { // 转移至Empty状态
            buf.type = TokenType::FLOATLTR;
            buf.value = cur_str;

            cur_str = "";
            cur_state = State::Empty;
            return true;
        }
    }
    //-----------------------------------------------------------------------
    else if (cur_state == State::IntLiteral)
    { // 整数状态的状态转移过程
        if ((int(input) >= int('0') && int(input) <= int('9')) || (cur_str == "0" && (input == 'b' || input == 'B' || input == 'x' || input == 'X')))
        { // 保持Int状态
            cur_str += input;
            return false;
        }
        else if (input == '.')
        { // 转移至Float状态
            cur_str += input;
            cur_state = State::FloatLiteral;
            return false;
        }
        else if (isoperator(input))
        { // 转移至OP状态
            buf.type = TokenType::INTLTR;
            buf.value = cur_str;

            cur_str = input;
            cur_state = State::op;
            return true;
        }
        else if ((int(input) >= int('a') && int(input) <= int('f')) || (int(input) >= int('A') && int(input) <= int('F')) && cur_str.length() > 1 && (cur_str[1] == 'X' || cur_str[1] == 'x'))
        {
            cur_str += input;
            return false;
        }
        else if ((int(input) >= int('a') && int(input) <= int('z')) || (int(input) >= int('A') && int(input) <= int('Z')) || input == '_')
        { // 转移至Ident状态
            buf.type = TokenType::INTLTR;
            buf.value = cur_str;

            cur_str = input;
            cur_state = State::Ident;
            return true;
        }
        else
        { // 转移至Empty状态
            buf.type = TokenType::INTLTR;
            buf.value = cur_str;

            cur_str = "";
            cur_state = State::Empty;
            return true;
        }
    }
    //---------------------------------------------------------------------
    else if (cur_state == State::Ident)
    { // 关键词/标识符状态的状态转换过程
        if (isoperator(input))
        { // 转移至OP状态
            buf.type = get_Ident_type(cur_str);
            buf.value = cur_str;

            cur_str = input;
            cur_state = State::op;
            return true;
        }
        else if (input == '.')
        { // 转移至Float状态
            buf.type = get_Ident_type(cur_str);
            buf.value = cur_str;

            cur_str = input;
            cur_state = State::FloatLiteral;
            return true;
        }
        else if ((int(input) >= int('a') && int(input) <= int('z')) ||
                 (int(input) >= int('A') && int(input) <= int('Z')) ||
                 (int(input) >= int('0') && int(input) <= int('9')) ||
                 input == '_')
        { // 保持Ident状态
            cur_str += input;
            return false;
        }
        else
        { // 转移至Empty状态
            buf.type = get_Ident_type(cur_str);
            buf.value = cur_str;

            cur_str = "";
            cur_state = State::Empty;
            return true;
        }
    }
    //-------------------------------------------------------------------------
    else
    { // operators and '{', '[', '(', ',' ...的状态转移过程
        if ((int(input) >= int('0') && int(input) <= int('9')) || (cur_str == "0" && (input == 'b' || input == 'B' || input == 'x' || input == 'X')))
        { // 转移至Int状态
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;

            cur_str = input;
            cur_state = State::IntLiteral;
            return true;
        }
        else if (input == '.')
        { // 转移至Float状态
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;

            cur_str = input;
            cur_state = State::FloatLiteral;
            return true;
        }
        else if (isoperator(input))
        { // 保持op状态，判断是否输出当前op
            if (((cur_str == "<" || cur_str == ">" || cur_str == "=" || cur_str == "!") && input == '=') ||
                (cur_str == "&" && input == '&') ||
                (cur_str == "|" && input == '|'))
            {
                // 维持
                cur_str += input;
                return false;
            }
            else
            {
                // 输出
                buf.type = get_op_type(cur_str);
                buf.value = cur_str;

                cur_str = input;
                return true;
            }
        }
        else if ((int(input) >= int('a') && int(input) <= int('z')) || (int(input) >= int('A') && int(input) <= int('Z')) || input == '_')
        { // 转移至Ident状态
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;

            cur_str = input;
            cur_state = State::Ident;
            return true;
        }
        else
        { // 转移至Empty状态
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;

            cur_str = "";
            cur_state = State::Empty;
            return true;
        }
    }

#ifdef DEBUG_DFA
    std::cout << "next state is [" << toString(cur_state) << "], next str = " << cur_str << "\t, ret = " << ret << std::endl;
#endif
}

void frontend::DFA::reset()
{
    cur_state = State::Empty;
    cur_str = "";
}

frontend::Scanner::Scanner(std::string filename) : fin(filename)
{
    if (!fin.is_open())
    {
        assert(0 && "in Scanner constructor, input file cannot open");
    }
}

frontend::Scanner::~Scanner()
{
    fin.close();
}

std::string preproccess(std::string s, bool &flag)
{ // 代码预处理，用于过滤代码中的注释内容(包括单行、多行注释)
    s += "\n";
    std::string output = "";
    for (int i = 0; i < s.length() - 1; i++)
    {
        if (flag)
        { // 处于多行注释中
            if (s[i] == '*' && s[i + 1] == '/')
            {
                // 多行注释结束
                flag = false;
                i++;
            }
        }
        else if (s[i] == '/' && s[i + 1] == '/')
        { // 未处于多行注释，但是遇到单行注释
            return output;
        }
        else if (s[i] == '/' && s[i + 1] == '*')
        { // 进入多行注释
            flag = true;
            i++;
        }
        else
        {
            output += s[i];
        }
    }
    output += "\n";
    return output;
}

std::vector<frontend::Token> frontend::Scanner::run()
{
    std::vector<Token> ret;
    Token tk;
    DFA dfa;
    dfa.reset();
    std::string s = "";
    bool flag = false; // 用于标记是否处于多行注释状态
    while (std::getline(fin, s))
    { // 按行处理
        s = preproccess(s, flag);
        for (auto c : s)
        {
            if (dfa.next(c, tk))
            {
                ret.push_back(tk);
#ifdef DEBUG_SCANNER
#include <iostream>
                std::cout << "token: " << toString(tk.type) << "\t" << tk.value << std::endl;
#endif
            }
        }
    }
    return ret;
}