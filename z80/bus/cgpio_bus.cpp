#include <circle/gpiopin.h>
#include "cgpio_bus.hpp"
#include "../macros.hpp"

const u8 CGpioBus::RPi_BUS_OUT[8] = {21, 26, 20, 19, 16, 13, 12, 6};
const u8 CGpioBus::RPi_BUS_IN[8] = {5, 1, 0, 7, 8, 11, 25, 9};

LOGMODULE("CGpioBus");

CGpioBus::CGpioBus()
   : m_OE_DATA_OUT(RPi_GPIO_OE_DATA_OUT, GPIOModeOutput),
     m_OE_CTRL_IN(RPi_GPIO_OE_CTRL_IN, GPIOModeOutput),
     m_OE_DATA_IN(RPi_GPIO_OE_DATA_IN, GPIOModeOutput),
     m_LE_ADDR_LOW(RPi_GPIO_LE_ADDR_LOW, GPIOModeOutput),
     m_LE_ADDR_HIGH(RPi_GPIO_LE_ADDR_HIGH, GPIOModeOutput),
     m_LE_DATA(RPi_GPIO_LE_DATA, GPIOModeOutput),
     m_LE_CTRL(RPi_GPIO_LE_CTRL, GPIOModeOutput)
{
    // Initialize GPIOs for output
    LOGDBG("Initializing GPIOs for output");
    for (int i = 0; i <= 7; i++) {
        LOGDBG("  RPi.OUT%d = GPIO %d", i, CGpioBus::RPi_BUS_OUT[i]);
        m_OUT[i] = CGPIOPin(CGpioBus::RPi_BUS_OUT[i], GPIOModeOutput);
        m_OUT[i].Write(LOW);
    }
    // Initialize GPIOs for input
    LOGDBG("Initializing GPIOs for input");
    for (int i = 0; i <= 7; i++){
        LOGDBG("  RPi.IN%d = GPIO %d", i, CGpioBus::RPi_BUS_IN[i]);
        m_IN[i] = CGPIOPin(CGpioBus::RPi_BUS_IN[i], GPIOModeInputPullDown);
    }
    // Data bus direction
    // m_OE_DATA_IN: high = U6 is isolated from the data bus, low: U6 is connected to the data bus
    m_OE_DATA_IN.Write(BUS_TRANSCEIVER_ISOLATED);
    m_OE_CTRL_IN.Write(BUS_TRANSCEIVER_ISOLATED);

    // Output data initialize
    latchHold(LATCH_ADDRESS_HIGH);
    latchHold(LATCH_ADDRESS_LOW);
    latchHold(LATCH_DATA);

    // Control initialize
    for (int i = 0; i <= 7; i++) {
        m_OUT[i].Write(HIGH);
    }
    latchHold(LATCH_CONTROL_OUTPUT);
}

/**
 * @param device LATCH_ADDRESS_LOW|LATCH_ADDRESS_HIGH|LATCH_CONTROL_OUTPUT|LATCH_DATA
 */
void CGpioBus::latchHold(u8 device){
    switch (device) {
        case LATCH_ADDRESS_LOW:
            m_LE_ADDR_LOW.Write(LATCH_TRANSPARENT);
            m_LE_ADDR_LOW.Write(LATCH_HOLDING);
            break;
        case LATCH_ADDRESS_HIGH:
            m_LE_ADDR_HIGH.Write(LATCH_TRANSPARENT);
            m_LE_ADDR_HIGH.Write(LATCH_HOLDING);
            break;
        case LATCH_DATA:
            m_LE_DATA.Write(LATCH_TRANSPARENT);
            m_LE_DATA.Write(LATCH_HOLDING);
            break;
        case LATCH_CONTROL_OUTPUT:
            m_LE_CTRL.Write(LATCH_TRANSPARENT);
            m_LE_CTRL.Write(LATCH_HOLDING);
            break;
        default:
            LOGPANIC("Invalid device (selectOutput)");
            HALT()
    }
}
/**
 * @param device BUS_TRANSCEIVER_DATA|BUS_TRANSCEIVER_CONTROL
 */
void CGpioBus::selectInput(u8 device){
    switch (device) {
        case BUS_TRANSCEIVER_DATA:
            if (inputMode != BUS_TRANSCEIVER_DATA){
                m_OE_CTRL_IN.Write(BUS_TRANSCEIVER_ISOLATED);
                m_OE_DATA_IN.Write(BUS_TRANSCEIVER_ENABLED);
                inputMode = BUS_TRANSCEIVER_DATA;
            }
            break;
        case BUS_TRANSCEIVER_CONTROL:
            if (inputMode != BUS_TRANSCEIVER_CONTROL){
                m_OE_DATA_IN.Write(BUS_TRANSCEIVER_ISOLATED);
                m_OE_CTRL_IN.Write(BUS_TRANSCEIVER_ENABLED);
                inputMode = BUS_TRANSCEIVER_CONTROL;
            }
            break;
        default:
            LOGPANIC("Invalid device (selectInput)");
            HALT()
    }
}
/**
 * @param direction DATA_BUS_DIR_OUT|DATA_BUS_DIR_IN
 */
void CGpioBus::setDataBusDirection(u8 direction){
    if (direction == DATA_BUS_DIR_OUT){
        if (dataBusDirection != DATA_BUS_DIR_OUT){
            // Isolate bus transceivers
            m_OE_DATA_IN.Write(BUS_TRANSCEIVER_ISOLATED);
            // Enable latch
            m_OE_DATA_OUT.Write(DATA_LATCH_ENABLED);

            dataBusDirection = DATA_BUS_DIR_OUT;
        }
    } else if (direction == DATA_BUS_DIR_IN){
        if (dataBusDirection != DATA_BUS_DIR_IN){
            // Isolate latch
            m_OE_DATA_OUT.Write(DATA_LATCH_ISOLATED);
            // Enable bus transceivers
            m_OE_DATA_IN.Write(BUS_TRANSCEIVER_ENABLED);

            dataBusDirection = DATA_BUS_DIR_IN;
        }
    } else if (direction == DATA_BUS_ISOLATED){
        if (dataBusDirection != DATA_BUS_ISOLATED){
            m_OE_DATA_IN.Write(BUS_TRANSCEIVER_ISOLATED);
            m_OE_DATA_OUT.Write(DATA_LATCH_ISOLATED);

            dataBusDirection = DATA_BUS_ISOLATED;
        }
    }
}


void CGpioBus::setAddress(u16 addr){
    u32 mask = 0;
    for (u8 i = 0; i <= 7; i++){
        mask |= (1 << CGpioBus::RPi_BUS_OUT[i]);
    }

    // Upper 8bit of address to GPIO 0 - 7
    if ((addr & 0xff00) != (this->address & 0xff00)){
        u32 upper = 0;
        for (u8 i = 0; i <= 7; i++){
            upper |= ((addr >> (i + 8)) & 0x01) << CGpioBus::RPi_BUS_OUT[i];
        }
        CGPIOPin::WriteAll(upper, mask);
        latchHold(LATCH_ADDRESS_HIGH);
    }
    // Lower 8bit of address to GPIO 0 - 7
    if ((addr & 0x00ff) != (this->address & 0x00ff)) {
        u32 lower = 0;
        for (u8 i = 0; i <= 7; i++){
            lower |= ((addr >> i) & 0x01) << CGpioBus::RPi_BUS_OUT[i];
        }
        CGPIOPin::WriteAll(lower, mask);
        latchHold(LATCH_ADDRESS_LOW);
    }

    this->address = addr;
}

void CGpioBus::setDataBegin(u8 data){
    setDataBusDirection(DATA_BUS_DIR_OUT);

    u32 mask = 0;
    for (u8 i = 0; i <= 7; i++){
        mask |= (1 << CGpioBus::RPi_BUS_OUT[i]);
    }

    u32 dataBits = 0;
    for (u8 i = 0; i <= 7; i++){
        dataBits |= ((data >> i) & 0x01) << CGpioBus::RPi_BUS_OUT[i];
    }
    CGPIOPin::WriteAll(dataBits, mask);
    latchHold(LATCH_DATA);
}

void CGpioBus::setDataEnd(){
    setDataBusDirection(DATA_BUS_ISOLATED);
}

u8 CGpioBus::getData(){
    setDataBusDirection(DATA_BUS_DIR_IN);
    selectInput(BUS_TRANSCEIVER_DATA);

    u32 bits = CGPIOPin::ReadAll();
    u8 data = 0;
    for (u8 i = 0; i <= 7; i++){
        data |= ((bits >> CGpioBus::RPi_BUS_IN[i]) & 0x01) << i;
    }

    return data;
}

void CGpioBus::setControl(u8 z80PinName, bool level){
    switch (z80PinName){
        case Z80_PIN_O_HALT:    this->pin_o_halt = level;   break;
        case Z80_PIN_O_MERQ:    this->pin_o_mreq = level;   break;
        case Z80_PIN_O_IORQ:    this->pin_o_iorq = level;   break;
        case Z80_PIN_O_RD:      this->pin_o_rd = level;     break;
        case Z80_PIN_O_WR:      this->pin_o_wr = level;     break;
        case Z80_PIN_O_BUSACK:  this->pin_o_busack = level; break;
        case Z80_PIN_O_M1:      this->pin_o_m1 = level;     break;
        case Z80_PIN_O_RFSH:    this->pin_o_rfsh = level;   break;
        default:
            LOGPANIC("Invalid Z80 pin (setControl)");
            HALT()
    }
}

bool CGpioBus::getInput(u8 z80PinName){
    selectInput(BUS_TRANSCEIVER_CONTROL);
    u32 bits = CGPIOPin::ReadAll();

    switch (z80PinName){
        case Z80_PIN_I_WAIT:
            return (bits >> CGpioBus::RPi_BUS_IN[0]) & 0x01;
        case Z80_PIN_I_BUSRQ:
            return (bits >> CGpioBus::RPi_BUS_IN[1]) & 0x01;
        case Z80_PIN_I_RESET:
            return (bits >> CGpioBus::RPi_BUS_IN[2]) & 0x01;
        case Z80_PIN_I_INT:
            return (bits >> CGpioBus::RPi_BUS_IN[3]) & 0x01;
        case Z80_PIN_I_NMI:
            return (bits >> CGpioBus::RPi_BUS_IN[4]) & 0x01;
        case Z80_PIN_I_CLK:
            return (bits >> CGpioBus::RPi_BUS_IN[5]) & 0x01;
        default:
            LOGPANIC("Invalid Z80 pin (getInput)");
            HALT()
    }
}

void CGpioBus::syncControl(){
    u8 control = 0;
    if (this->pin_o_halt){ control |= (1 << L_HALT); }
    if (this->pin_o_mreq){ control |= (1 << L_MREQ); }
    if (this->pin_o_iorq){ control |= (1 << L_IORQ); }
    if (this->pin_o_rd){ control |= (1 << L_RD); }
    if (this->pin_o_wr){ control |= (1 << L_WR); }
    if (this->pin_o_busack){ control |= (1 << L_BUSACK); }
    if (this->pin_o_m1){ control |= (1 << L_M1); }
    if (this->pin_o_rfsh){ control |= (1 << L_RFSH); }

    u32 mask = 0;
    for (u8 i = 0; i <= 7; i++){
        mask |= (1 << CGpioBus::RPi_BUS_OUT[i]);
    }
    u32 bits = 0;
    for (u8 i = 0; i <= 7; i++){
        bits |= ((control >> i) & 0x01) << CGpioBus::RPi_BUS_OUT[i];
    }

    CGPIOPin::WriteAll(bits, mask);
    latchHold(LATCH_CONTROL_OUTPUT);
}

void CGpioBus::waitClockRising(bool force){
    if (! force){
        return;
    }
    if (this->getInput(Z80_PIN_I_CLK)){
        while(this->getInput(Z80_PIN_I_CLK));
    }
    while(! this->getInput(Z80_PIN_I_CLK));
}
void CGpioBus::waitClockFalling(bool force){
    if (! force){
        return;
    }
    if (! this->getInput(Z80_PIN_I_CLK)){
        while(! this->getInput(Z80_PIN_I_CLK));
    }
    while(this->getInput(Z80_PIN_I_CLK));
}
