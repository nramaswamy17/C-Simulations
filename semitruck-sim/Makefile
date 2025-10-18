# Makefile for Lane Keeping System

CXX = g++
CXXFLAGS = -std=c++17
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

all: build/lane_keeping

build/lane_keeping: main.cpp Car.h Environment.h SemiTruck.h Lane.h Controller.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) main.cpp -o build/lane_keeping $(LIBS)

run: build/lane_keeping
	./build/lane_keeping

clean:
	rm -rf build

.PHONY: all run clean