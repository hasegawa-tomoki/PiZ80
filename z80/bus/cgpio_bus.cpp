#include <circle/gpiopin.h>
#include "cgpio_bus.hpp"
#include "../macros.hpp"

const u8 CGpioBus::RPi_GPIO_L_A[8] = {0, 1, 2, 3, 4, 5, 6, 7};
const u8 CGpioBus::RPi_GPIO_D[8] = {8, 9, 10, 11, 12, 13, 18, 27};

LOGMODULE("CGpioBus");

CGpioBus::CGpioBus()
  : m_I_RESET(RPi_GPIO_I_RESET, GPIOModeInput),
    m_I_CLK(RPi_GPIO_I_CLK, GPIOModeInput),
    //m_I_NMI(RPi_GPIO_I_NMI, GPIOModeInput),
    m_I_INT(RPi_GPIO_I_INT, GPIOModeInput),
    m_I_WAIT(RPi_GPIO_I_WAIT, GPIOModeInput),
    m_I_BUSRQ(RPi_GPIO_I_BUSRQ, GPIOModeInput),
    m_DATA_BUS_DIR(RPi_GPIO_DATA_BUS_DIR, GPIOModeOutput),
    m_LE_ADDRESS_LOW(RPi_GPIO_LE_ADDRESS_LOW, GPIOModeOutput),
    m_LE_ADDRESS_HIGH(RPi_GPIO_LE_ADDRESS_HIGH, GPIOModeOutput),
    m_LE_CONTROL(RPi_GPIO_LE_CONTROL, GPIOModeOutput),
    m_DATA_BUS_OE(RPi_GPIO_DATA_BUS_OE, GPIOModeOutput)
{
    // Initialize GPIOs for Latch
    LOGDBG("Initializing GPIOs for Latch");
    for (int i = 0; i <= 7; i++){
        LOGDBG("Latch A%d = GPIO %d", i, CGpioBus::RPi_GPIO_L_A[i]);
        m_ADDR[i] = CGPIOPin(CGpioBus::RPi_GPIO_L_A[i], GPIOModeOutput);
        m_ADDR[i].SetPullMode(GPIOPullModeOff);
        m_ADDR[i].Write(LOW);
    }
    // Initialize GPIOs for Data
    LOGDBG("Initializing GPIOs for Data");
    for (int i = 0; i <= 7; i++) {
        LOGDBG("Data bus D%d = GPIO %d", i, CGpioBus::RPi_GPIO_D[i]);
        m_DATA[i] = CGPIOPin(CGpioBus::RPi_GPIO_D[i], GPIOModeInputPullDown);
        m_DATA[i].SetPullMode(GPIOPullModeOff);
        m_DATA[i].Write(LOW);
    }
    // Latch selector
    m_LE_ADDRESS_LOW.Write(HIGH);
    m_LE_ADDRESS_LOW.Write(LOW);
    m_LE_ADDRESS_HIGH.Write(HIGH);
    m_LE_ADDRESS_HIGH.Write(LOW);
    m_LE_CONTROL.Write(HIGH);
    m_LE_CONTROL.Write(LOW);

    // Data bus Isolation (0: Enable 1: Isolated)
    m_DATA_BUS_OE.Write(DATA_BUS_ISOLATED);
    // Data bus Direction (0: input, 1: output)
    m_DATA_BUS_DIR.Write(DATA_BUS_DIR_OUT);
    // Input Pins
    m_I_RESET.SetPullMode(GPIOPullModeOff);
    m_I_CLK.SetPullMode(GPIOPullModeOff);
    //m_I_NMI.SetPullMode(GPIOPullModeOff);
    m_I_INT.SetPullMode(GPIOPullModeOff);
    m_I_WAIT.SetPullMode(GPIOPullModeOff);
    m_I_BUSRQ.SetPullMode(GPIOPullModeOff);
}

void CGpioBus::setAddress(u16 addr){
    u32 mask = 0x000000ff;
    // Upper 8bit of address to GPIO 0 - 7
    if ((addr & 0xff00) != (this->address & 0xff00)){
        u32 upper = addr >> 8;
        CGPIOPin::WriteAll(upper, mask);

        m_LE_ADDRESS_HIGH.Write(HIGH);
        //CTimer::Get()->nsDelay(10);
        m_LE_ADDRESS_HIGH.Write(LOW);
        //CTimer::Get()->nsDelay(10);
    }
    // Lower 8bit of address to GPIO 0 - 7
    if ((addr & 0x00ff) != (this->address & 0x00ff)) {
        u32 lower = (addr & 0xff);
        CGPIOPin::WriteAll(lower, mask);

        m_LE_ADDRESS_LOW.Write(HIGH);
        //CTimer::Get()->nsDelay(10);
        m_LE_ADDRESS_LOW.Write(LOW);
        //CTimer::Get()->nsDelay(10);
    }

    this->address = addr;
}

void CGpioBus::setDataBusOutput(){
    if (this->currentDataBusMode != DATA_BUS_DIR_OUT){
        this->currentDataBusMode = DATA_BUS_DIR_OUT;

        for (int i = 0; i <= 7; i++) {
            m_DATA[i].SetMode(GPIOModeOutput);
        }
        m_DATA_BUS_DIR.Write(DATA_BUS_DIR_OUT);
    }
}
void CGpioBus::setDataBusInput(){
    if (this->currentDataBusMode != DATA_BUS_DIR_IN){
        this->currentDataBusMode = DATA_BUS_DIR_IN;

        for (int i = 0; i <= 7; i++) {
            m_DATA[i].SetMode(GPIOModeInputPullDown);
        }
        m_DATA_BUS_DIR.Write(DATA_BUS_DIR_IN);
    }
}

void CGpioBus::setDataBegin(u8 data){
    setDataBusOutput();

    m_DATA_BUS_OE.Write(DATA_BUS_ENABLED);
    //u32 mask = 0x0000ff00;
    u32 mask = 0;
    for (u8 i = 0; i <= 7; i++){
        mask |= (1 << CGpioBus::RPi_GPIO_D[i]);
    }
    u32 dataBits = 0;
    for (u8 i = 0; i <= 7; i++){
        dataBits |= ((data >> i) & 0x01) << CGpioBus::RPi_GPIO_D[i];
    }
    //CGPIOPin::WriteAll(data << 8, mask);
    CGPIOPin::WriteAll(dataBits, mask);
}

void CGpioBus::setDataEnd(){
    m_DATA_BUS_OE.Write(DATA_BUS_ISOLATED);
}

u8 CGpioBus::getData(){
    setDataBusInput();

    m_DATA_BUS_OE.Write(DATA_BUS_ENABLED);
    u32 bits = CGPIOPin::ReadAll();
    //u8 data = 0x000000ff & (bits >> 8);
    u8 data = 0;
    for (u8 i = 0; i <= 7; i++){
        data |= ((bits >> CGpioBus::RPi_GPIO_D[i]) & 0x01) << i;
    }
    m_DATA_BUS_OE.Write(DATA_BUS_ISOLATED);

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
    switch (z80PinName){
        case Z80_PIN_I_CLK:
            return m_I_CLK.Read();
        case Z80_PIN_I_INT:
            return m_I_INT.Read();
        case Z80_PIN_I_NMI:
            //return m_I_NMI.Read();
            return true;
        case Z80_PIN_I_WAIT:
            return m_I_WAIT.Read();
        case Z80_PIN_I_BUSRQ:
            return m_I_BUSRQ.Read();
        case Z80_PIN_I_RESET:
            return m_I_RESET.Read();
        default:
            LOGPANIC("Invalid Z80 pin (setControl)");
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

    CGPIOPin::WriteAll(control, 0xff);

    m_LE_CONTROL.Write(HIGH);
    m_LE_CONTROL.Write(LOW);
}

void CGpioBus::waitClockRising(){
    return;
    if (m_I_CLK.Read()){
        while(m_I_CLK.Read());
    }
    while (! m_I_CLK.Read());

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
    return;
    if (! m_I_CLK.Read()){
        while(! m_I_CLK.Read());
    }
    while (m_I_CLK.Read());

    //LOGDBG("Clock falling edge detected.");

    // Commented out because the operating speed is significantly slow relative to the clock.
    /*
    if (! gpioRead(RPi_GPIO_I_CLK)){
        while(! gpioRead(RPi_GPIO_I_CLK));
    }
    while(gpioRead(RPi_GPIO_I_CLK));
     */
}
