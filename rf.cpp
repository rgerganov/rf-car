#include <libhackrf/hackrf.h>
#include <cstdio>
#include <vector>
#include "rf.h"

using namespace std;

static const int FREQ = 40684300; // 40.684 MHz
static const int SAMPLE_RATE = 2000000;
static const int SYMBOL_RATE = 2018;

static vector<int> patterns[9];
static vector<float> filter;
static volatile Direction last_dir = none;
static volatile unsigned int pos = 0;

static hackrf_device *device = NULL;
static int last_gain_tx = 20;

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
    int spb = SAMPLE_RATE / SYMBOL_RATE; // samples per bit
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
    int result = hackrf_open_by_serial(NULL, &device);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_open() failed: (%d)\n", result);
    }
    result = hackrf_set_sample_rate_manual(device, SAMPLE_RATE, 1);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_sample_rate_set() failed: (%d)\n", result);
    }
    uint32_t baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw_round_down_lt(SAMPLE_RATE);
    result = hackrf_set_baseband_filter_bandwidth(device, baseband_filter_bw_hz);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_baseband_filter_bandwidth_set() failed: (%d)\n", result);
    }
    result = hackrf_set_freq(device, FREQ);
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

