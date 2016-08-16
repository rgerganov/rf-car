#pragma once

enum Direction
{
    fwd, fwd_left, fwd_right, back, back_left, back_right, left, right, none
};

bool init_rf();

void state_change(Direction dir, int gain_tx);

void close_rf();

