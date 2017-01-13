#include <libhackrf/hackrf.h>
#include <cstdio>
#include <vector>
#include "rf.h"

using namespace std;

struct global_args_t rf_global_args;

static vector<int> patterns[9];
static vector<float> filter;
static volatile Direction last_dir = none;
static volatile unsigned int pos = 0;

static hackrf_device *device = NULL;
static int last_gain_tx = 20;

void swap_direction(Direction *a, Direction *b) {
    Direction tmp = *a;
    *a = *b;
    *b = tmp;
}

void set_direction_map(struct direction_map_t *map, bool invert_steering, bool invert_throttle, bool swap_axes) {
    /* we are going to treat our struct like a 3x3 array */
    Direction *map_arr = (Direction *)map;

    map_arr[0] = fwd_left;
    map_arr[1] = fwd;
    map_arr[2] = fwd_right;
    map_arr[3] = left;
    map_arr[4] = none;
    map_arr[5] = right;
    map_arr[6] = back_left;
    map_arr[7] = back;
    map_arr[8] = back_right;

    if (swap_axes) {
        /* treat map_arr as 3x3 array, transform s.t. map_arr[i][j] becomes map_arr[j][i] */
        /* do this first so that invert_* work on the transformed values */
        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                if (i<j) {
                    swap_direction(map_arr + 3*j + i, map_arr + 3*i+ j);
                }
            }
        }
    }

    if (invert_steering) {
        /* treat map_arr as a 3x3 array, swap left and right columns */
        for (int i = 0; i < 9; i+=3) {
            swap_direction(map_arr + i, map_arr + i + 2);
        }
    }

    if (invert_throttle) {
        /* treat map_arr as a 3x3 array, swap top and bottom rows */
        for (int i = 0; i < 3; i++) {
            swap_direction(map_arr + i, map_arr + i + 6);
        }
    }
}

static void make_short_pulses(vector<int> &v, int num)
{
    for (int i = 0 ; i < num ; i++) {
        v.push_back(1);
        v.push_back(0);
    }
}

static void init_patterns()
{
    for (int i = 0 ; i < 8 ; i++) {
        // each pattern start with 4 long pulses
        for (int j = 0 ; j < 4 ; j++) {
            patterns[i].push_back(1);
            patterns[i].push_back(1);
            patterns[i].push_back(1);
            patterns[i].push_back(0);
        }
    }
    make_short_pulses(patterns[fwd], 10);
    make_short_pulses(patterns[fwd_left], 28);
    make_short_pulses(patterns[fwd_right], 34);
    make_short_pulses(patterns[back], 40);
    make_short_pulses(patterns[back_left], 52);
    make_short_pulses(patterns[back_right], 46);
    make_short_pulses(patterns[left], 58);
    make_short_pulses(patterns[right], 64);
    patterns[none].push_back(0);
    patterns[none].push_back(0);
    patterns[none].push_back(0);
    // moving averarge can be implemented more efficiently
    // but this allows playing with other type of filters
    for (int i = 0 ; i < 20 ; i++) {
        filter.push_back(0.9/20);
    }
}

static int tx_callback(hackrf_transfer* transfer) {
    int spb = rf_global_args.SAMPLE_RATE / rf_global_args.SYMBOL_RATE; // samples per bit
    for (int i = 0 ; i < transfer->valid_length/2 ; i++) {
        vector<int> &pattern = patterns[last_dir];
        int pattern_size = pattern.size();
        float sum = 0;
        for (int j = 0 ; j < (int)filter.size() ; j++) {
            int sample = pattern[((pos + j)/spb) % pattern_size];
            sum += filter[j] * sample;
        }
        pos += 1;
        transfer->buffer[i*2] = sum * 127;
        transfer->buffer[i*2+1] = 0;
    }
    return 0;
}

bool init_rf()
{
    init_patterns();
    int result = hackrf_init();
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_init() failed: (%d)\n", result);
        return false;
    }
    return true;
}

static void start_tx()
{
    int result = hackrf_open(&device);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_open() failed: (%d)\n", result);
    }
    result = hackrf_set_sample_rate_manual(device, rf_global_args.SAMPLE_RATE, 1);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_sample_rate_set() failed: (%d)\n", result);
    }
    uint32_t baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw_round_down_lt(rf_global_args.SAMPLE_RATE);
    result = hackrf_set_baseband_filter_bandwidth(device, baseband_filter_bw_hz);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_baseband_filter_bandwidth_set() failed: (%d)\n", result);
    }
    result = hackrf_set_freq(device, rf_global_args.FREQ);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_set_freq() failed: (%d)\n", result);
    }
    result = hackrf_set_amp_enable(device, 1);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_set_amp_enable() failed: (%d)\n", result);
    }
    result = hackrf_set_txvga_gain(device, last_gain_tx);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_set_txvga_gain() failed: (%d)\n", result);
    }
    result = hackrf_start_tx(device, tx_callback, NULL);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_start_tx() failed: (%d)\n", result);
    }
}

static void stop_tx()
{
    int result = hackrf_stop_tx(device);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_stop_tx() failed: (%d)\n", result);
    }
    result = hackrf_close(device);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_close() failed: (%d)\n", result);
    }
}

void state_change(Direction dir, int gain_tx)
{
    if (gain_tx != last_gain_tx) {
        last_gain_tx = gain_tx;
    }
    if (dir != last_dir) {
        if (last_dir == none) {
            last_dir = dir;
            pos = 0;
            start_tx();
            return;
        } else if (dir == none) {
            stop_tx();
        }
        last_dir = dir;
        pos = 0;
    }
}

void close_rf()
{
    hackrf_exit();
}

