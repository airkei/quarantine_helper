#include "pulse.h"

// using a macro to avoid function call overhead
#define WAIT_FOR_PIN_STATE(state)             \
    while (digitalRead(pin) != (state)) {     \
        if (micros() - timestamp > timeout) { \
            return 0;                         \
        }                                     \
    }

unsigned long newPulseIn(const byte pin, const byte state,
                         const unsigned long timeout) {
    unsigned long timestamp = micros();
    WAIT_FOR_PIN_STATE(!state);
    WAIT_FOR_PIN_STATE(state);
    timestamp = micros();
    WAIT_FOR_PIN_STATE(!state);
    return micros() - timestamp;
}
