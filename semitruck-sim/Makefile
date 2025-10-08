# Target: what to build
# Dependencies: what files it needs
# Command: how to build it (must be indented with a TAB, not spaces!)

CXX = g++
CXXFLAGS = -std=c++17
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

all: build/car_sim

build/car_sim: main.cpp Car.h Environment.h SemiTruck.h ParkingSpot.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) main.cpp -o build/car_sim $(LIBS)

run: build/car_sim
	./build/car_sim

clean:
	rm -rf build

.PHONY: all run clean