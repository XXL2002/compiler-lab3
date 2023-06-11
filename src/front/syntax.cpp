#include "front/syntax.h"
#include <iostream>
// #define NDEBUG //用于禁用assert
#include <cassert>

using frontend::Parser;

// #define DEBUG_PARSER

#define ERROR                                                   \
    std::cout << "parse " + toString(root->type) + " ERROR!\n"; \
    assert(0 && "ERROR");
#define CUR_TOKEN token_stream[index]
#define TODO assert(0 && "todo")
#define CUR_TOKEN_IS(tk_type) (token_stream[index].type == tk_type)
#define PARSE_TOKEN(tk_type) root->children.push_back(parseTerm(root, tk_type))
#define PARSE(name, type)       \
    auto name = new type(root); \
    assert(parse##type(name));  \
    root->children.push_back(name);

Parser::Parser(const std::vector<frontend::Token> &tokens) : index(0), token_stream(tokens)
{
}

Parser::~Parser() {}

frontend::CompUnit *Parser::get_abstract_syntax_tree()
{
    // 开始构建抽象语法树
    // 注意为指针new空间，否则指针指向的节点可能会被析构，导致成为野指针
    frontend::CompUnit *root = new frontend::CompUnit(); // 生成根节点指针
    // 解析函数
    parseCompUnit(root);
    return root;
}

frontend::Term *Parser::parseTerm(frontend::AstNode *parent, frontend::TokenType expected)
{
    // 终结符解析函数
    log(parent);
    if (CUR_TOKEN_IS(expected))
    {
        // 匹配
        Term *node = new Term(CUR_TOKEN, parent);
        index++;
        return node;
    }
    else
    {
        // 不匹配
        assert(0 && "Term not match!");
    }
}

bool Parser::parseCompUnit(frontend::CompUnit *root)
{
    // CompUnit解析函数
    // CompUnit -> (Decl | FuncDef) [CompUnit]
    // First3(Decl)={const + (int | float) + Ident , (int | float) + Ident + ...}
    // First3(FuncDef)={(void | int | float) + Ident + '('}
    log(root);
    if ((CUR_TOKEN_IS(TokenType::VOIDTK) || CUR_TOKEN_IS(TokenType::INTTK) || CUR_TOKEN_IS(TokenType::FLOATTK)) && token_stream[index + 1].type == TokenType::IDENFR && token_stream[index + 2].type == TokenType::LPARENT)
    {
        PARSE(funcdef, FuncDef);
    }
    else
    {
        PARSE(decl, Decl);
    }

    // if (index == token_stream.size() - 1)
    if (CUR_TOKEN_IS(TokenType::VOIDTK) || CUR_TOKEN_IS(TokenType::INTTK) || CUR_TOKEN_IS(TokenType::FLOATTK) || CUR_TOKEN_IS(TokenType::CONSTTK))
    {
        PARSE(compunit, CompUnit);
    }
    return true;
}

bool Parser::parseDecl(frontend::Decl *root)
{
    // Decl解析函数
    // Decl -> ConstDecl | VarDecl
    log(root);
    if (CUR_TOKEN_IS(TokenType::CONSTTK))
    {
        PARSE(constdecl, ConstDecl);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::INTTK) || CUR_TOKEN_IS(TokenType::FLOATTK))
    {
        PARSE(vardecl, VarDecl);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseConstDecl(frontend::ConstDecl *root)
{
    // ConstDecl解析函数
    // ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    /*
        ConstDecl.t
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::CONSTTK))
    {
        PARSE_TOKEN(TokenType::CONSTTK);
        PARSE(btype, BType);
        PARSE(constdef, ConstDef);
        while (CUR_TOKEN_IS(TokenType::COMMA))
        {
            PARSE_TOKEN(TokenType::COMMA);
            PARSE(constdef, ConstDef);
        }
        PARSE_TOKEN(TokenType::SEMICN);
        return true;
    }
    else
    {
        ERROR;
    }
}
bool Parser::parseBType(frontend::BType *root)
{
    // Btype解析函数
    // BType -> 'int' | 'float'
    /*
        BType.t
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::INTTK))
    {
        PARSE_TOKEN(TokenType::INTTK);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::FLOATTK))
    {
        PARSE_TOKEN(TokenType::FLOATTK);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseConstDef(frontend::ConstDef *root)
{
    // ConstDef解析函数
    // ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    /*
        ConstDef.arr_name
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::IDENFR))
    {
        PARSE_TOKEN(TokenType::IDENFR);
        while (CUR_TOKEN_IS(TokenType::LBRACK))
        {
            PARSE_TOKEN(TokenType::LBRACK);
            PARSE(constexp, ConstExp);
            PARSE_TOKEN(TokenType::RBRACK);
        }
        PARSE_TOKEN(TokenType::ASSIGN);
        PARSE(constinitval, ConstInitVal);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseConstInitVal(frontend::ConstInitVal *root)
{
    // ConstInitVal解析函数
    // ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    /*
        ConstInitVal.v
        ConstInitVal.t
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::LBRACE))
    {
        PARSE_TOKEN(TokenType::LBRACE);
        if (!CUR_TOKEN_IS(TokenType::RBRACE))
        {
            PARSE(constinitval, ConstInitVal);
            while (CUR_TOKEN_IS(TokenType::COMMA))
            {
                PARSE_TOKEN(TokenType::COMMA);
                PARSE(constinitval, ConstInitVal);
            }
        }
        PARSE_TOKEN(TokenType::RBRACE);
        return true;
    }
    else
    { // ERROR下放
        PARSE(constexp, ConstExp);
        return true;
    }
}

bool Parser::parseVarDecl(frontend::VarDecl *root)
{
    // VarDecl解析函数
    // VarDecl -> BType VarDef { ',' VarDef } ';'
    /*
        VarDecl.t
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::INTTK) || CUR_TOKEN_IS(TokenType::FLOATTK))
    {
        PARSE(btype, BType);
        PARSE(vardef, VarDef);
        while (CUR_TOKEN_IS(TokenType::COMMA))
        {
            PARSE_TOKEN(TokenType::COMMA);
            PARSE(vardef, VarDef);
        }
        PARSE_TOKEN(TokenType::SEMICN);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseVarDef(frontend::VarDef *root)
{
    // VarDef解析函数
    // VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
    /*
        VarDef.arr_name
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::IDENFR))
    {
        PARSE_TOKEN(TokenType::IDENFR);
        while (CUR_TOKEN_IS(TokenType::LBRACK))
        {
            PARSE_TOKEN(TokenType::LBRACK);
            PARSE(constexp, ConstExp);
            PARSE_TOKEN(TokenType::RBRACK);
        }
        if (CUR_TOKEN_IS(TokenType::ASSIGN))
        {
            PARSE_TOKEN(TokenType::ASSIGN);
            PARSE(initval, InitVal);
        }
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseInitVal(frontend::InitVal *root)
{
    // Initval解析函数
    // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    /*
        InitVal.is_computable
        InitVal.v
        InitVal.t
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::LBRACE))
    {
        PARSE_TOKEN(TokenType::LBRACE);
        if (!CUR_TOKEN_IS(TokenType::RBRACE))
        {
            PARSE(initval, InitVal);
            while (CUR_TOKEN_IS(TokenType::COMMA))
            {
                PARSE_TOKEN(TokenType::COMMA);
                PARSE(initval, InitVal);
            }
        }
        PARSE_TOKEN(TokenType::RBRACE);
        return true;
    }
    else
    { // ERROR下放
        PARSE(exp, Exp);
        return true;
    }
}

bool Parser::parseFuncDef(frontend::FuncDef *root)
{
    // FuncDef解析函数
    // FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
    /*
        FuncDef.t
        FuncDef.n
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::VOIDTK) || CUR_TOKEN_IS(TokenType::INTTK) || CUR_TOKEN_IS(TokenType::FLOATTK))
    {
        PARSE(functype, FuncType);
        PARSE_TOKEN(TokenType::IDENFR);
        PARSE_TOKEN(TokenType::LPARENT);
        if (!CUR_TOKEN_IS(TokenType::RPARENT))
        {
            PARSE(funcfparams, FuncFParams);
        }
        PARSE_TOKEN(TokenType::RPARENT);
        PARSE(block, Block);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseFuncType(frontend::FuncType *root)
{
    // FuncType解析函数
    // FuncType -> 'void' | 'int' | 'float'
    log(root);
    if (CUR_TOKEN_IS(TokenType::VOIDTK))
    {
        PARSE_TOKEN(TokenType::VOIDTK);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::INTTK))
    {
        PARSE_TOKEN(TokenType::INTTK);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::FLOATTK))
    {
        PARSE_TOKEN(TokenType::FLOATTK);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseFuncFParam(frontend::FuncFParam *root)
{
    // FuncFParm解析函数
    // FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
    log(root);
    if (CUR_TOKEN_IS(TokenType::INTTK) || CUR_TOKEN_IS(TokenType::FLOATTK))
    {
        PARSE(btype, BType);
        PARSE_TOKEN(TokenType::IDENFR);
        if (CUR_TOKEN_IS(TokenType::LBRACK))
        {
            PARSE_TOKEN(TokenType::LBRACK);
            PARSE_TOKEN(TokenType::RBRACK);
            while (CUR_TOKEN_IS(TokenType::LBRACK))
            {
                PARSE_TOKEN(TokenType::LBRACK);
                PARSE(exp, Exp);
                PARSE_TOKEN(TokenType::RBRACK);
            }
        }
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseFuncFParams(frontend::FuncFParams *root)
{
    // FuncFParams解析函数
    // FuncFParams -> FuncFParam { ',' FuncFParam }
    log(root);
    if (CUR_TOKEN_IS(TokenType::INTTK) || CUR_TOKEN_IS(TokenType::FLOATTK))
    {
        PARSE(funcfparam, FuncFParam);
        while (CUR_TOKEN_IS(TokenType::COMMA))
        {
            PARSE_TOKEN(TokenType::COMMA);
            PARSE(funcfparam, FuncFParam);
        }
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseBlock(frontend::Block *root)
{
    // Block解析函数
    // Block -> '{' { BlockItem } '}'
    log(root);
    if (CUR_TOKEN_IS(TokenType::LBRACE))
    {
        PARSE_TOKEN(TokenType::LBRACE);
        while (!CUR_TOKEN_IS(TokenType::RBRACE))
        {
            PARSE(blockitem, BlockItem);
        }
        PARSE_TOKEN(TokenType::RBRACE);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseBlockItem(frontend::BlockItem *root)
{
    // BlockItem解析函数
    // BlockItem -> Decl | Stmt
    log(root);
    if (CUR_TOKEN_IS(TokenType::CONSTTK) || CUR_TOKEN_IS(TokenType::INTTK) || CUR_TOKEN_IS(TokenType::FLOATTK))
    {
        PARSE(decl, Decl);
        return true;
    }
    else
    {
        // ERROR下放
        PARSE(stmt, Stmt);
        return true;
    }
}

bool Parser::parseStmt(frontend::Stmt *root)
{
    // Stmt解析函数
    // Stmt -> LVal '=' Exp ';' | Block | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] | 'while' '(' Cond ')' Stmt | 'break' ';' | 'continue' ';' | 'return' [Exp] ';' | [Exp] ';'
    log(root);
    if (CUR_TOKEN_IS(TokenType::IDENFR) && (token_stream[index + 1].type == TokenType::ASSIGN || token_stream[index + 1].type == TokenType::LBRACK))
    {
        PARSE(lval, LVal);
        PARSE_TOKEN(TokenType::ASSIGN);
        PARSE(exp, Exp);
        PARSE_TOKEN(TokenType::SEMICN);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::LBRACE))
    {
        PARSE(block, Block);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::IFTK))
    {
        PARSE_TOKEN(TokenType::IFTK);
        PARSE_TOKEN(TokenType::LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(TokenType::RPARENT);
        PARSE(stmt, Stmt);
        if (CUR_TOKEN_IS(TokenType::ELSETK))
        {
            PARSE_TOKEN(TokenType::ELSETK);
            PARSE(stmt, Stmt);
        }
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::WHILETK))
    {
        PARSE_TOKEN(TokenType::WHILETK);
        PARSE_TOKEN(TokenType::LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(TokenType::RPARENT);
        PARSE(stmt, Stmt);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::BREAKTK))
    {
        PARSE_TOKEN(TokenType::BREAKTK);
        PARSE_TOKEN(TokenType::SEMICN);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::CONTINUETK))
    {
        PARSE_TOKEN(TokenType::CONTINUETK);
        PARSE_TOKEN(TokenType::SEMICN);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::RETURNTK))
    {
        PARSE_TOKEN(TokenType::RETURNTK);
        if (!CUR_TOKEN_IS(TokenType::SEMICN))
        {
            PARSE(exp, Exp);
        }
        PARSE_TOKEN(TokenType::SEMICN);
        return true;
    }
    else
    {
        // ERROR下放
        if (!CUR_TOKEN_IS(TokenType::SEMICN))
        {
            PARSE(exp, Exp);
        }
        PARSE_TOKEN(TokenType::SEMICN);
        return true;
    }
}

bool Parser::parseExp(frontend::Exp *root)
{
    // Exp解析函数
    // Exp -> AddExp
    /*
        Exp.is_computable
        Exp.v
        Exp.t
    */
    log(root);
    // ERROR下放
    PARSE(addexp, AddExp);
    return true;
}

bool Parser::parseCond(frontend::Cond *root)
{
    // Cond解析函数
    // Cond -> LOrExp
    /*
        Cond.is_computable
        Cond.v
        Cond.t
    */
    log(root);
    // ERROR下放
    PARSE(lorexp, LOrExp);
    return true;
}

bool Parser::parseLVal(frontend::LVal *root)
{
    // Lval解析函数
    // LVal -> Ident {'[' Exp ']'}
    /*
        LVal.is_computable
        LVal.v
        LVal.t
        LVal.i
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::IDENFR))
    {
        PARSE_TOKEN(TokenType::IDENFR);
        while (CUR_TOKEN_IS(TokenType::LBRACK))
        {
            PARSE_TOKEN(TokenType::LBRACK);
            PARSE(exp, Exp);
            PARSE_TOKEN(TokenType::RBRACK);
        }
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseNumber(frontend::Number *root)
{
    // Number解析函数
    // Number -> IntConst | floatConst
    log(root);
    if (CUR_TOKEN_IS(TokenType::INTLTR))
    {
        PARSE_TOKEN(TokenType::INTLTR);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::FLOATLTR))
    {
        PARSE_TOKEN(TokenType::FLOATLTR);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parsePrimaryExp(frontend::PrimaryExp *root)
{
    // PrimaryExp解析函数
    // PrimaryExp -> '(' Exp ')' | LVal | Number
    /*
        PrimaryExp.is_computable
        PrimaryExp.v
        PrimaryExp.t
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::LPARENT))
    {
        PARSE_TOKEN(TokenType::LPARENT);
        PARSE(exp, Exp);
        PARSE_TOKEN(TokenType::RPARENT);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::IDENFR))
    {
        PARSE(lval, LVal);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::INTLTR) || CUR_TOKEN_IS(TokenType::FLOATLTR))
    {
        PARSE(number, Number);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseUnaryExp(frontend::UnaryExp *root)
{
    // UnaryExp解析函数
    // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    // FIRST(PrimaryExp)={'(',Ident,intConst,floatConst} 其中Ident有重合
    // First2(PrimaryExp)对于产生Ident的first值为 Ident {'['}
    /*
        UnaryExp.is_computable
        UnaryExp.v
        UnaryExp.t
    */
    log(root);
    if (CUR_TOKEN_IS(TokenType::LPARENT))
    {
        PARSE(primaryexp, PrimaryExp);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::INTLTR) || CUR_TOKEN_IS(TokenType::FLOATLTR))
    {
        PARSE(primaryexp, PrimaryExp);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::IDENFR) && token_stream[index + 1].type != TokenType::LPARENT)
    {
        PARSE(primaryexp, PrimaryExp);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::IDENFR))
    {
        PARSE_TOKEN(TokenType::IDENFR);
        PARSE_TOKEN(TokenType::LPARENT);
        if (!CUR_TOKEN_IS(TokenType::RPARENT))
        {
            PARSE(funcrparams, FuncRParams);
        }
        PARSE_TOKEN(TokenType::RPARENT);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::PLUS) || CUR_TOKEN_IS(TokenType::MINU) || CUR_TOKEN_IS(TokenType::NOT))
    {
        PARSE(unaryop, UnaryOp);
        PARSE(unaryexp, UnaryExp);
        return true;
    }
    else
    {
        std::cout << toString(CUR_TOKEN.type) << std::endl;
        ERROR;
    }
}

bool Parser::parseUnaryOp(frontend::UnaryOp *root)
{
    // UnaryOp解析函数
    // UnaryOp -> '+' | '-' | '!'
    log(root);
    if (CUR_TOKEN_IS(TokenType::PLUS))
    {
        PARSE_TOKEN(TokenType::PLUS);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::MINU))
    {
        PARSE_TOKEN(TokenType::MINU);
        return true;
    }
    else if (CUR_TOKEN_IS(TokenType::NOT))
    {
        PARSE_TOKEN(TokenType::NOT);
        return true;
    }
    else
    {
        ERROR;
    }
}

bool Parser::parseFuncRParams(frontend::FuncRParams *root)
{
    // FuncRParams解析函数
    // FuncRParams -> Exp { ',' Exp }
    log(root);
    // ERROR下放
    PARSE(exp, Exp);
    while (CUR_TOKEN_IS(TokenType::COMMA))
    {
        PARSE_TOKEN(TokenType::COMMA);
        PARSE(exp, Exp);
    }
    return true;
}

bool Parser::parseMulExp(frontend::MulExp *root)
{
    // MulExp解析函数
    // MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
    /*
        MulExp.is_computable
        MulExp.v
        MulExp.t
    */
    log(root);
    // ERROR下放
    PARSE(unaryexp, UnaryExp);
    while (CUR_TOKEN_IS(TokenType::MULT) || CUR_TOKEN_IS(TokenType::DIV) || CUR_TOKEN_IS(TokenType::MOD))
    {
        PARSE_TOKEN(CUR_TOKEN.type);
        PARSE(unaryexp, UnaryExp);
    }
    return true;
}

bool Parser::parseAddExp(frontend::AddExp *root)
{
    // AddExp解析函数
    // AddExp -> MulExp { ('+' | '-') MulExp }
    /*
        AddExp.is_computable
        AddExp.v
        AddExp.t
    */
    log(root);
    // ERROR下放
    PARSE(mulexp, MulExp);
    while (CUR_TOKEN_IS(TokenType::PLUS) || CUR_TOKEN_IS(TokenType::MINU))
    {
        PARSE_TOKEN(CUR_TOKEN.type);
        PARSE(mulexp, MulExp);
    }
    return true;
}

bool Parser::parseRelExp(frontend::RelExp *root)
{
    // RelExp解析函数
    // RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
    /*
        RelExp.is_computable
        RelExp.v
        RelExp.t
    */
    log(root);
    // ERROR下放
    PARSE(addexp, AddExp);
    while (CUR_TOKEN_IS(TokenType::LSS) || CUR_TOKEN_IS(TokenType::GTR) || CUR_TOKEN_IS(TokenType::LEQ) || CUR_TOKEN_IS(TokenType::GEQ))
    {
        PARSE_TOKEN(CUR_TOKEN.type);
        PARSE(addexp, AddExp);
    }
    return true;
}

bool Parser::parseEqExp(frontend::EqExp *root)
{
    // EqExp解析函数
    // EqExp -> RelExp { ('==' | '!=') RelExp }
    /*
        EqExp.is_computable
        EqExp.v
        EqExp.t
    */
    log(root);
    // ERROR下放
    PARSE(relexp, RelExp);
    while (CUR_TOKEN_IS(TokenType::EQL) || CUR_TOKEN_IS(TokenType::NEQ))
    {
        PARSE_TOKEN(CUR_TOKEN.type);
        PARSE(relexp, RelExp);
    }
    return true;
}

bool Parser::parseLAndExp(frontend::LAndExp *root)
{
    // LAndExp解析函数
    // LAndExp -> EqExp [ '&&' LAndExp ]
    /*
        LAndExp.is_computable
        LAndExp.v
        LAndExp.t
    */
    log(root);
    // ERROR下放
    PARSE(eqexp, EqExp);
    if (CUR_TOKEN_IS(TokenType::AND))
    {
        PARSE_TOKEN(TokenType::AND);
        PARSE(landexp, LAndExp);
    }
    return true;
}

bool Parser::parseLOrExp(frontend::LOrExp *root)
{
    // LOrExp解析函数
    // LOrExp -> LAndExp [ '||' LOrExp ]
    /*
        LOrExp.is_computable
        LOrExp.v
        LOrExp.t
    */
    log(root);
    // ERROR下放
    PARSE(landexp, LAndExp);
    if (CUR_TOKEN_IS(TokenType::OR))
    {
        PARSE_TOKEN(TokenType::OR);
        PARSE(lorexp, LOrExp);
    }
    return true;
}

bool Parser::parseConstExp(frontend::ConstExp *root)
{
    // ConstExp解析函数
    // ConstExp -> AddExp
    /*
        ConstExp.is_computable: true
        ConstExp.v
        ConstExp.t
    */
    log(root);
    // ERROR下放
    PARSE(addexp, AddExp);
    return true;
}

void Parser::log(AstNode *node)
{
#ifdef DEBUG_PARSER
    std::cout << "in parse" << toString(node->type) << ", cur_token_type::" << toString(token_stream[index].type) << ", token_val::" << token_stream[index].value << '\n';
#endif
}
