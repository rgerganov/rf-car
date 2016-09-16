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

bool init_rf();

void state_change(Direction dir, int gain_tx);

void close_rf();

