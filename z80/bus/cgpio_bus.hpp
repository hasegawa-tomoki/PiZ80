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
    void setDataBusDirection(u8 direction);

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

    CGPIOPin m_OE_DATA_OUT;
    CGPIOPin m_OE_CTRL_IN;
    CGPIOPin m_OE_DATA_IN;
    CGPIOPin m_LE_ADDR_LOW;
    CGPIOPin m_LE_ADDR_HIGH;
    CGPIOPin m_LE_DATA;
    CGPIOPin m_LE_CTRL;

    static const u8 RPi_BUS_OUT[8];
    static const u8 RPi_BUS_IN[8];

    static const u8 RPi_GPIO_OE_DATA_OUT = 10;
    static const u8 RPi_GPIO_OE_CTRL_IN = 22;
    static const u8 RPi_GPIO_OE_DATA_IN = 27;
    static const u8 RPi_GPIO_LE_ADDR_LOW = 3;
    static const u8 RPi_GPIO_LE_ADDR_HIGH = 2;
    static const u8 RPi_GPIO_LE_DATA = 17;
    static const u8 RPi_GPIO_LE_CTRL = 4;

    static const u8 LATCH_ADDRESS_LOW = 0;
    static const u8 LATCH_ADDRESS_HIGH = 1;
    static const u8 LATCH_CONTROL_OUTPUT = 2;
    static const u8 LATCH_DATA = 3;

    static const u8 BUS_TRANSCEIVER_DATA = 0;
    static const u8 BUS_TRANSCEIVER_CONTROL = 1;

    static const u8 L_HALT = 0;
    static const u8 L_MREQ = 1;
    static const u8 L_IORQ = 2;
    static const u8 L_RD = 3;
    static const u8 L_WR = 4;
    static const u8 L_BUSACK = 5;
    static const u8 L_M1 = 6;
    static const u8 L_RFSH = 7;

    static const u8 BUS_TRANSCEIVER_ISOLATED = HIGH;
    static const u8 BUS_TRANSCEIVER_ENABLED = LOW;

    static const u8 DATA_LATCH_ISOLATED = HIGH;
    static const u8 DATA_LATCH_ENABLED = LOW;

    static const u8 LATCH_TRANSPARENT = HIGH;
    static const u8 LATCH_HOLDING = LOW;

    static const u8 DATA_BUS_DIR_OUT = 0;
    static const u8 DATA_BUS_DIR_IN = 1;
    static const u8 DATA_BUS_ISOLATED = 2;
};


#endif //Z80EMU_PICGPIOBUS_HPP
