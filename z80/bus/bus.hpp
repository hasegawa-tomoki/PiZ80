#ifndef Z80EMU_BUS_HPP
#define Z80EMU_BUS_HPP

#include <circle/types.h>
#include <circle/timer.h>
#include <circle/logger.h>

class Bus {
public:
    Bus() = default;
    ~Bus() = default;

    virtual void setAddress(u16 addr) = 0;
    virtual void setDataBegin(u8 data) = 0;
    virtual void setDataEnd() = 0;
    virtual u8 getData() = 0;
    virtual void setControl(u8 z80PinName, bool level) = 0;
    virtual bool getInput(u8 z80PinName) = 0;
    virtual void syncControl() = 0;

    virtual void waitClockRising(bool force) = 0;
    virtual void waitClockFalling(bool force) = 0;

    static void waitNanoSec(unsigned ns);

    u16 address = 0;

    u8 pin_o_m1 = PIN_HIGH;
    u8 pin_o_rfsh = PIN_HIGH;
    u8 pin_o_halt = PIN_HIGH;
    u8 pin_o_rd = PIN_HIGH;
    u8 pin_o_wr = PIN_HIGH;
    u8 pin_o_mreq = PIN_HIGH;
    u8 pin_o_iorq = PIN_HIGH;
    u8 pin_o_busack = PIN_HIGH;

    volatile u8 pin_i_reset = PIN_HIGH;
    volatile u8 pin_i_nmi = PIN_HIGH;
    volatile u8 pin_i_nmi_prev = PIN_HIGH;
    volatile u8 pin_i_int = PIN_HIGH;
    volatile u8 pin_i_int_prev = PIN_HIGH;
    volatile u8 pin_i_wait = PIN_HIGH;
    volatile u8 pin_i_busrq = PIN_HIGH;

    static const u8 Z80_PIN_I_CLK = 6;
    static const u8 Z80_PIN_I_INT = 16;
    static const u8 Z80_PIN_I_NMI = 17;
    static const u8 Z80_PIN_O_HALT = 18;
    static const u8 Z80_PIN_O_MERQ = 19;
    static const u8 Z80_PIN_O_IORQ = 20;
    static const u8 Z80_PIN_O_RD = 21;
    static const u8 Z80_PIN_O_WR = 22;
    static const u8 Z80_PIN_O_BUSACK = 23;
    static const u8 Z80_PIN_I_WAIT = 24;
    static const u8 Z80_PIN_I_BUSRQ = 25;
    static const u8 Z80_PIN_I_RESET = 26;
    static const u8 Z80_PIN_O_M1 = 27;
    static const u8 Z80_PIN_O_RFSH = 28;
    static const u8 Z80_PIN_A0 = 30;
    static const u8 Z80_PIN_A1 = 31;
    static const u8 Z80_PIN_A2 = 32;
    static const u8 Z80_PIN_A3 = 33;
    static const u8 Z80_PIN_A4 = 34;
    static const u8 Z80_PIN_A5 = 35;
    static const u8 Z80_PIN_A6 = 36;
    static const u8 Z80_PIN_A7 = 37;
    static const u8 Z80_PIN_A8 = 38;
    static const u8 Z80_PIN_A9 = 39;
    static const u8 Z80_PIN_A10 = 40;
    static const u8 Z80_PIN_A11 = 1;
    static const u8 Z80_PIN_A12 = 2;
    static const u8 Z80_PIN_A13 = 3;
    static const u8 Z80_PIN_A14 = 4;
    static const u8 Z80_PIN_A15 = 5;
    static const u8 Z80_PIN_D0 = 14;
    static const u8 Z80_PIN_D1 = 15;
    static const u8 Z80_PIN_D2 = 12;
    static const u8 Z80_PIN_D3 = 8;
    static const u8 Z80_PIN_D4 = 7;
    static const u8 Z80_PIN_D5 = 9;
    static const u8 Z80_PIN_D6 = 10;
    static const u8 Z80_PIN_D7 = 13;

    static const u8 PIN_HIGH = 1;
    static const u8 PIN_LOW = 0;
};


#endif //Z80EMU_BUS_HPP
