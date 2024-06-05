#ifndef Z80EMU_REGISTERS_HPP
#define Z80EMU_REGISTERS_HPP

#include <circle/types.h>

class Registers
{
public:
    u8 a = 0;
    u8 b = 0;
    u8 c = 0;
    u8 d = 0;
    u8 e = 0;
    u8 h = 0;
    u8 l = 0;
//    u16 ix = 0;
//    u16 iy = 0;

    // C: Carry
    bool FC_Carry = false;
    // N: Add/Subtract
    bool FN_Subtract = false;
    // P/V: Parity/Overflow Flag
    bool FPV_ParityOverflow = false;
    // X: bit3
    bool F_X = false;
    // H: Half Carry Flag
    bool FH_HalfCarry = false;
    // X: bit5
    bool F_Y = false;
    // Z: Zero Flag
    bool FZ_Zero = false;
    // S: Sign Flag
    bool FS_Sign = false;

    void f(u8 value);
    [[nodiscard]] u8 f() const;

    void af(u16 value);
    [[nodiscard]] u16 af() const;
    void bc(u16 value);
    [[nodiscard]] u16 bc() const;
    void de(u16 value);
    [[nodiscard]] u16 de() const;
    void hl(u16 value);
    [[nodiscard]] u16 hl() const;

    u8 carry_by_val();

//    void setFlagC();
//    void resetFlagC();
//    void setFlagN();
//    void resetFlagN();
//    void setFlagPV();
//    void resetFlagPV();
//    void setFlagH();
//    void resetFlagH();
//    void setFlagZ();
//    void resetFlagZ();
//    void setFlagS();
//    void resetFlagS();
};

#endif //Z80EMU_REGISTERS_HPP
