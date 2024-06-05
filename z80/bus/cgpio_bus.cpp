#include <circle/gpiopin.h>
#include "cgpio_bus.hpp"
#include "../macros.hpp"

const u8 CGpioBus::RPi_BUS_OUT[8] = {21, 26, 20, 19, 16, 13, 12, 6};
const u8 CGpioBus::RPi_BUS_IN[8] = {5, 1, 0, 7, 8, 11, 25, 9};

LOGMODULE("CGpioBus");

CGpioBus::CGpioBus()
  : m_DATA_OE(RPi_GPIO_DATA_OE, GPIOModeOutput)
{
    // Initialize GPIOs for output
    LOGDBG("Initializing GPIOs for output");
    for (int i = 0; i <= 7; i++) {
        LOGDBG("  RPi.OUT%d = GPIO %d", i, CGpioBus::RPi_BUS_OUT[i]);
        m_OUT[i] = CGPIOPin(CGpioBus::RPi_BUS_OUT[i], GPIOModeInputPullDown);
        m_OUT[i].SetPullMode(GPIOPullModeOff);
        m_OUT[i].Write(LOW);
    }
    // Initialize GPIOs for input
    LOGDBG("Initializing GPIOs for input");
    for (int i = 0; i <= 7; i++){
        LOGDBG("  RPi.IN%d = GPIO %d", i, CGpioBus::RPi_BUS_IN[i]);
        m_IN[i] = CGPIOPin(CGpioBus::RPi_BUS_IN[i], GPIOModeOutput);
        m_IN[i].SetPullMode(GPIOPullModeOff);
        m_IN[i].Write(LOW);
    }
    // Data bus direction
    // DATA_OE: high = U5 is isolated from the data bus, low: U5 is connected to the data bus
    m_DATA_OE.Write(HIGH);

    // Control pins for Output latch
    // BA: Y0 Y1 Y2 Y3
    //   LL: L H H H      LE of Higher 8bit of address bus
    //   LH: H L H H      LE of Lower 8bit of address bus
    //   HL: H H L H      LE of Outgoing control signals
    //   HH: H H H L      LE of Data bus
    // G: high = H H H H  To hold the latch output, Switch the GBA from H-any-any > L-target-target.
    m_LC_OUT_A = CGPIOPin(RPi_GPIO_LC_OUT_A, GPIOModeOutput);
    m_LC_OUT_B = CGPIOPin(RPi_GPIO_LC_OUT_B, GPIOModeOutput);
    m_LC_OUT_G = CGPIOPin(RPi_GPIO_LC_OUT_G, GPIOModeOutput);
    m_LC_OUT_G.Write(HIGH);
    // Control pins for Input bus transceiver
    // BA: Y0 Y1 Y2 Y3
    //   LL: L H H H      Connect incoming control signals
    //   LH: H L H H      Connect data bus
    //   HL: H H L H      Not used
    //   HH: H H H L      Not used
    // G: high = H H H H  GAB = H-any-any: Isolate all signals, L-target-target: Connect the selected signals to the RPi GPIOs for input.
    m_LC_IN_A = CGPIOPin(RPi_GPIO_LC_IN_A, GPIOModeOutput);
    m_LC_IN_B = CGPIOPin(RPi_GPIO_LC_IN_B, GPIOModeOutput);
    m_LC_IN_G = CGPIOPin(RPi_GPIO_LC_IN_G, GPIOModeOutput);
    m_LC_IN_G.Write(HIGH);
}

/**
 * @param device LATCH_ADDRESS_LOW|LATCH_ADDRESS_HIGH|LATCH_CONTROL_OUTPUT|LATCH_DATA
 */
void CGpioBus::latchHold(u8 device){
    m_LC_OUT_G.Write(HIGH);
    switch (device) {
        case LATCH_ADDRESS_LOW:
            m_LC_OUT_B.Write(LOW);
            m_LC_OUT_A.Write(LOW);
            break;
        case LATCH_ADDRESS_HIGH:
            m_LC_OUT_B.Write(LOW);
            m_LC_OUT_A.Write(HIGH);
            break;
        case LATCH_CONTROL_OUTPUT:
            m_LC_OUT_B.Write(HIGH);
            m_LC_OUT_A.Write(LOW);
            break;
        case LATCH_DATA:
            m_LC_OUT_B.Write(HIGH);
            m_LC_OUT_A.Write(HIGH);
            break;
        default:
            LOGPANIC("Invalid device (selectOutput)");
            HALT()
    }
    m_LC_OUT_G.Write(LOW);
}
/**
 * @param device BUS_TRANSCEIVER_DATA|BUS_TRANSCEIVER_CONTROL_INPUT
 */
void CGpioBus::selectInput(u8 device){
    switch (device) {
        case BUS_TRANSCEIVER_DATA:
            m_LC_IN_B.Write(LOW);
            m_LC_IN_A.Write(LOW);
            break;
        case BUS_TRANSCEIVER_CONTROL_INPUT:
            m_LC_IN_B.Write(LOW);
            m_LC_IN_A.Write(HIGH);
            break;
        default:
            LOGPANIC("Invalid device (selectInput)");
            HALT()
    }
}
/**
 * @param direction DATA_BUS_DIR_OUT|DATA_BUS_DIR_IN
 */
void CGpioBus::setDataBusDirection(bool direction){
    m_DATA_OE.Write(LOW);

    if (direction == DATA_BUS_DIR_OUT){
        // Enable latch
        m_DATA_OE.Write(LATCH_FOR_DATA_BUS_ENABLED);
        // Isolate bus transceivers
        m_LC_IN_G.Write(HIGH);
    } else {
        // Isolate latch
        m_DATA_OE.Write(LATCH_FOR_DATA_BUS_ISOLATED);
        // Enable bus transceivers
        m_LC_IN_G.Write(LOW);
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
}

void CGpioBus::setDataEnd(){
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
    if (this->pin_o_m1){ control |= (1 << L_M1); }
    if (this->pin_o_rfsh){ control |= (1 << L_RFSH); }
    if (this->pin_o_halt){ control |= (1 << L_HALT); }
    if (this->pin_o_rd){ control |= (1 << L_RD); }
    if (this->pin_o_wr){ control |= (1 << L_WR); }
    if (this->pin_o_mreq){ control |= (1 << L_MREQ); }
    if (this->pin_o_iorq){ control |= (1 << L_IORQ); }
    if (this->pin_o_busack){ control |= (1 << L_BUSACK); }

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

void CGpioBus::waitClockRising(){

    //LOGDBG("Clock rising edge detected.");

    // Commented out because the operating speed is significantly slow relative to the clock.
    /*
    if (gpioRead(RPi_GPIO_I_CLK)){
        while(gpioRead(RPi_GPIO_I_CLK));
    }
    while(! gpioRead(RPi_GPIO_I_CLK));
     */
}
void CGpioBus::waitClockFalling(){

    //LOGDBG("Clock falling edge detected.");

    // Commented out because the operating speed is significantly slow relative to the clock.
    /*
    if (! gpioRead(RPi_GPIO_I_CLK)){
        while(! gpioRead(RPi_GPIO_I_CLK));
    }
    while(gpioRead(RPi_GPIO_I_CLK));
     */
}
