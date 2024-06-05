#include "special_registers.hpp"

void SpecialRegisters::ixh(u8 value){
    this->ix = (value << 8) | (this->ix & 0xff);
}
u8 SpecialRegisters::ixh() const {
    return this->ix >> 8;
}
void SpecialRegisters::ixl(u8 value){
    this->ix = (this->ix & 0xff00) | value;
}
u8 SpecialRegisters::ixl() const {
    return this->ix & 0xff;
}
void SpecialRegisters::iyh(u8 value){
    this->iy = (value << 8) | (this->iy & 0xff);
}
u8 SpecialRegisters::iyh() const {
    return this->iy >> 8;
}
void SpecialRegisters::iyl(u8 value){
    this->iy = (this->iy & 0xff00) | value;
}
u8 SpecialRegisters::iyl() const {
    return this->iy & 0xff;
}
