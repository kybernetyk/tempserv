#include "Sensor.h"
#include <hidapi.h>

namespace sensor {
    Result<double> readTemp() {
        hid_device *handle = hid_open(0x16c0, 0x0480, nullptr);
        if (!handle) {
            return jsz::Error(1, __PRETTY_FUNCTION__, "No sensor found!");
        }

        unsigned char buf[65];
        int num = hid_read(handle, buf, 64);
        if (num < 0) {
            return jsz::Error(2, __PRETTY_FUNCTION__, "Could not read from sensor!");
        }
        if (num == 64) {
            short temp = *(short *) &buf[4]; //holy fuck!
            return double(temp);
        }

        return jsz::Error(3, __PRETTY_FUNCTION__, "Sensor returned unexpected data!");
    }
}
