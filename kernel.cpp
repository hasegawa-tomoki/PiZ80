//
// kernel.cpp
//
#include "kernel.h"
#include "z80/mcycle.hpp"
#pragma clang diagnostic ignored "-Wunknown-pragmas"

LOGMODULE("Kernel");

CKernel::CKernel()
:	m_Screen(m_Options.GetWidth(), m_Options.GetHeight()),
    m_Serial(&m_Interrupt, TRUE),
	m_Timer(&m_Interrupt),
	m_Logger(m_Options.GetLogLevel(), &m_Timer),
    m_USBHCI (&m_Interrupt, &m_Timer)
{
}

boolean CKernel::Initialize()
{
    boolean bOK = TRUE;

    bOK = m_Screen.Initialize();

    if (bOK){
        bOK = m_Interrupt.Initialize ();
    }

    if (bOK){
        bOK = m_Serial.Initialize(115200);
    }

    if (bOK){
        CDevice *pTarget = m_DeviceNameService.GetDevice(m_Options.GetLogDevice(), FALSE);
        if (pTarget == nullptr){
            pTarget = &m_Screen;
        }

        bOK = m_Logger.Initialize(pTarget);
        //bOK = m_Logger.Initialize(&m_Serial);
    }

    if (bOK){
        bOK = m_Timer.Initialize ();
    }

    if (bOK)
    {
        bOK = m_USBHCI.Initialize ();
    }

    return bOK;
}

TShutdownMode CKernel::Run()
{
    //void (*handler)() = [](){ LOGDBG("Magic received"); };
    //m_Serial.RegisterMagicReceivedHandler("z80_reboot", handler);

    LOGDBG("Compile time: " __DATE__ " " __TIME__);
    LOGDBG("Hello z80");

    CCPUThrottle CpuThrottle(CPUSpeedMaximum);

    CGpioBus bus = CGpioBus();
    bus.syncControl();

    Cpu cpu(&bus, &m_Timer);

    //return ROMCheck(&cpu);
    //return RAMCheck(&cpu);
    //return MemoryDump(&cpu);

    cpu.instructionCycle();

    LOGPANIC("ShutdownHalt");
	return ShutdownHalt;
}

TShutdownMode CKernel::MemoryDump(Cpu* cpu)
{
    #define PARTITION	"umsd1-1"
    #define FILENAME	"dump.txt"

    u16 from = 0x0000;
    u16 to = 0x7fff;

    unsigned hFile = openFile();

    for (u16 addr = from; addr <= to; addr += 16){
        CString buffer;
        buffer.Format("%04x ", addr);
        for (u16 i = 0; i < 16; i++){
            u8 data = Mcycle::m2(cpu, addr + i);
            CString byteString;
            byteString.Format("%02x ", data);
            buffer.Append(byteString);
        }
        LOGDBG(buffer);
        buffer.Append("\n");
        m_FileSystem.FileWrite(hFile, buffer, buffer.GetLength());
    }

    closeFile(hFile);

    return ShutdownHalt;
}

unsigned CKernel::openFile()
{
    // Mount file system
    CDevice *pPartition = m_DeviceNameService.GetDevice(PARTITION, TRUE);
    if (pPartition == 0)
    {
        LOGPANIC("Partition not found: %s", PARTITION);
    }

    if (! m_FileSystem.Mount(pPartition))
    {
        LOGPANIC("Cannot mount partition: %s", PARTITION);
    }

    unsigned hFile = m_FileSystem.FileCreate (FILENAME);
    if (hFile == 0)
    {
        LOGPANIC("Cannot create file: %s", FILENAME);
    }

    return hFile;
}

void CKernel::closeFile(unsigned hFile)
{
    if (!m_FileSystem.FileClose(hFile))
    {
        LOGPANIC("Cannot close file");
    }

    m_FileSystem.UnMount();
}

TShutdownMode CKernel::ROMCheck(Cpu* cpu)
{
    //// ROM check
    const u8 rom[32] = {
            0xf3, 0x31, 0xed, 0x80, 0xc3, 0x1b, 0x00, 0xff, 0xc3, 0xc9, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xc3, 0x9d, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xc3, 0x00, 0x21, 0x00, 0x80, 0x11, 0x00,
    };
    int maxCount = 100;
    for (int i = 0; i < maxCount; i++){
        LOGDBG("Trying %d / %d", i, maxCount);
        for (u16 address = 0; address < (u16)(sizeof rom); address++){
            u16 data = Mcycle::m2(cpu, address);
            if (data != rom[address]){
                LOGDBG("ERROR Address: %04x  Data: %02x  Read: %02x", address, rom[address], data);
                LOGPANIC("ShutdownHalt");
                return ShutdownHalt;
            }
        }
    }
    LOGDBG("ROM check OK");
    LOGPANIC("ShutdownHalt");
    return ShutdownHalt;
}

TShutdownMode CKernel::RAMCheck(Cpu* cpu)
{
    ////// RAM flip check
    u16 start = 0x8000;
    u16 end = 0x80ff;
    for (u16 data = 0; data < 0x100; data++){
        for (u16 address = start; address <= end; address++){
            Mcycle::m3(cpu, address, data);
        }
        LOGDBG("Offset: %x", data);
        for (u16 address = start; address <= end; address++){
            if (address % 16 == 0){
                LOGDBG("    %04x ", address);
            }
            u8 before = Mcycle::m2(cpu, address);
            u8 flip = ~before;
            Mcycle::m3(cpu, address, flip);
            u8 after = Mcycle::m2(cpu, address);
            if (before != after){
                if (flip == after){
                    // OK
                } else {
                    LOGDBG("NG (flip != after) before: %02x flip: %02x after: %02x ", before, flip, after);
                    LOGPANIC("ShutdownHalt");
                    return ShutdownHalt;
                }
            } else {
                LOGDBG("NG (before == after)  before: %02x  flip: %02x  after: %02x", before, flip, after);
                LOGPANIC("ShutdownHalt");
                return ShutdownHalt;
            }
            if (address == end){
                LOGDBG("RAM access check OK");
                break;
            }
        }
    }
    LOGPANIC("ShutdownHalt");
    return ShutdownHalt;
}
/*
    //// ROM check
    const u8 rom[32] = {
            0xf3, 0x31, 0xed, 0x80, 0xc3, 0x1b, 0x00, 0xff, 0xc3, 0xc9, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xc3, 0x9d, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xc3, 0x00, 0x21, 0x00, 0x80, 0x11, 0x00,
    };
    int maxCount = 100;
    for (int i = 0; i < maxCount; i++){
        LOGDBG("Trying %d / %d", i, maxCount);
        for (u16 address = 0; address < (u16)(sizeof rom); address++){
            u16 data = Mcycle::m2(&cpu, address);
            if (data != rom[address]){
                LOGDBG("ERROR Address: %04x  Data: %02x  Read: %02x", address, rom[address], data);
                LOGPANIC("ShutdownHalt");
                return ShutdownHalt;
            }
        }
    }
    LOGDBG("ROM check OK");
    LOGPANIC("ShutdownHalt");
    return ShutdownHalt;

   ////// RAM flip check
    u16 start = 0x8000;
    u16 end = 0x80ff;
    for (u16 address = start; address <= end; address++){
        Mcycle::m3(&cpu, address, 0x55);
    }
    for (u16 address = start; address <= end; address++){
        if (address % 16 == 0){
            LOGDBG("%04x ", address);
        }
        u8 before = Mcycle::m2(&cpu, address);
        u8 flip = ~before;
        Mcycle::m3(&cpu, address, flip);
        u8 after = Mcycle::m2(&cpu, address);
        if (before != after){
            if (flip == after){
                // OK
            } else {
                LOGDBG("NG (flip != after) before: %02x flip: %02x after: %02x ", before, flip, after);

                LOGPANIC("ShutdownHalt");
                return ShutdownHalt;
            }
        } else {
            LOGDBG("NG (before == after)  before: %02x  flip: %02x  after: %02x", before, flip, after);

            LOGPANIC("ShutdownHalt");
            return ShutdownHalt;
        }
        if (address == end){
            LOGDBG("RAM access check OK");
            break;
        }
    }
    LOGPANIC("ShutdownHalt");
    return ShutdownHalt;

    // Input Pins test
    bus.selectInput(1);
    while(true){
        u32 bits = CGPIOPin::ReadAll();

        CString buffer;

        CString waitString;
        waitString.Format("WAIT: %d  ", (bits >> CGpioBus::RPi_BUS_IN[0]) & 0x01);
        buffer.Append(waitString);
        CString busRqString;
        busRqString.Format("BUSRQ: %d  ", (bits >> CGpioBus::RPi_BUS_IN[1]) & 0x01);
        buffer.Append(busRqString);
        CString resetString;
        resetString.Format("RESET: %d  ", (bits >> CGpioBus::RPi_BUS_IN[2]) & 0x01);
        buffer.Append(resetString);
        CString intString;
        intString.Format("INT: %d  ", (bits >> CGpioBus::RPi_BUS_IN[3]) & 0x01);
        buffer.Append(intString);
        CString nmiString;
        nmiString.Format("NMI: %d  ", (bits >> CGpioBus::RPi_BUS_IN[4]) & 0x01);
        buffer.Append(nmiString);
        CString clkString;
        clkString.Format("CLK: %d  ", (bits >> CGpioBus::RPi_BUS_IN[5]) & 0x01);
        buffer.Append(clkString);
        LOGDBG(buffer);
        CTimer::Get()->MsDelay(1000);
    }

    // Primitive memory access test
    CString buffer;
    for (u8 i = 0; i < 0xff; i++){
        u16 addr = i << 8;
        bus.setAddress(addr);

        buffer = CString();
        buffer.Format("Address: %04x ", addr);
        LOGDBG(buffer);
    }
    CString buffer;
    for (u8 i = 0; i < 0xff; i++){
        bus.setDataBegin(i);

        buffer = CString();
        buffer.Format("Data: %02x ", i);
        LOGDBG(buffer);
    }

    for (u16 addr = 0; addr < 32; addr += 16){
        CString buffer;
        buffer.Format("%04x ", addr);
        for (u16 i = 0; i < 16; i++){
            u8 data = Mcycle::m2(&cpu, addr + i);
            CString byteString;
            byteString.Format("%02x ", data);
            buffer.Append(byteString);
        }
        LOGDBG(buffer);
    }

    while(true){
        for (u16 addr = 0; addr < 32; addr += 16){
            CString buffer;
            buffer.Format("%04x ", addr);
            u8 sum = 0;
            for (u16 i = 0; i < 16; i++){
                u8 data = Mcycle::m2(&cpu, addr + i);
                CString byteString;
                byteString.Format("%02x ", data);
                buffer.Append(byteString);
                sum += data;
            }
            CString sumString;
            sumString.Format("  Sum: %02x", sum);
            buffer.Append(sumString);

            LOGDBG(buffer);
        }
        LOGDBG("---");
        CTimer::Get()->MsDelay(2000);
    }
    LOGPANIC("ShutdownHalt");
    return ShutdownHalt;
*/
