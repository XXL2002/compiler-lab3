#include "front/semantic.h"

#include <cassert>
#include <iostream>
// #include "semantic.h"

using ir::Function;
using ir::Instruction;
using ir::Operand;
using ir::Operator;

#define TODO assert(0 && "TODO");
#define todo ;

#define GET_CHILD_PTR(node, type, index)                     \
    auto node = dynamic_cast<type *>(root->children[index]); \
    assert(node);
#define ANALYSIS(node, type, index)                          \
    auto node = dynamic_cast<type *>(root->children[index]); \
    assert(node);                                            \
    analysis##type(node, buffer);
#define COPY_EXP_NODE(from, to)              \
    to->is_computable = from->is_computable; \
    to->v = from->v;                         \
    to->t = from->t;

/*
    没有提供输入输出相关的 IR，可以通过调用库函数来与标准输入输出进行交互
*/
map<std::string, ir::Function *> *frontend::get_lib_funcs()
{
    static map<std::string, ir::Function *> lib_funcs = {
        {"getint", new Function("getint", Type::Int)},
        {"getch", new Function("getch", Type::Int)},
        {"getfloat", new Function("getfloat", Type::Float)},
        {"getarray", new Function("getarray", {Operand("arr", Type::IntPtr)}, Type::Int)},
        {"getfarray", new Function("getfarray", {Operand("arr", Type::FloatPtr)}, Type::Int)},
        {"putint", new Function("putint", {Operand("i", Type::Int)}, Type::null)},
        {"putch", new Function("putch", {Operand("i", Type::Int)}, Type::null)},
        {"putfloat", new Function("putfloat", {Operand("f", Type::Float)}, Type::null)},
        {"putarray", new Function("putarray", {Operand("n", Type::Int), Operand("arr", Type::IntPtr)}, Type::null)},
        {"putfarray", new Function("putfarray", {Operand("n", Type::Int), Operand("arr", Type::FloatPtr)}, Type::null)},
    };
    return &lib_funcs;
}

void frontend::SymbolTable::add_scope(Block *root)
{
    // 唯一标识符
    int cnt = ++scope_cnt;

    // 作用域类型分辨符
    string name;
    // if (root->parent->type == NodeType::FUNCDEF)
    // {
    //     name = "b";
    // }
    // else if (root->parent->parent->children[0]->type == NodeType::TERMINAL)
    // { // 长兄节点是TERM
    //     if (dynamic_cast<Term *>(root->parent->parent->children[0])->token.type == TokenType::WHILETK)
    //     { // 长兄节点是while
    //         name = "w";
    //     }
    //     else if (dynamic_cast<Term *>(root->parent->parent->children[0])->token.type == TokenType::IFTK)
    //     {                                                  // 长兄节点是if
    //         if (root->parent->parent->children.size() > 5) // 判断是否有else语句
    //         {
    //             if (root->parent->parent->children[-1] == root->parent) // 比对末尾的stmt是否是当前block的父节点指针
    //             {
    //                 name = "e";
    //             }
    //             else
    //             {
    //                 name = 'i';
    //             }
    //         }
    //         else
    //         { // 无else语句
    //             name = "i";
    //         }
    //     }
    // }
    // else
    // {
    //     name = "b";
    // }
    // 符号表
    map_str_ste table;
    this->scope_stack.push_back(*new ScopeInfo{cnt, name, table});
}

void frontend::SymbolTable::exit_scope()
{
    // 退出作用域时弹栈
    this->scope_stack.pop_back();
}

string frontend::SymbolTable::get_scoped_name(string id) const
{
    // 重命名
    return id + std::to_string(this->scope_stack.back().cnt);
}

Operand frontend::SymbolTable::get_operand(string id) const
{
    // 由近及远查找同名变量
    for (int i = this->scope_stack.size()-1; i >= 0; i--)
    {
        auto it = this->scope_stack[i].table.find(id);
        if (it != this->scope_stack[i].table.end())
        {
            return it->second.operand;
        }
    }
}

frontend::STE frontend::SymbolTable::get_ste(string id) const
{
    // 由近及远查找同名变量
    for (int i = this->scope_stack.size()-1; i >= 0; i--)
    {
        auto it = this->scope_stack[i].table.find(id);
        if (it != this->scope_stack[i].table.end())
        {
            return it->second;
        }
    }
}

frontend::Analyzer::Analyzer() : tmp_cnt(0), symbol_table()
{
    // 添加全局作用域
    map_str_ste table;
    symbol_table.scope_stack.push_back(*new ScopeInfo{0, "global", table});
    // 添加库函数
    symbol_table.functions = *get_lib_funcs();
}

ir::Program frontend::Analyzer::get_ir_program(CompUnit *root)
{
    // 主接口
    std::cout << "start!\n";
    ir::Program program = *new ir::Program;
    auto globalFunc = *new ir::Function("globalFunc", ir::Type::null);
    analysisCompUnit(root, program, globalFunc);
    std::cout << program.draw();
    std::cout << "end!\n";
    return program;
}

// add
// analysis functions
void frontend::Analyzer::analysisCompUnit(CompUnit *root, ir::Program &program, ir::Function &globalFunc)
{
    /*
    CompUnit -> (Decl | FuncDef) [CompUnit]
    */
    for (int i = 0; i < root->children.size(); i++)
    { // 遍历每一个子节点
        if (root->children[i]->type == NodeType::COMPUINT)
        {
            auto node = dynamic_cast<CompUnit *>(root->children[i]);
            assert(node);
            analysisCompUnit(node, program, globalFunc);
        }
        else if (root->children[i]->type == NodeType::DECL)
        {
            // 生成buffer，并解析Decl
            vector<ir::Instruction *> buffer = *new vector<ir::Instruction *>;
            ANALYSIS(decl, Decl, i);
            // 压入解析过程中生成的指令
            for (int j = 0; j < buffer.size(); j++)
            {
                globalFunc.addInst(buffer[j]);
            }
            // 判断是否已是globalFunc的最后一条
            if (i != root->children.size() - 1)
            { // 不是最后一个节点
                if (root->children[i + 1]->type == NodeType::COMPUINT)
                {
                    if (root->children[i + 1]->children[0]->type == NodeType::DECL)
                    {
                        // 不是最后一条DECL
                        continue;
                    }
                }
            }
            // 是最后一条global
            // 生成global return，并添加
            ir::Instruction *globalreturn = new ir::Instruction(ir::Operand(),
                                                                ir::Operand(),
                                                                ir::Operand(), ir::Operator::_return);
            globalFunc.addInst(globalreturn);

            // 设置globalValue
            for (auto it = symbol_table.scope_stack[0].table.begin(); it != symbol_table.scope_stack[0].table.end(); it++)
            {
                if (it->second.dimension.size() == 0)
                {
                    if (it->second.operand.type == Type::FloatLiteral || it->second.operand.type == Type::IntLiteral)
                    {
                        // 常量无需添加
                        continue;
                    }
                    else
                    {
                        // 单一变量
                        // string new_name = symbol_table.get_scoped_name(it->second.operand.name);
                        program.globalVal.push_back({{it->second.operand.name, it->second.operand.type}});
                    }
                }
                else
                {
                    // 数组变量
                    int maxlen = 1;
                    for (int i = 0; i < it->second.dimension.size(); i++)
                    {
                        maxlen *= it->second.dimension[i];
                    }
                    // string new_name = symbol_table.get_scoped_name(it->second.operand.name);
                    program.globalVal.push_back({{it->second.operand.name, it->second.operand.type}, maxlen});
                }
            }
            // program.globalVal.emplace_back(ir::Operand());  // emplace_back() 和 push_back() 的区别，就在于底层实现的机制不同。push_back() 向容器尾部添加元素时，首先会创建这个元素，然后再将这个元素拷贝或者移动到容器中（如果是拷贝的话，事后会自行销毁先前创建的这个元素）；而 emplace_back() 在实现时，则是直接在容器尾部创建这个元素，省去了拷贝或移动元素的过程。

            program.addFunction(globalFunc);
        }
        else if (root->children[i]->type == NodeType::FUNCDEF)
        {
            if (globalFunc.InstVec.empty())
            {
                ir::Instruction *globalreturn = new ir::Instruction(ir::Operand(),
                                                                    ir::Operand(),
                                                                    ir::Operand(), ir::Operator::_return);
                globalFunc.addInst(globalreturn);
                program.addFunction(globalFunc);
            }

            // 创建Function buffer，解析完成后添加至program中
            ir::Function buffer = *new ir::Function;
            ANALYSIS(node, FuncDef, i);
            program.addFunction(buffer);
        }
    }
}

void frontend::Analyzer::analysisFuncDef(FuncDef *root, ir::Function &function)
{
    // PPT
    /*
    FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
        FuncDef.t
        FuncDef.n
    FuncType -> 'void' | 'int' | 'float'
    */
    // 从FuncType处获取类型
    auto tk = dynamic_cast<Term *>(root->children[0]->children[0])->token.type;
    root->t = (tk == TokenType::VOIDTK) ? Type::null : (tk == TokenType::INTTK) ? Type::Int
                                                                                : Type::Float;
    // 从Ident处获取函数名称
    root->n = dynamic_cast<Term *>(root->children[1])->token.value;
    // 设置ir::function的函数名称和返回值类型
    function.name = root->n;
    function.returnType = root->t;
    // 添加function
    symbol_table.functions.insert({root->n, &function});

    if (root->n == "main")
    {
        Operand tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::null);
        function.addInst(new ir::CallInst({"globalFunc", Type::null}, {}, tmp));
    }
    // 标记当前func
    cur_func = &function;

    // 开辟并进入新作用域
    symbol_table.scope_stack.push_back({1, "fp", map_str_ste()}); // function param
    // 获取func param
    auto paras = dynamic_cast<FuncFParams *>(root->children[3]);
    if (paras)
    { // 有参数，block位于第5位
        analysisFuncFParams(paras, function);
        analysisBlock(dynamic_cast<Block *>(root->children[5]), function.InstVec);
    }
    else
    { // 无参数，block位于第4位
        analysisBlock(dynamic_cast<Block *>(root->children[4]), function.InstVec);
    }
    // 退出function param作用域
    symbol_table.exit_scope();
    if (root->t == Type::null)
    {
        function.addInst(new Instruction({}, {}, {}, Operator::_return));
    }
    // function.draw();
}

void frontend::Analyzer::analysisDecl(Decl *root, vector<ir::Instruction *> &buffer)
{
    // Decl -> ConstDecl | VarDecl
    if (root->children[0]->type == NodeType::CONSTDECL)
    { // Decl -> ConstDecl
        ANALYSIS(constdecl, ConstDecl, 0);
    }
    else
    { // Decl -> VarDecl
        ANALYSIS(vardecl, VarDecl, 0);
    }
}

void frontend::Analyzer::analysisConstDecl(ConstDecl *root, vector<ir::Instruction *> &buffer)
{
    /*
    ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
        ConstDecl.t
    */
    ANALYSIS(btype, BType, 1);
    root->t = btype->t;
    int idx = 2;
    while (idx < root->children.size())
    {
        auto paras = dynamic_cast<ConstDef *>(root->children[idx]);
        if (paras)
        {
            ANALYSIS(constdef, ConstDef, idx);
        }
        else
        {
            break;
        }
        idx += 2;
    }
}

void frontend::Analyzer::analysisBType(BType *root, vector<ir::Instruction *> &buffer)
{
    /*
    BType -> 'int' | 'float'
        BType.t
    */
    auto tk = dynamic_cast<Term *>(root->children[0])->token.type;
    root->t = (tk == TokenType::INTTK) ? Type::Int : Type::Float;
}

void frontend::Analyzer::analysisConstDef(ConstDef *root, vector<ir::Instruction *> &buffer)
{
    // video
    /*
    ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
        ConstDecl.t

    ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
        ConstDef.arr_name
    举例：const int a = 10.0;
    */
    // 类型是继承属性，来自父节点(ConstDecl)属性t [来自Btype]
    auto root_type = dynamic_cast<ConstDecl *>(root->parent)->t;
    // identifier是ConstDef的第一个子节点，Ident
    GET_CHILD_PTR(identifier, Term, 0);
    // 变量原名["a"]
    string id = identifier->token.value;
    // 符号表里的名字["a_g"]
    string new_name = symbol_table.get_scoped_name(id);

    // term是ConstDef的第2个子节点[可能是'='或'[']
    GET_CHILD_PTR(term, Term, 1);
    if (term->token.type == TokenType::ASSIGN)
    {
        // ConstDef -> Ident '=' ConstInitVal 非数组
        ANALYSIS(cinitval, ConstInitVal, 2);
        // 添加变量
        // auto des = Operand(new_name, cinitval->t); // name type
        // 要在IR中生成的操作数
        // auto des = Operand(new_name, root_type); // name type
        assert((cinitval->t == Type::IntLiteral) || (cinitval->t == Type::FloatLiteral));
        // // 确定def类型
        // auto def_type = (root_type == Type::Float || root_type == Type::FloatLiteral) ? Operator::fdef : Operator::def;
        if (root_type == Type::Float)
        {
            if (cinitval->t == Type::IntLiteral)
            {
                cinitval->t = Type::FloatLiteral;
            }
        }
        auto des = Operand(cinitval->v, cinitval->t);
        // 插入符号表项
        symbol_table.scope_stack.back().table.insert({id, {des}}); // map_str_ste{string:STE{Operand,dimension}}
        // }
    }
    else
    { // 数组
        // ConstDef -> Ident { '[' ConstExp ']' }+ '=' ConstInitVal
        // TODO;
        // 修改为指针类型
        root_type = root_type == Type::Int ? Type::IntPtr : Type::FloatPtr;
        ANALYSIS(cexp1, ConstExp, 2);
        // 必定是常量，直接赋值
        auto c1 = Operand(cexp1->v, cexp1->t);
        // 类型检查
        assert(cexp1->t == Type::IntLiteral);
        // 要存入IR中的变量
        auto arr = Operand(new_name, root_type);

        if (root->children.size() == 6)
        {
            // ConstDef -> Ident '[' ConstExp ']' '=' ConstInitVal
            // IR数组声明
            auto alloc = new Instruction(c1, {}, arr, Operator::alloc);
            buffer.push_back(alloc);
            // 添加至符号表，包括维度向量
            auto dim = new std::vector<int>;
            dim->push_back(std::stoi(c1.name));
            symbol_table.scope_stack.back().table.insert({id, {arr, *dim}});

            ANALYSIS(cinit, ConstInitVal, 5);
            int i = 0;
            for (; i < cinit->value.size(); i++)
            {
                buffer.push_back(new Instruction(arr, {std::to_string(i), Type::IntLiteral}, cinit->value[i], Operator::store));
            }
            for (; i < std::stoi(c1.name); i++)
            {
                buffer.push_back(new Instruction(arr, {std::to_string(i), Type::IntLiteral}, {"0",Type::IntLiteral}, Operator::store));
            }
        }
        else
        {
            assert(root->children.size() == 9);
            // ConstDef -> Ident '[' ConstExp ']' '[' ConstExp ']' '=' ConstInitVal
            ANALYSIS(cexp2, ConstExp, 5);
            // 必定是常量，直接赋值
            auto c2 = Operand(cexp2->v, cexp2->t);
            // 类型检查
            assert(cexp2->t == Type::IntLiteral);
            // 计算数组长度
            auto len = Operand(std::to_string(stoi(c1.name) * stoi(c2.name)), Type::IntLiteral);
            // IR数组声明
            auto alloc = new Instruction(len, {}, arr, Operator::alloc);
            buffer.push_back(alloc);
            // 添加至符号表，包括维度向量
            auto dim = new std::vector<int>;
            dim->push_back(std::stoi(c1.name));
            dim->push_back(std::stoi(c2.name));
            symbol_table.scope_stack.back().table.insert({id, {arr, *dim}});

            // 赋值
            ANALYSIS(cinit, ConstInitVal, 8);
            int i = 0;
            for (; i < cinit->value.size(); i++)
            {
                buffer.push_back(new Instruction(arr, {std::to_string(i), Type::IntLiteral}, cinit->value[i], Operator::store));
            }
            for (; i < std::stoi(c1.name)*std::stoi(c2.name); i++)
            {
                buffer.push_back(new Instruction(arr, {std::to_string(i), Type::IntLiteral}, {"0",Type::IntLiteral}, Operator::store));
            }
        }
        todo;
    }
}
void frontend::Analyzer::analysisConstInitVal(ConstInitVal *root, vector<ir::Instruction *> &buffer)
{
    /*
    ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
        ConstInitVal.v
        ConstInitVal.t
    */
    if (root->children.size() == 1)
    {
        // ConstInitVal -> ConstExp
        ANALYSIS(cexp, ConstExp, 0);
        root->v = cexp->v;
        root->t = cexp->t;
    }
    else
    {
        // 数组
        /*
            ConstInitVal -> '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
        */
        // TODO;
        if (root->children.size() == 2)
        {
            // ConstInitVal -> '{' '}'
            // do nothing
        }
        else
        {
            for (int idx = 1; idx < root->children.size(); idx += 2)
            {
                ANALYSIS(child, ConstInitVal, idx)
                root->value.push_back({child->v, child->t});
                root->v = "vector<Operand>";
                root->t = Type::null;
            }
        }
        todo;
    }
}
void frontend::Analyzer::analysisVarDecl(VarDecl *root, vector<ir::Instruction *> &buffer)
{
    /*
    VarDecl -> BType VarDef { ',' VarDef } ';'
        VarDecl.t
    */
    ANALYSIS(btype, BType, 0);
    root->t = btype->t;
    int idx = 1;
    while (idx < root->children.size())
    {
        auto paras = dynamic_cast<VarDef *>(root->children[idx]);
        if (paras)
        {
            ANALYSIS(vardef, VarDef, idx);
        }
        else
        {
            break;
        }
        idx += 2;
    }
}

void frontend::Analyzer::analysisVarDef(VarDef *root, vector<ir::Instruction *> &buffer)
{
    /*
      // VarDecl -> BType VarDef { ',' VarDef } ';'
      //    VarDecl.t

    VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
        VarDef.arr_name
    */
    // 类型是继承属性，来自父节点(VarDecl)属性t [来自Btype]
    auto root_type = dynamic_cast<VarDecl *>(root->parent)->t;
    // identifier是ValDef的第一个子节点，Ident
    GET_CHILD_PTR(identifier, Term, 0);
    // 变量原名
    string id = identifier->token.value;
    // 要存进符号表里的名字【供IR使用】
    string new_name = symbol_table.get_scoped_name(id);

    if (root->children.size() == 1)
    {
        // VarDef -> Ident 无赋值
        // TODO;
        auto def_type = (root_type == Type::Int) ? Operator::def : Operator::fdef;
        // 添加进IR,！！需要给定一个初始值，否则会报op1错误！！
        auto init = def_type == Operator::def ? Operand("0", Type::IntLiteral) : Operand("0.0", Type::FloatLiteral);
        buffer.push_back(new Instruction(init, {}, {new_name, root_type}, def_type));
        // buffer.back()->draw();

        // 添加进符号表
        symbol_table.scope_stack.back().table.insert({id, {{new_name, root_type}}});
    }
    else
    {
        /*
        VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
            VarDef.arr_name
        */
        // term是VarDef的第2个子节点[可能是'='或'[']
        GET_CHILD_PTR(term, Term, 1);
        if (term->token.type == TokenType::ASSIGN)
        {
            // VarDef -> Ident  '=' InitVal 非数组
            ANALYSIS(initval, InitVal, 2);
            // 添加变量
            // auto des = Operand(new_name, initval->t);
            // 要在IR中生成的操作数
            auto des = Operand(new_name, root_type); // name type

            // 确定def类型
            auto def_type = (root_type == Type::Float || root_type == Type::FloatLiteral) ? Operator::fdef : Operator::def;
            // 待写入的变量值
            // auto operand1 = Operand(initval->v, initval->t);
            // 立即数直接使用自己的名字，否则查符号表
            auto operand1 = initval->is_computable ? Operand(initval->v, initval->t) : symbol_table.get_operand(initval->v);
            // std::cout<<"prepare to write {"<<operand1.name<<" , "<<toString(operand1.type)<<"} into {"<<id<<"->"<<new_name<<" , "<< toString(des.type)<<"}\n";
            // 类型检查
            if (root_type == Type::Float)
            { // 浮点变量定义
                if (initval->t == Type::Int)
                {
                    // int -> float类型转换临时变量
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // 需要类型转换IR
                    buffer.push_back(new Instruction(operand1, {}, tmp, Operator::cvt_i2f));
                    operand1 = tmp;
                }
                else if (initval->t == Type::IntLiteral)
                { // 整数立即数
                    // 直接修改类型为浮点数即可
                    operand1.type = Type::FloatLiteral;
                }
            }
            else
            {
                // 整型变量定义
                assert(root_type == Type::Int);
                if (initval->t == Type::Float)
                {
                    // 浮点数
                    // float -> int类型转换临时变量("t1",Type::Int)
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // 需要类型转化IR
                    buffer.push_back(new Instruction(operand1, {}, tmp, Operator::cvt_f2i));
                    // 此时operand1不再是float类型的oprand，而是int类型的tmp
                    operand1 = tmp;
                }
                else if (initval->t == Type::FloatLiteral)
                {
                    // 浮点立即数
                    // 转换为整数立即数
                    operand1.name == std::to_string((int)std::atof(operand1.name.c_str())); // 转换过程：string->char*->float->int->string
                    operand1.type = Type::IntLiteral;
                }
            }
            // 定义变量IR
            buffer.push_back(new Instruction(operand1, Operand(), des, def_type)); // 第二个操作数不使用

            // buffer.back()->draw();
            // 插入符号表项
            // symbol_table.scope_stack.back().table.insert({id, {operand1, {}}}); // map_str_ste{string:STE{Operand,dimension}}
            symbol_table.scope_stack.back().table.insert({id, {des}}); // map_str_ste{string:STE{Operand,dimension}}
        }
        else
        {
            // 数组
            // VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
            // TODO;
            // 修改为指针类型
            root_type = root_type == Type::Int ? Type::IntPtr : Type::FloatPtr;
            ANALYSIS(cexp1, ConstExp, 2);
            // 必定是常量，直接赋值
            auto c1 = Operand(cexp1->v, cexp1->t);
            // 类型检查
            assert(cexp1->t == Type::IntLiteral);
            // 要存入IR中的变量
            auto arr = Operand(new_name, root_type);

            if (root->children.size() == 4)
            {
                // VarDef -> Ident '[' ConstExp ']'
                // IR数组声明
                auto alloc = new Instruction(c1, {}, arr, Operator::alloc);
                buffer.push_back(alloc);
                for (int i = 0;i<stoi(c1.name);i++){
                    // 添加进IR,！！需要给定一个初始值，否则会出现段错误！！
                    auto init = root_type == Type::IntPtr ? Operand("0", Type::IntLiteral) : Operand("0.0", Type::FloatLiteral);
                    buffer.push_back(new Instruction(arr, {std::to_string(i),Type::IntLiteral},init, Operator::store));
                }
                // 添加至符号表，包括维度向量
                auto dim = new std::vector<int>;
                dim->push_back(std::stoi(c1.name));
                symbol_table.scope_stack.back().table.insert({id, {arr, *dim}});
            }
            else if (root->children.size() == 6)
            {
                // VarDef -> Ident '[' ConstExp ']' '=' InitVal
                // IR数组声明
                auto alloc = new Instruction(c1, {}, arr, Operator::alloc);
                buffer.push_back(alloc);
                // 添加至符号表，包括维度向量
                auto dim = new std::vector<int>;
                dim->push_back(std::stoi(c1.name));
                symbol_table.scope_stack.back().table.insert({id, {arr, *dim}});

                ANALYSIS(init, InitVal, 5);
                int i = 0;
                for (; i < init->value.size(); i++)
                {
                    buffer.push_back(new Instruction(arr, {std::to_string(i), Type::IntLiteral}, init->value[i], Operator::store));
                }
                for (; i < std::stoi(c1.name); i++)
                {
                    buffer.push_back(new Instruction(arr, {std::to_string(i), Type::IntLiteral}, {"0",Type::IntLiteral}, Operator::store));
                }
                todo;
            }
            else if (root->children.size() == 7)
            {
                // VarDef -> Ident '[' ConstExp ']' '[' ConstExp ']'
                ANALYSIS(cexp2, ConstExp, 5);
                // 必定是常量，直接赋值
                auto c2 = Operand(cexp2->v, cexp2->t);
                // 类型检查
                assert(cexp2->t == Type::IntLiteral);
                // 计算数组长度
                auto len = Operand(std::to_string(stoi(c1.name) * stoi(c2.name)), Type::IntLiteral);
                // IR数组声明
                auto alloc = new Instruction(len, {}, arr, Operator::alloc);
                buffer.push_back(alloc);
                for (int i = 0;i<stoi(c1.name);i++){
                    // 添加进IR,！！需要给定一个初始值，否则会出现段错误！！
                    auto init = root_type == Type::IntPtr ? Operand("0", Type::IntLiteral) : Operand("0.0", Type::FloatLiteral);
                    buffer.push_back(new Instruction(arr, {std::to_string(i),Type::IntLiteral},init, Operator::store));
                }
                // 添加至符号表，包括维度向量
                auto dim = new std::vector<int>;
                dim->push_back(std::stoi(c1.name));
                dim->push_back(std::stoi(c2.name));
                symbol_table.scope_stack.back().table.insert({id, {arr, *dim}});
            }
            else
            {
                assert(root->children.size() == 9);
                // VarDef -> Ident '[' ConstExp ']' '[' ConstExp ']' '=' InitVal
                ANALYSIS(cexp2, ConstExp, 5);
                // 必定是常量，直接赋值
                auto c2 = Operand(cexp2->v, cexp2->t);
                // 类型检查
                assert(cexp2->t == Type::IntLiteral);
                // 计算数组长度
                auto len = Operand(std::to_string(stoi(c1.name) * stoi(c2.name)), Type::IntLiteral);
                // IR数组声明
                auto alloc = new Instruction(len, {}, arr, Operator::alloc);
                buffer.push_back(alloc);
                // 添加至符号表，包括维度向量
                auto dim = new std::vector<int>;
                dim->push_back(std::stoi(c1.name));
                dim->push_back(std::stoi(c2.name));
                symbol_table.scope_stack.back().table.insert({id, {arr, *dim}});

                // 赋值
                ANALYSIS(init, InitVal, 8);
                int i = 0;
                for (; i < init->value.size(); i++)
                {
                    buffer.push_back(new Instruction(arr, {std::to_string(i), Type::IntLiteral}, init->value[i], Operator::store));
                }
                for (; i < std::stoi(c1.name)*std::stoi(c2.name); i++)
                {
                    buffer.push_back(new Instruction(arr, {std::to_string(i), Type::IntLiteral}, {"0",Type::IntLiteral}, Operator::store));
                }
                todo;
            }
            todo;
        }
    }
}

void frontend::Analyzer::analysisInitVal(InitVal *root, vector<ir::Instruction *> &buffer)
{
    /*
    InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
        InitVal.is_computable
        InitVal.v
        InitVal.t
    */
    if (root->children.size() == 1)
    {
        // InitVal -> Exp
        ANALYSIS(exp, Exp, 0);
        COPY_EXP_NODE(exp, root);
    }
    else
    {
        // 数组
        /*
            InitVal -> '{' [ InitVal { ',' InitVal } ] '}'
        */
        // TODO;
        if (root->children.size() == 2)
        {
            // ConstInitVal -> '{' '}'
            // do nothing
        }
        else
        {
            for (int idx = 1; idx < root->children.size(); idx += 2)
            {
                ANALYSIS(child, InitVal, idx)
                root->value.push_back({child->v, child->t});
                root->v = "vector<Operand>";
                root->t = Type::null;
            }
        }
        todo;
    }
}

// void frontend::Analyzer::analysisFuncType(FuncType *node, vector<ir::Instruction *> &buffer)
// {
//     /*
//     FuncType -> 'void' | 'int' | 'float'
//     没必要
//     */
// }

void frontend::Analyzer::analysisFuncFParam(FuncFParam *root, ir::Function &function)
{
    /*
    FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
    */
    auto buffer = *new vector<ir::Instruction *>;
    ANALYSIS(btype, BType, 0);
    Type parma_type = btype->t;

    string param_name = dynamic_cast<Term *>(root->children[1])->token.value;

    string new_name = symbol_table.get_scoped_name(param_name);
    std::vector<int> dim = {};
    if (root->children.size() > 2)
    {
        // FuncFParam -> BType Ident '[' ']' { '[' Exp ']' }
        // 设置为指针类型
        parma_type = (parma_type == Type::Int) ? Type::IntPtr : Type::FloatPtr;
        dim.push_back(0);
        if (root->children.size() > 4)
        {
            ANALYSIS(exp, Exp, 5);
            assert(exp->is_computable);
            dim.push_back(std::stoi(exp->v));
        }
    }
    Operand op = {new_name, parma_type};
    symbol_table.scope_stack.back().table.insert({param_name, {op, dim}});
    // 防止overflow导致失效
    function.ParameterList.push_back(symbol_table.get_operand(param_name));
}

void frontend::Analyzer::analysisFuncFParams(FuncFParams *root, ir::Function &buffer)
{
    /*
    FuncFParams -> FuncFParam { ',' FuncFParam }
    */
    int idx = 0;
    while (idx < root->children.size())
    {
        auto paras = dynamic_cast<FuncFParam *>(root->children[idx]);
        if (paras)
        {
            ANALYSIS(funcfparam, FuncFParam, idx);
        }
        else
        {
            break;
        }
        idx += 2;
    }
}

void frontend::Analyzer::analysisBlock(Block *root, vector<ir::Instruction *> &buffer)
{
    /*
    Block -> '{' { BlockItem } '}'
    */
    symbol_table.add_scope(root);
    int idx = 1;
    while (idx < root->children.size())
    {
        auto paras = dynamic_cast<BlockItem *>(root->children[idx]);
        if (paras)
        {
            ANALYSIS(bitem, BlockItem, idx);
        }
        else
        {
            break;
        }
        idx++;
    }
    symbol_table.exit_scope();
}

void frontend::Analyzer::analysisBlockItem(BlockItem *root, vector<ir::Instruction *> &buffer)
{
    /*
    BlockItem -> Decl | Stmt
    */
    if (root->children[0]->type == NodeType::DECL)
    {
        ANALYSIS(decl, Decl, 0);
    }
    else
    {
        ANALYSIS(stmt, Stmt, 0);
    }
}

void frontend::Analyzer::analysisStmt(Stmt *root, vector<ir::Instruction *> &buffer)
{
    /*
    Stmt -> LVal '=' Exp ';' | Block | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] | 'while' '(' Cond ')' Stmt | 'break' ';' | 'continue' ';' | 'return' [Exp] ';' | [Exp] ';'
    */
    if (root->children[0]->type == NodeType::LVAL)
    {
        // Stmt -> LVal '=' Exp ';'
        ANALYSIS(lval, LVal, 0);
        ANALYSIS(exp, Exp, 2);
        // string lnew_name = symbol_table.get_scoped_name(lval->v);
        // auto des = Operand(lnew_name, lval->t);
        auto src = exp->is_computable ? Operand(exp->v, exp->t) : symbol_table.get_operand(exp->v);
        // auto des = lval->is_computable ? Operand(lval->v, lval->t) : symbol_table.get_operand(lval->v);
        // std::cout << "!!!!!!!!!!!left.name:" << des.name << "\nleft.type:" << toString(des.type) << "\n";
        if (lval->t == Type::Int)
        {
            // 加上类型检查更好
            assert(exp->t == Type::Int || exp->t == Type::IntLiteral);
            auto des = lval->is_computable ? Operand(lval->v, lval->t) : symbol_table.get_operand(lval->v);
            // if (exp->t == Type::IntLiteral)
            // {
            //     auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
            //     buffer.push_back(new Instruction(src, Operand(), tmp, Operator::def));
            //     buffer.push_back(new Instruction(tmp, Operand(), des, Operator::mov));
            // }else{
            //     buffer.push_back(new Instruction(src, Operand(), des, Operator::mov));
            // }
            if (lval->is_arr_element)
            {
                buffer.push_back(new Instruction({lval->arr_name, Type::IntPtr}, lval->i, src, Operator::store));
            }
            else
            {
                buffer.push_back(new Instruction(src, Operand(), des, Operator::def));
            }
        }
        else if (lval->t == Type::Float)
        {
            // 加上类型检查更好
            assert(exp->t == Type::Float || exp->t == Type::FloatLiteral);
            auto des = lval->is_computable ? Operand(lval->v, lval->t) : symbol_table.get_operand(lval->v);
            // if (exp->t == Type::FloatLiteral)
            // {
            //     auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
            //     buffer.push_back(new Instruction(src, Operand(), tmp, Operator::fdef));
            //     buffer.push_back(new Instruction({lval->arr_name, Type::FloatPtr}, lval->i, src, Operator::store));
            // }
            // else
            // {
            //     buffer.push_back(new Instruction({lval->arr_name, Type::FloatPtr}, lval->i, src, Operator::store));
            // }
            if (lval->is_arr_element)
            {
                buffer.push_back(new Instruction({lval->arr_name, Type::FloatPtr}, lval->i, src, Operator::store));
            }
            else
            {
                buffer.push_back(new Instruction(src, Operand(), des, Operator::fdef));
            }
        }
        else
        {
            // std::cout << "lval.t:" << toString(lval->t) << ",lval.name:" << lval->v << "\n";
            assert((lval->t == Type::IntPtr || lval->t == Type::FloatPtr));
            // 加上类型检查更好
            // TODO;
            // assert(exp->t == ((lval->t == Type::IntPtr) ? Type::Int : Type::Float));
            // auto zero = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
            // buffer.push_back(new Instruction({"0", Type::IntLiteral}, Operand(), zero, Operator::def));
            // buffer.push_back(new Instruction(des, zero, src, Operator::store));
            // std::cout << "arr_name:" << lval->arr_name << std::endl;
            buffer.push_back(new Instruction({lval->arr_name, lval->t}, lval->i, src, Operator::store));
        }
    }
    else if (root->children[0]->type == NodeType::BLOCK)
    {
        // Stmt -> Block
        ANALYSIS(block, Block, 0);
    }
    else if (root->children[0]->type == NodeType::EXP)
    {
        // Stmt -> [Exp] ';'
        ANALYSIS(exp, Exp, 0);
    }
    else
    {
        assert(root->children[0]->type == NodeType::TERMINAL);
        GET_CHILD_PTR(term, Term, 0);
        if (term->token.type == TokenType::IFTK)
        {
            // Stmt -> 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
            if (root->children.size() == 5)
            {
                // Stmt -> 'if' '(' Cond ')' Stmt
                ANALYSIS(cond, Cond, 2);
                // 判断正确->执行stmt
                buffer.push_back(new Instruction(symbol_table.get_operand(cond->v), Operand(), {"2", Type::IntLiteral}, Operator::_goto));
                // 判断错误->跳过stmt
                auto go_false = new Instruction(Operand(), Operand(), {}, Operator::_goto);

                std::vector<Instruction *> tmp = {};
                auto stmt = dynamic_cast<Stmt *>(root->children[4]);
                assert(stmt);
                analysisStmt(stmt, tmp);

                // 第一个stmt的指令数
                int offset1 = tmp.size();
                go_false->des = Operand(std::to_string(offset1 + 1), Type::IntLiteral);
                buffer.push_back(go_false);
                buffer.insert(buffer.end(), tmp.begin(), tmp.end());
                // 添加空指令，防止越界
                buffer.push_back(new Instruction({}, {}, {}, Operator::__unuse__));
            }
            else
            {
                // Stmt -> 'if' '(' Cond ')' Stmt 'else' Stmt
                ANALYSIS(cond, Cond, 2);
                // 判断正确->执行stmt1
                buffer.push_back(new Instruction(symbol_table.get_operand(cond->v), Operand(), {"2", Type::IntLiteral}, Operator::_goto));
                // 判断错误->跳过stmt1
                auto go_false = new Instruction(Operand(), Operand(), {}, Operator::_goto);

                std::vector<Instruction *> tmp1 = {};
                auto stmt1 = dynamic_cast<Stmt *>(root->children[4]);
                assert(stmt1);
                analysisStmt(stmt1, tmp1);

                // 第一个stmt的指令数
                int offset1 = tmp1.size();
                go_false->des = Operand(std::to_string(offset1 + 1), Type::IntLiteral);
                buffer.push_back(go_false);
                buffer.insert(buffer.end(), tmp1.begin(), tmp1.end());

                auto go_true = new Instruction(symbol_table.get_operand(cond->v), Operand(), {}, Operator::_goto);

                std::vector<Instruction *> tmp2 = {};
                auto stmt2 = dynamic_cast<Stmt *>(root->children[6]);
                assert(stmt2);
                analysisStmt(stmt2, tmp2);

                int offset2 = tmp2.size();
                // 判断正确->跳过stmt2
                go_true->des = Operand(std::to_string(offset2 + 1), Type::IntLiteral);
                buffer.push_back(go_true);
                buffer.insert(buffer.end(), tmp2.begin(), tmp2.end());
                // 添加空指令，防止越界
                buffer.push_back(new Instruction({}, {}, {}, Operator::__unuse__));
            }
        }
        else if (term->token.type == TokenType::WHILETK)
        {
            // Stmt -> 'while' '(' Cond ')' Stmt
            int before_cond = buffer.size();
            ANALYSIS(cond, Cond, 2);
            int after_cond = buffer.size();
            int cnt_cond = after_cond - before_cond;
            // 判断正确->执行stmt
            buffer.push_back(new Instruction(symbol_table.get_operand(cond->v), Operand(), {"2", Type::IntLiteral}, Operator::_goto));
            // 判断错误->跳过stmt
            auto go_false = new Instruction(Operand(), Operand(), {}, Operator::_goto);

            std::vector<Instruction *> tmp = {};
            auto stmt = dynamic_cast<Stmt *>(root->children[4]);
            assert(stmt);
            analysisStmt(stmt, tmp);
            // 添加：无条件跳转回判断阶段
            auto go_back = new Instruction({}, {}, {}, Operator::_goto);

            // stmt的指令数
            int offset = tmp.size();
            go_back->des = Operand(std::to_string(-offset - 2 - cnt_cond), Type::IntLiteral);
            tmp.push_back(go_back);
            go_false->des = Operand(std::to_string(offset + 2), Type::IntLiteral);
            buffer.push_back(go_false);

            // 处理break和continue
            for (int i = 0; i < tmp.size(); i++)
            {
                if (tmp[i]->op1.name == "break")
                {
                    // 处理break
                    tmp[i]->op1 = Operand();
                    tmp[i]->op2 = Operand();
                    tmp[i]->des = Operand(std::to_string(offset - i + 1), Type::IntLiteral);
                    tmp[i]->op = Operator::_goto;
                }
                else if (tmp[i]->op1.name == "continue")
                {
                    // 处理continue
                    tmp[i]->op1 = Operand();
                    tmp[i]->op2 = Operand();
                    // 跳到go_back处
                    tmp[i]->des = Operand(std::to_string(offset - i), Type::IntLiteral);
                    tmp[i]->op = Operator::_goto;
                }
            }

            buffer.insert(buffer.end(), tmp.begin(), tmp.end());

            // 添加空指令，防止越界
            buffer.push_back(new Instruction({}, {}, {}, Operator::__unuse__));
        }
        else if (term->token.type == TokenType::BREAKTK)
        {
            // Stmt -> 'break' ';'
            // 暂时标记一下
            buffer.push_back(new Instruction({"break", Type::null}, {}, {}, Operator::__unuse__));
        }
        else if (term->token.type == TokenType::CONTINUETK)
        {
            // Stmt -> 'continue' ';'
            // 暂时标记一下
            buffer.push_back(new Instruction({"continue", Type::null}, {}, {}, Operator::__unuse__));
        }
        else if (term->token.type == TokenType::SEMICN)
        {
            // do nothing
        }
        else
        {
            assert(term->token.type == TokenType::RETURNTK);
            // Stmt -> 'return' [Exp] ';'
            if (root->children.size() == 2)
            {
                // Stmt -> 'return' ';'
                buffer.push_back(new Instruction({}, {}, {}, Operator::_return));
            }
            else
            {
                assert(root->children.size() == 3);
                // Stmt -> 'return' Exp ';'
                ANALYSIS(exp, Exp, 1);
                // std::cout << "in Stmt -> 'return' [Exp] ';'  : exp.v" << exp->v << "\n";
                // auto src = Operand(exp->v, exp->t);
                auto src = exp->is_computable ? Operand(exp->v, exp->t) : symbol_table.get_operand(exp->v);
                // std::cout << "-----------src.name:" << src.name << " + src.type:" << toString(src.type) << "\n";
                auto res = Operand("t~" + std::to_string(tmp_cnt++), cur_func->returnType);
                // 类型检查
                if (cur_func->returnType == Type::Int)
                {
                    if (exp->t == Type::Int)
                    {
                        buffer.push_back(new Instruction(src, {}, res, Operator::mov));
                    }
                    else if (exp->t == Type::IntLiteral)
                    {
                        // 立即数转变量
                        buffer.push_back(new Instruction(src, {}, res, Operator::def));
                    }
                    else
                    {
                        buffer.push_back(new Instruction(src, {}, res, Operator::cvt_f2i));
                        assert(0 && "return type not match! [expected int]");
                    }
                }
                else if (cur_func->returnType == Type::Float)
                {
                    if (exp->t == Type::Float)
                    {
                        buffer.push_back(new Instruction(src, {}, res, Operator::fmov));
                    }
                    else if (exp->t == Type::FloatLiteral)
                    {
                        // 立即数转变量
                        buffer.push_back(new Instruction(src, {}, res, Operator::fdef));
                    }
                    else
                    {
                        assert(0 && "return type not match! [expected float]");
                    }
                }
                buffer.push_back(new Instruction(res, {}, {}, Operator::_return));
            }
        }
    }
}

void frontend::Analyzer::analysisExp(Exp *root, vector<ir::Instruction *> &buffer)
{
    /*
    Exp -> AddExp
        Exp.is_computable
        Exp.v
        Exp.t
    */
    ANALYSIS(addexp, AddExp, 0);
    COPY_EXP_NODE(addexp, root);
    // std::cout << "Exp.v: " << root->v << "\nExp.t:" << toString(root->t) <<"\nExp.is_computable:"<<root->is_computable<< "---------------------" <<std::endl;
}

void frontend::Analyzer::analysisCond(Cond *root, vector<ir::Instruction *> &buffer)
{
    /*
    Cond -> LOrExp
        Cond.is_computable
        Cond.v
        Cond.t
    */
    ANALYSIS(lorexp, LOrExp, 0);
    COPY_EXP_NODE(lorexp, root);
}

void frontend::Analyzer::analysisLVal(LVal *root, vector<ir::Instruction *> &buffer)
{
    /*
    LVal -> Ident {'[' Exp ']'}
        LVal.is_computable
        LVal.v
        LVal.t
        LVal.i
    */
    GET_CHILD_PTR(term, Term, 0);
    auto ident_ste = symbol_table.get_ste(term->token.value);
    auto ident = ident_ste.operand;
    if (root->children.size() == 1)
    {
        // LVal -> Ident
        // 此时Ident是变量/常量
        root->is_computable = (ident.type == Type::IntLiteral || ident.type == Type::FloatLiteral) ? true : false;
        root->t = ident.type;
        // root->v = term->token.value;
        root->v = root->is_computable ? ident.name : term->token.value;
        // root->v = (ident.type == Type::IntLiteral || ident.type == Type::FloatLiteral) ? ident.name : term->token.value;
        // std::cout<<"Lval------------\n---v:"<< root->v<<"->"<< symbol_table.get_operand(root->v).name << "\nprimaryExp.t:" << toString(root->t) <<"\nprimaryExp.is_computable:"<<root->is_computable<< std::endl;
    }
    else if (root->children.size() == 4)
    {
        // LVal -> Ident '[' Exp ']'
        // 此时Ident是指针,但是可能是一维数组/二维数组
        // 标记数组名
        root->arr_name = ident.name;
        root->is_arr_element = true;
        ANALYSIS(exp, Exp, 2);
        if (ident_ste.dimension.size() == 1)
        {
            // 一维数组，元素是int/float
            auto element_type = (ident.type == Type::IntPtr) ? Type::Int : Type::Float;
            auto dim = exp->is_computable ? Operand(exp->v, exp->t) : symbol_table.get_operand(exp->v);
            // 可能需要深拷贝
            root->i = dim;
            if (root->parent->type == NodeType::PRIMARYEXP)
            {
                // 是右值
                auto des = Operand("t~" + std::to_string(tmp_cnt++), element_type);
                buffer.push_back(new Instruction(ident, dim, des, Operator::load));
                symbol_table.scope_stack.back().table.insert({des.name, {des}});
                root->t = des.type;
                root->v = des.name;
                root->is_computable = false;
            }
            else
            {
                // 是左值
                // 数组类型
                root->t = ident.type;
                // 数组原名
                root->v = term->token.value;
            }
        }
        else
        {
            // std::cout << ident_ste.operand.name << " : " << ident_ste.dimension.size() << std::endl;
            assert(ident_ste.dimension.size() == 2);
            // 二维数组，元素是intptr/floatptr
            auto element_type = ident.type;
            // 第二维size
            auto sec_size = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
            // 当前所需的第一维idx
            auto fir_dim = Operand(exp->v, exp->t);
            auto offset = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
            auto des = Operand("t~" + std::to_string(tmp_cnt++), element_type);
            // 获取第二维的size
            buffer.push_back(new Instruction({std::to_string(ident_ste.dimension[1]), Type::IntLiteral}, Operand(), sec_size, Operator::def));
            // 计算偏移量
            if (exp->t == Type::IntLiteral)
            {
                // 立即数转变量
                auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                buffer.push_back(new Instruction(fir_dim, Operand(), tmp, Operator::def));
                buffer.push_back(new Instruction(sec_size, tmp, offset, Operator::mul));
            }
            else
            {
                assert(exp->t == Type::Int);
                buffer.push_back(new Instruction(sec_size, fir_dim, offset, Operator::mul));
            }
            // 执行
            buffer.push_back(new Instruction(ident, offset, des, Operator::getptr));
            // symbol_table.scope_stack.back().table.insert({des.name, {des}});
            root->t = des.type;
            root->v = des.name;
        }
    }
    else
    {
        root->arr_name = ident.name;
        root->is_arr_element = true;
        assert(root->children.size() == 7);
        // 简便起见，只考虑到二维数组
        // LVal -> Ident '[' Exp ']' '[' Exp ']'
        ANALYSIS(exp1, Exp, 2);
        ANALYSIS(exp2, Exp, 5);
        auto element_type = (ident.type == Type::IntPtr) ? Type::Int : Type::Float;
        // 第二维size
        auto sec_size = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        // 当前所需的第一维idx
        auto fir_dim = exp1->is_computable ? Operand(exp1->v, exp1->t) : symbol_table.get_operand(exp1->v);;
        // 当前所需的第二维idx
        auto sec_dim = exp2->is_computable ? Operand(exp2->v, exp2->t) : symbol_table.get_operand(exp2->v);
        // 到达元素所在行的偏移量
        auto offset1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        // 元素所在行内的偏移量
        auto offset2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        // 总偏移量
        auto offset = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        auto des = Operand("t~" + std::to_string(tmp_cnt++), element_type);
        // 获取第二维的size
        buffer.push_back(new Instruction({std::to_string(ident_ste.dimension[1]), Type::IntLiteral}, Operand(), sec_size, Operator::def));
        // 计算到达元素所在行的偏移量
        if (exp1->t == Type::IntLiteral)
        {
            // 立即数转变量
            auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
            buffer.push_back(new Instruction(fir_dim, Operand(), tmp, Operator::def));
            buffer.push_back(new Instruction(sec_size, tmp, offset1, Operator::mul));
        }
        else
        {
            assert(exp1->t == Type::Int);
            buffer.push_back(new Instruction(sec_size, fir_dim, offset1, Operator::mul));
        }
        // 计算元素所在行内的偏移量偏移量
        if (exp2->t == Type::IntLiteral)
        {
            // 立即数转变量
            buffer.push_back(new Instruction(sec_dim, Operand(), offset2, Operator::def));
        }
        else
        {
            assert(exp2->t == Type::Int);
            offset2 = sec_dim;
        }
        // 计算总偏移量
        buffer.push_back(new Instruction(offset1, offset2, offset, Operator::add));
        if (root->parent->type == NodeType::PRIMARYEXP){
            // 是右值
            // 获取对应元素
            buffer.push_back(new Instruction(ident, offset, des, Operator::load));
            symbol_table.scope_stack.back().table.insert({des.name, {des}});
            root->t = des.type;
            root->v = des.name;
            root->is_computable = false; 
        }
        else
        {
            // 是左值
            // 数组类型
            root->t = ident.type;
            // 数组原名
            root->v = term->token.value;
            root->i = offset;
        }
    }
}

void frontend::Analyzer::analysisNumber(Number *root, vector<ir::Instruction *> &buffer)
{
    /*
    Number -> IntConst | floatConst
        PrimaryExp.is_computable
        PrimaryExp.v
        PrimaryExp.t
    */
    root->is_computable = true;
    GET_CHILD_PTR(term, Term, 0);
    // 类型检查,统一转换为十进制
    string value = term->token.value;
    TokenType tp = term->token.type;
    if (tp == TokenType::INTLTR)
    {
        // 处理进制问题
        if (value.length() > 1)
        {
            if (value[0] == '0')
            { // 特殊进制
                if (value[1] == 'b')
                {
                    // 二进制
                    value = std::to_string(stoi(value.substr(2, value.length() - 2), 0, 2));
                }
                else if (value[1] == 'x')
                {
                    // 十六进制
                    value = std::to_string(stoi(value.substr(2, value.length() - 2), 0, 16));
                }
                else
                {
                    // 八进制
                    value = std::to_string(stoi(value.substr(1, value.length() - 1), 0, 8));
                }
            }
        }
    }
    root->v = value;
    root->t = (tp == TokenType::INTLTR) ? Type::IntLiteral : Type::FloatLiteral;
    // std::cout << "number.v: " << root->v << "\nnumber.t:" << toString(root->t) << std::endl;
}

void frontend::Analyzer::analysisPrimaryExp(PrimaryExp *root, vector<ir::Instruction *> &buffer)
{
    /*
    PrimaryExp -> '(' Exp ')' | LVal | Number
        PrimaryExp.is_computable
        PrimaryExp.v
        PrimaryExp.t
    */
    if (root->children[0]->type == NodeType::TERMINAL)
    {
        // PrimaryExp -> '(' Exp ')'
        ANALYSIS(exp, Exp, 1);
        COPY_EXP_NODE(exp, root);
    }
    else if (root->children[0]->type == NodeType::LVAL)
    {
        // PrimaryExp -> LVal
        ANALYSIS(lval, LVal, 0);
        COPY_EXP_NODE(lval, root);
        // std::cout << "primaryExp.v: " << root->v << "\nprimaryExp.t:" << toString(root->t) <<"\nprimaryExp.is_computable:"<<root->is_computable<< std::endl;
    }
    else
    {
        // PrimaryExp -> Number
        assert(root->children[0]->type == NodeType::NUMBER);
        ANALYSIS(number, Number, 0);
        COPY_EXP_NODE(number, root);
        // std::cout << "primaryExp.v: " << root->v << "\nprimaryExp.t:" << toString(root->t) <<"\nprimaryExp.is_computable:"<<root->is_computable<< std::endl;
    }
}

void frontend::Analyzer::analysisUnaryExp(UnaryExp *root, vector<ir::Instruction *> &buffer)
{
    /*
    UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
        UnaryExp.is_computable
        UnaryExp.v
        UnaryExp.t
    */
    if (root->children[0]->type == NodeType::PRIMARYEXP)
    {
        // UnaryExp -> PrimaryExp
        ANALYSIS(primaryexp, PrimaryExp, 0);
        COPY_EXP_NODE(primaryexp, root);
        // std::cout << "unaryExp.v: " << root->v << "\nunaryExp.t:" << toString(root->t) <<"\nunaryExp.is_computable:"<<root->is_computable<< std::endl;
    }
    else if (root->children[0]->type == NodeType::TERMINAL)
    {
        // UnaryExp -> Ident '(' [FuncRParams] ')'
        // 函数调用
        root->is_computable = false;
        GET_CHILD_PTR(func, Term, 0);
        string func_name = func->token.value;
        Type return_type = symbol_table.functions[func_name]->returnType;
        // std::cout << "returnType:" << toString(return_type) << "\n";
        if (root->children.size() > 3)
        {
            // 有实参
            GET_CHILD_PTR(funcps, FuncRParams, 2);
            // 生成空callinst
            auto des = Operand("t~" + std::to_string(tmp_cnt++), return_type);
            auto call = new ir::CallInst({func_name, return_type}, {}, des);
            // 在FuncRParmas中处理call指令
            analysisFuncRParams(funcps, buffer, *call);
            buffer.push_back(call);
            // 向符号表中添加des
            symbol_table.scope_stack.back().table.insert({des.name, {des}});
            root->v = des.name;
        }
        else
        {
            // 无实参
            auto des = Operand("t~" + std::to_string(tmp_cnt++), return_type);
            buffer.push_back(new ir::CallInst({func_name, return_type}, des));
            root->v = des.name;
            // 向符号表中添加des
            symbol_table.scope_stack.back().table.insert({des.name, {des}});
        }
        root->t = return_type;
    }
    else
    {
        // UnaryExp -> UnaryOp UnaryExp
        assert(root->children[0]->type == NodeType::UNARYOP);
        GET_CHILD_PTR(uop, UnaryOp, 0);
        ANALYSIS(unaryexp, UnaryExp, 1);
        auto src = unaryexp->is_computable ? Operand(unaryexp->v, unaryexp->t) : symbol_table.get_operand(unaryexp->v);
        auto uop_child = dynamic_cast<Term *>(uop->children[0]);
        if (uop_child->token.type == TokenType::PLUS)
        {
            // '+'
            COPY_EXP_NODE(unaryexp, root);
        }
        else if (uop_child->token.type == TokenType::MINU)
        {
            // '-'
            if (unaryexp->t == Type::IntLiteral)
            {
                root->v = std::to_string(-stoi(unaryexp->v));
                root->t = unaryexp->t;
                root->is_computable = true;
            }
            else if (unaryexp->t == Type::FloatLiteral)
            {
                root->v = std::to_string(-stof(unaryexp->v));
                root->t = unaryexp->t;
                root->is_computable = true;
            }
            else if (unaryexp->t == Type::Int)
            {
                auto zero = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                buffer.push_back(new Instruction(Operand("0", Type::IntLiteral), Operand(), zero, Operator::def));
                auto des = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                buffer.push_back(new Instruction(zero, src, des, Operator::sub));
                root->v = des.name;
                root->t = des.type;
                root->is_computable = false;
                // 告知符号表
                symbol_table.scope_stack.back().table.insert({des.name, {des}});
            }
            else if (unaryexp->t == Type::Float)
            {
                auto zero = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                buffer.push_back(new Instruction(Operand("0", Type::FloatLiteral), Operand(), zero, Operator::fdef));
                auto des = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                buffer.push_back(new Instruction(zero, src, des, Operator::fsub));
                root->v = des.name;
                root->t = des.type;
                root->is_computable = false;
                // 告知符号表
                symbol_table.scope_stack.back().table.insert({des.name, {des}});
            }
        }
        else
        {
            // '!'
            assert(uop_child->token.type == TokenType::NOT);
            if (unaryexp->t == Type::IntLiteral)
            {
                root->v = std::to_string(!std::stoi(unaryexp->v));
                root->t = Type::IntLiteral;
                root->is_computable = true;
            }
            else if (unaryexp->t == Type::FloatLiteral)
            {
                root->v = std::to_string(!std::stof(unaryexp->v));
                root->t = Type::FloatLiteral;
                root->is_computable = true;
            }
            else if (unaryexp->t == Type::Int)
            {
                auto des = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                buffer.push_back(new Instruction(src, Operand(), des, Operator::_not));
                root->v = des.name;
                root->t = des.type;
                root->is_computable = false;
                // 告知符号表
                symbol_table.scope_stack.back().table.insert({des.name, {des}});
            }
            else
            {
                assert(unaryexp->t == Type::Float);
                auto des = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                buffer.push_back(new Instruction(src, Operand(), des, Operator::_not));
                root->v = des.name;
                root->t = des.type;
                root->is_computable = false;
                // 告知符号表
                symbol_table.scope_stack.back().table.insert({des.name, {des}});
            }
        }
    }
}

// void frontend::Analyzer::analysisUnaryOp(UnaryOp *root, vector<ir::Instruction *> &buffer)
// {
//     /*
//     UnaryOp -> '+' | '-' | '!'
//     */
//     // 感觉没必要
// }

void frontend::Analyzer::analysisFuncRParams(FuncRParams *root, vector<ir::Instruction *> &buffer, ir::CallInst &callinst)
{
    // 函数实参表
    // FuncRParams -> Exp { ',' Exp }
    // vector<Operand> params;
    int idx = 0, p_idx = 0;
    for (; idx < root->children.size(); idx += 2, p_idx++)
    {
        ANALYSIS(exp, Exp, idx);
        // param type
        Type ptp = exp->t;
        // 从callinst中获取当前函数的名称
        string func_name = callinst.op1.name;
        // 获取定义函数时约定的参数
        // auto p_need = symbol_table.functions[func_name]->ParameterList;
        
        // 生成实参
        auto param = (ptp == Type::FloatLiteral || ptp == Type::IntLiteral) ? Operand(exp->v, exp->t) : symbol_table.get_operand(exp->v);
        // if (ptp == Type::IntLiteral || ptp == Type::FloatLiteral)
        // {
        //     param = Operand(exp->v, exp->t);
        // }
        // else
        // {
        //     // string new_name = symbol_table.get_scoped_name(exp->v);
        //     param = Operand(new_name, exp->t);
        // }
        // 类型检查
        // if (ptp == p_need[p_idx].type)
        // {
        // 类型相符
        // std::cout<<param.name<<" "<<toString(param.type)<<std::endl;
        callinst.argumentList.push_back(param);
        // }
        // else
        // {
        //     // 不匹配
        //     if (p_need[p_idx].type == Type::Int)
        //     {
        //         if (ptp == Type::IntLiteral)
        //         {
        //             Operand tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        //             buffer.push_back(new Instruction(param, Operand(), tmp, Operator::def));
        //             param = tmp;
        //         }
        //         else if (ptp == Type::Float)
        //         {
        //             Operand tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        //             buffer.push_back(new Instruction(param, Operand(), tmp, Operator::cvt_f2i));
        //             param = tmp;
        //         }
        //         else if (ptp == Type::FloatLiteral)
        //         {
        //             Operand tmp1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
        //             buffer.push_back(new Instruction(param, Operand(), tmp1, Operator::fdef));
        //             Operand tmp2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        //             buffer.push_back(new Instruction(param, Operand(), tmp2, Operator::cvt_f2i));
        //             param = tmp2;
        //         }
        //     }
        //     else if (p_need[p_idx].type == Type::Float)
        //     {
        //         if (ptp == Type::FloatLiteral)
        //         {
        //             Operand tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
        //             buffer.push_back(new Instruction(param, Operand(), tmp, Operator::fdef));
        //             param = tmp;
        //         }
        //         else if (ptp == Type::Int)
        //         {
        //             Operand tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
        //             buffer.push_back(new Instruction(param, Operand(), tmp, Operator::cvt_i2f));
        //             param = tmp;
        //         }
        //         else if (ptp == Type::IntLiteral)
        //         {
        //             Operand tmp1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        //             buffer.push_back(new Instruction(param, Operand(), tmp1, Operator::def));
        //             Operand tmp2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
        //             buffer.push_back(new Instruction(param, Operand(), tmp2, Operator::cvt_i2f));
        //             param = tmp2;
        //         }
        //     }
        //     callinst.argumentList.push_back(param);
        // }
        // std::cout << "GET Params\n";
        // callinst.argumentList = params;
    }
}

void frontend::Analyzer::analysisMulExp(MulExp *root, vector<ir::Instruction *> &buffer)
{
    /*
    MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
        MulExp.is_computable
        MulExp.v
        MulExp.t
    */
    ANALYSIS(uexp, UnaryExp, 0);
    // 暂时先赋值第一个子节点的值
    COPY_EXP_NODE(uexp, root);
    if (root->children.size() > 1)
    {
        // 待写入IR的addexp结果
        Operand result;
        result.name = "t~" + std::to_string(tmp_cnt++);
        for (int idx = 2; idx < root->children.size(); idx += 2)
        {
            // 获取操作类型
            TokenType optp = dynamic_cast<Term *>(root->children[idx - 1])->token.type;
            ANALYSIS(uexp, UnaryExp, idx);
            // auto child = Operand(uexp->v, uexp->t);
            // 右节点[IR中的]
            auto child = uexp->is_computable ? Operand(uexp->v, uexp->t) : symbol_table.get_operand(uexp->v);
            // std::cout << "--child.name:" << child.name << "\nchild.type:" << toString(child.type) << "\n";
            // auto pa = Operand(root->v, root->t);
            // 左节点[IR中的]
            auto pa = root->is_computable ? Operand(root->v, root->t) : symbol_table.get_operand(root->v);
            // std::cout << "--pa.name:" << pa.name << "\npa.type:" << toString(pa.type) << "\n";
            Operand op1;
            Operand op2;

            // 处理is_computable:出现不是立即数的直接置为false，否则保持不变
            root->is_computable = uexp->is_computable ? root->is_computable : false;

            // 处理t，并做类型检查
            if (pa.type == Type::Int)
            {
                if (child.type == Type::Int)
                {
                    // do nothing
                    op1 = pa;
                    op2 = child;
                    root->t = Type::Int;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;
                    // op2立即数转变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::def));
                    root->t = Type::Int;
                }
                else if (child.type == Type::Float)
                {
                    op2 = child;
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // op2立即数变变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
            }
            else if (pa.type == Type::IntLiteral)
            {
                if (child.type == Type::Int)
                {
                    // op1立即数变变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::def));
                    op2 = child;
                    root->t = Type::Int;
                }
                else if (child.type == Type::IntLiteral)
                {
                    // // op1立即数变变量
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(pa, Operand(), op1, Operator::def));
                    // // op2立即数变变量
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(child, Operand(), op2, Operator::def));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::IntLiteral;
                }
                else if (child.type == Type::Float)
                {
                    // op1立即数变变量
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(pa, Operand(), tmp, Operator::def));
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));
                    op2 = child;
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // // op1立即数转变量
                    // auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(pa, Operand(), tmp, Operator::def));
                    // // op1->float
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(tmp, Operand(), op1, Operator::cvt_i2f));
                    // // op2立即数转变量
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::FloatLiteral;
                }
            }
            else if (pa.type == Type::Float)
            {
                if (child.type == Type::Int)
                {
                    op1 = pa;
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;
                    // op2立即数转变量
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(child, Operand(), tmp, Operator::def));
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(tmp, Operand(), op2, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else if (child.type == Type::Float)
                {
                    // do nothing
                    op1 = pa;
                    op2 = child;
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    op1 = pa;
                    // op2立即数转变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    root->t = Type::Float;
                }
            }
            else
            {
                assert(pa.type == Type::FloatLiteral);
                if (child.type == Type::Int)
                {
                    // op1立即数转变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else if (child.type == Type::IntLiteral)
                {
                    // // op1立即数转变量
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    // // op2立即数转变量
                    // auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(child, Operand(), tmp, Operator::def));
                    // // op2->float
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(tmp, Operand(), op2, Operator::cvt_i2f));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::FloatLiteral;
                }
                else if (child.type == Type::Float)
                {
                    // op1立即数转变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    op2 = child;
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // // op1立即数转变量
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    // // op2立即数转变量
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::FloatLiteral;
                }
            }
            result.type = root->t;

            // 处理操作符,并赋值
            if (optp == TokenType::MULT)
            {
                if (root->t == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::mul));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                }
                else if (root->t == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::fmul));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    root->v = std::to_string(stoi(op1.name) * stoi(op2.name));
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    root->v = std::to_string(stof(op1.name) * stof(op2.name));
                }
            }
            else if (optp == TokenType::DIV)
            {
                if (root->t == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::div));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                }
                else if (root->t == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::fdiv));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    root->v = std::to_string(stoi(op1.name) / stoi(op2.name));
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    root->v = std::to_string(stof(op1.name) / stof(op2.name));
                }
            }
            else
            {
                assert(optp == TokenType::MOD);
                if (root->t == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::mod));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                }
                else
                {
                    assert(root->t == Type::IntLiteral);
                    root->v = std::to_string(stoi(op1.name) % stoi(op2.name));
                }
            }
        }
    }
    // std::cout << "mulExp.v: " << root->v << "\nmulExp.t:" << toString(root->t) <<"\nmulExp.is_computable:"<<root->is_computable<< std::endl;
}

void frontend::Analyzer::analysisAddExp(AddExp *root, vector<ir::Instruction *> &buffer)
{
    /*
    AddExp -> MulExp { ('+' | '-') MulExp }
        AddExp.is_computable
        AddExp.v
        AddExp.t
    */
    ANALYSIS(mulexp, MulExp, 0);
    // 暂时先赋值第一个子节点的值
    COPY_EXP_NODE(mulexp, root);
    if (root->children.size() > 1)
    {
        // 待写入IR的addexp结果
        Operand result;
        result.name = "t~" + std::to_string(tmp_cnt++);
        for (int idx = 2; idx < root->children.size(); idx += 2)
        {
            // 获取操作类型
            TokenType optp = dynamic_cast<Term *>(root->children[idx - 1])->token.type;
            ANALYSIS(mulexp, MulExp, idx);
            // auto child = Operand(mulexp->v, mulexp->t);
            // 右节点[IR中的]
            auto child = mulexp->is_computable ? Operand(mulexp->v, mulexp->t) : symbol_table.get_operand(mulexp->v);
            // std::cout << "--child.name:" << child.name << "\nchild.type:" << toString(child.type) << "\n";
            // auto pa = Operand(root->v, root->t);
            // 左节点[IR中的]
            auto pa = root->is_computable ? Operand(root->v, root->t) : symbol_table.get_operand(root->v);
            // std::cout << "--pa.name:" << pa.name << "\npa.type:" << toString(pa.type) << "\n";
            Operand op1;
            Operand op2;

            // 处理is_computable:出现不是立即数的直接置为false，否则保持不变
            root->is_computable = mulexp->is_computable ? root->is_computable : false;

            // 处理t，并做类型检查
            if (pa.type == Type::Int)
            {
                if (child.type == Type::Int)
                {
                    // do nothing
                    op1 = pa;
                    op2 = child;
                    root->t = Type::Int;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;
                    // op2立即数转变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::def));
                    root->t = Type::Int;
                }
                else if (child.type == Type::Float)
                {
                    op2 = child;
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // op2立即数变变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
            }
            else if (pa.type == Type::IntLiteral)
            {
                if (child.type == Type::Int)
                {
                    // op1立即数变变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::def));
                    op2 = child;
                    root->t = Type::Int;
                }
                else if (child.type == Type::IntLiteral)
                {
                    // // op1立即数变变量
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(pa, Operand(), op1, Operator::def));
                    // // op2立即数变变量
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(child, Operand(), op2, Operator::def));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::IntLiteral;
                }
                else if (child.type == Type::Float)
                {
                    // op1立即数变变量
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(pa, Operand(), tmp, Operator::def));
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));
                    op2 = child;
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // // op1立即数转变量
                    // auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(pa, Operand(), tmp, Operator::def));
                    // // op1->float
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(tmp, Operand(), op1, Operator::cvt_i2f));
                    // // op2立即数转变量
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::FloatLiteral;
                }
            }
            else if (pa.type == Type::Float)
            {
                if (child.type == Type::Int)
                {
                    op1 = pa;
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;
                    // op2立即数转变量
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(child, Operand(), tmp, Operator::def));
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(tmp, Operand(), op2, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else if (child.type == Type::Float)
                {
                    // do nothing
                    op1 = pa;
                    op2 = child;
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    op1 = pa;
                    // op2立即数转变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    root->t = Type::Float;
                }
            }
            else
            {
                assert(pa.type == Type::FloatLiteral);
                if (child.type == Type::Int)
                {
                    // op1立即数转变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else if (child.type == Type::IntLiteral)
                {
                    // // op1立即数转变量
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    // // op2立即数转变量
                    // auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(child, Operand(), tmp, Operator::def));
                    // // op2->float
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(tmp, Operand(), op2, Operator::cvt_i2f));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::FloatLiteral;
                }
                else if (child.type == Type::Float)
                {
                    // op1立即数转变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    op2 = child;
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // // op1立即数转变量
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    // // op2立即数转变量
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::FloatLiteral;
                }
            }
            result.type = root->t;

            // 处理操作符,并赋值
            if (optp == TokenType::PLUS)
            {
                if (root->t == Type::Int)
                {
                    // std::cout << "PLUS INT\n   RESULT in "<<result.name<<"\n";
                    buffer.push_back(new Instruction(op1, op2, result, Operator::add));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                }
                else if (root->t == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::fadd));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    root->v = std::to_string(stoi(op1.name) + stoi(op2.name));
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    root->v = std::to_string(stof(op1.name) + stof(op2.name));
                }
            }
            else
            {
                assert(optp == TokenType::MINU);
                if (root->t == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::sub));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                }
                else if (root->t == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::fsub));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    root->v = std::to_string(stoi(op1.name) - stoi(op2.name));
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    root->v = std::to_string(stof(op1.name) - stof(op2.name));
                }
            }
        }
    }
    // std::cout << "addExp.v: " << root->v << "\naddExp.t:" << toString(root->t) <<"\naddExp.is_computable:"<<root->is_computable<< std::endl;
}

void frontend::Analyzer::analysisRelExp(RelExp *root, vector<ir::Instruction *> &buffer)
{
    /*
    RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
        RelExp.is_computable
        RelExp.v
        RelExp.t
    */
    ANALYSIS(addexp, AddExp, 0);
    // 暂时先赋值第一个子节点的值
    COPY_EXP_NODE(addexp, root);
    if (root->children.size() > 1)
    {
        // 待写入IR的relexp结果
        Operand result;
        result.name = "t~" + std::to_string(tmp_cnt++);
        for (int idx = 2; idx < root->children.size(); idx += 2)
        {
            // 获取操作类型
            TokenType optp = dynamic_cast<Term *>(root->children[idx - 1])->token.type;
            ANALYSIS(addexp, AddExp, idx);
            // auto child = Operand(mulexp->v, mulexp->t);
            // 右节点[IR中的]
            auto child = addexp->is_computable ? Operand(addexp->v, addexp->t) : symbol_table.get_operand(addexp->v);
            // std::cout << "--child.name:" << child.name << "\nchild.type:" << toString(child.type) << "\n";
            // auto pa = Operand(root->v, root->t);
            // 左节点[IR中的]
            auto pa = root->is_computable ? Operand(root->v, root->t) : symbol_table.get_operand(root->v);
            // std::cout << "--pa.name:" << pa.name << "\npa.type:" << toString(pa.type) << "\n";
            Operand op1;
            Operand op2;

            // 处理is_computable:出现不是立即数的直接置为false，否则保持不变
            root->is_computable = addexp->is_computable ? root->is_computable : false;

            // 处理t，并做类型检查
            if (pa.type == Type::Int)
            {
                if (child.type == Type::Int)
                {
                    // do nothing
                    op1 = pa;
                    op2 = child;
                    root->t = Type::Int;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;
                    // op2立即数转变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::def));
                    root->t = Type::Int;
                }
                else if (child.type == Type::Float)
                {
                    op2 = child;
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // op2立即数变变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
            }
            else if (pa.type == Type::IntLiteral)
            {
                if (child.type == Type::Int)
                {
                    // op1立即数变变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::def));
                    op2 = child;
                    root->t = Type::Int;
                }
                else if (child.type == Type::IntLiteral)
                {
                    // // op1立即数变变量
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(pa, Operand(), op1, Operator::def));
                    // // op2立即数变变量
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(child, Operand(), op2, Operator::def));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::IntLiteral;
                }
                else if (child.type == Type::Float)
                {
                    // op1立即数变变量
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(pa, Operand(), tmp, Operator::def));
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));
                    op2 = child;
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // // op1立即数转变量
                    // auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(pa, Operand(), tmp, Operator::def));
                    // // op1->float
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(tmp, Operand(), op1, Operator::cvt_i2f));
                    // // op2立即数转变量
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::FloatLiteral;
                }
            }
            else if (pa.type == Type::Float)
            {
                if (child.type == Type::Int)
                {
                    op1 = pa;
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;
                    // op2立即数转变量
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(child, Operand(), tmp, Operator::def));
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(tmp, Operand(), op2, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else if (child.type == Type::Float)
                {
                    // do nothing
                    op1 = pa;
                    op2 = child;
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    op1 = pa;
                    // op2立即数转变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    root->t = Type::Float;
                }
            }
            else
            {
                assert(pa.type == Type::FloatLiteral);
                if (child.type == Type::Int)
                {
                    // op1立即数转变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::cvt_i2f));
                    root->t = Type::Float;
                }
                else if (child.type == Type::IntLiteral)
                {
                    // // op1立即数转变量
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    // // op2立即数转变量
                    // auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    // buffer.push_back(new Instruction(child, Operand(), tmp, Operator::def));
                    // // op2->float
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(tmp, Operand(), op2, Operator::cvt_i2f));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::FloatLiteral;
                }
                else if (child.type == Type::Float)
                {
                    // op1立即数转变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    op2 = child;
                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // // op1立即数转变量
                    // op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));
                    // // op2立即数转变量
                    // op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    // buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));
                    op1 = pa;
                    op2 = child;
                    root->t = Type::FloatLiteral;
                }
            }
            result.type = root->t;

            // 处理操作符,并赋值
            if (optp == TokenType::LSS)
            {
                if (root->t == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::lss));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::flss));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    root->v = stoi(op1.name) < stoi(op2.name) ? "1" : "0";
                    root->t = Type::IntLiteral;
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    root->v = stof(op1.name) < stof(op2.name) ? "1" : "0";
                    root->t = Type::IntLiteral;
                }
            }
            else if (optp == TokenType::GTR)
            {
                if (root->t == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::gtr));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::fgtr));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    root->v = stoi(op1.name) > stoi(op2.name) ? "1" : "0";
                    root->t = Type::IntLiteral;
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    root->v = stof(op1.name) > stof(op2.name) ? "1" : "0";
                    root->t = Type::IntLiteral;
                }
            }
            else if (optp == TokenType::LEQ)
            {
                if (root->t == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::leq));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::fleq));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    root->v = stoi(op1.name) <= stoi(op2.name) ? "1" : "0";
                    root->t = Type::IntLiteral;
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    root->v = stof(op1.name) <= stof(op2.name) ? "1" : "0";
                    root->t = Type::IntLiteral;
                }
            }
            else
            {
                assert(optp == TokenType::GEQ);
                if (root->t == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::geq));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::geq));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    root->v = stoi(op1.name) >= stoi(op2.name) ? "1" : "0";
                    root->t = Type::IntLiteral;
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    root->v = stof(op1.name) >= stof(op2.name) ? "1" : "0";
                    root->t = Type::IntLiteral;
                }
            }
        }
    }
}

void frontend::Analyzer::analysisEqExp(EqExp *root, vector<ir::Instruction *> &buffer)
{
    /*
    EqExp -> RelExp { ('==' | '!=') RelExp }
        EqExp.is_computable
        EqExp.v
        EqExp.t
    */
    /*
         修改思路：结果过滤为int变量
    */
    ANALYSIS(relexp, RelExp, 0);
    // 暂时先赋值第一个子节点的值
    COPY_EXP_NODE(relexp, root);
    if (root->children.size() == 1)
    {
        // EqExp -> RelExp
        // 类型检查->int
        root->is_computable = false;
        root->t = Type::Int;

        auto src = relexp->is_computable ? Operand(relexp->v, relexp->t) : symbol_table.get_operand(relexp->v);
        if (relexp->t == Type::IntLiteral)
        {
            auto des = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
            buffer.push_back(new Instruction(src, {}, des, Operator::def));
            root->v = des.name;
            symbol_table.scope_stack.back().table.insert({des.name, {des}});
        }
        else if (relexp->t == Type::FloatLiteral)
        {
            auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
            auto des = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
            buffer.push_back(new Instruction(src, {}, tmp, Operator::fdef));
            buffer.push_back(new Instruction(tmp, {}, des, Operator::cvt_f2i));
            root->v = des.name;
            symbol_table.scope_stack.back().table.insert({des.name, {des}});
        }
        else if (relexp->t == Type::Float)
        {
            auto des = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
            buffer.push_back(new Instruction(src, {}, des, Operator::cvt_f2i));
            root->v = des.name;
            symbol_table.scope_stack.back().table.insert({des.name, {des}});
        }
        else
        {
            assert(relexp->t == Type::Int);
            root->v = relexp->v;
        }
    }
    else
    {
        // EqExp -> RelExp { ('==' | '!=') RelExp }+
        // 待写入IR的addexp结果
        // Operand result;
        // result.name = "t~" + std::to_string(tmp_cnt++);
        auto result = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        for (int idx = 2; idx < root->children.size(); idx += 2)
        {
            // 获取操作类型 == / !=
            TokenType optp = dynamic_cast<Term *>(root->children[idx - 1])->token.type;
            ANALYSIS(relexp, RelExp, idx);
            // 右节点[IR中的]
            auto child = relexp->is_computable ? Operand(relexp->v, relexp->t) : symbol_table.get_operand(relexp->v);
            // 左节点[IR中的]
            auto pa = root->is_computable ? Operand(root->v, root->t) : symbol_table.get_operand(root->v);
            Operand op1;
            Operand op2;
            // 类型检查后op1/op2的类型
            Type common_type;

            root->is_computable = false;

            // 处理t，并做类型检查
            if (pa.type == Type::Int)
            {
                if (child.type == Type::Int)
                {
                    // do nothing
                    op1 = pa;

                    op2 = child;

                    common_type = Type::Int;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;

                    // op2立即数转变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::def));

                    common_type = Type::Int;
                }
                else if (child.type == Type::Float)
                {
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));

                    op2 = child;

                    common_type = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::cvt_i2f));

                    // op2立即数变变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));

                    common_type = Type::Float;
                }
            }
            else if (pa.type == Type::IntLiteral)
            {
                if (child.type == Type::Int)
                {
                    // op1立即数变变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::def));

                    op2 = child;

                    common_type = Type::Int;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;

                    op2 = child;

                    common_type = Type::IntLiteral;
                }
                else if (child.type == Type::Float)
                {
                    // op1立即数变变量
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(pa, Operand(), tmp, Operator::def));
                    // op1->float
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(tmp, Operand(), op1, Operator::cvt_i2f));

                    op2 = child;

                    common_type = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    op1 = pa;
                    op1.type = Type::FloatLiteral;

                    op2 = child;

                    common_type = Type::FloatLiteral;
                }
            }
            else if (pa.type == Type::Float)
            {
                if (child.type == Type::Int)
                {
                    op1 = pa;

                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::cvt_i2f));

                    common_type = Type::Float;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;

                    // op2立即数转变量
                    auto tmp = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(child, Operand(), tmp, Operator::def));
                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(tmp, Operand(), op2, Operator::cvt_i2f));

                    common_type = Type::Float;
                }
                else if (child.type == Type::Float)
                {
                    // do nothing
                    op1 = pa;

                    op2 = child;

                    root->t = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    op1 = pa;

                    // op2立即数转变量
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::fdef));

                    common_type = Type::Float;
                }
            }
            else
            {
                assert(pa.type == Type::FloatLiteral);
                if (child.type == Type::Int)
                {
                    // op1立即数转变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));

                    // op2->float
                    op2 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(child, Operand(), op2, Operator::cvt_i2f));

                    common_type = Type::Float;
                }
                else if (child.type == Type::IntLiteral)
                {
                    op1 = pa;

                    op2 = child;
                    op2.type = Type::FloatLiteral;

                    common_type = Type::FloatLiteral;
                }
                else if (child.type == Type::Float)
                {
                    // op1立即数转变量
                    op1 = Operand("t~" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(pa, Operand(), op1, Operator::fdef));

                    op2 = child;

                    common_type = Type::Float;
                }
                else
                {
                    assert(child.type == Type::FloatLiteral);
                    op1 = pa;

                    op2 = child;

                    common_type = Type::FloatLiteral;
                }
            }

            // 处理操作符,并赋值
            if (optp == TokenType::EQL)
            {
                if (common_type == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::eq));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (common_type == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::eq));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    buffer.push_back(new Instruction({stoi(op1.name) == stoi(op2.name) ? "1" : "0", Type::IntLiteral}, {}, result, Operator::def));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    // 直接计算
                    buffer.push_back(new Instruction({stof(op1.name) == stof(op2.name) ? "1" : "0", Type::IntLiteral}, {}, result, Operator::def));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
            }
            else
            {
                assert(optp == TokenType::NEQ);
                if (common_type == Type::Int)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::neq));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (common_type == Type::Float)
                {
                    buffer.push_back(new Instruction(op1, op2, result, Operator::neq));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else if (root->t == Type::IntLiteral)
                {
                    // 直接计算
                    buffer.push_back(new Instruction({stoi(op1.name) != stoi(op2.name) ? "1" : "0", Type::IntLiteral}, {}, result, Operator::def));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
                else
                {
                    assert(root->t == Type::FloatLiteral);
                    // 直接计算
                    buffer.push_back(new Instruction({stof(op1.name) != stof(op2.name) ? "1" : "0", Type::IntLiteral}, {}, result, Operator::def));
                    // 加入符号表
                    symbol_table.scope_stack.back().table.insert({result.name, {result}});
                    root->v = result.name;
                    root->t = Type::Int;
                }
            }
        }
    }
}

void frontend::Analyzer::analysisLAndExp(LAndExp *root, vector<ir::Instruction *> &buffer)
{
    /*
    LAndExp -> EqExp [ '&&' LAndExp ]
        LAndExp.is_computable
        LAndExp.v
        LAndExp.t
    */
    ANALYSIS(eqexp, EqExp, 0);
    // eqexp已将结果全部过滤为int变量
    assert(eqexp->t == Type::Int);
    COPY_EXP_NODE(eqexp, root);
    if (root->children.size() != 1)
    {
        // LAndExp -> EqExp '&&' LAndExp
        // 待写入IR的landexp结果
        auto result = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);

        // 确认操作类型
        assert(dynamic_cast<Term *>(root->children[1])->token.type == TokenType::AND);

        auto tmp = *new std::vector<Instruction *>;
        auto landexp = dynamic_cast<LAndExp *>(root->children[2]);
        assert(landexp);
        analysisLAndExp(landexp, tmp);

        // 右节点[IR中的]
        auto child = symbol_table.get_operand(landexp->v);

        // 左节点[IR中的]
        auto pa = symbol_table.get_operand(root->v);

        root->is_computable = false;

        // 处理操作符,并赋值
        // if (!op1) -> des = 0
        auto tmp_operand = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);
        buffer.push_back(new Instruction(pa, {}, tmp_operand, Operator::_not));
        buffer.push_back(new Instruction(tmp_operand, {}, {std::to_string(tmp.size() + 3), Type::IntLiteral}, Operator::_goto));
        // else des = child
        buffer.insert(buffer.end(), tmp.begin(), tmp.end());
        buffer.push_back(new Instruction(child, {}, result, Operator::mov));
        buffer.push_back(new Instruction({}, {}, {"2", Type::IntLiteral}, Operator::_goto));
        buffer.push_back(new Instruction({"0", Type::IntLiteral}, {}, result, Operator::def));

        // buffer.push_back(new Instruction(pa, child, result, Operator::_and));
        symbol_table.scope_stack.back().table.insert({result.name, {result}});
        root->v = result.name;
        root->t = result.type;
    }
}

void frontend::Analyzer::analysisLOrExp(LOrExp *root, vector<ir::Instruction *> &buffer)
{
    /*
    LOrExp -> LAndExp [ '||' LOrExp ]
        LOrExp.is_computable
        LOrExp.v
        LOrExp.t
    */
    ANALYSIS(landexp, LAndExp, 0);
    // eqexp已将结果全部过滤为int变量
    assert(landexp->t == Type::Int);
    COPY_EXP_NODE(landexp, root);
    if (root->children.size() != 1)
    {
        // LOrExp -> LAndExp '||' LOrExp
        // 待写入IR的lorexp结果
        auto result = Operand("t~" + std::to_string(tmp_cnt++), Type::Int);

        // 确认操作类型
        assert(dynamic_cast<Term *>(root->children[1])->token.type == TokenType::OR);

        auto tmp = *new std::vector<Instruction *>;
        auto lorexp = dynamic_cast<LOrExp *>(root->children[2]);
        assert(lorexp);
        analysisLOrExp(lorexp, tmp);

        // 右节点[IR中的]
        auto child = symbol_table.get_operand(lorexp->v);
        // 左节点[IR中的]
        auto pa = symbol_table.get_operand(root->v);

        root->is_computable = false;

        // 处理操作符,并赋值
        // if (pa) -> des = 1
        buffer.push_back(new Instruction(pa, {}, {std::to_string(tmp.size() + 3), Type::IntLiteral}, Operator::_goto));
        // else -> des = child
        buffer.insert(buffer.end(), tmp.begin(), tmp.end());
        buffer.push_back(new Instruction(child, {}, result, Operator::mov));
        buffer.push_back(new Instruction({}, {}, {"2", Type::IntLiteral}, Operator::_goto));
        buffer.push_back(new Instruction({"1", Type::IntLiteral}, {}, result, Operator::def));
        // buffer.push_back(new Instruction(pa, child, result, Operator::_or));
        symbol_table.scope_stack.back().table.insert({result.name, {result}});
        root->v = result.name;
        root->t = result.type;
    }
}

void frontend::Analyzer::analysisConstExp(ConstExp *root, vector<ir::Instruction *> &buffer)
{
    /*
    ConstExp -> AddExp
        ConstExp.is_computable: true
        ConstExp.v
        ConstExp.t
    */
    ANALYSIS(addexp, AddExp, 0);
    COPY_EXP_NODE(addexp, root);
    root->is_computable = true;
}
