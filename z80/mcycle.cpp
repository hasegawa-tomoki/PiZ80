#include "mcycle.hpp"
#include "cpu.hpp"

LOGMODULE("Mcycle");

void Mcycle::int_m1t1t2t3(Cpu *cpu){
    // t1
    cpu->bus->waitClockRising(false);
    cpu->bus->setAddress(cpu->special_registers.pc);
    cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
    cpu->bus->pin_o_rd = Bus::PIN_HIGH;
    cpu->bus->pin_o_m1 = Bus::PIN_LOW;
    cpu->bus->syncControl();
    cpu->bus->waitClockFalling(false);
    // t2
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    // tw
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_iorq = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // tw
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    while (! cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling(false);
    }
    // T3-rising: Fetch data. Output refresh address. Update control signals
    cpu->bus->waitClockRising(false);
    cpu->executing = cpu->bus->getData();

    auto refreshAddr = (u16)((cpu->special_registers.i << 8) | cpu->special_registers.r);
    cpu->bus->setAddress(refreshAddr);

    cpu->bus->pin_o_iorq = Bus::PIN_HIGH;
    cpu->bus->pin_o_rd = Bus::PIN_HIGH;
    cpu->bus->pin_o_m1 = Bus::PIN_HIGH;
    cpu->bus->pin_o_rfsh = Bus::PIN_LOW;
    cpu->bus->syncControl();

    // T3-falling: Activate MREQ
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_mreq = Bus::PIN_LOW;
    cpu->bus->syncControl();
}

void Mcycle::m1vm(Cpu *cpu){
    cpu->executing = cpu->virtual_memory[cpu->special_registers.pc];
    cpu->special_registers.pc++;
}

void Mcycle::m1halt(Cpu *cpu){
    // T1
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    // T2
    cpu->executing = 0x00;
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    // T3
    // T3-rising
    cpu->bus->waitClockRising(false);
    auto refreshAddr = (u16)((cpu->special_registers.i << 8) | cpu->special_registers.r);
    cpu->bus->setAddress(refreshAddr);
    cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
    cpu->bus->pin_o_rd = Bus::PIN_HIGH;
    cpu->bus->pin_o_m1 = Bus::PIN_HIGH;
    cpu->bus->pin_o_rfsh = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // T3-falling: Activate MREQ
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_mreq = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // T4
    Mcycle::m1t4(cpu);
}

void Mcycle::m1t1(Cpu *cpu){
    // T1: Output PC's address
    cpu->bus->syncControl();

    cpu->bus->waitClockRising(false);
    cpu->bus->setAddress(cpu->special_registers.pc);
    cpu->special_registers.pc++;
    cpu->bus->pin_o_m1 = Bus::PIN_LOW;
    cpu->bus->syncControl();
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_mreq = Bus::PIN_LOW;
    cpu->bus->pin_o_rd = Bus::PIN_LOW;
    cpu->bus->syncControl();
}

void Mcycle::m1t2(Cpu* cpu){
    // T2: Wait memory until WAIT is inactive
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    while (! cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling(false);
    }
}

void Mcycle::m1t3(Cpu* cpu){
    // T3-rising: Fetch data. Output refresh address. Update control signals
    cpu->bus->waitClockRising(false);
    cpu->executing = cpu->bus->getData();

    auto refreshAddr = (u16)((cpu->special_registers.i << 8) | cpu->special_registers.r);
    cpu->bus->setAddress(refreshAddr);

    cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
    cpu->bus->pin_o_rd = Bus::PIN_HIGH;
    cpu->bus->pin_o_m1 = Bus::PIN_HIGH;
    cpu->bus->pin_o_rfsh = Bus::PIN_LOW;
    cpu->bus->syncControl();

    // T3-falling: Activate MREQ
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_mreq = Bus::PIN_LOW;
    cpu->bus->syncControl();
}

void Mcycle::m1t4(Cpu* cpu){
    // T4: Inactivate MREQ, RFSH. Increment R resistor.
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
    cpu->bus->syncControl();

    cpu->bus->pin_o_rfsh = Bus::PIN_HIGH;

    u8 r1 = (cpu->special_registers.r & 0b10000000);
    u8 r7 = ((cpu->special_registers.r + 1) & 0b01111111);
    cpu->special_registers.r = r1 | r7;
}

u8 Mcycle::m2(Cpu* cpu, u16 addr){
    if (cpu->enable_virtual_memory){
        //Log::mem_read(cpu, addr, cpu->virtual_memory[addr]);
        return cpu->virtual_memory[addr];
    }

    // T1
    cpu->bus->waitClockRising(false);
    cpu->bus->setAddress(addr);
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_mreq = Bus::PIN_LOW;
    cpu->bus->pin_o_rd = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // T2
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    while (! cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling(false);
    }
    // T3
    cpu->bus->waitClockRising(false);
    u8 data = cpu->bus->getData();
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
    cpu->bus->pin_o_rd = Bus::PIN_HIGH;
    cpu->bus->syncControl();

    //Log::mem_read(cpu, addr, data);

    return data;
}

void Mcycle::m3(Cpu* cpu, u16 addr, u8 data){
    if (cpu->enable_virtual_memory){
        cpu->virtual_memory[addr] = data;
        //Log::mem_write(cpu, addr, data);
        return;
    }

    // T1
    cpu->bus->waitClockRising(false);
    cpu->bus->setAddress(addr);
    cpu->bus->waitClockFalling(false);
    cpu->bus->setDataBegin(data);
    cpu->bus->pin_o_mreq = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // T2
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    while (! cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling(false);
    }
    cpu->bus->pin_o_wr = Bus::PIN_LOW;
    cpu->bus->syncControl();
    // T3
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_mreq = Bus::PIN_HIGH;
    cpu->bus->pin_o_wr = Bus::PIN_HIGH;
    cpu->bus->syncControl();
    cpu->bus->setDataEnd();

    //Log::mem_write(cpu, addr, data);
}

u8 Mcycle::in(Cpu* cpu, u8 portL, u8 portH){
    // T1
    cpu->bus->waitClockRising(false);
    u16 port = (portH << 8) | portL;
    cpu->bus->setAddress(port);
    cpu->bus->waitClockFalling(false);
    // T2
    cpu->bus->waitClockRising(false);
    cpu->bus->pin_o_iorq = Bus::PIN_LOW;
    cpu->bus->pin_o_rd = Bus::PIN_LOW;
    cpu->bus->syncControl();
    cpu->bus->waitClockFalling(false);
    // TW
    cpu->bus->waitClockRising(true);
    cpu->bus->waitClockFalling(true);
    while (! cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling(false);
    }
    // T3
    cpu->bus->waitClockRising(false);
    u8 data = cpu->bus->getData();
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_iorq = Bus::PIN_HIGH;
    cpu->bus->pin_o_rd = Bus::PIN_HIGH;
    cpu->bus->syncControl();

    //LOGDBG("IN: Port: 0x%04X, Data: 0x%02X", port, data);

    return data;
}

void Mcycle::out(Cpu* cpu, u8 portL, u8 portH, u8 data){
    // T1
    cpu->bus->waitClockRising(false);
    u16 port = (portH << 8) | portL;
    cpu->bus->setAddress(port);
    cpu->bus->setDataBegin(data);
    cpu->bus->waitClockFalling(false);
    // T2
    cpu->bus->waitClockRising(false);
    cpu->bus->pin_o_iorq = Bus::PIN_LOW;
    cpu->bus->pin_o_wr = Bus::PIN_LOW;
    cpu->bus->syncControl();
    cpu->bus->waitClockFalling(false);

    // TW
    cpu->bus->waitClockRising(true);
    cpu->bus->waitClockFalling(true);
    while (! cpu->bus->getInput(Bus::Z80_PIN_I_WAIT)){
        cpu->bus->waitClockFalling(false);
    }
    // T3
    cpu->bus->waitClockRising(false);
    cpu->bus->waitClockFalling(false);
    cpu->bus->pin_o_iorq = Bus::PIN_HIGH;
    cpu->bus->pin_o_wr = Bus::PIN_HIGH;
    cpu->bus->syncControl();

    cpu->bus->setDataEnd();
}
