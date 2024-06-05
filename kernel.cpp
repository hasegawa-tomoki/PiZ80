//
// kernel.cpp
//
#include "kernel.h"
#include <circle/timer.h>

LOGMODULE("Kernel");

CKernel::CKernel()
:	m_Screen(m_Options.GetWidth(), m_Options.GetHeight()),
	m_Timer(&m_Interrupt),
    m_Serial(&m_Interrupt, TRUE),
	m_Logger(m_Options.GetLogLevel(), &m_Timer)
	// TODO: add more member initializers here
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
    }

    if (bOK){
        bOK = m_Timer.Initialize ();
    }

    // TODO: call Initialize () of added members here (if required)

    return bOK;
}

TShutdownMode CKernel::Run()
{
    //void (*handler)() = [](){ LOGDBG("Magic received"); };
    //m_Serial.RegisterMagicReceivedHandler("z80_reboot", handler);

    LOGDBG("Compile time: " __DATE__ " " __TIME__);
    LOGDBG("Hello z80");

    CGpioBus bus = CGpioBus();
    bus.syncControl();

    Cpu cpu(&bus, &m_Timer);

    cpu.instructionCycle();
	return ShutdownHalt;
}
