#include "dust.h"

#include "pulse.h"

unsigned long duration = 0;
unsigned long starttime = 0;

static float calcPPMV(float concentration, float temperature) {
    // ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
    // 0.08205   = Universal gas constant in atm·m3/(kmol·K)
    float ppmv = (concentration * 0.0283168 / 100 / 1000) *
                 (0.08205 * temperature) / 0.01;
    ppmv *= 1000;  // to µg/m³
    return ppmv;
}

static float pcs2ugm3(float concentration) {
    float pi = 3.14159;
    // 全粒子密度(1.65E12μg/ m3)
    float density = 1.65 * pow(10, 12);
    // PM2.5粒子の半径(2.5μm)
    float r25 = 2.5 * pow(10, -6);
    float vol25 = (4 / 3) * pi * pow(r25, 3);
    float mass25 = density * vol25;  // μg
    float K = 3531.5;                // per m^3
    // μg/m^3に変換して返す
    return concentration * K * mass25;
}

void dust_setup(int pin) {
    pinMode(pin, INPUT);

    starttime = millis();
}

bool dust_read(int pin, unsigned long ts, float temperature, double *dust) {
    duration += newPulseIn(pin, LOW);

    if ((millis() - starttime) > ts) {
        float ratio = 0;
        long concentration = 0;

        // LOW Ratio
        ratio = duration / (ts * 10.0);
        // Calculate Content
        concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) +
                        520 * ratio + 0.62;  // using spec sheet curve
        *dust = (concentration * 0.0283168 / 100 / 1000) *
                (0.08205 * temperature) / 0.01;

        duration = 0;
        starttime = millis();

        return true;
    }

    return false;
}
