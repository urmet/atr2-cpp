#ifndef OPCODES_H
#define OPCODES_H

enum Opcode {
    NOP=0,
    ADD=1,
    SUB=2,
    OR=3,
    AND=4,
    XOR=5,
    NOT=6,
    MPY=7,
    DIV=8,
    MOD=9,
    RET=10,
    CALL=11,
    JMP=12,
    JLS=13,
    JGR=14,
    JNE=15,
    JE=16,
    SWAP=17,
    DO=18,
    LOOP=19,
    CMP=20,
    TEST=21,
    MOV=22,
    LOC=23,
    GET=24,
    PUT=25,
    INT=26,
    IPO=27,
    OPO=28,
    DELAY=29,
    PUSH=30,
    POP=31,
    ERR=32,
    INC=33,
    DEC=34,
    SHL=35,
    SHR=36,
    ROL=37,
    ROR=38,
    JZ=39,
    JNZ=40,
    JGE=41,
    JLE=42,
    SAL=43,
    SAR=44,
    NEG=45,
    JTL=46
};

#endif
