CXX = g++
CXXFLAGS = -std=c++20 -Wall
SRC = src/datafield.cpp src/canpacket.cpp src/canbus.cpp src/shareddata.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)
INCLUDE = -Iinclude
LIBS =
TARGET = fsdash

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -g -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -g -c $< -o $@
	
clean:
	rm -f src/*.o $(TARGET)
