#pragma once

#define DEFAULT_FREQ 40684300
#define DEFAULT_SAMPLE_RATE 2000000
#define DEFAULT_SYMBOL_RATE 2018

struct global_args_t {
    int FREQ;
    int SAMPLE_RATE;
    int SYMBOL_RATE;
};

extern struct global_args_t rf_global_args;

enum Direction
{
    fwd, fwd_left, fwd_right, back, back_left, back_right, left, right, none
};

struct direction_map_t {
    /* order is critical here */
    Direction fwd_left, fwd, fwd_right, left, none, right, back_left, back, back_right;
};

void set_direction_map(struct direction_map_t *map, bool invert_steering, bool invert_throttle, bool swap_axes);

bool init_rf();

void state_change(Direction dir, int gain_tx);

void close_rf();

