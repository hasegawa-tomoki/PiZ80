#include "macros.hpp"
#include "cpu.hpp"
#include "mcycle.hpp"
#include "opcode.hpp"
#include <circle/time.h>

LOGMODULE("Cpu");

Cpu::Cpu(Bus *_bus, CTimer *_m_Timer)
{
    this->bus = _bus;
    this->m_Timer = _m_Timer;

    OpCode _opCode(this);
    this->opCode = _opCode;

    Registers resistors;
    this->registers = resistors;
    Registers resistorsAlternate;
    this->registers_alternate = resistorsAlternate;
    SpecialRegisters specialResistors;
    this->special_registers = specialResistors;
}

void Cpu::reset()
{
    // it resets the interrupt enable flip-flop
    this->iff1 = false;
    this->iff2 = false;
    // clears the PC and registers I and R
    this->special_registers.pc = 0;
    this->special_registers.i = 0;
    this->special_registers.r = 0;
    // sets the interrupt status to Mode 0
    this->interrupt_mode = 0;
    // all control output signals go to the inactive state
    this->bus->pin_o_m1 = true;
    this->bus->pin_o_rfsh = true;
    this->bus->pin_o_halt = true;
    this->bus->pin_o_rd = true;
    this->bus->pin_o_wr = true;
    this->bus->pin_o_mreq = true;
    this->bus->pin_o_iorq = true;
    this->bus->pin_o_busack = true;

    this->bus->syncControl();
}

void Cpu::instructionCycle(){
    #pragma clang diagnostic push
    #pragma ide diagnostic ignored "EndlessLoop"
    int instructions = 0;
    CTime ctime = CTime();
    time_t start = ctime.Get();

    while(true){
        if (this->bus->getInput(Bus::Z80_PIN_I_RESET) == Bus::PIN_LOW){
            LOGDBG("Reset activated");
            CTimer::Get()->MsDelay(200);
            this->reset();
            continue;
        }
        if (this->halt){
            LOGDBG("Halt");
            Mcycle::m1halt(this);
        } else if (this->enable_virtual_memory){
            Mcycle::m1vm(this);
        } else {
            Mcycle::m1t1(this);
            Mcycle::m1t2(this);
            Mcycle::m1t3(this);
            Mcycle::m1t4(this);
        }
        this->opCode.execute(this->executing);

        // Disable / Enable interrupt
        if (this->waitingEI > 0){
            this->waitingEI--;
            if (this->waitingEI == 0){
                //LOGDBG("INT enabled");
                this->iff1 = true;
                this->iff2 = true;
            }
        }
        if (this->waitingDI > 0){
            this->waitingDI--;
            if (this->waitingDI == 0){
                //LOGDBG("INT disabled");
                this->iff1 = false;
                this->iff2 = false;
            }
        }

        // NMI
        if ((! this->bus->getInput(Bus::Z80_PIN_I_NMI))){
            LOGDBG("NMI-activated");
            this->iff2 = this->iff1;
            this->iff1 = false;

            u16 nmi_jump_addr = 0x0066;
            this->special_registers.sp--;
            Mcycle::m3(this, this->special_registers.sp, this->special_registers.pc >> 8);
            this->special_registers.sp--;
            Mcycle::m3(this, this->special_registers.sp, this->special_registers.pc & 0xff);
            this->special_registers.pc = nmi_jump_addr;
        }
        // INT
        if ((! this->bus->getInput(Bus::Z80_PIN_I_INT)) && this->iff1){
            if (! this->bus->getInput(Bus::Z80_PIN_I_BUSRQ)){
                LOGDBG("INT-activated but BUSRQ is low.");
            } else {
                //LOGDBG("INT-activated.");
                Mcycle::int_m1t1t2t3(this);
                Mcycle::m1t4(this);

                Mcycle::m3(this, this->special_registers.sp - 2, this->special_registers.pc & 0xff);
                Mcycle::m3(this, this->special_registers.sp - 1, this->special_registers.pc >> 8);
                this->special_registers.sp -= 2;

                u8 int_vector = this->executing;
                //LOGDBG("PC: %04x  Int vector: %02x", this->special_registers.pc, int_vector);
                switch (this->interrupt_mode){
                    case 0:
                        this->opCode.execute(int_vector);
                        break;
                    case 1:
                        this->special_registers.pc = 0x0038;
                        break;
                    case 2: {
                        u16 int_vector_pointer = (this->special_registers.i << 8) + (int_vector & 0b11111110);
                        u16 int_vector_addr =
                                Mcycle::m2(this, int_vector_pointer) +
                                (Mcycle::m2(this, int_vector_pointer + 1) << 8);
                        //LOGDBG("INT mode 2 vector: %04x", int_vector_addr);
                        this->special_registers.pc = int_vector_addr;
                        break;
                    }
                    default:
                        LOGPANIC("Invalid interrupt mode.");
                        this->halt = true;
                }
            }
        }

        instructions++;
        if (instructions == 1000 * 1000){
            LOGDBG("1M instructions in %ld sec.", this->m_Timer->GetTime() - start);
            LOGDBG("  PC: %04x", this->special_registers.pc);
            start = this->m_Timer->GetTime();
            instructions = 0;
        }
        //LOGDBG("PC: %04x", this->special_registers.pc);
    }
    #pragma clang diagnostic pop
}