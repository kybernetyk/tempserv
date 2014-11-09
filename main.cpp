#include <cstdio>
#include <ctime>
#include <unistd.h>
#include "Sensor.h"
#include "Database.h"

void print_error(jsz::Error err) {
    printf("Error: %s\n", err.description.c_str());
}

int main(int argc, char **argv) {
    auto temp = sensor::readTemp();
    if (!temp) {
        print_error(temp.error());
        return 1;
    }

    auto stat = db::addEntry(temp.value()/10.0);
    if (!stat) {
        print_error(stat.error());
        return 2;
    }

    std::time_t now;
    std::time(&now);
    std::tm loctm;
    localtime_r(&now, &loctm);
    
    printf("<%02d:%02d:%02d> temp: %+.1f°\n",loctm.tm_hour, loctm.tm_min, loctm.tm_sec, (float)temp.value()/10.0f);

    return 0;
}

