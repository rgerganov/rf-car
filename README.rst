rf-car
------
This is a small program for controlling RC cars with HackRF. 
Most of the RC toys use the same protocol, so you just need to find the
frequency on which the toy operate. In my case it is 40.684 MHz. If your RC toy
can move in 8 directions (forward, backward, left, right, forward-right,
forward-left, backward-right, backward-left), then there is a great chance that
you can use this program to control it.

how it works
------------
The remote control is using OOK modulation with long and short pulses. One long
pulse is equal to three short pulses. For example, to move the car forward, we
need to send 4 long pulses followed by 10 short pulses. We can easily find the
control sequence for each direction by recording the signal from the RC and
then analyse it with `inspectrum <https://github.com/miek/inspectrum>`_:

.. image:: inspectrum.png
   :scale: 67 %

To synthesize the signal with the HackRF, we need to transmit
``SAMPLE_RATE/SYMBOL_RATE`` samples ('1' or '0') for each bit of the control
sequence. We can find the ``SYMBOL_RATE`` with inspectrum, it is about 2018.
We choose the ``SAMPLE_RATE`` to be 2M.

build & run
-----------
The program depends only on SDL2, SDL2_image and libhackrf. To build on Linux::

    $ sudo apt-get install libsdl2-dev libsdl2-image-dev libhackrf-dev
    $ make
    $ ./rf-car

To build on OSX::

    $ brew install sdl2 sdl2_image hackrf
    $ make
    $ ./rf-car

