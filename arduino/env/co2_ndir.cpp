#include "co2_ndir.h"

MHZ19 myMHZ19;

void co2_ndir_setup() {
    Serial1.begin(9600);
    while (!Serial1)
        ;

    myMHZ19.begin(Serial1);
    myMHZ19.autoCalibration();
}

bool co2_ndir_read(uint16_t *co2) {
    *co2 = myMHZ19.getCO2();
    return true;
}
