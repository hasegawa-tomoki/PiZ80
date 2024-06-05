#ifndef Z80EMU_SPECIAL_REGISTERS_HPP
#define Z80EMU_SPECIAL_REGISTERS_HPP

#include <circle/types.h>

class SpecialRegisters {
public:
    u8 i = 0;
    u8 r = 0;
    u16 sp = 0;
    u16 pc = 0;
    u16 ix = 0;
    u16 iy = 0;

    void ixh(u8 value);
    [[nodiscard]] u8 ixh() const;
    void ixl(u8 value);
    [[nodiscard]] u8 ixl() const;
    void iyh(u8 value);
    [[nodiscard]] u8 iyh() const;
    void iyl(u8 value);
    [[nodiscard]] u8 iyl() const;
};


#endif //Z80EMU_SPECIAL_REGISTERS_HPP
