#include "backend/generator.h"

#include <assert.h>

#define TODO assert(0 && "todo")

backend::Generator::Generator(ir::Program &p, std::ofstream &f) : program(p), fout(f) {}

std::string rv::toString(rvREG r)
{
    switch (r)
    {
    case rvREG::X0:
        return "X0";
        break;
    case rvREG::X1:
        return "X1";
        break;
    case rvREG::X2:
        return "X2";
        break;
    case rvREG::X3:
        return "X3";
        break;
    case rvREG::X4:
        return "X4";
        break;
    case rvREG::X5:
        return "X5";
        break;
    case rvREG::X6:
        return "X6";
        break;
    case rvREG::X7:
        return "X7";
        break;
    case rvREG::X8:
        return "X8";
        break;
    case rvREG::X9:
        return "X9";
        break;
    case rvREG::X10:
        return "X10";
        break;
    case rvREG::X11:
        return "X11";
        break;
    case rvREG::X12:
        return "X12";
        break;
    case rvREG::X13:
        return "X13";
        break;
    case rvREG::X14:
        return "X14";
        break;
    case rvREG::X15:
        return "X15";
        break;
    case rvREG::X16:
        return "X16";
        break;
    case rvREG::X17:
        return "X17";
        break;
    case rvREG::X18:
        return "X18";
        break;
    case rvREG::X19:
        return "X19";
        break;
    case rvREG::X20:
        return "X20";
        break;
    case rvREG::X21:
        return "X21";
        break;
    case rvREG::X22:
        return "X22";
        break;
    case rvREG::X23:
        return "X23";
        break;
    case rvREG::X24:
        return "X24";
        break;
    case rvREG::X25:
        return "X25";
        break;
    case rvREG::X26:
        return "X26";
        break;
    case rvREG::X27:
        return "X27";
        break;
    case rvREG::X28:
        return "X28";
        break;
    case rvREG::X29:
        return "X29";
        break;
    case rvREG::X30:
        return "X30";
        break;
    case rvREG::X31:
        return "X31";
        break;
    default:
        assert(0 && "No This Kind Of rvREG");
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
        assert(0 && "No This Kind Of rvOPCODE");
        break;
    }
}

std::string rv::toString(rvFREG r)
{
    switch (r)
    {
    case rvFREG::F0:
        return "F0";
    case rvFREG::F1:
        return "F1";
    case rvFREG::F2:
        return "F2";
    case rvFREG::F3:
        return "F3";
    case rvFREG::F4:
        return "F4";
    case rvFREG::F5:
        return "F5";
    case rvFREG::F6:
        return "F6";
    case rvFREG::F7:
        return "F7";
    case rvFREG::F8:
        return "F8";
    case rvFREG::F9:
        return "F9";
    case rvFREG::F10:
        return "F10";
    case rvFREG::F11:
        return "F11";
    case rvFREG::F12:
        return "F12";
    case rvFREG::F13:
        return "F13";
    case rvFREG::F14:
        return "F14";
    case rvFREG::F15:
        return "F15";
    case rvFREG::F16:
        return "F16";
    case rvFREG::F17:
        return "F17";
    case rvFREG::F18:
        return "F18";
    case rvFREG::F19:
        return "F19";
    case rvFREG::F20:
        return "F20";
    case rvFREG::F21:
        return "F21";
    case rvFREG::F22:
        return "F22";
    case rvFREG::F23:
        return "F23";
    case rvFREG::F24:
        return "F24";
    case rvFREG::F25:
        return "F25";
    case rvFREG::F26:
        return "F26";
    case rvFREG::F27:
        return "F27";
    case rvFREG::F28:
        return "F28";
    case rvFREG::F29:
        return "F29";
    case rvFREG::F30:
        return "F30";
    case rvFREG::F31:
        return "F31";
    default:
        assert(0 && "No This Kind Of rvFREG");
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
