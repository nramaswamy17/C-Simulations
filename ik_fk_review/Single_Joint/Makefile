CXX = g++
CXXFLAGS = -std=c++11 -Wall
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Console version (no visualization)
robot: main.cpp kinematics.h controller.h
	$(CXX) $(CXXFLAGS) main.cpp -o robot

# SFML visualization version
robot-viz: main.cpp kinematics.h controller.h visualize.h
	$(CXX) $(CXXFLAGS) main.cpp -o robot-viz $(SFML_FLAGS)

# Build all
all: robot robot-viz

# Clean up
clean:
	rm -f robot robot-viz

# Run console version
run: robot
	./robot

# Run visualization version
run-viz: robot-viz
	./robot-viz

.PHONY: all clean run run-viz