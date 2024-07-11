#include "macros.hpp"
#include "opcode.hpp"
#include "cpu.hpp"
#include "mcycle.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
LOGMODULE("OpCode");

OpCode::OpCode(){
    this->_cpu = nullptr;
}
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
OpCode::OpCode(Cpu* cpu){
    this->_cpu = cpu;
}
#pragma clang diagnostic pop

void OpCode::execute(u8 opCode){
    if ((opCode >> 6) == 0b01 && (opCode & 0b00111000) != 0b00110000 && (opCode & 0b00000111) != 0b00000110){
        // ld r, r'
        //Log::execute(this->_cpu, opCode, "ld r, r'");
        u8 *reg = this->targetRegister(opCode, 3);
        u8 *reg_dash = this->targetRegister(opCode, 0);
        *reg = *reg_dash;
        //Log::dump_registers(this->_cpu);
        return;
    }

    switch (opCode){
        case 0x00: // nop
            //Log::execute(this->_cpu, opCode, "nop");
            break;
        case 0x01: // ld bc, nn
            //Log::execute(this->_cpu, opCode, "ld bc, nn");
            this->_cpu->registers.c = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->registers.b = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1);
            this->_cpu->special_registers.pc += 2;
            break;
        case 0x02: { // ld (bc), a
            //Log::execute(this->_cpu, opCode, "ld (bc), a");
            Mcycle::m3(this->_cpu, this->_cpu->registers.bc(), this->_cpu->registers.a);
            break;
        }
        case 0x03: // inc bc
            //Log::execute(this->_cpu, opCode, "inc bc");
            this->_cpu->registers.bc(this->_cpu->registers.bc() + 1);
            break;
        case 0x04: // inc r
        case 0x0C:
        case 0x14:
        case 0x1C:
        case 0x24:
        case 0x2C:
        case 0x3C: {
            //Log::execute(this->_cpu, opCode, "inc r");
            u8 *reg = this->targetRegister(opCode, 3);
            this->setFlagsByIncrement(*reg);
            (*reg)++;
            break;
        }
        case 0x05: // dec r
        case 0x0D:
        case 0x15:
        case 0x1D:
        case 0x25:
        case 0x2D:
        case 0x3D: {
            //Log::execute(this->_cpu, opCode, "dec r");
            u8 *reg = this->targetRegister(opCode, 3);
            this->setFlagsByDecrement(*reg);
            (*reg)--;
            break;
        }
        case 0x06: // ld r, n
        case 0x0e:
        case 0x16:
        case 0x1e:
        case 0x26:
        case 0x2e:
        case 0x3e: {
            //Log::execute(this->_cpu, opCode, "ld r, n");
            u8* reg = this->targetRegister(opCode, 3);
            *reg = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            break;
        }
        case 0x07: { // rlca
            //Log::execute(this->_cpu, opCode, "rlca");
            bool carry_bit = (this->_cpu->registers.a >> 7);
            this->_cpu->registers.a = (this->_cpu->registers.a << 1) | carry_bit;
            this->_cpu->registers.FC_Carry = carry_bit;
            this->_cpu->registers.FH_HalfCarry = false;
            this->_cpu->registers.FN_Subtract = false;
            break;
        }
        case 0x08: { // ex af, af'
            //Log::execute(this->_cpu, opCode, "ex af, af'");
            u16 af = this->_cpu->registers.af();
            this->_cpu->registers.af(this->_cpu->registers_alternate.af());
            this->_cpu->registers_alternate.af(af);
            break;
        }
        case 0x09: // add hl, rr
        case 0x19:
        case 0x29:
        case 0x39: {
            //Log::execute(this->_cpu, opCode, "add hl, rr");
            u16 value = 0;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x09: value = this->_cpu->registers.bc(); break;
                case 0x19: value = this->_cpu->registers.de(); break;
                case 0x29: value = this->_cpu->registers.hl(); break;
                case 0x39: value = this->_cpu->special_registers.sp; break;
            }
            this->setFlagsByAdd16(this->_cpu->registers.hl(), value);
            this->_cpu->registers.hl(this->_cpu->registers.hl() + value);
            break;
        }
        case 0x0A: // ld a,(bc)
            //Log::execute(this->_cpu, opCode, "ld a,(bc)");
            this->_cpu->registers.a = Mcycle::m2(this->_cpu, this->_cpu->registers.bc());
            break;
        case 0x0B: // dec bc
            //Log::execute(this->_cpu, opCode, "dec bc");
            this->_cpu->registers.bc(this->_cpu->registers.bc() - 1);
            break;
        case 0x0F: { // rrca
            //Log::execute(this->_cpu, opCode, "rrca");
            bool carry_bit = ((this->_cpu->registers.a & 1) > 0);
            this->_cpu->registers.a = (this->_cpu->registers.a >> 1) + ((this->_cpu->registers.a & 1) << 7);
            this->_cpu->registers.FH_HalfCarry = false;
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FC_Carry = carry_bit;
            break;
        }
        case 0x10: { // djnz n
            //Log::execute(this->_cpu, opCode, "djnz n");
            this->_cpu->registers.b--;
            if (this->_cpu->registers.b != 0){
                auto diff = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
                this->_cpu->special_registers.pc++;
                this->_cpu->special_registers.pc += diff;
            } else {
                this->_cpu->special_registers.pc++;
            }
            break;
        }
        case 0x11: // ld de, nn
            //Log::execute(this->_cpu, opCode, "ld de, nn");
            this->_cpu->registers.e = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->registers.d = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1);
            this->_cpu->special_registers.pc += 2;
            break;
        case 0x12: // ld (de),a
            //Log::execute(this->_cpu, opCode, "ld (de),a");
            Mcycle::m3(this->_cpu, this->_cpu->registers.de(), this->_cpu->registers.a);
            break;
        case 0x13: // inc de
            //Log::execute(this->_cpu, opCode, "inc de");
            this->_cpu->registers.de(this->_cpu->registers.de() + 1);
            break;
        case 0x17: { // rla
            //Log::execute(this->_cpu, opCode, "rla");
            bool carry_flg = this->_cpu->registers.FC_Carry;
            this->_cpu->registers.FC_Carry = this->_cpu->registers.a >> 7;
            this->_cpu->registers.a = (this->_cpu->registers.a << 1) | carry_flg;
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = false;
            break;
        }
        case 0x18: { // jr n
            //Log::execute(this->_cpu, opCode, "jr n");
            auto diff = (s8)(Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc));
            this->_cpu->special_registers.pc++;
            this->_cpu->special_registers.pc += diff;
            break;
        }
        case 0x1A: // ld a,(de)
            //Log::execute(this->_cpu, opCode, "ld a,(de)");
            this->_cpu->registers.a = Mcycle::m2(this->_cpu, this->_cpu->registers.de());
            break;
        case 0x1B: // dec de
            //Log::execute(this->_cpu, opCode, "dec de");
            this->_cpu->registers.de(this->_cpu->registers.de() - 1);
            break;
        case 0x1F: { // rra
            //Log::execute(this->_cpu, opCode, "rra");
            bool carry_flg = this->_cpu->registers.FC_Carry;
            this->_cpu->registers.FC_Carry = this->_cpu->registers.a & 1;
            this->_cpu->registers.a = (this->_cpu->registers.a >> 1) | (carry_flg << 7);
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = false;
            break;
        }
        case 0x20: // jr nz, n
            //Log::execute(this->_cpu, opCode, "jr nz, n");
            if (! this->_cpu->registers.FZ_Zero){
                auto diff = (s8)(Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc));
                this->_cpu->special_registers.pc++;
                this->_cpu->special_registers.pc += diff;
            } else {
                this->_cpu->special_registers.pc++;
            }
            break;
        case 0x21: // ld hl, nn
            //Log::execute(this->_cpu, opCode, "ld hl, nn");
            this->_cpu->registers.hl(
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8)
            );
            this->_cpu->special_registers.pc += 2;
            break;
        case 0x22: { // ld (nn), hl
            //Log::execute(this->_cpu, opCode, "ld (nn), hl");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            Mcycle::m3(this->_cpu, addr, this->_cpu->registers.l);
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->registers.h);
            this->_cpu->special_registers.pc += 2;
            break;
        }
        case 0x23: // inc hl
            //Log::execute(this->_cpu, opCode, "inc hl");
            this->_cpu->registers.hl(this->_cpu->registers.hl() + 1);
            break;
        case 0x27: { // daa
            //Log::execute(this->_cpu, opCode, "daa");
            u8 cr = 0;
            if ((this->_cpu->registers.a & 0x0f) > 0x09 || this->_cpu->registers.FH_HalfCarry){
                cr += 0x06;
            }
            if (this->_cpu->registers.a > 0x99 || this->_cpu->registers.FC_Carry){
                cr += 0x60;
                this->_cpu->registers.FC_Carry = true;
            }
            if (this->_cpu->registers.FN_Subtract){
                this->_cpu->registers.FH_HalfCarry =
                        this->_cpu->registers.FH_HalfCarry &&
                        (this->_cpu->registers.a & 0x0f) < 0x06;
                this->_cpu->registers.a -= cr;
            } else {
                this->_cpu->registers.FH_HalfCarry = (this->_cpu->registers.a & 0x0f) > 0x09;
                this->_cpu->registers.a += cr;
            }
            this->_cpu->registers.FS_Sign = this->_cpu->registers.a >> 7;
            this->_cpu->registers.FZ_Zero = this->_cpu->registers.a == 0;
            this->_cpu->registers.FPV_ParityOverflow = (OpCode::count1(this->_cpu->registers.a) % 2 == 0);
            break;
        }
        case 0x28: // jr z, n
            //Log::execute(this->_cpu, opCode, "jr z, n");
            if (this->_cpu->registers.FZ_Zero){
                auto diff = (s8)(Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc));
                this->_cpu->special_registers.pc++;
                this->_cpu->special_registers.pc += diff;
            } else {
                this->_cpu->special_registers.pc++;
            }
            break;
        case 0x2A: { // ld hl, (nn)
            //Log::execute(this->_cpu, opCode, "ld hl, (nn)");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            this->_cpu->registers.l = Mcycle::m2(this->_cpu, addr);
            this->_cpu->registers.h = Mcycle::m2(this->_cpu, addr + 1);
            //Log::dump_registers(this->_cpu);
            break;
        }
        case 0x2B: // dec hl
            //Log::execute(this->_cpu, opCode, "dec hl");
            this->_cpu->registers.hl(this->_cpu->registers.hl() - 1);
            break;
        case 0x2F: // cpl
            //Log::execute(this->_cpu, opCode, "cpl");
            this->_cpu->registers.a ^= 0xff;
            this->_cpu->registers.FN_Subtract = true;
            this->_cpu->registers.FH_HalfCarry = true;
            break;
        case 0x30: // jr nc, n
            //Log::execute(this->_cpu, opCode, "jr nc, n");
            if (! this->_cpu->registers.FC_Carry){
                auto diff = (s8)(Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc));
                this->_cpu->special_registers.pc++;
                this->_cpu->special_registers.pc += diff;
            } else {
                this->_cpu->special_registers.pc++;
            }
            break;
        case 0x31: // ld sp, nn
            //Log::execute(this->_cpu, opCode, "ld sp, nn");
            this->_cpu->special_registers.sp =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            break;
        case 0x32: { // ld (nn), a
            //Log::execute(this->_cpu, opCode, "ld (nn), a");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            Mcycle::m3(this->_cpu, addr, this->_cpu->registers.a);
            this->_cpu->special_registers.pc += 2;
            break;
        }
        case 0x33: // inc sp
            //Log::execute(this->_cpu, opCode, "inc sp");
            this->_cpu->special_registers.sp++;
            break;
        case 0x34: { // inc (hl)
            //Log::execute(this->_cpu, opCode, "inc (hl)");
            u16 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsByIncrement(value);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value + 1);
            break;
        }
        case 0x35: { // dec (hl)
            //Log::execute(this->_cpu, opCode, "dec (hl)");
            u16 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsByDecrement(value);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value - 1);
            break;
        }
        case 0x36: // ld (hl), n
            //Log::execute(this->_cpu, opCode, "ld (hl), n");
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc));
            this->_cpu->special_registers.pc++;
            break;
        case 0x37: // scf
            //Log::execute(this->_cpu, opCode, "scf");
            this->_cpu->registers.FC_Carry = true;
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = false;
            break;
        case 0x38: // jr c, n
            //Log::execute(this->_cpu, opCode, "jr c, n");
            if (this->_cpu->registers.FC_Carry){
                auto diff = (s8)(Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc));
                this->_cpu->special_registers.pc++;
                this->_cpu->special_registers.pc += diff;
            } else {
                this->_cpu->special_registers.pc++;
            }
            break;
        case 0x3A: { // ld a, (nn)
            //Log::execute(this->_cpu, opCode, "ld a, (nn)");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            this->_cpu->registers.a = Mcycle::m2(this->_cpu, addr);
            //Log::dump_registers(this->_cpu);
            break;
        }
        case 0x3B: // dec sp
            //Log::execute(this->_cpu, opCode, "dec sp");
            this->_cpu->special_registers.sp--;
            break;
        case 0x3F: { // ccf
            //Log::execute(this->_cpu, opCode, "ccf");
            bool saved_carry = this->_cpu->registers.FC_Carry;
            this->_cpu->registers.FC_Carry = !this->_cpu->registers.FC_Carry;
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = saved_carry;
            break;
        }
        case 0x46: // ld r, (hl)
        case 0x4E:
        case 0x56:
        case 0x5E:
        case 0x66:
        case 0x6E: {
            //Log::execute(this->_cpu, opCode, "ld r, (hl)");
            u8 *reg = this->targetRegister(opCode, 3);
            *reg = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            break;
        }
        case 0x70: // ld (hl), r
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77:
            //Log::execute(this->_cpu, opCode, "ld (hl), r");
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), *(this->targetRegister(opCode, 0)));
            break;
        case 0x76: // halt
            //Log::execute(this->_cpu, opCode, "halt");
            this->_cpu->halt = true;
            break;
        case 0x7E: // ld a,(hl)
            //Log::execute(this->_cpu, opCode, "ld a,(hl)");
            this->_cpu->registers.a = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            break;
        case 0x80: // add a, r
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x87: {
            //Log::execute(this->_cpu, opCode, "add a, r");
            u8* reg = this->targetRegister(opCode, 0);
            this->setFlagsByAddition(this->_cpu->registers.a, *reg, 0);
            this->_cpu->registers.a += *reg;
            break;
        }
        case 0x86: { // add a, (hl)
            //Log::execute(this->_cpu, opCode, "add a, (hl)");
            u16 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsByAddition(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a += value;
            break;
        }
        case 0x88: // adc a, r
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8F: {
            //Log::execute(this->_cpu, opCode, "adc a, r");
            u8* reg = this->targetRegister(opCode, 0);
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->registers.a, *reg, carry);
            this->_cpu->registers.a += *reg + carry;
            break;
        }
        case 0x8E: { // adc a, (hl)
            //Log::execute(this->_cpu, opCode, "adc a, (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a += value + carry;
            break;
        }
        case 0x90: // sub r
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x97: {
            //Log::execute(this->_cpu, opCode, "sub r");
            u8* reg = this->targetRegister(opCode, 0);
            this->setFlagsBySubtract(this->_cpu->registers.a, *reg, 0);
            this->_cpu->registers.a -= *reg;
            break;
        }
        case 0x96: { // sub (hl)
            //Log::execute(this->_cpu, opCode, "sub (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a -= value;
            break;
        }
        case 0x98: // sbc a, r
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9F: {
            //Log::execute(this->_cpu, opCode, "sbc a, r");
            u8* reg = this->targetRegister(opCode, 0);
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->registers.a, *reg, carry);
            this->_cpu->registers.a -= *reg + carry;
            break;
        }
        case 0x9E: { // sbc a,(hl)
            //Log::execute(this->_cpu, opCode, "sbc a,(hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a -= value + carry;
            break;
        }
        case 0xA0: // and r
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA7: {
            //Log::execute(this->_cpu, opCode, "and r");
            u8* reg = this->targetRegister(opCode, 0);
            this->_cpu->registers.a &= *reg;
            this->setFlagsByLogical(true);
            break;
        }
        case 0xA6: // and (hl)
            //Log::execute(this->_cpu, opCode, "and (hl)");
            this->_cpu->registers.a &= Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsByLogical(true);
            break;
        case 0xA8: // xor r
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAF: {
            //Log::execute(this->_cpu, opCode, "xor r");
            u8* reg = this->targetRegister(opCode, 0);
            this->_cpu->registers.a ^= *reg;
            this->setFlagsByLogical(false);
            break;
        }
        case 0xAE: // xor (hl)
            //Log::execute(this->_cpu, opCode, "xor (hl)");
            this->_cpu->registers.a ^= Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsByLogical(false);
            break;
        case 0xB0: // or r
        case 0xB1:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB5:
        case 0xB7: {
            //Log::execute(this->_cpu, opCode, "or r");
            u8* reg = this->targetRegister(opCode, 0);
            this->_cpu->registers.a |= *reg;
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB6: // or (hl)
            //Log::execute(this->_cpu, opCode, "or (hl)");
            this->_cpu->registers.a |= Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsByLogical(false);
            break;
        case 0xB8: // cp r
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBF:
            //Log::execute(this->_cpu, opCode, "cp r");
            this->setFlagsBySubtract(this->_cpu->registers.a, *(this->targetRegister(opCode, 0)), 0);
            break;
        case 0xBE: { // cp (hl)
            //Log::execute(this->_cpu, opCode, "cp (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            break;
        }
        case 0xC0: // ret nz
            //Log::execute(this->_cpu, opCode, "ret nz");
            if (! this->_cpu->registers.FZ_Zero){
                executeRet();
            }
            break;
        case 0xC1: // pop bc
            //Log::execute(this->_cpu, opCode, "pop bc");
            this->_cpu->registers.c = Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp);
            this->_cpu->registers.b = Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp + 1);
            this->_cpu->special_registers.sp += 2;
            //Log::dump_registers(this->_cpu);
            break;
        case 0xC2: // jp nz, nn
            //Log::execute(this->_cpu, opCode, "jp nz, nn");
            if (! this->_cpu->registers.FZ_Zero){
                this->_cpu->special_registers.pc =
                        Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                        (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xC3: // jp nn
            //Log::execute(this->_cpu, opCode, "jp nn");
            this->_cpu->special_registers.pc =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            break;
        case 0xC4: // call nz, nn
            //Log::execute(this->_cpu, opCode, "call nz, nn");
            if (! this->_cpu->registers.FZ_Zero){
                this->executeCall();
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xC5: // push bc
            //Log::execute(this->_cpu, opCode, "push bc");
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->registers.b);
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->registers.c);
            //Log::dump_registers(this->_cpu);
            break;
        case 0xC6: { // add a, n
            //Log::execute(this->_cpu, opCode, "add a, n");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->setFlagsByAddition(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a += value;
            break;
        }
        case 0xC7: // rst n (n = 0 - 7)
        case 0xCF:
        case 0xD7:
        case 0xDF:
        case 0xE7:
        case 0xEF:
        case 0xF7:
        case 0xFF:
            //Log::execute(this->_cpu, opCode, "rst n (n = 0 - 7)");
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->special_registers.pc >> 8);
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->special_registers.pc & 0xff);
            this->_cpu->special_registers.pc = (opCode & 0b00111000);
            break;
        case 0xC8: // ret z
            //Log::execute(this->_cpu, opCode, "ret z");
            if (this->_cpu->registers.FZ_Zero){
                executeRet();
            }
            break;
        case 0xC9: // ret
            //Log::execute(this->_cpu, opCode, "ret");
            executeRet();
            break;
        case 0xCA: // jp z, nn
            //Log::execute(this->_cpu, opCode, "jp z, nn");
            if (this->_cpu->registers.FZ_Zero){
                this->_cpu->special_registers.pc =
                        Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                        (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xCB: { // BITS
            //Log::execute(this->_cpu, opCode, "BITS");
            u8 opcode = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            executeCb(opcode);
            break;
        }
        case 0xCC: // call z, nn
            //Log::execute(this->_cpu, opCode, "call z, nn");
            if (this->_cpu->registers.FZ_Zero){
                this->executeCall();
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xCD: { // call nn
            //Log::execute(this->_cpu, opCode, "call nn");
            this->executeCall();
            break;
        }
        case 0xCE: { // adc a, n
            //Log::execute(this->_cpu, opCode, "adc a, n");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a += value + carry;
            break;
        }
        case 0xD0: // ret nc
            //Log::execute(this->_cpu, opCode, "ret nc");
            if (! this->_cpu->registers.FC_Carry){
                executeRet();
            }
            break;
        case 0xD1: // pop de
            //Log::execute(this->_cpu, opCode, "pop de");
            this->_cpu->registers.e = Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp);
            this->_cpu->registers.d = Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp + 1);
            this->_cpu->special_registers.sp += 2;
            //Log::dump_registers(this->_cpu);
            break;
        case 0xD2: // jp nc, nn
            //Log::execute(this->_cpu, opCode, "jp nc, nn");
            if (! this->_cpu->registers.FC_Carry){
                this->_cpu->special_registers.pc =
                        Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                        (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xD3: { // out (n),a
            //Log::execute(this->_cpu, opCode, "out (n),a");
            u8 port = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc += 1;
            Mcycle::out(this->_cpu, port, this->_cpu->registers.a, this->_cpu->registers.a);
            break;
        }
        case 0xD4: // call nc, nn
            //Log::execute(this->_cpu, opCode, "call nc, nn");
            if (! this->_cpu->registers.FC_Carry){
                this->executeCall();
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xD5: // push de
            //Log::execute(this->_cpu, opCode, "push de");
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->registers.d);
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->registers.e);
            //Log::dump_registers(this->_cpu);
            break;
        case 0xD6: { // sub n
            //Log::execute(this->_cpu, opCode, "sub n");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a -= value;
            break;
        }
        case 0xD8: // ret c
            //Log::execute(this->_cpu, opCode, "ret c");
            if (this->_cpu->registers.FC_Carry){
                executeRet();
            }
            break;
        case 0xD9: { // exx
            //Log::execute(this->_cpu, opCode, "exx");
            u16 temp;
            temp = this->_cpu->registers.bc();
            this->_cpu->registers.bc(this->_cpu->registers_alternate.bc());
            this->_cpu->registers_alternate.bc(temp);
            temp = this->_cpu->registers.de();
            this->_cpu->registers.de(this->_cpu->registers_alternate.de());
            this->_cpu->registers_alternate.de(temp);
            temp = this->_cpu->registers.hl();
            this->_cpu->registers.hl(this->_cpu->registers_alternate.hl());
            this->_cpu->registers_alternate.hl(temp);
            break;
        }
        case 0xDA: // jp c, nn
            //Log::execute(this->_cpu, opCode, "jp c, nn");
            if (this->_cpu->registers.FC_Carry){
                this->_cpu->special_registers.pc =
                        Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                        (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xDB: { // in a, (n)
            //Log::execute(this->_cpu, opCode, "in a, (n)");
            u8 port = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->registers.a = Mcycle::in(this->_cpu, port, this->_cpu->registers.a);
            break;
        }
        case 0xDC: // call c, nn
            //Log::execute(this->_cpu, opCode, "call c, nn");
            if (this->_cpu->registers.FC_Carry){
                this->executeCall();
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xDD: { // IX
            //Log::execute(this->_cpu, opCode, "IX");
            u8 opcode = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            executeDd(opcode);
            break;
        }
        case 0xDE: { // sbc a, n
            //Log::execute(this->_cpu, opCode, "sbc a, n");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a -= value + carry;
            break;
        }
        case 0xE0: // ret po
            //Log::execute(this->_cpu, opCode, "ret po");
            if (! this->_cpu->registers.FPV_ParityOverflow){
                executeRet();
            }
            break;
        case 0xE1: // pop hl
            //Log::execute(this->_cpu, opCode, "pop hl");
            this->_cpu->registers.hl(
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp + 1) << 8)
            );
            this->_cpu->special_registers.sp += 2;
            //Log::dump_registers(this->_cpu);
            break;
        case 0xE2: // jp po, nn
            //Log::execute(this->_cpu, opCode, "jp po, nn");
            if (! this->_cpu->registers.FPV_ParityOverflow){
                this->_cpu->special_registers.pc =
                        Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                        (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xE3: { // ex (sp), hl
            //Log::execute(this->_cpu, opCode, "ex (sp), hl");
            u16 mem_value =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp + 1) << 8);
            u16 temp_hl = this->_cpu->registers.hl();
            this->_cpu->registers.hl(mem_value);
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, temp_hl & 0xff);
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp + 1, temp_hl >> 8);
            break;
        }
        case 0xE4: // call po, nn
            //Log::execute(this->_cpu, opCode, "call po, nn");
            if (! this->_cpu->registers.FPV_ParityOverflow){
                this->executeCall();
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xE5: // push hl
            //Log::execute(this->_cpu, opCode, "push hl");
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->registers.hl() >> 8);
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->registers.hl() & 0xff);
            //Log::dump_registers(this->_cpu);
            break;
        case 0xE6: // and n
            //Log::execute(this->_cpu, opCode, "and n");
            this->_cpu->registers.a &= Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->setFlagsByLogical(true);
            break;
        case 0xE8: // ret pe
            //Log::execute(this->_cpu, opCode, "ret pe");
            if (this->_cpu->registers.FPV_ParityOverflow){
                executeRet();
            }
            break;
        case 0xE9: // jp (hl)
            //Log::execute(this->_cpu, opCode, "jp (hl)");
            this->_cpu->special_registers.pc = this->_cpu->registers.hl();
            break;
        case 0xEA: // jp pe, nn
            //Log::execute(this->_cpu, opCode, "jp pe, nn");
            if (this->_cpu->registers.FPV_ParityOverflow){
                this->_cpu->special_registers.pc =
                        Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                        (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xEB: { // ex de,hl
            //Log::execute(this->_cpu, opCode, "ex de,hl");
            u16 de = this->_cpu->registers.de();
            this->_cpu->registers.de(this->_cpu->registers.hl());
            this->_cpu->registers.hl(de);
            break;
        }
        case 0xEC: // call pe, nn
            //Log::execute(this->_cpu, opCode, "call pe, nn");
            if (this->_cpu->registers.FPV_ParityOverflow){
                this->executeCall();
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xED: { // EXTD
            //Log::execute(this->_cpu, opCode, "EXTD");
            u8 opcode = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            //LOGDBG("PC: %04x   ED 2nd byte opcode: %02x", this->_cpu->special_registers.pc, opcode);
            this->_cpu->special_registers.pc++;
            executeEd(opcode);
            break;
        }
        case 0xEE: // xor n
            //Log::execute(this->_cpu, opCode, "xor n");
            this->_cpu->registers.a ^= Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->setFlagsByLogical(false);
            break;
        case 0xF0: // ret p
            //Log::execute(this->_cpu, opCode, "ret p");
            if (! this->_cpu->registers.FS_Sign){
                executeRet();
            }
            break;
        case 0xF1: // pop af
            //Log::execute(this->_cpu, opCode, "pop af");
            this->_cpu->registers.f(Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp));
            this->_cpu->registers.a = Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp + 1);
            this->_cpu->special_registers.sp += 2;
            //Log::dump_registers(this->_cpu);
            break;
        case 0xF2: // jp p, nn
            //Log::execute(this->_cpu, opCode, "jp p, nn");
            if (! this->_cpu->registers.FS_Sign){
                this->_cpu->special_registers.pc =
                        Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                        (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xF3: // di
            //Log::execute(this->_cpu, opCode, "di");
            this->_cpu->waitingDI = 1;
            break;
        case 0xF4: // call p, nn
            //Log::execute(this->_cpu, opCode, "call p, nn");
            if (! this->_cpu->registers.FS_Sign){
                this->executeCall();
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xF5: // push af
            //Log::execute(this->_cpu, opCode, "push af");
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->registers.a);
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->registers.f());
            //Log::dump_registers(this->_cpu);
            break;
        case 0xF6: // or n
            //Log::execute(this->_cpu, opCode, "or n");
            this->_cpu->registers.a |= Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->setFlagsByLogical(false);
            break;
        case 0xF8: // ret m
            //Log::execute(this->_cpu, opCode, "ret m");
            if (this->_cpu->registers.FS_Sign){
                executeRet();
            }
            break;
        case 0xF9: // ld sp,hl
            //Log::execute(this->_cpu, opCode, "ld sp,hl");
            this->_cpu->special_registers.sp = this->_cpu->registers.hl();
            break;
        case 0xFA: // jp m, nn
            //Log::execute(this->_cpu, opCode, "jp m, nn");
            if (this->_cpu->registers.FS_Sign){
                this->_cpu->special_registers.pc =
                        Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                        (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xFB: // ei
            //Log::execute(this->_cpu, opCode, "ei");
            this->_cpu->waitingEI = 2;
            break;
        case 0xFC: // call m, nn
            //Log::execute(this->_cpu, opCode, "call m, nn");
            if (this->_cpu->registers.FS_Sign){
                this->executeCall();
            } else {
                this->_cpu->special_registers.pc += 2;
            }
            break;
        case 0xFD: { // IY
            //Log::execute(this->_cpu, opCode, "IY");
            u8 opcode = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            executeFd(opcode);
            break;
        }
        case 0xFE: { // cp n
            //Log::execute(this->_cpu, opCode, "cp n");
            this->setFlagsBySubtract(this->_cpu->registers.a, Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc), 0);
            this->_cpu->special_registers.pc++;
            break;
        }
        default:
            //char error[100];
            //sprintf(error, "Invalid op code: %02x", opCode);
            LOGPANIC("Invalid op code: %02x", opCode);
            //Log::error(this->_cpu, error);
            break;
    }
}

void OpCode::executeCb(u8 opCode){
    u8 type = ((opCode & 0b11000000) >> 6);

    if (type == 0b00){
        u8 sub_type = ((opCode & 0b00111000) >> 3);
        u8 reg_idx = (opCode & 0b00000111);
        u8 value;
        if (reg_idx == 0b110){
            // rot* (hl)
            //Log::execute(this->_cpu, opCode, "rot* (hl)");
            value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
        } else {
            // rot* r
            //Log::execute(this->_cpu, opCode, "rot* r");
            value = *(this->targetRegister(opCode, 0));
        }
        switch (sub_type){
            case 0b000: { value = cb_rlc(value); break; } // rlc
            case 0b001: { value = cb_rrc(value); break; } // rrc
            case 0b010: { value = cb_rl(value); break; } // rl
            case 0b011: { value = cb_rr(value); break; } // rr
            case 0b100: { value = cb_sla(value); break; } // sla
            case 0b101: { value = cb_sra(value); break; } // sra
            case 0b110: { value = cb_sll(value); break; } // sll
            case 0b111: { value = cb_srl(value); break; } // srl
            default: break;
        }
        this->_cpu->registers.F_X = getBit(3, value);
        this->_cpu->registers.F_Y = getBit(5, value);

        if (reg_idx == 0b110){
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
        } else {
            *(this->targetRegister(opCode, 0)) = value;
        }
        return;
    }
    if (type == 0b01){
        u8 bit = ((opCode & 0b00111000) >> 3);
        u8 reg_idx = (opCode & 0b00000111);
        u8 value;
        if (reg_idx == 0b110){
            // bit b, (hl)
            //Log::execute(this->_cpu, opCode, "bit b, (hl)");
            value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
        } else {
            // bit b, r
            //Log::execute(this->_cpu, opCode, "bit b, r");
            value = *(this->targetRegister(opCode, 0));
        }
        //this->_cpu->registers.F_X = getBit(3, value);
        //this->_cpu->registers.F_Y = getBit(5, value);
        value &= (1 << bit);
        this->_cpu->registers.FZ_Zero = (value == 0);
        this->_cpu->registers.FS_Sign = (! this->_cpu->registers.FZ_Zero && bit == 7);
        this->_cpu->registers.FN_Subtract = false;
        this->_cpu->registers.FH_HalfCarry = true;
        this->_cpu->registers.FPV_ParityOverflow = this->_cpu->registers.FZ_Zero;
        return;
    }
    if (type == 0b10){
        u8 bit = ((opCode & 0b00111000) >> 3);
        u8 reg_idx = (opCode & 0b00000111);
        if (reg_idx == 0b110){
            // res b, (hl)
            //Log::execute(this->_cpu, opCode, "res b, (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            value &= ~(1 << bit);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
        } else {
            // res b, r
            //Log::execute(this->_cpu, opCode, "res b, r");
            u8* reg = this->targetRegister(opCode, 0);
            *reg &= ~(1 << bit);
        }
        return;
    }
    if (type == 0b11){
        u8 bit = ((opCode & 0b00111000) >> 3);
        u8 reg_idx = (opCode & 0b00000111);
        if (reg_idx == 0b110){
            // set b, (hl)
            //Log::execute(this->_cpu, opCode, "set b, (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            value |= (1 << bit);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
        } else {
            // set b, r
            //Log::execute(this->_cpu, opCode, "set b, r");
            u8* reg = this->targetRegister(opCode, 0);
            *reg |= (1 << bit);
        }
        return;
    }

    switch (opCode){
        case 0x00: // rlc r
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x07: {
            //Log::execute(this->_cpu, opCode, "rlc r");
            u8* reg = this->targetRegister(opCode, 0);
            bool carry_bit = ((*reg & 0x80) > 0);
            *reg = (*reg << 1) | (*reg >> 7);
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x06: { // rlc (hl)
            //Log::execute(this->_cpu, opCode, "rlc (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            bool carry_bit = ((value & 0x80) > 0);
            value = (value << 1) | (value >> 7);
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
            break;
        }
        case 0x08: // rrc r
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0F: {
            //Log::execute(this->_cpu, opCode, "rrc r");
            u8 *reg = this->targetRegister(opCode, 0);
            bool carry_bit = ((*reg & 1) > 0);
            *reg = (*reg >> 1) + ((*reg & 1) << 7);
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x0E: { // rrc (hl)
            //Log::execute(this->_cpu, opCode, "rrc (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            bool carry_bit = ((value & 1) > 0);
            value = (value >> 1) + ((value & 1) << 7);
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
            break;
        }
        case 0x10: // rl r
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x17: {
            //Log::execute(this->_cpu, opCode, "rl r");
            u8* reg = this->targetRegister(opCode, 0);
            bool carry_bit = ((*reg & 0x80) > 0);
            *reg = ((*reg << 1) & 0xff) | this->_cpu->registers.carry_by_val();
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x16: { // rl (hl)
            //Log::execute(this->_cpu, opCode, "rl (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            bool carry_bit = ((value & 0x80) > 0);
            value = ((value << 1) & 0xff) | this->_cpu->registers.carry_by_val();
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
            break;
        }
        case 0x18: // rr r
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1F: {
            //Log::execute(this->_cpu, opCode, "rr r");
            u8* reg = this->targetRegister(opCode, 0);
            bool carry_bit = ((*reg & 1) > 0);
            *reg = (*reg >> 1) | (this->_cpu->registers.carry_by_val() << 7);
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x1E: { // rr (hl)
            //Log::execute(this->_cpu, opCode, "rr (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            bool carry_bit = ((value & 1) > 0);
            value = (value >> 1) | (this->_cpu->registers.carry_by_val() << 7);
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
            break;
        }
        case 0x20: // sla r
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x27: {
            //Log::execute(this->_cpu, opCode, "sla r");
            u8* reg = this->targetRegister(opCode, 0);
            bool carry_bit = ((*reg & 0x80) > 0);
            *reg = (*reg << 1) & 0xff;
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x26: { // sla (hl)
            //Log::execute(this->_cpu, opCode, "sla (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            bool carry_bit = ((value & 0x80) > 0);
            value = (value << 1) & 0xff;
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
            break;
        }
        case 0x28: // sra r
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2F: {
            //Log::execute(this->_cpu, opCode, "sra r");
            u8* reg = this->targetRegister(opCode, 0);
            bool carry_bit = ((*reg & 0x01) > 0);
            *reg = (*reg & 0x80) | (*reg >> 1);
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x2E: { // sra (hl)
            //Log::execute(this->_cpu, opCode, "sra (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            bool carry_bit = ((value & 0x01) > 0);
            value = (value & 0x80) | (value >> 1);
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
            break;
        }
        case 0x38: // srl r
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3F: {
            //Log::execute(this->_cpu, opCode, "srl r");
            u8* reg = this->targetRegister(opCode, 0);
            bool carry_bit = ((*reg & 0x01) > 0);
            *reg >>= 1;
            this->setFlagsByRotate(*reg, carry_bit);
            break;
        }
        case 0x3E: { // srl (hl)
            //Log::execute(this->_cpu, opCode, "srl (hl)");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            bool carry_bit = ((value & 0x01) > 0);
            value >>= 1;
            this->setFlagsByRotate(value, carry_bit);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
            break;
        }
        default:
            //char error[100];
            //sprintf(error, "Invalid op code: CB %02x", opCode);
            LOGPANIC("Invalid op code: CB %02x", opCode);
            //Log::error(this->_cpu, error);
            HALT()
    }
}

void OpCode::executeDd(u8 opCode){
    if ((opCode >> 6) == 0b01 && ((opCode & 0b00111000) >> 3) != 0b110 && (opCode & 0b00000111) != 0b110){
        u8 reg_src = (opCode & 0b00000111);
        u8 value;
        switch (reg_src){
            case 0b100:
                value = this->_cpu->special_registers.ixh();
                break;
            case 0b101:
                value = this->_cpu->special_registers.ixl();
                break;
            default:
                value = *(this->targetRegister(reg_src, 0));
        }

        u8 reg_dst = ((opCode & 0b00111000) >> 3);
        switch (reg_dst){
            case 0b100:
                this->_cpu->special_registers.ixh(value);
                break;
            case 0b101:
                this->_cpu->special_registers.ixl(value);
                break;
            default:
                *(this->targetRegister(reg_dst, 0)) = value;
        }
        return;
    }
    switch (opCode){
        case 0x09: // add ix, rr
        case 0x19:
        case 0x29:
        case 0x39: {
            //Log::execute(this->_cpu, opCode, "add ix, rr");
            u16 value = 0;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x09: value = this->_cpu->registers.bc(); break;
                case 0x19: value = this->_cpu->registers.de(); break;
                case 0x29: value = this->_cpu->special_registers.ix; break;
                case 0x39: value = this->_cpu->special_registers.sp; break;
            }
            this->setFlagsByAdd16(this->_cpu->special_registers.ix, value);
            this->_cpu->special_registers.ix += value;
            break;
        }
        case 0x21: { // ld ix, nn
            //Log::execute(this->_cpu, opCode, "ld ix, nn");
            u16 data =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            this->_cpu->special_registers.ix = data;
            break;
        }
        case 0x22: { // ld (nn), ix
            //Log::execute(this->_cpu, opCode, "ld (nn), ix");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr, this->_cpu->special_registers.ix & 0xff);
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->special_registers.ix >> 8);
            break;
        }
        case 0x23: // inc ix
            //Log::execute(this->_cpu, opCode, "inc ix");
            this->_cpu->special_registers.ix++;
            break;
        case 0x24: { // inc ixh
            //Log::execute(this->_cpu, opCode, "inc ixh");
            this->setFlagsByIncrement(this->_cpu->special_registers.ixh());
            this->_cpu->special_registers.ixh(this->_cpu->special_registers.ixh() + 1);
            break;
        }
        case 0x25: { // dec ixh
            //Log::execute(this->_cpu, opCode, "dec ixh");
            this->setFlagsByDecrement(this->_cpu->special_registers.ixh());
            this->_cpu->special_registers.ixh(this->_cpu->special_registers.ixh() - 1);
            break;
        }
        case 0x26: { // ld ixh, n
            //Log::execute(this->_cpu, opCode, "ld ixh, n");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->special_registers.ixh(value);
            break;
        }
        case 0x2A: { // ld ix, (nn)
            //Log::execute(this->_cpu, opCode, "ld ix, (nn)");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            this->_cpu->special_registers.ix =
                    Mcycle::m2(this->_cpu, addr) +
                    (Mcycle::m2(this->_cpu, addr + 1) << 8);
            break;
        }
        case 0x2B: // dec ix
            //Log::execute(this->_cpu, opCode, "dec ix");
            this->_cpu->special_registers.ix--;
            break;
        case 0x2C: { // inc ixl
            //Log::execute(this->_cpu, opCode, "inc ixl");
            this->setFlagsByIncrement(this->_cpu->special_registers.ixl());
            this->_cpu->special_registers.ixl(this->_cpu->special_registers.ixl() + 1);
            break;
        }
        case 0x2D: { // dec ixl
            //Log::execute(this->_cpu, opCode, "dec ixl");
            this->setFlagsByDecrement(this->_cpu->special_registers.ixl());
            this->_cpu->special_registers.ixl(this->_cpu->special_registers.ixl() - 1);
            break;
        }
        case 0x2E: { // ld ixl, n
            //Log::execute(this->_cpu, opCode, "ld ixl, n");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->special_registers.ix = (this->_cpu->special_registers.ix & 0xff00) | value;
            break;
        }
        case 0x34: { // inc (ix + d)
            //Log::execute(this->_cpu, opCode, "inc (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u16 addr = this->_cpu->special_registers.ix + d;
            u16 data = Mcycle::m2(this->_cpu, addr);
            this->setFlagsByIncrement(data);
            Mcycle::m3(this->_cpu, addr, data + 1);
            break;
        }
        case 0x35: { // dec (ix + d)
            //Log::execute(this->_cpu, opCode, "dec (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u16 addr = this->_cpu->special_registers.ix + d;
            u16 data = Mcycle::m2(this->_cpu, addr);
            this->setFlagsByDecrement(data);
            Mcycle::m3(this->_cpu, addr, data - 1);
            break;
        }
        case 0x36: { // ld (ix + d), n
            //Log::execute(this->_cpu, opCode, "ld (ix + d), n");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 data = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.ix + d, data);
            break;
        }
        case 0x46: // ld r, (ix + d)
        case 0x4E:
        case 0x56:
        case 0x5E:
        case 0x66:
        case 0x6E:
        case 0x7E: {
            //Log::execute(this->_cpu, opCode, "ld r, (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8* reg = this->targetRegister(opCode, 3);
            *reg = Mcycle::m2(this->_cpu, this->_cpu->special_registers.ix + d);
            break;
        }
        case 0x70: // ld (ix + d), r
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77: {
            //Log::execute(this->_cpu, opCode, "ld (ix + d), r");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8* reg = this->targetRegister(opCode, 0);
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.ix + d, *reg);
            break;
        }
        case 0x84: { // add a, ixh
            //Log::execute(this->_cpu, opCode, "add a, ixh");
            u8 value = this->_cpu->special_registers.ixh();
            this->setFlagsByAddition(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a += value;
            break;
        }
        case 0x85: { // add a, ixl
            //Log::execute(this->_cpu, opCode, "add a, ixl");
            u8 value = this->_cpu->special_registers.ixl();
            this->setFlagsByAddition(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a += value;
            break;
        }
        case 0x86: { // add a, (ix + d)
            //Log::execute(this->_cpu, opCode, "add a, (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.ix + d);
            this->setFlagsByAddition(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a += value;
            break;
        }
        case 0x8C: { // adc a, ixh
            //Log::execute(this->_cpu, opCode, "adc a, ixh");
            u8 value = this->_cpu->special_registers.ixh();
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a += value + carry;
            break;
        }
        case 0x8D: { // adc a, ixl
            //Log::execute(this->_cpu, opCode, "adc a, ixl");
            u8 value = this->_cpu->special_registers.ixl();
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a += value + carry;
            break;
        }
        case 0x8E: { // adc a, (ix + d)
            //Log::execute(this->_cpu, opCode, "adc a, (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.ix + d);
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a += value + carry;
            break;
        }
        case 0x94: { // sub a, ixh
            //Log::execute(this->_cpu, opCode, "sub a, ixh");
            u8 value = this->_cpu->special_registers.ixh();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a -= value;
            break;
        }
        case 0x95: { // sub a, ixl
            //Log::execute(this->_cpu, opCode, "sub a, ixl");
            u8 value = this->_cpu->special_registers.ixl();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a -= value;
            break;
        }
        case 0x96: { // sub (ix + d)
            //Log::execute(this->_cpu, opCode, "sub (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.ix + d);
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a -= value;
            break;
        }
        case 0x9C: { // sbc a, ixh
            //Log::execute(this->_cpu, opCode, "sbc a, ixh");
            u8 value = this->_cpu->special_registers.ixh();
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a -= value + carry;
            break;
        }
        case 0x9D: { // sbc a, ixl
            //Log::execute(this->_cpu, opCode, "sbc a, ixl");
            u8 value = this->_cpu->special_registers.ixl();
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a -= value + carry;
            break;
        }
        case 0x9E: { // sbc a, (ix + d)
            //Log::execute(this->_cpu, opCode, "sbc a, (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.ix + d);
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a -= value + carry;
            break;
        }
        case 0xA4: // and ixh
            //Log::execute(this->_cpu, opCode, "and ixh");
            this->_cpu->registers.a &= this->_cpu->special_registers.ixh();
            this->setFlagsByLogical(true);
            break;
        case 0xA5: // and ixl
            //Log::execute(this->_cpu, opCode, "and ixl");
            this->_cpu->registers.a &= this->_cpu->special_registers.ixl();
            this->setFlagsByLogical(true);
            break;
        case 0xA6: { // and (ix + d)
            //Log::execute(this->_cpu, opCode, "and (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->registers.a &= Mcycle::m2(this->_cpu, this->_cpu->special_registers.ix + d);
            this->setFlagsByLogical(true);
            break;
        }
        case 0xAC: // xor ixh
            //Log::execute(this->_cpu, opCode, "xor ixh");
            this->_cpu->registers.a ^= this->_cpu->special_registers.ixh();
            this->setFlagsByLogical(false);
            break;
        case 0xAD: // xor ixl
            //Log::execute(this->_cpu, opCode, "xor ixl");
            this->_cpu->registers.a ^= this->_cpu->special_registers.ixl();
            this->setFlagsByLogical(false);
            break;
        case 0xAE: { // xor (ix + d)
            //Log::execute(this->_cpu, opCode, "xor (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->registers.a ^= Mcycle::m2(this->_cpu, this->_cpu->special_registers.ix + d);
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB4: { // or ixh
            //Log::execute(this->_cpu, opCode, "or ixh");
            this->_cpu->registers.a |= this->_cpu->special_registers.ixh();
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB5: { // or ixl
            //Log::execute(this->_cpu, opCode, "or ixl");
            this->_cpu->registers.a |= this->_cpu->special_registers.ixl();
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB6: { // or (ix + d)
            //Log::execute(this->_cpu, opCode, "or (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->registers.a |= Mcycle::m2(this->_cpu, this->_cpu->special_registers.ix + d);
            this->setFlagsByLogical(false);
            break;
        }
        case 0xBC: // cp ixh
            //Log::execute(this->_cpu, opCode, "cp ixh");
            this->setFlagsBySubtract(this->_cpu->registers.a, this->_cpu->special_registers.ixh(), 0);
            break;
        case 0xBD: // cp ixl
            //Log::execute(this->_cpu, opCode, "cp ixl");
            this->setFlagsBySubtract(this->_cpu->registers.a, this->_cpu->special_registers.ixl(), 0);
            break;
        case 0xBE: { // cp (ix + d)
            //Log::execute(this->_cpu, opCode, "cp (ix + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.ix + d);
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            break;
        }
        case 0xCB: {
            //Log::execute(this->_cpu, opCode, "DD CB");
            this->executeXxCb(this->_cpu->special_registers.ix);
            break;
        }
        case 0xE1: // pop ix
            //Log::execute(this->_cpu, opCode, "pop ix");
            this->_cpu->special_registers.ix =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp + 1) << 8);
            this->_cpu->special_registers.sp += 2;
            //Log::dump_registers(this->_cpu);
            break;
        case 0xE3: { // ex (sp), ix
            //Log::execute(this->_cpu, opCode, "ex (sp), ix");
            u8 temp_ix = this->_cpu->special_registers.ix;
            this->_cpu->special_registers.ix = Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp);
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, temp_ix & 0xff);
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp + 1, temp_ix >> 8);
            break;
        }
        case 0xE5: // push ix
            //Log::execute(this->_cpu, opCode, "push ix");
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->special_registers.ix >> 8);
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->special_registers.ix & 0xff);
            //Log::dump_registers(this->_cpu);
            break;
        case 0xE9: // jp (ix)
            //Log::execute(this->_cpu, opCode, "jp (ix)");
            this->_cpu->special_registers.pc = this->_cpu->special_registers.ix;
            break;
        case 0xF9: // ld sp, ix
            //Log::execute(this->_cpu, opCode, "ld sp, ix");
            this->_cpu->special_registers.sp = this->_cpu->special_registers.ix;
            break;
        default: {
            //char error[100];
            //sprintf(error, "Invalid op code: DD %02x", opCode);
            //Log::error(this->_cpu, error);
            LOGPANIC("Invalid op code: DD %02x", opCode);
            HALT()
        }
    }
}

// XX CB d ex
void OpCode::executeXxCb(u16 idx){
    auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
    this->_cpu->special_registers.pc++;
    u8 ex = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
    this->_cpu->special_registers.pc++;
    // value = (ix + d)
    u8 value = Mcycle::m2(this->_cpu, idx + d);

    u8 x = ((ex & 0b11000000) >> 6);
    u8 y = ((ex & 0b00111000) >> 3);
    u8 z = (ex & 0b00000111);

    switch (x){
        case 0b00: { // rot[y](iz + d)
            switch (y){
                case 0b000: { value = cb_rlc(value); break; } // rlc
                case 0b001: { value = cb_rrc(value); break; } // rrc
                case 0b010: { value = cb_rl(value); break; } // rl
                case 0b011: { value = cb_rr(value); break; } // rr
                case 0b100: { value = cb_sla(value); break; } // sla
                case 0b101: { value = cb_sra(value); break; } // sra
                case 0b110: { value = cb_sll(value); break; } // sll
                case 0b111: { value = cb_srl(value); break; } // srl
                default: break;
            }
            this->_cpu->registers.F_X = getBit(3, value);
            this->_cpu->registers.F_Y = getBit(5, value);
            break;
        }
        case 0b01: { // bit y, (iz+d)
            value &= (1 << y);
            this->_cpu->registers.FS_Sign = (value >> 7);
            this->_cpu->registers.FZ_Zero = (value == 0);
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = true;
            this->_cpu->registers.FPV_ParityOverflow = this->_cpu->registers.FZ_Zero;
            break;
        }
        case 0b10: // res y, (iz+d)
            value &= ~(1 << y);
            break;
        case 0b11: // set y, (iz+d)
            value |= (1 << y);
            break;
        default:
            //char error[100];
            //sprintf(error, "Invalid target register code: XX CB %02x %02x", d, ex);
            LOGPANIC("Invalid target register code: XX CB %02x %02x", d, ex);
            //Log::error(this->_cpu, error);
            HALT()
    }

    // ld r[z], rot[y](iz+d)
    // ld r[z], res y,(iz+d)
    // ld r[z], set y,(iz+d)
    if (x != 0b01 && z != 0b110){
        switch (z){
            case 0b000: this->_cpu->registers.b = value; break;
            case 0b001: this->_cpu->registers.c = value; break;
            case 0b010: this->_cpu->registers.d = value; break;
            case 0b011: this->_cpu->registers.e = value; break;
            case 0b100: this->_cpu->registers.h = value; break;
            case 0b101: this->_cpu->registers.l = value; break;
            case 0b110: Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value); break;
            case 0b111: this->_cpu->registers.a = value; break;
            default: break;
        }
    }

    if (x != 0b01){
        Mcycle::m3(this->_cpu, idx + d, value);
    }
}

/*
// XX CB d ex
void OpCode::_executeXxCb(u16 idx){
    auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
    this->_cpu->special_registers.pc++;
    u8 ex = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
    this->_cpu->special_registers.pc++;
    u8 value = Mcycle::m2(this->_cpu, idx + d);
    if ((ex & 0b11000111) == 0b01000110){
        // bit b, (ix + d) // bit b, (iy + d)
        u8 bit = ((ex & 0b00111000) >> 3);
        this->_cpu->registers.FZ_Zero = ((value & (1 << bit)) == 0);
        this->_cpu->registers.FPV_ParityOverflow = this->_cpu->registers.FZ_Zero;
        this->_cpu->registers.FS_Sign = (!this->_cpu->registers.FZ_Zero && 7 == bit);
        this->_cpu->registers.FH_HalfCarry = true;
        this->_cpu->registers.FN_Subtract = false;
    } else if ((ex & 0b11000111) == 0b10000110){
        // res b, (ix + d) // res b, (iy + d)
        u8 bit = ((ex & 0b0011100) >> 3);
        value &= -(value & (1 << bit));
        Mcycle::m3(this->_cpu, idx + d, value);
    } else if ((ex & 0b11000111) == 0b11000110){
        // set b, (ix + d) // set b, (iy + d)
        u8 bit = ((ex & 0b0011100) >> 3);
        value |= (value & (1 << bit));
        Mcycle::m3(this->_cpu, idx + d, value);
    } else {
        bool carry_bit;
        switch (ex){
            case 0x06: // rlc (ix + d) // rlc (iy + d)
                carry_bit = ((value & 0x80) > 0);
                value = (value << 1) & 0xff | (value >> 7);
                break;
            case 0x16: // rl (ix + d) // rl (iy + d)
                carry_bit = ((value & 0x80) > 0);
                value = (value << 1) & 0xff | this->_cpu->registers.carry_by_val();
                break;
            case 0x26: // sla (ix + d) // sla (iy + d)
                carry_bit = ((value & 0x80) > 0);
                value = (value << 1) & 0xff;
                break;
            case 0x0E: // rrc (ix + d) // rrc (iy + d)
                carry_bit = ((value & 1) > 0);
                value = (value >> 1) | ((value & 1) << 7);
                break;
            case 0x1E: // rr (ix + d) // rr (iy + d)
                carry_bit = ((value & 1) > 0);
                value = (value >> 1) | (this->_cpu->registers.carry_by_val() << 7);
                break;
            case 0x2E: // sra (ix + d) // sra (iy + d)
                carry_bit = ((value & 0x01) > 0);
                value = (value & 0x80) | (value >> 1);
                break;
            case 0x3E: // srl (ix + d) // srl (iy + d)
                carry_bit = ((value & 0x01) > 0);
                value >>= 1;
                break;
            default:
                char error[100];
                sprintf(error, "Invalid target register code: XX CB %02x %02x", d, ex);
                Log::error(this->_cpu, error);
                throw std::runtime_error(error);
        }
        this->setFlagsByRotate(value, carry_bit);
        Mcycle::m3(this->_cpu, idx + d, value);
    }
}
*/

void OpCode::executeEd(u8 opCode){
    switch (opCode){
        case 0x40: // in r, (c)
        case 0x48:
        case 0x50:
        case 0x58:
        case 0x60:
        case 0x68:
        case 0x78: {
            //Log::execute(this->_cpu, opCode, "in r, (c)");
            u8* reg = this->targetRegister(opCode, 3);
            u8 value = Mcycle::in(this->_cpu, *reg, this->_cpu->registers.b);
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = false;
            this->_cpu->registers.FZ_Zero = (value == 0);
            this->_cpu->registers.FS_Sign = ((value & 0x80) > 0);
            this->_cpu->registers.FPV_ParityOverflow = (OpCode::count1(value) % 2 == 0);
            break;
        }
        case 0x41: // out (c), r
        case 0x49:
        case 0x51:
        case 0x59:
        case 0x61:
        case 0x69:
        case 0x79: {
            //Log::execute(this->_cpu, opCode, "out (c), r");
            u8* reg = this->targetRegister(opCode, 3);
            this->_cpu->special_registers.pc++;
            Mcycle::out(this->_cpu, this->_cpu->registers.c, this->_cpu->registers.b, *reg);
            break;
        }
        case 0x42: // sbc hl, rr
        case 0x52:
        case 0x62:
        case 0x72: {
            //Log::execute(this->_cpu, opCode, "sbc hl, rr");
            u16 value = 0;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x42: value = this->_cpu->registers.bc(); break;
                case 0x52: value = this->_cpu->registers.de(); break;
                case 0x62: value = this->_cpu->registers.hl(); break;
                case 0x72: value = this->_cpu->special_registers.sp; break;
            }
            u16 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySbc16(this->_cpu->registers.hl(), value + carry);
            this->_cpu->registers.hl(this->_cpu->registers.hl() - (value + carry));
            break;
        }
        case 0x43: { // ld (nn), bc
            //Log::execute(this->_cpu, opCode, "ld (nn), bc");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr, this->_cpu->registers.c);
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->registers.b);
            break;
        }
        case 0x44: { // neg
            //Log::execute(this->_cpu, opCode, "neg");
            u8 a = this->_cpu->registers.a;
            this->_cpu->registers.a = 0;
            this->setFlagsBySubtract(this->_cpu->registers.a, a, 0);
            this->_cpu->registers.a -= a;
            break;
        }
        case 0x45: // retn
            //Log::execute(this->_cpu, opCode, "retn");
            executeRet();
            this->_cpu->iff1 = this->_cpu->iff2;
            break;
        case 0x46: // im 0
            //Log::execute(this->_cpu, opCode, "im 0");
            this->_cpu->interrupt_mode = 0;
            break;
        case 0x47: // ld i, a
            //Log::execute(this->_cpu, opCode, "ld i, a");
            this->_cpu->special_registers.i = this->_cpu->registers.a;
            break;
        case 0x4A: // adc hl, rr
        case 0x5A:
        case 0x6A:
        case 0x7A: {
            //Log::execute(this->_cpu, opCode, "adc hl, rr");
            u16 value = 0;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x4A: value = this->_cpu->registers.bc(); break;
                case 0x5A: value = this->_cpu->registers.de(); break;
                case 0x6A: value = this->_cpu->registers.hl(); break;
                case 0x7A: value = this->_cpu->special_registers.sp; break;
            }
            u16 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAdc16(this->_cpu->registers.hl(), value + carry);
            this->_cpu->registers.hl(this->_cpu->registers.hl() + value + carry);

            break;
        }
        case 0x4B: { // ld bc, (nn)
            //Log::execute(this->_cpu, opCode, "ld bc, (nn)");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            this->_cpu->registers.b = Mcycle::m2(this->_cpu, addr + 1);
            this->_cpu->registers.c = Mcycle::m2(this->_cpu, addr);
            break;
        }
        case 0x4D: // reti
            //Log::execute(this->_cpu, opCode, "reti");
            executeRet();
            break;
        case 0x4F: // ld r, a
            //Log::execute(this->_cpu, opCode, "ld r, a");
            this->_cpu->special_registers.r = this->_cpu->registers.a;
            break;
        case 0x53: { // ld (nn), de
            //Log::execute(this->_cpu, opCode, "ld (nn), de");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr, this->_cpu->registers.e);
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->registers.d);
            break;
        }
        case 0x56: // im 1
            //Log::execute(this->_cpu, opCode, "im 1");
            this->_cpu->interrupt_mode = 1;
            break;
        case 0x57: // ld a, i
            //Log::execute(this->_cpu, opCode, "ld a, i");
            this->_cpu->registers.a = this->_cpu->special_registers.i;
            break;
        case 0x5B: { // ld de, (nn)
            //Log::execute(this->_cpu, opCode, "ld de, (nn)");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            this->_cpu->registers.d = Mcycle::m2(this->_cpu, addr + 1);
            this->_cpu->registers.e = Mcycle::m2(this->_cpu, addr);
            break;
        }
        case 0x5E: // im 2
            //Log::execute(this->_cpu, opCode, "im 2");
            this->_cpu->interrupt_mode = 2;
            break;
        case 0x5F: // ld a, r
            //Log::execute(this->_cpu, opCode, "ld a, r");
            this->_cpu->registers.a = this->_cpu->special_registers.r;
            break;
        case 0x67: { // rrd
            //Log::execute(this->_cpu, opCode, "rrd");
            unsigned char beforeN = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            unsigned char nH = (beforeN & 0b11110000) >> 4;
            unsigned char nL = beforeN & 0b00001111;
            unsigned char aH = (this->_cpu->registers.a & 0b11110000) >> 4;
            unsigned char aL = this->_cpu->registers.a & 0b00001111;
            //unsigned char beforeA = this->_cpu->registers.a;
            unsigned char afterA = (aH << 4) | nL;
            unsigned char afterN = (aL << 4) | nH;
            this->_cpu->registers.a = afterA;
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), afterN);
            this->_cpu->registers.FS_Sign = ((this->_cpu->registers.a & 0x80) > 0);
            this->_cpu->registers.FZ_Zero = (this->_cpu->registers.a == 0);
            this->_cpu->registers.FH_HalfCarry = false;
            this->_cpu->registers.FPV_ParityOverflow = (OpCode::count1(this->_cpu->registers.a) % 2 == 0);
            this->_cpu->registers.FN_Subtract = false;
            break;
        }
        case 0x6F: { // rld
            //Log::execute(this->_cpu, opCode, "rld");
            u8 beforeN = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            u8 nH = (beforeN & 0b11110000) >> 4;
            u8 nL = beforeN & 0b00001111;
            u8 aH = (this->_cpu->registers.a & 0b11110000) >> 4;
            u8 aL = this->_cpu->registers.a & 0b00001111;
            //u8 beforeA = this->_cpu->registers.a;
            u8 afterA = (aH << 4) | nH;
            u8 afterN = (nL << 4) | aL;
            this->_cpu->registers.a = afterA;
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), afterN);
            this->_cpu->registers.FS_Sign = ((this->_cpu->registers.a & 0x80) > 0);
            this->_cpu->registers.FZ_Zero = (this->_cpu->registers.a == 0);
            this->_cpu->registers.FH_HalfCarry = false;
            this->_cpu->registers.FPV_ParityOverflow = (OpCode::count1(this->_cpu->registers.a) % 2 == 0);
            this->_cpu->registers.FN_Subtract = false;
            break;
        }
        case 0x73: { // ld (nn), sp
            //Log::execute(this->_cpu, opCode, "ld (nn), sp");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr, this->_cpu->special_registers.sp & 0xff);
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->special_registers.sp >> 8);
            break;
        }
        case 0x7B: { // ld sp, (nn)
            //Log::execute(this->_cpu, opCode, "ld sp, (nn)");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            this->_cpu->special_registers.sp =
                    Mcycle::m2(this->_cpu, addr) +
                    (Mcycle::m2(this->_cpu, addr + 1) << 8);
            break;
        }
        case 0xA0: { // ldi
            //Log::execute(this->_cpu, opCode, "ldi");
            u8 data = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            Mcycle::m3(this->_cpu, this->_cpu->registers.de(), data);
            this->_cpu->registers.hl(this->_cpu->registers.hl() + 1);
            this->_cpu->registers.de(this->_cpu->registers.de() + 1);
            this->_cpu->registers.bc(this->_cpu->registers.bc() - 1);
            this->_cpu->registers.FPV_ParityOverflow = (this->_cpu->registers.bc() != 0);
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = false;
            break;
        }
        case 0xA1: { // cpi
            //Log::execute(this->_cpu, opCode, "cpi");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0, false);
            this->_cpu->registers.hl(this->_cpu->registers.hl() + 1);
            this->_cpu->registers.bc(this->_cpu->registers.bc() - 1);
            this->_cpu->registers.FPV_ParityOverflow = (this->_cpu->registers.bc() != 0);
            break;
        }
        case 0xA2: { // ini
            //Log::execute(this->_cpu, opCode, "ini");
            u8 value = Mcycle::in(this->_cpu, this->_cpu->registers.c, this->_cpu->registers.b);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
            this->_cpu->registers.b--;
            this->_cpu->registers.hl(this->_cpu->registers.hl() + 1);
            this->_cpu->registers.FN_Subtract = true;
            this->_cpu->registers.FZ_Zero = (this->_cpu->registers.b == 0);
            break;
        }
        case 0xA3: { // outi
            //Log::execute(this->_cpu, opCode, "outi");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            Mcycle::out(this->_cpu, this->_cpu->registers.c, this->_cpu->registers.b, value);
            this->_cpu->registers.b--;
            this->_cpu->registers.hl(this->_cpu->registers.hl() + 1);
            this->_cpu->registers.FN_Subtract = true;
            this->_cpu->registers.FZ_Zero = (this->_cpu->registers.b == 0);
            break;
        }
        case 0xA8: { // ldd
            //Log::execute(this->_cpu, opCode, "ldd");
            u8 data = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            Mcycle::m3(this->_cpu, this->_cpu->registers.de(), data);
            this->_cpu->registers.hl(this->_cpu->registers.hl() - 1);
            this->_cpu->registers.de(this->_cpu->registers.de() - 1);
            this->_cpu->registers.bc(this->_cpu->registers.bc() - 1);
            this->_cpu->registers.FPV_ParityOverflow = (this->_cpu->registers.bc() != 0);
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = false;
            break;
        }
        case 0xA9: { // cpd
            //Log::execute(this->_cpu, opCode, "cpd");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0, false);
            this->_cpu->registers.hl(this->_cpu->registers.hl() - 1);
            this->_cpu->registers.bc(this->_cpu->registers.bc() - 1);
            this->_cpu->registers.FPV_ParityOverflow = (this->_cpu->registers.bc() != 0);
            break;
        }
        case 0xAA: { // ind
            //Log::execute(this->_cpu, opCode, "ind");
            u8 value = Mcycle::in(this->_cpu, this->_cpu->registers.c, this->_cpu->registers.b);
            Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
            this->_cpu->registers.b--;
            this->_cpu->registers.hl(this->_cpu->registers.hl() - 1);
            this->_cpu->registers.FN_Subtract = true;
            this->_cpu->registers.FZ_Zero = (this->_cpu->registers.b == 0);
            break;
        }
        case 0xAB: { // outd
            //Log::execute(this->_cpu, opCode, "outd");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
            Mcycle::out(this->_cpu, this->_cpu->registers.c, this->_cpu->registers.b, value);
            this->_cpu->registers.b--;
            this->_cpu->registers.hl(this->_cpu->registers.hl() - 1);
            this->_cpu->registers.FN_Subtract = true;
            this->_cpu->registers.FZ_Zero = (this->_cpu->registers.b == 0);
            break;
        }
        case 0xB0: { // ldir
            //Log::execute(this->_cpu, opCode, "ldir");
//            if (this->_cpu->special_registers.pc == 0x0026){
//                Log::general(this->_cpu, "SKIP***");
//                break;
//            }
            do {
                //Log::dump_registers(this->_cpu);
                u8 data = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
                Mcycle::m3(this->_cpu, this->_cpu->registers.de(), data);
                this->_cpu->registers.hl(this->_cpu->registers.hl() + 1);
                this->_cpu->registers.de(this->_cpu->registers.de() + 1);
                this->_cpu->registers.bc(this->_cpu->registers.bc() - 1);
            } while(this->_cpu->registers.bc() > 0);
            this->_cpu->registers.FPV_ParityOverflow = false;
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = false;
            break;
        }
        case 0xB1: { // cpir
            //Log::execute(this->_cpu, opCode, "cpir");
            do {
                u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
                this->setFlagsBySubtract(this->_cpu->registers.a, value, 0, false);
                this->_cpu->registers.hl(this->_cpu->registers.hl() + 1);
                this->_cpu->registers.bc(this->_cpu->registers.bc() - 1);
                this->_cpu->registers.FPV_ParityOverflow = (this->_cpu->registers.bc() != 0);
            } while(this->_cpu->registers.bc() > 0 && !this->_cpu->registers.FZ_Zero);
            break;
        }
        case 0xB2: { // inir
            //Log::execute(this->_cpu, opCode, "inir");
            do {
                u8 value = Mcycle::in(this->_cpu, this->_cpu->registers.c, this->_cpu->registers.b);
                Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
                this->_cpu->registers.b--;
                this->_cpu->registers.hl(this->_cpu->registers.hl() + 1);
            } while(this->_cpu->registers.b > 0);
            this->_cpu->registers.FN_Subtract = true;
            this->_cpu->registers.FZ_Zero = true;
            break;
        }
        case 0xB3: { // otir
            //Log::execute(this->_cpu, opCode, "otir");
            do {
                u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
                Mcycle::out(this->_cpu, this->_cpu->registers.c, this->_cpu->registers.b, value);
                this->_cpu->registers.b--;
                this->_cpu->registers.hl(this->_cpu->registers.hl() + 1);
            } while(this->_cpu->registers.b > 0);
            this->_cpu->registers.FN_Subtract = true;
            this->_cpu->registers.FZ_Zero = true;
            break;
        }
        case 0xB8: { // lddr
            //Log::execute(this->_cpu, opCode, "lddr");
            do {
                u8 data = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
                Mcycle::m3(this->_cpu, this->_cpu->registers.de(), data);
                this->_cpu->registers.hl(this->_cpu->registers.hl() - 1);
                this->_cpu->registers.de(this->_cpu->registers.de() - 1);
                this->_cpu->registers.bc(this->_cpu->registers.bc() - 1);
            } while(this->_cpu->registers.bc() > 0);
            this->_cpu->registers.FPV_ParityOverflow = false;
            this->_cpu->registers.FN_Subtract = false;
            this->_cpu->registers.FH_HalfCarry = false;
            break;
        }
        case 0xB9: { // cpdr
            //Log::execute(this->_cpu, opCode, "cpdr");
            do {
                u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
                this->setFlagsBySubtract(this->_cpu->registers.a, value, 0, false);
                this->_cpu->registers.hl(this->_cpu->registers.hl() - 1);
                this->_cpu->registers.bc(this->_cpu->registers.bc() - 1);
                this->_cpu->registers.FPV_ParityOverflow = (this->_cpu->registers.bc() != 0);
            } while(this->_cpu->registers.bc() > 0 && !this->_cpu->registers.FZ_Zero);
            break;
        }
        case 0xBA: { // indr
            //Log::execute(this->_cpu, opCode, "indr");
            do {
                u8 value = Mcycle::in(this->_cpu, this->_cpu->registers.c, this->_cpu->registers.b);
                Mcycle::m3(this->_cpu, this->_cpu->registers.hl(), value);
                this->_cpu->registers.b--;
                this->_cpu->registers.hl(this->_cpu->registers.hl() - 1);
            } while(this->_cpu->registers.b > 0);
            this->_cpu->registers.FN_Subtract = true;
            this->_cpu->registers.FZ_Zero = true;
            break;
        }
        case 0xBB: { // otdr
            //Log::execute(this->_cpu, opCode, "otdr");
            do {
                u8 value = Mcycle::m2(this->_cpu, this->_cpu->registers.hl());
                Mcycle::out(this->_cpu, this->_cpu->registers.c, this->_cpu->registers.b, value);
                this->_cpu->registers.b--;
                this->_cpu->registers.hl(this->_cpu->registers.hl() - 1);
            } while(this->_cpu->registers.b > 0);
            this->_cpu->registers.FN_Subtract = true;
            this->_cpu->registers.FZ_Zero = true;
            break;
        }
        default: {
            //char error[100];
            //sprintf(error, "Invalid op code: ED %02x", opCode);
            LOGPANIC("Invalid op code: ED %02x   PC: %04x", opCode, this->_cpu->special_registers.pc);
            //Log::error(this->_cpu, error);
            HALT()
        }
    }
}

void OpCode::executeFd(u8 opCode){
    if ((opCode >> 6) == 0b01 && ((opCode & 0b00111000) >> 3) != 0b110 && (opCode & 0b00000111) != 0b110){
        u8 reg_src = (opCode & 0b00000111);
        u8 value;
        switch (reg_src){
            case 0b100:
                value = this->_cpu->special_registers.iyh();
                break;
            case 0b101:
                value = this->_cpu->special_registers.iyl();
                break;
            default:
                value = *(this->targetRegister(reg_src, 0));
        }

        u8 reg_dst = ((opCode & 0b00111000) >> 3);
        switch (reg_dst){
            case 0b100:
                this->_cpu->special_registers.iyh(value);
                break;
            case 0b101:
                this->_cpu->special_registers.iyl(value);
                break;
            default:
                *(this->targetRegister(reg_dst, 0)) = value;
        }
        return;
    }
    switch (opCode){
        case 0x09: // add iy, rr
        case 0x19:
        case 0x29:
        case 0x39: {
            //Log::execute(this->_cpu, opCode, "add iy, rr");
            u16 value = 0;
            switch (opCode){ // NOLINT(hicpp-multiway-paths-covered)
                case 0x09: value = this->_cpu->registers.bc(); break;
                case 0x19: value = this->_cpu->registers.de(); break;
                case 0x29: value = this->_cpu->special_registers.iy; break;
                case 0x39: value = this->_cpu->special_registers.sp; break;
            }
            this->setFlagsByAdd16(this->_cpu->special_registers.iy, value);
            this->_cpu->special_registers.iy += value;
            break;
        }
        case 0x21: { // ld ix, nn
            //Log::execute(this->_cpu, opCode, "ld ix, nn");
            u16 data =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            this->_cpu->special_registers.iy = data;
            break;
        }
        case 0x22: { // ld (nn), iy
            //Log::execute(this->_cpu, opCode, "ld (nn), iy");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            Mcycle::m3(this->_cpu, addr, this->_cpu->special_registers.iy & 0xff);
            Mcycle::m3(this->_cpu, addr + 1, this->_cpu->special_registers.iy >> 8);
            break;
        }
        case 0x23: // inc iy
            //Log::execute(this->_cpu, opCode, "inc iy");
            this->_cpu->special_registers.iy++;
            break;
        case 0x24: { // inc iyh
            //Log::execute(this->_cpu, opCode, "inc iyh");
            this->setFlagsByIncrement(this->_cpu->special_registers.iyh());
            this->_cpu->special_registers.iyh(this->_cpu->special_registers.iyh() + 1);
            break;
        }
        case 0x25: { // dec iyh
            //Log::execute(this->_cpu, opCode, "dec iyh");
            this->setFlagsByDecrement(this->_cpu->special_registers.iyh());
            this->_cpu->special_registers.iyh(this->_cpu->special_registers.iyh() - 1);
            break;
        }
        case 0x26: { // ld iyh, n
            //Log::execute(this->_cpu, opCode, "ld iyh, n");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->special_registers.iyh(value);
            break;
        }
        case 0x2A: { // ld iy, (nn)
            //Log::execute(this->_cpu, opCode, "ld iy, (nn)");
            u16 addr =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
            this->_cpu->special_registers.pc += 2;
            this->_cpu->special_registers.iy =
                    Mcycle::m2(this->_cpu, addr) +
                    (Mcycle::m2(this->_cpu, addr + 1) << 8);
            break;
        }
        case 0x2B: // dec iy
            //Log::execute(this->_cpu, opCode, "dec iy");
            this->_cpu->special_registers.iy--;
            break;
        case 0x2C: { // inc iyl
            //Log::execute(this->_cpu, opCode, "inc iyl");
            this->setFlagsByIncrement(this->_cpu->special_registers.iyl());
            this->_cpu->special_registers.iyl(this->_cpu->special_registers.iyl() + 1);
            break;
        }
        case 0x2D: { // dec iyl
            //Log::execute(this->_cpu, opCode, "dec iyl");
            this->setFlagsByDecrement(this->_cpu->special_registers.iyl());
            this->_cpu->special_registers.iyl(this->_cpu->special_registers.iyl() - 1);
            break;
        }
        case 0x2E: { // ld iyl, n
            //Log::execute(this->_cpu, opCode, "ld iyl, n");
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->special_registers.iyl(value);
            break;
        }
        case 0x34: { // inc (iy + d)
            //Log::execute(this->_cpu, opCode, "inc (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u16 addr = this->_cpu->special_registers.iy + d;
            u16 data = Mcycle::m2(this->_cpu, addr);
            this->setFlagsByIncrement(data);
            Mcycle::m3(this->_cpu, addr, data + 1);
            break;
        }
        case 0x35: { // dec (iy + d)
            //Log::execute(this->_cpu, opCode, "dec (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u16 addr = this->_cpu->special_registers.iy + d;
            u16 data = Mcycle::m2(this->_cpu, addr);
            this->setFlagsByDecrement(data);
            Mcycle::m3(this->_cpu, addr, data - 1);
            break;
        }
        case 0x36: { // ld (iy + d), n
            //Log::execute(this->_cpu, opCode, "ld (iy + d), n");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 data = Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.iy + d, data);
            break;
        }
        case 0x46: // ld r, (iy + d)
        case 0x4E:
        case 0x56:
        case 0x5E:
        case 0x66:
        case 0x6E:
        case 0x7E: {
            //Log::execute(this->_cpu, opCode, "ld r, (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8* reg = this->targetRegister(opCode, 3);
            *reg = Mcycle::m2(this->_cpu, this->_cpu->special_registers.iy + d);
            break;
        }
        case 0x70: // ld (iy + d), r
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77: {
            //Log::execute(this->_cpu, opCode, "ld (iy + d), r");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8* reg = this->targetRegister(opCode, 0);
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.iy + d, *reg);
            break;
        }
        case 0x84: { // add a, iyh
            //Log::execute(this->_cpu, opCode, "add a, iyh");
            u8 value = this->_cpu->special_registers.iyh();
            this->setFlagsByAddition(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a += value;
            break;
        }
        case 0x85: { // add a, iyl
            //Log::execute(this->_cpu, opCode, "add a, iyl");
            u8 value = this->_cpu->special_registers.iyl();
            this->setFlagsByAddition(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a += value;
            break;
        }
        case 0x86: { // add a, (iy + d)
            //Log::execute(this->_cpu, opCode, "add a, (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.iy + d);
            this->setFlagsByAddition(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a += value;
            break;
        }
        case 0x8C: { // adc a, iyh
            //Log::execute(this->_cpu, opCode, "adc a, iyh");
            u8 value = this->_cpu->special_registers.iyh();
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a += value + carry;
            break;
        }
        case 0x8D: { // adc a, iyl
            //Log::execute(this->_cpu, opCode, "adc a, iyl");
            u8 value = this->_cpu->special_registers.iyl();
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a += value + carry;
            break;
        }
        case 0x8E: { // adc a, (iy + d)
            //Log::execute(this->_cpu, opCode, "adc a, (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.iy + d);
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsByAddition(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a += value + carry;
            break;
        }
        case 0x94: { // sub a, iyh
            //Log::execute(this->_cpu, opCode, "sub a, iyh");
            u8 value = this->_cpu->special_registers.iyh();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a -= value;
            break;
        }
        case 0x95: { // sub a, iyl
            //Log::execute(this->_cpu, opCode, "sub a, iyl");
            u8 value = this->_cpu->special_registers.iyl();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a -= value;
            break;
        }
        case 0x96: { // sub (iy + d)
            //Log::execute(this->_cpu, opCode, "sub (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.iy + d);
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            this->_cpu->registers.a -= value;
            break;
        }
        case 0x9C: { // sbc a, iyh
            //Log::execute(this->_cpu, opCode, "sbc a, iyh");
            u8 value = this->_cpu->special_registers.iyh();
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a -= value + carry;
            break;
        }
        case 0x9D: { // sbc a, iyl
            //Log::execute(this->_cpu, opCode, "sbc a, iyl");
            u8 value = this->_cpu->special_registers.iyl();
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a -= value + carry;
            break;
        }
        case 0x9E: { // sbc a, (iy + d)
            //Log::execute(this->_cpu, opCode, "sbc a, (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.iy + d);
            u8 carry = this->_cpu->registers.carry_by_val();
            this->setFlagsBySubtract(this->_cpu->registers.a, value, carry);
            this->_cpu->registers.a -= value + carry;
            break;
        }
        case 0xA4: // and iyh
            //Log::execute(this->_cpu, opCode, "and iyh");
            this->_cpu->registers.a &= this->_cpu->special_registers.iyh();
            this->setFlagsByLogical(true);
            break;
        case 0xA5: // and iyl
            //Log::execute(this->_cpu, opCode, "and iyl");
            this->_cpu->registers.a &= this->_cpu->special_registers.iyl();
            this->setFlagsByLogical(true);
            break;
        case 0xA6: { // and (iy + d)
            //Log::execute(this->_cpu, opCode, "and (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->registers.a &= Mcycle::m2(this->_cpu, this->_cpu->special_registers.iy + d);
            this->setFlagsByLogical(true);
            break;
        }
        case 0xAC: // xor iyh
            //Log::execute(this->_cpu, opCode, "xor iyh");
            this->_cpu->registers.a ^= this->_cpu->special_registers.iyh();
            this->setFlagsByLogical(false);
            break;
        case 0xAD: // xor iyl
            //Log::execute(this->_cpu, opCode, "xor iyl");
            this->_cpu->registers.a ^= this->_cpu->special_registers.iyl();
            this->setFlagsByLogical(false);
            break;
        case 0xAE: { // xor (iy + d)
            //Log::execute(this->_cpu, opCode, "xor (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->registers.a ^= Mcycle::m2(this->_cpu, this->_cpu->special_registers.iy + d);
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB4: { // or iyh
            //Log::execute(this->_cpu, opCode, "or iyh");
            this->_cpu->registers.a |= this->_cpu->special_registers.iyh();
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB5: { // or iyl
            //Log::execute(this->_cpu, opCode, "or iyl");
            this->_cpu->registers.a |= this->_cpu->special_registers.iyl();
            this->setFlagsByLogical(false);
            break;
        }
        case 0xB6: { // or (iy + d)
            //Log::execute(this->_cpu, opCode, "or (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            this->_cpu->registers.a |= Mcycle::m2(this->_cpu, this->_cpu->special_registers.iy + d);
            this->setFlagsByLogical(false);
            break;
        }
        case 0xBC: // cp iyh
            //Log::execute(this->_cpu, opCode, "cp iyh");
            this->setFlagsBySubtract(this->_cpu->registers.a, this->_cpu->special_registers.iyh(), 0);
            break;
        case 0xBD: // cp iyl
            //Log::execute(this->_cpu, opCode, "cp iyl");
            this->setFlagsBySubtract(this->_cpu->registers.a, this->_cpu->special_registers.iyl(), 0);
            break;
        case 0xBE: { // cp (iy + d)
            //Log::execute(this->_cpu, opCode, "cp (iy + d)");
            auto d = (s8)Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc);
            this->_cpu->special_registers.pc++;
            u8 value = Mcycle::m2(this->_cpu, this->_cpu->special_registers.iy + d);
            this->setFlagsBySubtract(this->_cpu->registers.a, value, 0);
            break;
        }
        case 0xCB: {
            //Log::execute(this->_cpu, opCode, "FD CB");
            this->executeXxCb(this->_cpu->special_registers.iy);
            break;
        }
        case 0xE1: // pop iy
            //Log::execute(this->_cpu, opCode, "pop iy");
            this->_cpu->special_registers.iy =
                    Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp) +
                    (Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp + 1) << 8);
            this->_cpu->special_registers.sp += 2;
            //Log::dump_registers(this->_cpu);
            break;
        case 0xE3: { // ex (sp), iy
            //Log::execute(this->_cpu, opCode, "ex (sp), iy");
            u8 temp_iy = this->_cpu->special_registers.iy;
            this->_cpu->special_registers.iy = Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp);
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, temp_iy & 0xff);
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp + 1, temp_iy >> 8);
            break;
        }
        case 0xE5: // push iy
            //Log::execute(this->_cpu, opCode, "push iy");
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->special_registers.iy >> 8);
            this->_cpu->special_registers.sp--;
            Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->special_registers.iy & 0xff);
            //Log::dump_registers(this->_cpu);
            break;
        case 0xE9: // jp (iy)
            //Log::execute(this->_cpu, opCode, "jp (iy)");
            this->_cpu->special_registers.pc = this->_cpu->special_registers.iy;
            break;
        case 0xF9: // ld sp, iy
            //Log::execute(this->_cpu, opCode, "ld sp, iy");
            this->_cpu->special_registers.sp = this->_cpu->special_registers.iy;
            break;
        default: {
            //char error[100];
            //sprintf(error, "Invalid op code: FD %02x", opCode);
            LOGPANIC("Invalid op code: FD %02x", opCode);
            //Log::error(this->_cpu, error);
            HALT()
        }
    }
}

u8* OpCode::targetRegister(u8 opCode, int lsb) const {
    u8 reg_idx = ((opCode >> lsb) & 0b00000111);

    switch(reg_idx){
        case 0b000: { /*Log::target_register(this->_cpu, "b");*/ return &(this->_cpu->registers.b); }
        case 0b001: { /*Log::target_register(this->_cpu, "c");*/ return &(this->_cpu->registers.c); }
        case 0b010: { /*Log::target_register(this->_cpu, "d");*/ return &(this->_cpu->registers.d); }
        case 0b011: { /*Log::target_register(this->_cpu, "e");*/ return &(this->_cpu->registers.e); }
        case 0b100: { /*Log::target_register(this->_cpu, "h");*/ return &(this->_cpu->registers.h); }
        case 0b101: { /*Log::target_register(this->_cpu, "l");*/ return &(this->_cpu->registers.l); }
        case 0b111: { /*Log::target_register(this->_cpu, "a");*/ return &(this->_cpu->registers.a); }
        default:
            //char error[100];
            //sprintf(error, "Invalid target register code: %02x with lsb %d (reg_idx: %01x)", opCode, lsb, reg_idx);
            LOGPANIC("Invalid target register code: %02x with lsb %d (reg_idx: %01x)", opCode, lsb, reg_idx);
            //Log::error(this->_cpu, error);
            HALT()
    }
}

void OpCode::executeRet() const{
    this->_cpu->special_registers.pc =
            Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp) +
            (Mcycle::m2(this->_cpu, this->_cpu->special_registers.sp + 1) << 8);
    this->_cpu->special_registers.sp += 2;
}

void OpCode::executeCall() const{
    u16 jump_addr =
            Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc) +
            (Mcycle::m2(this->_cpu, this->_cpu->special_registers.pc + 1) << 8);
    this->_cpu->special_registers.pc += 2;
    if (! (this->_cpu->emulate_cpm_bdos_call && (jump_addr == 0x0000 || jump_addr == 0x0005))){
        this->_cpu->special_registers.sp--;
        Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->special_registers.pc >> 8);
        this->_cpu->special_registers.sp--;
        Mcycle::m3(this->_cpu, this->_cpu->special_registers.sp, this->_cpu->special_registers.pc & 0xff);
        this->_cpu->special_registers.pc = jump_addr;
    } else {
        // CP/M Warm boot
        if (jump_addr == 0x0000){
            LOGPANIC("CP/M Warm boot");
            HALT()
        }
        // CP/M BDOS system calls
        //Log::dump_registers(this->_cpu);
        switch (this->_cpu->registers.c){
            case 0x02:
                // Console output
                //printf("CP/M BDOS call 0x02 Console output: %c\n", this->_cpu->registers.e);
                LOGDBG("CP/M BDOS call 0x02 Console output: %c\n", this->_cpu->registers.e);
                break;
            case 0x09: {
                // Output string
                //printf("CP/M BDOS call 0x09 Output string: ");
                LOGDBG("CP/M BDOS call 0x09 Output string: ");
                u8 chr;
                do {
                    chr = Mcycle::m2(this->_cpu, this->_cpu->registers.de());
                    this->_cpu->registers.de(this->_cpu->registers.de() + 1);
                    if (chr == '$'){
                        break;
                    }
                    //printf("%c", chr);
                    LOGDBG("%c", chr);
                } while(true);
                //printf("\n");
                LOGDBG("\n");
                break;
            }
            default:
                break;
        }
    }
}

u8 OpCode::count1(u8 data){
    u8 count = 0;
    for (int i = 0; i < 8; i++){
        if ((data & (1 << i)) > 0){
            count++;
        }
    }
    return count;
}

bool OpCode::parity(u8 data){
    u8 nb_one_bits = 0;
    for (int i = 0; i < 8; i++){
        nb_one_bits += ((data >> i) & 1);
    }
    return (nb_one_bits & 1) == 0;
}

void OpCode::setFlagsXY(u8 value) const{
    this->_cpu->registers.F_X = ((value & 0b00001000) > 0);
    this->_cpu->registers.F_Y = ((value & 0b00100000) > 0);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
void OpCode::setFlagsByAddition(u8 before, u8 addition, u8 carry_value, bool set_carry) const {
    u16 result = before + addition + carry_value;
    u16 carry = before ^ addition ^ result;
    auto final_result = (u8)result;
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FZ_Zero = (final_result == 0);
    this->_cpu->registers.FS_Sign = ((final_result & 0x80) > 0);
    this->_cpu->registers.FH_HalfCarry = ((carry & 0x10) != 0);
    this->_cpu->registers.FPV_ParityOverflow = ((((carry << 1) ^ carry) & 0x100) != 0);
    this->setFlagsXY(final_result);
    if (set_carry){
        this->_cpu->registers.FC_Carry = ((carry & 0x100) != 0);
    }
}
#pragma clang diagnostic pop

void OpCode::setFlagsBySubtract(u8 before, u8 subtract, u8 carry_value, bool set_carry) const {
    int result = before - subtract - carry_value;
    int carry = before ^ subtract ^ result;
    auto final_result = (u8)result;
    this->_cpu->registers.FN_Subtract = true;
    this->_cpu->registers.FZ_Zero = (final_result == 0);
    this->_cpu->registers.FS_Sign = ((final_result & 0x80) > 0);
    this->_cpu->registers.FH_HalfCarry = ((carry & 0x10) != 0);
    this->_cpu->registers.FPV_ParityOverflow = ((((carry << 1) ^ carry) & 0x100) != 0);
    this->setFlagsXY(final_result);
    if (set_carry){
        this->_cpu->registers.FC_Carry = ((carry & 0x100) != 0);
    }
}

void OpCode::setFlagsByIncrement(u8 before) const{
    u8 final_result = before + 1;
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FZ_Zero = (final_result == 0);
    this->_cpu->registers.FS_Sign = ((final_result & 0x80) > 0);
    this->_cpu->registers.FH_HalfCarry = ((final_result & 0x0f) == 0);
    this->_cpu->registers.FPV_ParityOverflow = (final_result == 0x80);
    this->setFlagsXY(final_result);
}

void OpCode::setFlagsByDecrement(u8 before) const{
    unsigned char final_result = before - 1;
    this->_cpu->registers.FN_Subtract = true;
    this->_cpu->registers.FZ_Zero = (final_result == 0);
    this->_cpu->registers.FS_Sign = ((final_result & 0x80) > 0);
    this->_cpu->registers.FH_HalfCarry = ((final_result & 0x0f) == 0x0f);
    this->_cpu->registers.FPV_ParityOverflow = (final_result == 0x7f);
    this->setFlagsXY(final_result);
}

void OpCode::setFlagsBySbc16(u16 before, u16 subtract) const{
    int result = before - subtract;
    int carry = before ^ subtract ^ result;
    auto final_result = (u16)result;
    this->_cpu->registers.FN_Subtract = true;
    this->_cpu->registers.FC_Carry = ((carry & 0x10000) != 0);
    this->_cpu->registers.FH_HalfCarry = ((carry & 0x1000) != 0);
    this->_cpu->registers.FS_Sign = ((final_result & 0x8000) > 0);
    this->_cpu->registers.FZ_Zero = (final_result == 0);
    this->_cpu->registers.FPV_ParityOverflow = ((((carry << 1) ^ carry) & 0x10000) != 0);
    this->setFlagsXY((final_result & 0xff00) >> 8);
}

void OpCode::setFlagsByLogical(bool h){
    this->_cpu->registers.FS_Sign = ((this->_cpu->registers.a & 0x80) > 0);
    this->_cpu->registers.FZ_Zero = (this->_cpu->registers.a == 0);
    this->_cpu->registers.FH_HalfCarry = (h);
    this->_cpu->registers.FPV_ParityOverflow = (OpCode::count1(this->_cpu->registers.a) % 2 == 0);
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FC_Carry = false;
    this->setFlagsXY(this->_cpu->registers.a);
}

void OpCode::setFlagsByAdd16(u16 before, u16 addition) const{
    int result = before + addition;
    int carry = before ^ addition ^ result;
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FC_Carry = ((carry & 0x10000) != 0);
    this->_cpu->registers.FH_HalfCarry = ((carry & 0x1000) != 0);
    this->setFlagsXY((result & 0xff00) >> 8);
}

void OpCode::setFlagsByAdc16(u16 before, u16 addition) const{
    int result = before + addition;
    int carry = before ^ addition ^ result;
    auto final_result = (u16)result;
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FC_Carry = ((carry & 0x10000) != 0);
    this->_cpu->registers.FH_HalfCarry = ((carry & 0x1000) != 0);
    this->_cpu->registers.FS_Sign = ((final_result & 0x8000) > 0);
    this->_cpu->registers.FZ_Zero = (0 == final_result);
    this->_cpu->registers.FPV_ParityOverflow = ((((carry << 1) ^ carry) & 0x10000) != 0);
    this->setFlagsXY((final_result & 0xff00) >> 8);
}

void OpCode::setFlagsByRotate(u8 n, bool carry) const {
    this->_cpu->registers.FC_Carry = carry;
    this->_cpu->registers.FH_HalfCarry = false;
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FS_Sign = ((n & 0x80) > 0);
    this->_cpu->registers.FZ_Zero = (0 == n);
    this->_cpu->registers.FPV_ParityOverflow = (OpCode::count1(n) % 2 == 0);
    this->setFlagsXY(n);
}

bool OpCode::getBit(u8 bit, u8 value){
    return ((1 << bit) & value) > 0;
}

u8 OpCode::cb_rlc(u8 value) const{
    bool carry = value >> 7;
    value = (value << 1) | carry;
    this->_cpu->registers.FS_Sign = (value >> 7);
    this->_cpu->registers.FZ_Zero = (value == 0);
    this->_cpu->registers.FPV_ParityOverflow = parity(value);
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FH_HalfCarry = false;
    this->_cpu->registers.FC_Carry = carry;
    return value;
}

u8 OpCode::cb_rrc(u8 value) const{
    bool carry = value & 1;
    value = (value >> 1) | (carry << 7);
    this->_cpu->registers.FS_Sign = (value >> 7);
    this->_cpu->registers.FZ_Zero = (value == 0);
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FH_HalfCarry = false;
    this->_cpu->registers.FC_Carry = carry;
    this->_cpu->registers.FPV_ParityOverflow = parity(value);
    return value;
}
u8 OpCode::cb_rl(u8 value) const{
    bool carry = this->_cpu->registers.FC_Carry;
    this->_cpu->registers.FC_Carry = (value >> 7);
    value = (value << 1) | carry;
    this->_cpu->registers.FS_Sign = (value >> 7);
    this->_cpu->registers.FZ_Zero = (value == 0);
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FH_HalfCarry = false;
    this->_cpu->registers.FPV_ParityOverflow = parity(value);
    return value;
}
u8 OpCode::cb_rr(u8 value) const{
    bool carry = this->_cpu->registers.FC_Carry;
    this->_cpu->registers.FC_Carry = (value & 1);
    value = (value >> 1) | (carry << 7);
    this->_cpu->registers.FS_Sign = (value >> 7);
    this->_cpu->registers.FZ_Zero = (value == 0);
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FH_HalfCarry = false;
    this->_cpu->registers.FPV_ParityOverflow = parity(value);
    return value;
}
u8 OpCode::cb_sla(u8 value) const{
    this->_cpu->registers.FC_Carry = (value >> 7);
    value <<= 1;
    this->_cpu->registers.FS_Sign = (value >> 7);
    this->_cpu->registers.FZ_Zero = (value == 0);
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FH_HalfCarry = false;
    this->_cpu->registers.FPV_ParityOverflow = parity(value);
    return value;
}
u8 OpCode::cb_sra(u8 value) const{
    this->_cpu->registers.FC_Carry = (value & 1);
    value = (value >> 1) | (value & 0b10000000);
    this->_cpu->registers.FS_Sign = (value >> 7);
    this->_cpu->registers.FZ_Zero = (value == 0);
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FH_HalfCarry = false;
    this->_cpu->registers.FPV_ParityOverflow = parity(value);
    return value;
}
u8 OpCode::cb_sll(u8 value) const{
    this->_cpu->registers.FC_Carry = (value >> 7);
    value <<= 1;
    value |= 1;
    this->_cpu->registers.FS_Sign = (value >> 7);
    this->_cpu->registers.FZ_Zero = (value == 0);
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FH_HalfCarry = false;
    this->_cpu->registers.FPV_ParityOverflow = parity(value);
    return value;
}
u8 OpCode::cb_srl(u8 value) const{
    this->_cpu->registers.FC_Carry = (value & 1);
    value >>= 1;
    this->_cpu->registers.FS_Sign = (value >> 7);
    this->_cpu->registers.FZ_Zero = (value == 0);
    this->_cpu->registers.FN_Subtract = false;
    this->_cpu->registers.FH_HalfCarry = false;
    this->_cpu->registers.FPV_ParityOverflow = parity(value);
    return value;
}

#pragma clang diagnostic pop