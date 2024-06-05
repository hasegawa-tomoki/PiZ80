#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef Z80EMU_Z80_HPP
#define Z80EMU_Z80_HPP
#include <circle/time.h>
#include "registers.hpp"
#include "special_registers.hpp"
#include "opcode.hpp"
#include "bus/cgpio_bus.hpp"

class Cpu
{
public:
    explicit Cpu(Bus *bus, CTimer *m_Timer);

    Bus *bus;
    CTimer *m_Timer;
    OpCode opCode;
    SpecialRegisters special_registers;
    Registers registers;
    Registers registers_alternate;

    bool enable_virtual_memory = false;
    u8 virtual_memory[32] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    // refs: https://www.seasip.info/Cpm/bdos.html
    bool emulate_cpm_bdos_call = false;

    u32 tick = 0;

    bool iff1 = false;
    bool iff2 = false;
    bool halt = false;

    u8 waitingEI = 0;
    u8 waitingDI = 0;
    /*
    bool NMI_activated = false;
    bool INT_activated = false;
     */

    u8 interrupt_mode = 0;

    u8 executing = 0;

    void reset();

    void instructionCycle();
};

#endif //Z80EMU_Z80_HPP

#pragma clang diagnostic pop