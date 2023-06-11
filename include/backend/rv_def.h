#ifndef RV_DEF_H
#define RV_DEF_H

#include<string>

namespace rv {

// rv interger registers
/*
    FIX these comment by reading the risc-v ABI, 
    and u should figure out the role of every register in function call, 
    including its saver(caller/callee)
*/
enum class rvREG {
    /* Xn       its ABI name    role*/
    X0,         // zero         Hardwired Zero
    X1,         // ra           Return address
    X2,         // sp           Stack pointer
    X3,         // gp           Global pointer
    X4,         // tp           Thread pointer
    X5,         // t0           Temporary
    X6,         // t1           Temporary
    X7,         // t2           Temporary
    X8,         // s0 / fp      Saved register,frame pointer
    X9,         // s1           Saved register
    X10,        // a0           Function argument,return value
    X11,        // a1           Function argument,return value
    X12,        // a2           Function argument
    X13,        // a3           Function argument
    X14,        // a4           Function argument
    X15,        // a5           Function argument
    X16,        // a6           Function argument
    X17,        // a7           Function argument
    X18,        // s2           Saved register
    X19,        // s3           Saved register
    X20,        // s4           Saved register
    X21,        // s5           Saved register
    X22,        // s6           Saved register
    X23,        // s7           Saved register
    X24,        // s8           Saved register
    X25,        // s9           Saved register
    X26,        // s10          Saved register
    X27,        // s11          Saved register
    X28,        // t3           Temporary
    X29,        // t4           Temporary
    X30,        // t5           Temporary
    X31,        // t6           Temporary
};
std::string toString(rvREG r);  // implement this in ur own way

enum class rvFREG {
    F0,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    F25,
    F26,
    F27,
    F28,
    F29,
    F30,
    F31,
};
std::string toString(rvFREG r);  // implement this in ur own way

// rv32i instructions
// add instruction u need here!
enum class rvOPCODE {
    // RV32I Base Integer Instructions
    ADD, SUB, XOR, OR, AND, SLL, SRL, SRA, SLT, SLTU,       // arithmetic & logic
    ADDI, XORI, ORI, ANDI, SLLI, SRLI, SRAI, SLTI, SLTIU,   // immediate
    LW, SW,                                                 // load & store
    BEQ, BNE, BLT, BGE, BLTU, BGEU,                         // conditional branch
    JAL, JALR,                                              // jump

    // RV32M Multiply Extension

    // RV32F / D Floating-Point Extensions

    // Pseudo Instructions
    LA, LI, MOV, J,                                         // ...
};
std::string toString(rvOPCODE r);  // implement this in ur own way


} // namespace rv



#endif