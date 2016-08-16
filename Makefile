PROGNAME = rf-car
CFLAGS = -Wall $(shell pkg-config --cflags libhackrf) $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2) -lSDL2_image $(shell pkg-config --libs libhackrf)

all: $(PROGNAME)

$(PROGNAME): main.cpp rf.cpp rf.h
	$(CXX) main.cpp rf.cpp -o $(PROGNAME) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(PROGNAME) *.o
