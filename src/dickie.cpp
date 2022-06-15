#include <unistd.h>
#include "ui.h"
#include "rf.h"

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

    Dickie car(2444000000, 10000000, 1000000, always_tx);
    if (!car.init()) {
        return 1;
    }
    car.sendSync();
    return RenderUI(&car);
}
