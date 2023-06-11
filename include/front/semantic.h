/**
 * @file semantic.h
 * @author Yuntao Dai (d1581209858@live.com)
 * @brief
 * @version 0.1
 * @date 2023-01-06
 *
 * a Analyzer should
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ir/ir.h"
#include "front/abstract_syntax_tree.h"

#include <map>
#include <string>
#include <vector>
using std::map;
using std::string;
using std::vector;

namespace frontend
{

    // definition of symbol table entry
    // 符号表条目
    struct STE
    {
        ir::Operand operand;   // 记录符号的名字和类型(即ir::Operand操作数)
        vector<int> dimension; // 记录维度(数组中使用)
    };

    using map_str_ste = map<string, STE>;
    // definition of scope infomation
    // 作用域
    struct ScopeInfo
    {
        int cnt;           // 作用域标识符(唯一)
        string name;       // 用于分辨作用域的类别，'b' 代表是一个单独嵌套的作用域, 'i' 'e' 'w' 分别代表由 if else while 产生的新作用域
        map_str_ste table; ////存放符号的表, {string: STE}, string 是操作数的原始名称，在 STE 中存放的应该是变量重命名后的名称
    };

    // surpport lib functions
    // 没有提供输入输出相关的 IR，可以通过调用库函数来与标准输入输出进行交互
    map<std::string, ir::Function *> *get_lib_funcs();

    // definition of symbol table
    // 符号表定义
    struct SymbolTable
    {
        // 作用域是支持嵌套, 使用栈式结构来存储
        vector<ScopeInfo> scope_stack;
        // 存放函数块的表
        map<std::string, ir::Function *> functions;
        int scope_cnt = 0;

        /**
         * @brief enter a new scope, record the infomation in scope stacks
         * @param node: a Block node, entering a new Block means a new name scope
         * 进入新作用域时, 向符号表中添加 ScopeInfo, 相当于压栈
         */
        void add_scope(Block *);

        /**
         * @brief exit a scope, pop out infomations
         * 退出时弹栈
         */
        void exit_scope();

        /**
         * @brief Get the scoped name, to deal the same name in different scopes, we change origin id to a new one with scope infomation,
         * for example, we have these code:
         * "
         * int a;
         * {
         *      int a; ....
         * }
         * "
         * in this case, we have two variable both name 'a', after change they will be 'a' and 'a_block'
         * @param id: origin id
         * @return string: new name with scope infomations
         * 输入一个变量名, 返回其在当前作用域下重命名后的名字 (相当于加后缀)
         */
        string get_scoped_name(string id) const;

        /**
         * @brief get the right operand with the input name
         * @param id identifier name
         * @return Operand
         * 输入一个变量名, 在符号表中寻找最近的同名变量, 返回对应的 Operand操作数(注意，此 Operand 的 name 是重命名后的)
         */
        ir::Operand get_operand(string id) const;

        /**
         * @brief get the right ste with the input name
         * @param id identifier name
         * @return STE
         * 输入一个变量名, 在符号表中寻找最近的同名变量, 返回 STE
         */
        STE get_ste(string id) const;
    };

    // singleton class
    struct Analyzer
    {
        int tmp_cnt;
        vector<ir::Instruction *> g_init_inst;
        SymbolTable symbol_table;
        // 添加当前所在函数的标记，方便return时做类型检查
        ir::Function* cur_func;

        /**
         * @brief constructor
         */
        Analyzer();

        // analysis functions
        ir::Program get_ir_program(CompUnit *);

        // reject copy & assignment
        Analyzer(const Analyzer &) = delete;
        Analyzer &operator=(const Analyzer &) = delete;

        // add
        // analysis functions
        void analysisCompUnit(CompUnit *, ir::Program &, ir::Function &);
        void analysisFuncDef(FuncDef *, ir::Function &);
        void analysisDecl(Decl *, vector<ir::Instruction *> &);
        void analysisConstDecl(ConstDecl *, vector<ir::Instruction *> &);
        void analysisBType(BType *, vector<ir::Instruction *> &);
        void analysisConstDef(ConstDef *, vector<ir::Instruction *> &);
        void analysisConstInitVal(ConstInitVal *, vector<ir::Instruction *> &);
        void analysisVarDecl(VarDecl *, vector<ir::Instruction *> &);
        void analysisVarDef(VarDef *, vector<ir::Instruction *> &);
        void analysisInitVal(InitVal *, vector<ir::Instruction *> &);
        // void analysisFuncType(FuncType *, vector<ir::Instruction *> &);
        void analysisFuncFParam(FuncFParam *, ir::Function &);
        void analysisFuncFParams(FuncFParams *, ir::Function &);
        void analysisBlock(Block *, vector<ir::Instruction *> &);
        void analysisBlockItem(BlockItem *, vector<ir::Instruction *> &);
        void analysisStmt(Stmt *, vector<ir::Instruction *> &);
        void analysisExp(Exp *, vector<ir::Instruction *> &);
        void analysisCond(Cond *, vector<ir::Instruction *> &);
        void analysisLVal(LVal *, vector<ir::Instruction *> &);
        void analysisNumber(Number *, vector<ir::Instruction *> &);
        void analysisPrimaryExp(PrimaryExp *, vector<ir::Instruction *> &);
        void analysisUnaryExp(UnaryExp *, vector<ir::Instruction *> &);
        // void analysisUnaryOp(UnaryOp *, vector<ir::Instruction *> &);
        void analysisFuncRParams(FuncRParams *, vector<ir::Instruction *> &, ir::CallInst &);
        void analysisMulExp(MulExp *, vector<ir::Instruction *> &);
        void analysisAddExp(AddExp *, vector<ir::Instruction *> &);
        void analysisRelExp(RelExp *, vector<ir::Instruction *> &);
        void analysisEqExp(EqExp *, vector<ir::Instruction *> &);
        void analysisLAndExp(LAndExp *, vector<ir::Instruction *> &);
        void analysisLOrExp(LOrExp *, vector<ir::Instruction *> &);
        void analysisConstExp(ConstExp *, vector<ir::Instruction *> &);
    };

} // namespace frontend

#endif
/*
    符号表存储作用域与函数块
    作用域存储符号表条目
    符号表条目存储操作数与维度
*/