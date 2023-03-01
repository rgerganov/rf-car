#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include "rf.h"
#include "ui.h"

int main(int argc, char *argv[])
{
    char optc;
    bool always_tx = false;

    while ((optc = getopt(argc, argv, "X")) > 0) {
        switch (optc) {
        case 'X':
            always_tx = true;
            break;
        default:
            fprintf(stderr, "%s [args]\n", argv[0]);
            fprintf(stderr, "\t-X\t\talways TX\n");
            exit(EXIT_FAILURE);
        }
    }

    //note: correction -1.25MHz is made because of offset for LOW and HIGH bits in FskCar::txCallback
    Concept car(2417000000 - 1250000, 10000000, 1000000, always_tx);
    if (!car.init()) {
        return 1;
    }
    car.sendSync();
    return RenderUI(&car);
}
