PROGNAME = rf-car
CFLAGS = -Wall
LDFLAGS = -lSDL2 -lSDL2_image

all: $(PROGNAME)

$(PROGNAME): main.cpp
	$(CXX) main.cpp -o $(PROGNAME) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(PROGNAME) *.o
