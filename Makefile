CXX = g++
CXXFLAGS = -std=c++20 -Wall `pkg-config --cflags gtkmm-3.0`
SRC = src/datafield.cpp src/canpacket.cpp src/canbus.cpp src/shareddata.cpp src/display.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)
INCLUDE = -Iinclude
LIBS = `pkg-config --libs gtkmm-3.0`
TARGET = fsdash

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
	
clean:
	rm -f src/*.o $(TARGET)
