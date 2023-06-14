#include "ir/ir_operand.h"

#include <string>
#include <utility>

ir::Operand::Operand(std::string n, Type t) : name(std::move(n)), type(t) {}

bool operator<(Operand &that) const
{
    return this.name < that.name;
}