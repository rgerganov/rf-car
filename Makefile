SRC_DIR := src

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(SRC_DIR)/%.o)

CXXFLAGS = -Wall -std=c++11 -Iinclude $(shell pkg-config --cflags libhackrf) $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2) -lSDL2_image $(shell pkg-config --libs libhackrf)

.PHONY: all clean

all: ook_car dickie concept rover

ook_car: src/ook_car.o src/rf.o src/ui.o
	$(CXX) $^ -o $@ $(LDFLAGS)

dickie: src/dickie.o src/rf.o src/ui.o
	$(CXX) $^ -o $@ $(LDFLAGS)

concept: src/concept.o src/rf.o src/ui.o
	$(CXX) $^ -o $@ $(LDFLAGS)

rover: src/rover.o src/rf.o src/ui.o
	$(CXX) $^ -o $@ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	    $(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f ook_car dickie concept rover $(OBJ)
