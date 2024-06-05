#ifndef Z80EMU_MCYCLE_HPP
#define Z80EMU_MCYCLE_HPP

#include <circle/types.h>

class Cpu;

class Mcycle {
public:
    static void int_m1t1t2t3(Cpu* cpu);
    static void m1vm(Cpu* cpu);
    static void m1halt(Cpu* cpu);

    static void m1t1(Cpu* cpu);
    static void m1t2(Cpu* cpu);
    static void m1t3(Cpu* cpu);
    static void m1t4(Cpu* cpu);

    static u8 m2(Cpu* cpu, u16 addr);
    static void m3(Cpu* cpu, u16 addr, u8 data);

    static u8 in(Cpu* cpu, u8 portL, u8 portH);
    static void out(Cpu* cpu, u8 portL, u8 portH, u8 data);
};

#endif //Z80EMU_MCYCLE_HPP
