#ifndef Z80EMU_PICGPIOBUS_HPP
#define Z80EMU_PICGPIOBUS_HPP

#include <circle/gpiopin.h>
#include <circle/types.h>
#include <circle/logger.h>
#include "bus.hpp"

class CGpioBus : public Bus {
public:
    CGpioBus();
    ~CGpioBus() = default;

    void setAddress(u16 addr) override;
    void setDataBegin(u8 data) override;
    void setDataEnd() override;
    u8 getData() override;
    void setControl(u8 z80PinName, bool level) override;
    bool getInput(u8 z80PinName) override;
    void syncControl() override;

    void waitClockRising() override;
    void waitClockFalling() override;

    void setDataBusOutput();
    void setDataBusInput();

    CGPIOPin m_ADDR[8];
    CGPIOPin m_DATA[8];

    CGPIOPin m_I_RESET;
    CGPIOPin m_I_CLK;
    CGPIOPin m_I_NMI;
    CGPIOPin m_I_INT;
    CGPIOPin m_I_WAIT;
    CGPIOPin m_I_BUSRQ;
    CGPIOPin m_DATA_BUS_DIR;
    CGPIOPin m_LE_ADDRESS_LOW;
    CGPIOPin m_LE_ADDRESS_HIGH;
    CGPIOPin m_LE_CONTROL;
    CGPIOPin m_DATA_BUS_OE;

    static const u8 RPi_GPIO_L_A[8];
    static const u8 RPi_GPIO_D[8];

    static const u8 RPi_GPIO_L_A0 = 0;
    static const u8 RPi_GPIO_L_A1 = 1;
    static const u8 RPi_GPIO_L_A2 = 2;
    static const u8 RPi_GPIO_L_A3 = 3;
    static const u8 RPi_GPIO_L_A4 = 4;
    static const u8 RPi_GPIO_L_A5 = 5;
    static const u8 RPi_GPIO_L_A6 = 6;
    static const u8 RPi_GPIO_L_A7 = 7;
    static const u8 RPi_GPIO_D0 = 8;
    static const u8 RPi_GPIO_D1 = 9;
    static const u8 RPi_GPIO_D2 = 10;
    static const u8 RPi_GPIO_D3 = 11;
    static const u8 RPi_GPIO_D4 = 12;
    static const u8 RPi_GPIO_D5 = 13;
    //static const u8 RPi_GPIO_D6 = 14; // ここを
    //static const u8 RPi_GPIO_D7 = 15; // あけたい
    static const u8 RPi_GPIO_D6 = 18;
    static const u8 RPi_GPIO_D7 = 27;
    static const u8 RPi_GPIO_I_RESET = 16;
    static const u8 RPi_GPIO_I_CLK = 17;
    //static const u8 RPi_GPIO_I_NMI = 18; // あけられる
    static const u8 RPi_GPIO_I_INT = 19;
    static const u8 RPi_GPIO_I_WAIT = 20;
    static const u8 RPi_GPIO_I_BUSRQ = 21;
    static const u8 RPi_GPIO_DATA_BUS_DIR = 22;
    static const u8 RPi_GPIO_LE_ADDRESS_LOW = 23;
    static const u8 RPi_GPIO_LE_ADDRESS_HIGH = 24;
    static const u8 RPi_GPIO_LE_CONTROL = 25;
    static const u8 RPi_GPIO_DATA_BUS_OE = 26;

    static const u8 L_M1 = 0;
    static const u8 L_RFSH = 1;
    static const u8 L_HALT = 2;
    static const u8 L_RD = 3;
    static const u8 L_WR = 4;
    static const u8 L_MREQ = 5;
    static const u8 L_IORQ = 6;
    static const u8 L_BUSACK = 7;

    static const u8 DATA_BUS_ISOLATED = HIGH;
    static const u8 DATA_BUS_ENABLED = LOW;

    static const u8 DATA_BUS_DIR_OUT = HIGH;
    static const u8 DATA_BUS_DIR_IN = LOW;

    u8 currentDataBusMode = 0xff;
};


#endif //Z80EMU_PICGPIOBUS_HPP
