#include "backend/generator.h"
#include <iostream>
#include <assert.h>
// #include "generator.h"

#define TODO assert(0 && "todo")
#define LOAD_RS1                                                                                                                      \
    int offset1 = stackmap.find_operand(inst.op1);                                                                                     \
    if (offset1 == -1)                                                                                                                 \
    { /* 未在局部变量中找到*/                                                                                                \
        if (find_operand_global(inst.op1))                                                                                            \
        { /* 在全局变量中找到，直接用标签 */                                                                            \
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LA) + "\t" + rv::toString(rs1) + "," + inst.op1.name + "\n");               \
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rs1) + ",0(" + rv::toString(rs1) + ")\n");        \
        }                                                                                                                             \
        else                                                                                                                          \
        { /* 在全局变量中未找到，报错 */                                                                                  \
            assert(0 && "Can Not Find The Operand1");                                                                                 \
        }                                                                                                                             \
    }                                                                                                                                 \
    else                                                                                                                              \
    { /* 已找到 */                                                                                                                 \
        tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rs1) + "," + std::to_string(offset1) + "(sp)" + "\n"); \
    }\
    // std::cout << "--\n"<< tmp_out << "--\n";
#define LOAD_RS2                                                                                                                      \
    int offset2 = stackmap.find_operand(inst.op2);                                                                                     \
    if (offset2 == -1)                                                                                                                 \
    { /* 未在局部变量中找到*/                                                                                                \
        if (find_operand_global(inst.op2))                                                                                            \
        { /* 在全局变量中找到，直接用标签 */                                                                            \
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LA) + "\t" + rv::toString(rs2) + "," + inst.op2.name + "\n");               \
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rs2) + "," + "0(" + rv::toString(rs2) + ")\n");   \
        }                                                                                                                             \
        else                                                                                                                          \
        { /* 在全局变量中未找到，报错 */                                                                                  \
            assert(0 && "Can Not Find The Operand2");                                                                                 \
        }                                                                                                                             \
    }                                                                                                                                 \
    else                                                                                                                              \
    { /* 已找到 */                                                                                                                 \
        tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rs2) + "," + std::to_string(offset2) + "(sp)" + "\n"); \
    }
#define LOAD_RD                                                                                                                      \
    int offsetd = stackmap.find_operand(inst.des);                                                                                    \
    if (offsetd == -1)                                                                                                                \
    { /* 未在局部变量中找到*/                                                                                               \
        if (find_operand_global(inst.des))                                                                                           \
        { /* 在全局变量中找到，直接用标签 */                                                                           \
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LA) + "\t" + rv::toString(rd) + "," + inst.des.name + "\n");               \
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rd) + "," + "0(" + rv::toString(rd) + ")\n");    \
        }                                                                                                                            \
        else                                                                                                                         \
        { /* 在全局变量中未找到，报错 */                                                                                 \
            assert(0 && "Can Not Find The Operand DES");                                                                             \
        }                                                                                                                            \
    }                                                                                                                                \
    else                                                                                                                             \
    { /* 已找到 */                                                                                                                \
        tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rd) + "," + std::to_string(offsetd) + "(sp)" + "\n"); \
    }
#define LOAD_RS1_RS2 \
    LOAD_RS1;        \
    LOAD_RS2;

#define SAVE_BACK_RD                                                                                                                 \
    int offset = stackmap.find_operand(inst.des);                                                                                    \
    if (offset == -1)                                                                                                                \
    { /* 未在局部变量中找到*/                                                                                               \
        if (find_operand_global(inst.des))                                                                                           \
        { /* 在全局变量中找到，直接用标签 */                                                                           \
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LA) + "\t" + rv::toString(rs1) + "," + inst.op1.name + "\n");              \
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + rv::toString(rd) + ",0(" + rv::toString(rs1) + ")\n");        \
        }                                                                                                                            \
        else                                                                                                                         \
        { /* 在全局变量中未找到，在局部变量中为其分配空间 */                                                   \
            offset = stackmap.add_operand(inst.des);                                                                                 \
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + rv::toString(rd) + "," + std::to_string(offset) + "(sp)\n");  \
        }                                                                                                                            \
    }                                                                                                                                \
    else                                                                                                                             \
    { /* 已找到 */                                                                                                                \
        tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + rv::toString(rd) + "," + std::to_string(offset) + "(sp)" + "\n"); \
    }

backend::Generator::Generator(ir::Program &p, std::ofstream &f) : program(p), fout(f)
{
    stackmap = *new stackVarMap;
}

std::string rv::toString(rvREG r)
{
    switch (r)
    {
    case rvREG::X0:
        return "zero";
        break;
    case rvREG::X1:
        return "ra";
        break;
    case rvREG::X2:
        return "sp";
        break;
    case rvREG::X3:
        return "gp";
        break;
    case rvREG::X4:
        return "tp";
        break;
    case rvREG::X5:
        return "t0";
        break;
    case rvREG::X6:
        return "t1";
        break;
    case rvREG::X7:
        return "t2";
        break;
    case rvREG::X8:
        return "s0";
        break;
    case rvREG::X9:
        return "s1";
        break;
    case rvREG::X10:
        return "a0";
        break;
    case rvREG::X11:
        return "a1";
        break;
    case rvREG::X12:
        return "a2";
        break;
    case rvREG::X13:
        return "a3";
        break;
    case rvREG::X14:
        return "a4";
        break;
    case rvREG::X15:
        return "a5";
        break;
    case rvREG::X16:
        return "a6";
        break;
    case rvREG::X17:
        return "a7";
        break;
    case rvREG::X18:
        return "s2";
        break;
    case rvREG::X19:
        return "s3";
        break;
    case rvREG::X20:
        return "s4";
        break;
    case rvREG::X21:
        return "s5";
        break;
    case rvREG::X22:
        return "s6";
        break;
    case rvREG::X23:
        return "s7";
        break;
    case rvREG::X24:
        return "s8";
        break;
    case rvREG::X25:
        return "s9";
        break;
    case rvREG::X26:
        return "s10";
        break;
    case rvREG::X27:
        return "s11";
        break;
    case rvREG::X28:
        return "t3";
        break;
    case rvREG::X29:
        return "t4";
        break;
    case rvREG::X30:
        return "t5";
        break;
    case rvREG::X31:
        return "t6";
        break;
    default:
        assert(0 && "No Such Kind Of rvREG");
        break;
    }
}

std::string rv::toString(rvOPCODE r)
{
    switch (r)
    {
    case rvOPCODE::ADD:
        return "ADD";
    case rvOPCODE::SUB:
        return "SUB";
    case rvOPCODE::XOR:
        return "XOR";
    case rvOPCODE::OR:
        return "OR";
    case rvOPCODE::AND:
        return "AND";
    case rvOPCODE::SLL:
        return "SLL";
    case rvOPCODE::SRL:
        return "SRL";
    case rvOPCODE::SRA:
        return "SRA";
    case rvOPCODE::SLT:
        return "SLT";
    case rvOPCODE::SLTU:
        return "SLTU";
    case rvOPCODE::ADDI:
        return "ADDI";
    case rvOPCODE::XORI:
        return "XORI";
    case rvOPCODE::ORI:
        return "ORI";
    case rvOPCODE::ANDI:
        return "ANDI";
    case rvOPCODE::SLLI:
        return "SLLI";
    case rvOPCODE::SRLI:
        return "SRLI";
    case rvOPCODE::SRAI:
        return "SRAI";
    case rvOPCODE::SLTI:
        return "SLTI";
    case rvOPCODE::SLTIU:
        return "SLTIU";
    case rvOPCODE::LW:
        return "LW";
    case rvOPCODE::SW:
        return "SW";
    case rvOPCODE::BEQ:
        return "BEQ";
    case rvOPCODE::BNE:
        return "BNE";
    case rvOPCODE::BLT:
        return "BLT";
    case rvOPCODE::BGE:
        return "BGE";
    case rvOPCODE::BLTU:
        return "BLTU";
    case rvOPCODE::BGEU:
        return "BGEU";
    case rvOPCODE::JAL:
        return "JAL";
    case rvOPCODE::JALR:
        return "JALR";
    case rvOPCODE::LA:
        return "LA";
    case rvOPCODE::LI:
        return "LI";
    case rvOPCODE::MOV:
        return "MOV";
    case rvOPCODE::J:
        return "J";
    case rvOPCODE::MUL:
        return "MUL";
    case rvOPCODE::DIV:
        return "DIV";
    case rvOPCODE::REM:
        return "REM";
    case rvOPCODE::SNEZ:
        return "SNEZ";
    case rvOPCODE::SEQZ:
        return "SEQZ";
    case rvOPCODE::NOP:
        return "NOP";
    case rvOPCODE::MV:
        return "MV";
    case rvOPCODE::BNEZ:
        return "BNEZ";
    case rvOPCODE::JR:
        return "JR";
    case rvOPCODE::BEQZ:
        return "BEQZ";
    case rvOPCODE::CALL:
        return "CALL";
    case rvOPCODE::NOT:
        return "NOT";
    default:
        assert(0 && "No Such Kind Of rvOPCODE");
        break;
    }
}

std::string rv::toString(rvFREG r)
{
    switch (r)
    {
    case rvFREG::F0:
        return "ft0";
    case rvFREG::F1:
        return "ft1";
    case rvFREG::F2:
        return "ft2";
    case rvFREG::F3:
        return "ft3";
    case rvFREG::F4:
        return "ft4";
    case rvFREG::F5:
        return "ft5";
    case rvFREG::F6:
        return "ft6";
    case rvFREG::F7:
        return "ft7";
    case rvFREG::F8:
        return "fs0";
    case rvFREG::F9:
        return "fs1";
    case rvFREG::F10:
        return "fa0";
    case rvFREG::F11:
        return "fa1";
    case rvFREG::F12:
        return "fa2";
    case rvFREG::F13:
        return "fa3";
    case rvFREG::F14:
        return "fa4";
    case rvFREG::F15:
        return "fa5";
    case rvFREG::F16:
        return "fa6";
    case rvFREG::F17:
        return "fa7";
    case rvFREG::F18:
        return "fs2";
    case rvFREG::F19:
        return "fs3";
    case rvFREG::F20:
        return "fs4";
    case rvFREG::F21:
        return "fs5";
    case rvFREG::F22:
        return "fs6";
    case rvFREG::F23:
        return "fs7";
    case rvFREG::F24:
        return "fs8";
    case rvFREG::F25:
        return "fs9";
    case rvFREG::F26:
        return "fs10";
    case rvFREG::F27:
        return "fs11";
    case rvFREG::F28:
        return "ft8";
    case rvFREG::F29:
        return "ft9";
    case rvFREG::F30:
        return "ft10";
    case rvFREG::F31:
        return "ft11";
    default:
        assert(0 && "No Such Kind Of rvFREG");
        break;
    }
}

rv::rvREG backend::Generator::getRd(ir::Operand)
{
    return rv::rvREG::X7; // t2
}

rv::rvFREG backend::Generator::fgetRd(ir::Operand)
{
    return rv::rvFREG::F2; // ft2
}

rv::rvREG backend::Generator::getRs1(ir::Operand)
{
    return rv::rvREG::X5; // t0
}

rv::rvREG backend::Generator::getRs2(ir::Operand)
{
    return rv::rvREG::X6; // t1
}

rv::rvFREG backend::Generator::fgetRs1(ir::Operand)
{
    return rv::rvFREG::F0; // ft0
}

rv::rvFREG backend::Generator::fgetRs2(ir::Operand)
{
    return rv::rvFREG::F1; // ft1
}

/// @brief find the addr of a ir::Operand
/// @param op
/// @return the offset
int backend::stackVarMap::find_operand(ir::Operand op)
{
    auto it = _table.find(op.name);
    if (it == _table.end())
    {
        // 不在局部变量之中
        std::cout << "\t\tfind\t" << op.name << "\tresult:False\n";
        return -1; // 出现0则表示未找到
    }
    else
    {
        // 在局部变量之中
        std::cout << "\t\tfind\t" << op.name << "\tresult:" << _table[op.name] << "\n";
        return _table[op.name];
    }
}

/// @brief find the label name of a global ir::Operand
/// @param  op
/// @return the label name
bool backend::Generator::find_operand_global(ir::Operand op)
{
    std::cout << "\t\tfindGlobal\t" << op.name << "\n";
    for (auto it = program.globalVal.begin(); it != program.globalVal.end(); it++)
    {
        if (it->val.name == op.name)
        {
            return true;
        }
    }
    return false;
}

/// @brief add a ir::Operand into current map, alloc space for this variable in memory
/// @param op
/// @param size: the space needed(in byte)
/// @return the offset
int backend::stackVarMap::add_operand(ir::Operand op, uint32_t size = 4)
{
    int offset = cur_off;
    _table[op.name] = offset;
    cur_off += size;
    std::cout << "\t\t\tadd\t" << op.name << "\toffset:" << offset << "\n";
    return offset;
}

void backend::Generator::gen()
{
    // .option nopic
    fout << "\t.option nopic\n";
    std::cout << "\t.option nopic\n";
    // .data
    fout << "\t.data\n";
    std::cout<< "\t.data\n";
    // 添加全局变量 变量.word 0 数组.space 大小
    for (int i = 0; i < program.globalVal.size(); i++)
    {
        std::string varname = program.globalVal[i].val.name;
        if (program.globalVal[i].val.type==ir::Type::Int){
            // 变量
            fout << "\t.global\t" << varname << "\n";
            std::cout << "\t.global\t" << varname << "\n";
            fout << "\t.type\t" << varname << ",@object\n";
            std::cout << "\t.type\t" << varname << ",@object\n";
            fout << "\t.size\t" << varname << ",4\n";
            std::cout << "\t.size\t" << varname << ",4\n";
            fout << "\t.align\t4\n";
            std::cout << "\t.align\t4\n";
            fout << varname << ":\n";
            std::cout << varname << ":\n";
            fout << "\t.word\t0\n";
            std::cout << "\t.word\t0\n";
        }else{
            assert(program.globalVal[i].val.type == ir::Type::IntPtr);
            // 数组
            fout << "\t.global\t" << varname << "\n";
            std::cout << "\t.global\t" << varname << "\n";
            fout << "\t.type\t" << varname << ",@object\n";
            std::cout << "\t.type\t" << varname << ",@object\n";
            fout << "\t.size\t" << varname << ","<< std::to_string(program.globalVal[i].maxlen*4) <<"\n";
            std::cout << "\t.size\t" << varname << ","<< std::to_string(program.globalVal[i].maxlen*4) <<"\n";
            fout << "\t.align\t4\n";
            std::cout << "\t.align\t4\n";
            fout << varname << ":\n";
            std::cout << varname << ":\n";
            fout << "\t.space\t"<< std::to_string(program.globalVal[i].maxlen*4) <<"\n";
            std::cout << "\t.space\t"<< std::to_string(program.globalVal[i].maxlen*4) <<"\n";
        }
    }
    // .text
    fout << "\t.text\n";
    std::cout << "\t.text\n";
    // 添加函数
    for (int i = 0; i < program.functions.size();i++)
    {
        fout << "\t.global\t" << program.functions[i].name << "\n";
        std::cout << "\t.global\t" << program.functions[i].name << "\n";
        fout << "\t.type\t" << program.functions[i].name << ",@function\n";
        std::cout << "\t.type\t" << program.functions[i].name << ",@function\n";
        fout << program.functions[i].name << ":\n";
        std::cout << program.functions[i].name << ":\n";
        gen_func(program.functions[i]);
        fout << ("\t.size\t" + program.functions[i].name + ", .-" + program.functions[i].name + "\n");
        std::cout << ("\t.size\t" + program.functions[i].name + ", .-" + program.functions[i].name + "\n");
    }
}

void backend::Generator::gen_func(ir::Function &func)
{
    // ABI
    // 处理实参，暂存
    std::string tmp_var = "";
    for (int i = 0; i < func.ParameterList.size(); i++)
    {
        int offset = stackmap.add_operand(func.ParameterList[i]);
        tmp_var += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + "a" + std::to_string(i) + "," + std::to_string(offset) + "(sp)\n");
    }
    std::cout << "Param Bingo!\n";
    // 分析指令，暂存
    std::vector<std::string, int> tmp_inst_vec;
    for (int i = 0; i < func.InstVec.size(); i++)
    {
        std::string tmp_inst = "";
        bool is_global_func = (func.name == "globalFunc");
        gen_instr(*func.InstVec[i], tmp_inst, is_global_func);
        tmp_inst_vec.push_back({tmp_inst, 1});
    }
    std::cout << "Inst Bingo!\n";
    // 保存返回地址,暂存
    std::string tmp_addr = "";
    tmp_addr += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + "ra" + "," + std::to_string(stackmap.cur_off) + "(sp)\n");
    stackmap.cur_off += 4;
    std::cout << "RA Bingo!\n";

    // 正式开始：
    // 1.首先调整栈指针，为其分配空间[包括函数实参，返回地址，指令中的变量][get from cur_off]
    fout << ("\t" + rv::toString(rv::rvOPCODE::ADDI) + "\t" + "sp" + "," + "sp" + "," + std::to_string(-stackmap.cur_off) + "\n");
    std::cout << ("\t" + rv::toString(rv::rvOPCODE::ADDI) + "\t" + "sp" + "," + "sp" + "," + std::to_string(-stackmap.cur_off) + "\n");
    // 2.处理返回地址
    fout << tmp_addr;
    std::cout << tmp_addr;
    // 3.处理实参
    fout << tmp_var;
    std::cout << tmp_var;
    // 4.处理指令【包含return等操作】
    TODO;
    // fout << tmp_inst;
    // std::cout << tmp_inst;
}

int backend::Generator::gen_instr(ir::Instruction &inst, std::string &tmp_out, bool is_global_func)
{
    // 记录当前IR指令对应的汇编指令数量
    int cnt = 0;
    switch (inst.op)
    {
    case ir::Operator::mul:
    case ir::Operator::div:
    case ir::Operator::mod:
    case ir::Operator::sub:
    case ir::Operator::add:
    case ir::Operator::lss:
    {
        // add rd, rs, rt
        rv::rvREG rs1 = getRs1(inst.op1);
        rv::rvREG rs2 = getRs2(inst.op2);
        rv::rvREG rd = getRd(inst.des);

        LOAD_RS1_RS2;

        rv::rv_inst *op_inst = new rv::rv_inst();
        switch (inst.op)
        {
        case ir::Operator::add:
            op_inst->op = rv::rvOPCODE::ADD;
            break;
        case ir::Operator::sub:
            op_inst->op = rv::rvOPCODE::SUB;
            break;
        case ir::Operator::mul:
            op_inst->op = rv::rvOPCODE::MUL;
            break;
        case ir::Operator::div:
            op_inst->op = rv::rvOPCODE::DIV;
            break;
        case ir::Operator::mod:
            op_inst->op = rv::rvOPCODE::REM;
            break;
        case ir::Operator::lss:
            op_inst->op = rv::rvOPCODE::SLT;
            break;
        case ir::Operator::_not:
            op_inst->op = rv::rvOPCODE::NOT;
            break;
        default:
            assert(0 && "No Such Kind Of rvOPCODE");
            break;
        }
        op_inst->rs1 = rs1;
        op_inst->rs2 = rs2;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "," + rv::toString(op_inst->rs2) + "\n");
        cnt++;
        // save back rd
        SAVE_BACK_RD;
    }
    break;

    case ir::Operator::gtr:
    {
        rv::rvREG rs1 = getRs1(inst.op1);
        rv::rvREG rs2 = getRs2(inst.op2);
        rv::rvREG rd = getRd(inst.des);
        LOAD_RS1_RS2;

        rv::rv_inst *op_inst = new rv::rv_inst();
        op_inst->op = rv::rvOPCODE::SLT;
        op_inst->rs1 = rs2;
        op_inst->rs2 = rs1;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "," + rv::toString(op_inst->rs2) + "\n");
        cnt++;
        SAVE_BACK_RD;
    }
    break;

    case ir::Operator::leq:
    {
        rv::rvREG rs1 = getRs1(inst.op1);
        rv::rvREG rs2 = getRs2(inst.op2);
        rv::rvREG rd = getRd(inst.des);
        LOAD_RS1_RS2;

        rv::rv_inst *op_inst = new rv::rv_inst();
        // GTR
        op_inst->op = rv::rvOPCODE::SLT;
        op_inst->rs1 = rs2;
        op_inst->rs2 = rs1;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "," + rv::toString(op_inst->rs2) + "\n");

        // NOT
        op_inst->op = rv::rvOPCODE::NOT;
        op_inst->rs1 = rd;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "\n");
        SAVE_BACK_RD;
    }
    break;

    case ir::Operator::geq:
    {
        rv::rvREG rs1 = getRs1(inst.op1);
        rv::rvREG rs2 = getRs2(inst.op2);
        rv::rvREG rd = getRd(inst.des);
        LOAD_RS1_RS2;

        rv::rv_inst *op_inst = new rv::rv_inst();
        // GTR
        op_inst->op = rv::rvOPCODE::SLT;
        op_inst->rs1 = rs1;
        op_inst->rs2 = rs2;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "," + rv::toString(op_inst->rs2) + "\n");

        // NOT
        op_inst->op = rv::rvOPCODE::NOT;
        op_inst->rs1 = rd;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "\n");
        SAVE_BACK_RD;
    }
    break;

    case ir::Operator::eq:
    {
        rv::rvREG rs1 = getRs1(inst.op1);
        rv::rvREG rs2 = getRs2(inst.op2);
        rv::rvREG rd = getRd(inst.des);
        LOAD_RS1_RS2;

        rv::rv_inst *op_inst = new rv::rv_inst();
        // XOR
        op_inst->op = rv::rvOPCODE::XOR;
        op_inst->rs1 = rs1;
        op_inst->rs2 = rs2;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "," + rv::toString(op_inst->rs2) + "\n");

        // SNEZ
        op_inst->op = rv::rvOPCODE::SNEZ;
        op_inst->rs2 = rd;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs2) + "\n");
        SAVE_BACK_RD;
        // std::cout << tmp_out;
    }
    break;

    case ir::Operator::neq:
    {
        rv::rvREG rs1 = getRs1(inst.op1);
        rv::rvREG rs2 = getRs2(inst.op2);
        rv::rvREG rd = getRd(inst.des);
        LOAD_RS1_RS2;

        rv::rv_inst *op_inst = new rv::rv_inst();
        // XOR
        op_inst->op = rv::rvOPCODE::XOR;
        op_inst->rs1 = rs1;
        op_inst->rs2 = rs2;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "," + rv::toString(op_inst->rs2) + "\n");

        // SEQZ
        op_inst->op = rv::rvOPCODE::SEQZ;
        op_inst->rs2 = rd;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs2) + "\n");
        SAVE_BACK_RD;
    }
    break;

    case ir::Operator::_not:
    {
        rv::rvREG rs1 = getRs1(inst.op1);
        rv::rvREG rd = getRd(inst.des);
        LOAD_RS1;

        rv::rv_inst *op_inst = new rv::rv_inst();

        // SEQZ
        op_inst->op = rv::rvOPCODE::SEQZ;
        op_inst->rs1 = rd;
        op_inst->rd = rd;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "\n");

        SAVE_BACK_RD;
    }
    break;

    case ir::Operator::__unuse__:
    {
        rv::rv_inst *op_inst = new rv::rv_inst();
        // NOT
        op_inst->op = rv::rvOPCODE::NOP;

        tmp_out += ("\t" + rv::toString(op_inst->op) + "\n");
    }
    break;

    case ir::Operator::mov:
    case ir::Operator::def:
    {
        rv::rvREG rs1 = getRs1(inst.op1);
        rv::rvREG rd = getRd(inst.des);
        // LOAD_RS1;

        rv::rv_inst *op_inst = new rv::rv_inst();

        if (inst.op1.type == ir::Type::IntLiteral)
        {
            // li
            op_inst->op = rv::rvOPCODE::LI;
            op_inst->rs1 = rs1;
            op_inst->rd = rd;

            tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + inst.op1.name + "\n");
        }
        else
        {
            // MV
            LOAD_RS1;
            op_inst->op = rv::rvOPCODE::MV;
            op_inst->rs1 = rs1;
            op_inst->rd = rd;

            tmp_out += ("\t" + rv::toString(op_inst->op) + "\t" + rv::toString(op_inst->rd) + "," + rv::toString(op_inst->rs1) + "\n");
        }
        SAVE_BACK_RD;
        // std::cout << "\n" <<tmp_out<<"\n";
    }
    break;

    case ir::Operator::load:
    {
        rv::rvREG rs1 = getRs1(inst.op1);
        rv::rvREG rs2 = getRs1(inst.op2);
        rv::rvREG rd = getRd(inst.des);

        int offset = stackmap.find_operand(inst.op1);
        if (offset == -1)
        { /* 未在局部变量中找到*/
            if (find_operand_global(inst.op1))
            { /* 在全局变量中找到，直接用标签   偏移量为组内偏移量*/
                // 加载基址进s1
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LA) + "\t" + rv::toString(rs1) + "," + inst.op1.name + "\n");
                if (inst.op2.type == ir::Type::IntLiteral)
                {
                    // 组内偏移量为立即数【*4】，直接load     基址为标签地址
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rd) + "," + std::to_string(std::stoi(inst.op2.name) * 4) + "(" + rv::toString(rs1) + ")\n");
                    SAVE_BACK_RD;
                }
                else
                {
                    // 组内偏移量为变量     基址为标签地址+偏移量
                    LOAD_RS2;
                    // 组内偏移量 = 下标*4【左移2位】
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SLLI) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + "," + std::to_string(2) + "\n");
                    // 基址设为标签地址+偏移量，存在rs2中
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::ADD) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + "," + rv::toString(rs1) + "\n");
                    // load
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rd) + ",0(" + rv::toString(rs2) + ")\n");
                    SAVE_BACK_RD;
                }
            }
            else
            { /* 在全局变量中未找到，报错 */
                assert(0 && "Can Not Find The Operand1");
            }
        }
        else
        { /* 已找到,是局部变量  偏移量=数组偏移量[常量]+数组内偏移量*/
            rv::rv_inst *op_inst = new rv::rv_inst();

            if (inst.op2.type == ir::Type::IntLiteral)
            {
                // 组内偏移量为立即数[常量+常量]    基址为sp
                offset += std::stoi(inst.op2.name) * 4;
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rd) + "," + std::to_string(offset) + "(sp)\n");
                SAVE_BACK_RD;
            }
            else
            {
                // 组内偏移量为变量[常量+变量]      基址为sp+偏移量
                LOAD_RS2;
                // 组内偏移量 = 下标*4【左移2位】
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SLLI) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + "," + std::to_string(2) + "\n");
                // 偏移量相加
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::ADDI) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + "," + std::to_string(offset) + "\n");
                // 基址设为sp+偏移量，存在rs2中
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::ADD) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + ",sp\n");
                // load
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + rv::toString(rd) + ",0(" + rv::toString(rs2) + ")\n");
                SAVE_BACK_RD;
            }
        }
    }
    break;

    case ir::Operator::store:
    {
        rv::rvREG rs1 = getRs1(inst.op1); // 数组名
        rv::rvREG rs2 = getRs1(inst.op2); // 下标
        rv::rvREG rd = getRd(inst.des);   // 待存入的数

        int offset = stackmap.find_operand(inst.op1);
        if (offset == -1)
        { /* 未在局部变量中找到*/
            if (find_operand_global(inst.op1))
            { /* 在全局变量中找到，直接用标签   偏移量为组内偏移量*/
                // 加载基址进s1
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LA) + "\t" + rv::toString(rs1) + "," + inst.op1.name + "\n");
                if (inst.op2.type == ir::Type::IntLiteral)
                {
                    // 组内偏移量为立即数【*4】，直接load     基址为标签地址
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + rv::toString(rd) + "," + std::to_string(std::stoi(inst.op2.name) * 4) + "(" + rv::toString(rs1) + ")\n");
                }
                else
                {
                    // 组内偏移量为变量     基址为标签地址+偏移量
                    LOAD_RS2;
                    // 组内偏移量 = 下标*4【左移2位】
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SLLI) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + "," + std::to_string(2) + "\n");
                    // 基址设为标签地址+偏移量，存在rs2中
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::ADD) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + "," + rv::toString(rs1) + "\n");
                    // save
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + rv::toString(rd) + ",0(" + rv::toString(rs2) + ")\n");
                }
            }
            else
            { /* 在全局变量中未找到，报错 */
                assert(0 && "Can Not Find The Operand1");
            }
        }
        else
        { /* 已找到,是局部变量  偏移量=数组偏移量[常量]+数组内偏移量*/
            rv::rv_inst *op_inst = new rv::rv_inst();

            if (inst.op2.type == ir::Type::IntLiteral)
            {
                // 组内偏移量为立即数[常量+常量]    基址为sp
                offset += std::stoi(inst.op2.name) * 4;
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + rv::toString(rd) + "," + std::to_string(offset) + "(sp)\n");
            }
            else
            {
                // 组内偏移量为变量[常量+变量]      基址为sp+偏移量
                LOAD_RS2;
                // 组内偏移量 = 下标*4【左移2位】
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SLLI) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + "," + std::to_string(2) + "\n");
                // 偏移量相加
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::ADDI) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + "," + std::to_string(offset) + "\n");
                // 基址设为sp+偏移量，存在rs2中
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::ADD) + "\t" + rv::toString(rs2) + "," + rv::toString(rs2) + ",sp\n");
                // load
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + rv::toString(rd) + ",0(" + rv::toString(rs2) + ")\n");
            }
        }
    }
    break;

    case ir::Operator::alloc:
    {
        if (!is_global_func)
        { // globalfunc中的数组不需要再分配空间，直接是存在静态区的
            uint32_t size = 4 * std::stoi(inst.op1.name);
            stackmap.add_operand(inst.des, size);
        }
    }
    break;

    case ir::Operator::_goto:
    {
        rv::rvREG rs1 = getRs1(inst.op1); // 跳转条件
        rv::rvREG rd = getRd(inst.des);   // 偏移量
        // LOAD_RS1;
        // LOAD_RD;
        // 获取偏移量【并*4】   可能是变量/常量
        if (inst.des.type == ir::Type::Int)
        {
            LOAD_RD;
            // 偏移量为变量[左移两位]
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SLLI) + "\t" + rv::toString(rd) + "," + rv::toString(rd) + "," + std::to_string(2) + "\n");
            if (inst.op1.type == ir::Type::null)
            {
                // 无条件跳转   j
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::JR) + "\t" + rv::toString(rd) + "\n");
            }
            else
            {
                // 有条件跳转   beqz + j
                LOAD_RS1;
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::BEQZ) + "\t" + rv::toString(rs1) + ",8\n");
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::JR) + "\t" + rv::toString(rd) + "\n");
            }
        }
        else
        {
            // 偏移量为常量[×4]
            std::string off = std::to_string(std::stoi(inst.des.name) * 4);
            if (inst.op1.type == ir::Type::null)
            {
                // 无条件跳转   j
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::J) + "\t" + off + "\n");
            }
            else
            {
                // 有条件跳转   bnez
                LOAD_RS1;
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::BNEZ) + "\t" + rv::toString(rs1) + "," + off + "\n");
            }
        }
        // std::cout << tmp_out;
    }
    break;

    case ir::Operator::call:
    {
        auto callinst = dynamic_cast<ir::CallInst *>(&inst);
        // 先将参数保存在寄存器a0-a7中[查arguementlist]，gen_function中callee在处理完ABI后先将caller传来的参数压栈
        for (int i = 0; i < callinst->argumentList.size(); i++)
        {
            assert(i <= 7);
            int offset = stackmap.find_operand(callinst->argumentList[i]);
            if (offset == -1)
            { /* 未在局部变量中找到*/
                if (find_operand_global(callinst->argumentList[i]))
                { /* 在全局变量中找到，直接用标签 */
                    rv::rvREG tmp = getRd(inst.op1);
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LA) + "\t" + rv::toString(tmp) + "," + callinst->argumentList[i].name + "\n");
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + "a" + std::to_string(i) + "," + "0(" + rv::toString(tmp) + ")\n");
                }
                else
                { /* 在全局变量中未找到 */
                    // 可能是常量
                    if (callinst->argumentList[i].type!=ir::Type::IntLiteral)
                    {
                        assert(0 && "Can Not Find The Param");
                    }
                    tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LI) + "\t" + "a" + std::to_string(i) + "," + callinst->argumentList[i].name + "\n");
                }
            }
            else
            { /* 已找到 */
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + "a" + std::to_string(i) + "," + std::to_string(offset) + "(sp)" + "\n");
            }
        }

        // call     inst.op1.name为函数名，即标签   ra为返回地址【pc+8,因为call为伪指令，实际翻译为两条汇编】
        tmp_out += ("\t" + rv::toString(rv::rvOPCODE::CALL) + "\t" + "ra" + "," + inst.op1.name + "\n");

        // 最后取返回值     返回值在寄存器a0中
        int offset = stackmap.find_operand(inst.des);
        if (offset == -1)
        { /* 未在局部变量中找到*/
            if (find_operand_global(inst.des))
            { /* 在全局变量中找到，直接用标签 */
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LA) + "\t" + "t0" + "," + inst.op1.name + "\n");
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + "a0" + ",0(" + "t0" + ")\n");
            }
            else
            { /* 在全局变量中未找到，在局部变量中为其分配空间 */
                offset = stackmap.add_operand(inst.des);
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + "a0" + "," + std::to_string(offset) + "(sp)\n");
            }
        }
        else
        { /* 已找到 */
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::SW) + "\t" + "a0" + "," + std::to_string(offset) + "(sp)" + "\n");
        }
    }
    break;

    case ir::Operator::_return:
    {
        // 先将返回值压入寄存器a0, 由caller从寄存器中取回
        rv::rvREG rs1 = getRs1(inst.op1);
        int offset = stackmap.find_operand(inst.op1); // 返回值
        if (offset == -1)
        { /* 未在局部变量中找到*/
            if (find_operand_global(inst.op1))
            { /* 在全局变量中找到，直接用标签 */
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LA) + "\t" + rv::toString(rs1) + "," + inst.op1.name + "\n");
                tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + "a0" + ",0(" + rv::toString(rs1) + ")\n");
            }
            else
            { /* 在全局变量中未找到 */
                if (inst.op1.type!=ir::Type::null)
                    assert(0 && "Can Not Find The Return OP");
            }
        }
        else
        { /* 已找到 */
            tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + "a0" + "," + std::to_string(offset) + "(sp)" + "\n");
        }

        // jr ra
        // 从栈中取返回地址放于ra
        tmp_out += ("\t" + rv::toString(rv::rvOPCODE::LW) + "\t" + "ra" + "," + std::to_string(stackmap.cur_off) + "(sp)" + "\n");
        // 释放占空间【包括额外保存的返回地址】
        tmp_out += ("\t" + rv::toString(rv::rvOPCODE::ADDI) + "\t" + "sp" + "," + "sp" + "," + std::to_string(stackmap.cur_off + 4) + "\n");
        // 跳转返回
        tmp_out += ("\t" + rv::toString(rv::rvOPCODE::JR) + "\t" + "ra" + "\n");
        // std::cout << tmp_out;
    }
    break;

    default:
        break;
    }
}
