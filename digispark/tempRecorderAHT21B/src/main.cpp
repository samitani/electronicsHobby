#define EEPROM_V1 0x0000
#define EEPROM_SIZE 512
#define EEPROM_MAX EEPROM_SIZE - 4

#include <Arduino.h>
#include <DigiUSB.h>
#include <AHTxx.h>
#include <avr/eeprom.h>

float *addr;
unsigned char p;
AHTxx aht20(AHTXX_ADDRESS_X38, AHT2x_SENSOR);

void setup() {
    DigiUSB.begin();

    do {
        DigiUSB.delay(1024);
    } while (aht20.begin() != true);

    //eeprom_busy_wait();
    addr = (float *)(sizeof(float) * eeprom_read_dword((uint32_t *)EEPROM_V1));
    p = 0;
}

void loop() {
    DigiUSB.refresh();

    // record temparature every 512 seconds
    if (p == 0 && addr < (float *)(EEPROM_MAX)) {
        float ahtValue = aht20.readTemperature();
        if (ahtValue != AHTXX_ERROR) {
            eeprom_write_dword((uint32_t *)EEPROM_V1, (uint32_t)(addr - (float *)EEPROM_V1));
            addr++;
            eeprom_write_float(addr, ahtValue);
        }
    }

    p++;

    if (DigiUSB.available()) {
        DigiUSB.refresh();
        char c = DigiUSB.read();
        if (c == 'r') {
            DigiUSB.println(eeprom_read_dword((uint32_t *)EEPROM_V1)+1);
            for (float *a = addr; a > EEPROM_V1; a--) {
                eeprom_busy_wait();
                DigiUSB.println(eeprom_read_float(a));
                DigiUSB.delay(16);
            }
        } else if (c ==  'c') {
            addr = (float *)EEPROM_V1;
            p = 0;
            DigiUSB.println("ok");
        }
    }

    DigiUSB.delay(2000);
}
