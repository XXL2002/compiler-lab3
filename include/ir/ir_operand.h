#ifndef IROPERAND_H
#define IROPERAND_H

#include <string>


namespace ir {

enum class Type {
    Int,
    Float,
    IntLiteral,
    FloatLiteral,
    IntPtr,
    FloatPtr,
    null
};

std::string toString(Type t);

struct Operand {
    std::string name;
    Type type;
    Operand(std::string = "null", Type = Type::null);
    // 重载Operand的<符，否则lab3中的map无法使用【底层需要构建红黑树等索引，需要用到<号】
    bool operator<(Operand &) const;
};

}
#endif
