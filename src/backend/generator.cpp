#include "backend/generator.h"

#include <assert.h>

#define TODO assert(0 && "todo")

backend::Generator::Generator(ir::Program &p, std::ofstream &f) : program(p), fout(f) {}

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
    return rv::rvREG();
}

rv::rvFREG backend::Generator::fgetRd(ir::Operand)
{
    return rv::rvFREG();
}

rv::rvREG backend::Generator::getRs1(ir::Operand)
{
    return rv::rvREG();
}

rv::rvREG backend::Generator::getRs2(ir::Operand)
{
    return rv::rvREG();
}

rv::rvFREG backend::Generator::fgetRs1(ir::Operand)
{
    return rv::rvFREG();
}

rv::rvFREG backend::Generator::fgetRs2(ir::Operand)
{
    return rv::rvFREG();
}

/// @brief find the addr of a ir::Operand
/// @param op
/// @return the offset
int backend::stackVarMap::find_operand(ir::Operand op)
{
    return _table[op];
}

/// @brief add a ir::Operand into current map, alloc space for this variable in memory
/// @param op
/// @param size: the space needed(in byte)
/// @return the offset
int backend::stackVarMap::add_operand(ir::Operand op, uint32_t size = 4)
{

    return 0;
}

void backend::Generator::gen()
{
    TODO;
}

void backend::Generator::gen()
{
}

void backend::Generator::gen_func(const ir::Function &)
{
}

void backend::Generator::gen_instr(const ir::Instruction &)
{
}
