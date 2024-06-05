#ifndef Z80EMU_MACROS_HPP
#define Z80EMU_MACROS_HPP

#define HALT() \
    while (true) { \
        CTimer::Get()->MsDelay(1000); \
    }
#endif //Z80EMU_MACROS_HPP
