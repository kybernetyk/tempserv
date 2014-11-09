#include <cstdio>
#include <ctime>
#include <unistd.h>
#include "Sensor.h"

void print_error(jsz::Error err) {
    printf("Error: %s\n", err.description.c_str());
}

int main(int argc, char **argv) {
    auto temp = sensor::readTemp();
    if (!temp) {
        print_error(temp.error());
        return 1;
    }
    printf("temp is: %.2f\n", temp.value());

    char buf[255];
    getcwd(buf, 255);
    printf("cwd: %s\n", buf);
    return 0;
}

