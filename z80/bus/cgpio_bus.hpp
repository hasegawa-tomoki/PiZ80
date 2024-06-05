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

    void latchHold(u8 device);
    void selectInput(u8 device);
    void setDataBusDirection(bool direction);

    void setAddress(u16 addr) override;
    void setDataBegin(u8 data) override;
    void setDataEnd() override;
    u8 getData() override;
    void setControl(u8 z80PinName, bool level) override;
    bool getInput(u8 z80PinName) override;
    void syncControl() override;

    void waitClockRising() override;
    void waitClockFalling() override;

    CGPIOPin m_OUT[8];
    CGPIOPin m_IN[8];

    /*
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
     */
    CGPIOPin m_DATA_OE;
    CGPIOPin m_LC_OUT_A;
    CGPIOPin m_LC_OUT_B;
    CGPIOPin m_LC_OUT_G;
    CGPIOPin m_LC_IN_A;
    CGPIOPin m_LC_IN_B;
    CGPIOPin m_LC_IN_G;

    static const u8 RPi_BUS_OUT[8];
    static const u8 RPi_BUS_IN[8];

    static const u8 RPi_GPIO_DATA_OE = 10;
    static const u8 RPi_GPIO_LC_OUT_A = 3;
    static const u8 RPi_GPIO_LC_OUT_B = 2;
    static const u8 RPi_GPIO_LC_OUT_G = 4;
    static const u8 RPi_GPIO_LC_IN_A = 27;
    static const u8 RPi_GPIO_LC_IN_B = 22;
    static const u8 RPi_GPIO_LC_IN_G = 17;

    static const u8 LATCH_ADDRESS_LOW = 0;
    static const u8 LATCH_ADDRESS_HIGH = 1;
    static const u8 LATCH_CONTROL_OUTPUT = 2;
    static const u8 LATCH_DATA = 3;

    static const u8 BUS_TRANSCEIVER_DATA = 0;
    static const u8 BUS_TRANSCEIVER_CONTROL_INPUT = 1;

    static const u8 L_HALT = 0;
    static const u8 L_MREQ = 1;
    static const u8 L_IORQ = 2;
    static const u8 L_RD = 3;
    static const u8 L_WR = 4;
    static const u8 L_BUSACK = 5;
    static const u8 L_M1 = 6;
    static const u8 L_RFSH = 7;

    static const u8 LATCH_FOR_DATA_BUS_ISOLATED = HIGH;
    static const u8 LATCH_FOR_DATA_BUS_ENABLED = LOW;

    static const u8 DATA_BUS_DIR_OUT = 0;
    static const u8 DATA_BUS_DIR_IN = 1;

    u8 currentDataBusMode = 0xff;
};


#endif //Z80EMU_PICGPIOBUS_HPP
