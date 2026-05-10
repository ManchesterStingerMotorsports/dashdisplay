CC = gcc
CXX = g++
PKG_CONFIG = pkg-config
UI_BACKEND ?= sdl

CPP_SRC = src/datafield.cpp src/canpacket.cpp src/canbus.cpp src/shareddata.cpp src/dashboard_values.cpp src/dashboard_ui.cpp src/main.cpp
UI_SRC = ui/ui.c ui/ui_helpers.c ui/components/ui_comp_hook.c \
	$(wildcard ui/screens/*.c) \
	$(wildcard ui/images/*.c) \
	$(wildcard ui/fonts/*.c)
OBJ = $(CPP_SRC:.cpp=.o) $(UI_SRC:.c=.o)

LVGL_CFLAGS ?= $(shell $(PKG_CONFIG) --cflags lvgl 2>/dev/null)
LVGL_LIBS ?= $(shell $(PKG_CONFIG) --libs lvgl 2>/dev/null || echo -llvgl)
LV_DRIVERS_LIBS ?= -llv_drivers
SDL_CFLAGS ?= $(shell $(PKG_CONFIG) --cflags sdl2 2>/dev/null)
SDL_LIBS ?= $(shell $(PKG_CONFIG) --libs sdl2 2>/dev/null || echo -lSDL2)

ifeq ($(UI_BACKEND),fbdev)
BACKEND_FLAGS = -DUI_BACKEND_FBDEV
BACKEND_LIBS =
else
BACKEND_FLAGS = -DUI_BACKEND_SDL -DSDL_MAIN_HANDLED
BACKEND_LIBS = $(SDL_LIBS)
endif

INCLUDE = -Iinclude -I. -Iui $(LVGL_CFLAGS) $(SDL_CFLAGS)
CFLAGS = -std=c11 -Wall $(INCLUDE) $(BACKEND_FLAGS) -DLV_CONF_INCLUDE_SIMPLE -DLV_DRV_CONF_INCLUDE_SIMPLE
CXXFLAGS = -std=c++20 -Wall $(INCLUDE) $(BACKEND_FLAGS) -DLV_CONF_INCLUDE_SIMPLE -DLV_DRV_CONF_INCLUDE_SIMPLE
LIBS = $(LVGL_LIBS) $(LV_DRIVERS_LIBS) $(BACKEND_LIBS) -pthread
TARGET = fsdash

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -g -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -g -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -g -c $< -o $@
	
clean:
	rm -f src/*.o ui/*.o ui/screens/*.o ui/components/*.o ui/images/*.o ui/fonts/*.o $(TARGET)
