#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rf.h"
#include "ui.h"

int main(int argc, char *argv[])
{
    char optc;
    bool invert_steering = false;
    bool invert_throttle = false;
    bool always_tx = false;
    int64_t freq = 40684300;
    int sample_rate = 2000000;

    while ((optc = getopt(argc, argv, "f:s:STX")) > 0) {
        switch (optc) {
        case 'f':
            freq = atoi(optarg);
            break;
        case 's':
            sample_rate = atoi(optarg);
            break;
        case 'S':
            invert_steering = true;
            break;
        case 'T':
            invert_throttle = true;
            break;
        case 'X':
            always_tx = true;
            break;
        default:
            fprintf(stderr, "%s [args]\n", argv[0]);
            fprintf(stderr, "\t-f FREQUENCY\tset frequency (integer, HZ)\n");
            fprintf(stderr, "\t-s SAMPLE_RATE\tset sample rate\n");
            fprintf(stderr, "\t-S\t\tinvert steering\n");
            fprintf(stderr, "\t-T\t\tinvert throttle\n");
            fprintf(stderr, "\t-X\t\talways TX\n");
            exit(EXIT_FAILURE);
        }
    }

	OokCar car(freq, sample_rate, 2018, always_tx);
    if (!car.init()) {
        return 1;
    }
    if (invert_steering) {
        car.invertSteering();
    }
    if (invert_throttle) {
        car.invertThrottle();
    }
	return RenderUI(&car);
}
