# Overview
[![Build Status](https://github.com/rgerganov/rf-car/workflows/CI/badge.svg)](https://github.com/rgerganov/rf-car/actions)

Small programs for controlling RC cars with HackRF.

## `ook_car`
![ook-car](/ook-car.jpg)

This car works on 40.684 MHz and moves in 8 directions (forward, backward, left, right, forward-right,
forward-left, backward-right, backward-left). You can see it in action here:

[![demo-ook](https://img.youtube.com/vi/itS2pWkgNrM/0.jpg)](https://www.youtube.com/watch?v=itS2pWkgNrM)

The remote control is using OOK modulation with long and short pulses. One long
pulse is equal to three short pulses. For example, to move the car forward, we
need to send 4 long pulses followed by 10 short pulses. We can easily find the
control sequence for each direction by recording the signal from the RC and
then analyse it with [inspectrum](https://github.com/miek/inspectrum):

![ook-signal](/inspectrum-ook.png)]

To synthesize the signal with the HackRF, we need to transmit
`SAMPLE_RATE/SYMBOL_RATE` samples ('1' or '0') for each bit of the control
sequence. We can find the `SYMBOL_RATE` with inspectrum, it is about 2018.
We choose the `SAMPLE_RATE` to be 2M.

## `fsk_car`
![fsk-car](/fsk-car.jpg)

This is the [Dickie Toys Flippy RC Car](https://www.amazon.de/-/en/Dickie-Flippy-Control-Rotation-Function/dp/B084PY44PN) and it moves in 6 directions. You can see it in action here:

[![demo-fsk](https://img.youtube.com/vi/mqSv-Nycy_4/0.jpg)](https://www.youtube.com/watch?v=mqSv-Nycy_4)

First you need to turn on the car and then start `fsk_car`. This is because `fsk_car` sends a synchronization pattern upon start and the car needs to receive it. More details on the remote protcol coming soon.

# Build & run

The program depends only on SDL2, SDL2_image and libhackrf. To build on Linux:

    $ sudo apt-get install libsdl2-dev libsdl2-image-dev libhackrf-dev
    $ make
    $ ./rf-car

To build on OSX:

    $ brew install sdl2 sdl2_image hackrf
    $ make
    $ ./rf-car

# Support

If you want to support the project you can send me an RC toy and I will try to reverse engineer its protocol and add it here. Send me an [email](mailto:rgerganov@gmail.com) for more details.
