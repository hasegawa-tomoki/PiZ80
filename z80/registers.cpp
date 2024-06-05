#include "registers.hpp"

void Registers::af(u16 value){
    this->a = (u8)(value >> 8);
    this->f((u8)(value & 0xff));
}
u16 Registers::af() const{
    return (this->a << 8) + this->f();
}

void Registers::bc(u16 value){
    this->b = (u8)(value >> 8);
    this->c = (u8)(value & 0xff);
}
u16 Registers::bc() const{
    return (this->b << 8) + this->c;
}

void Registers::de(u16 value){
    this->d = (u8)(value >> 8);
    this->e = (u8)(value & 0xff);
}
u16 Registers::de() const{
    return (this->d << 8) + this->e;
}

void Registers::hl(u16 value){
    this->h = (u8)(value >> 8);
    this->l = (u8)(value & 0xff);
}
u16 Registers::hl() const{
    return (this->h << 8) + this->l;
}

void Registers::f(u8 value){
    this->FC_Carry =            ((value & 0b00000001) > 0);
    this->FN_Subtract =         ((value & 0b00000010) > 0);
    this->FPV_ParityOverflow =  ((value & 0b00000100) > 0);
    this->F_X =                 ((value & 0b00001000) > 0);
    this->FH_HalfCarry =        ((value & 0b00010000) > 0);
    this->F_Y =                 ((value & 0b00100000) > 0);
    this->FZ_Zero =             ((value & 0b01000000) > 0);
    this->FS_Sign =             ((value & 0b10000000) > 0);
}
u8 Registers::f() const {
    u8 value = 0;
    if (this->FC_Carry){            value |= 0b00000001; }
    if (this->FN_Subtract){         value |= 0b00000010; }
    if (this->FPV_ParityOverflow){  value |= 0b00000100; }
    if (this->F_X){                 value |= 0b00001000; }
    if (this->FH_HalfCarry){        value |= 0b00010000; }
    if (this->F_Y){                 value |= 0b00100000; }
    if (this->FZ_Zero){             value |= 0b01000000; }
    if (this->FS_Sign){             value |= 0b10000000; }
    return value;
}

u8 Registers::carry_by_val(){
    return ((this->FC_Carry) ? 1 : 0);
}
