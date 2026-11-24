CC = gcc
CXX = g++
PKG_CONFIG = pkg-config
ifeq ($(OS),Windows_NT)
UI_BACKEND ?= sdl
TARGET = fsdash.exe
else
UI_BACKEND ?= fbdev
TARGET = fsdash
endif

CPP_SRC = src/datafield.cpp src/shareddata.cpp src/dashboard_values.cpp src/dashboard_ui.cpp src/main.cpp
ifeq ($(OS),Windows_NT)
CPP_SRC += src/preview_input.cpp
PLATFORM_LIBS = -lws2_32
else
CPP_SRC += src/canpacket.cpp src/canbus.cpp
endif
UI_SRC = ui/ui.c ui/ui_helpers.c ui/components/ui_comp_hook.c \
	$(wildcard ui/screens/*.c) \
	$(wildcard ui/images/*.c) \
	$(wildcard ui/fonts/*.c)

DEPS_DIR ?= deps
LOCAL_LVGL_DIR = $(DEPS_DIR)/lvgl
LOCAL_LV_DRIVERS_DIR = $(DEPS_DIR)/lv_drivers

ifneq ($(wildcard $(LOCAL_LVGL_DIR)/src),)
LVGL_SRC = $(shell find $(LOCAL_LVGL_DIR)/src -name '*.c')
LVGL_CFLAGS ?= -I$(DEPS_DIR) -I$(LOCAL_LVGL_DIR)
LVGL_LIBS ?=
else
LVGL_SRC =
LVGL_CFLAGS ?= $(shell $(PKG_CONFIG) --cflags lvgl 2>/dev/null)
LVGL_LIBS ?= $(shell $(PKG_CONFIG) --libs lvgl 2>/dev/null || echo -llvgl)
endif

ifneq ($(wildcard $(LOCAL_LV_DRIVERS_DIR)),)
ifeq ($(UI_BACKEND),fbdev)
LV_DRIVERS_SRC = $(LOCAL_LV_DRIVERS_DIR)/display/fbdev.c
else
LV_DRIVERS_SRC = $(LOCAL_LV_DRIVERS_DIR)/sdl/sdl.c $(LOCAL_LV_DRIVERS_DIR)/sdl/sdl_common.c
endif
LV_DRIVERS_CFLAGS ?= -I$(DEPS_DIR)
LV_DRIVERS_LIBS ?=
else
LV_DRIVERS_SRC =
LV_DRIVERS_CFLAGS ?=
LV_DRIVERS_LIBS ?= -llv_drivers
endif

# DashDisplay provides main() and calls SDL_SetMainReady() itself.
SDL_CFLAGS ?= $(filter-out -Dmain=SDL_main,$(shell $(PKG_CONFIG) --cflags sdl2 2>/dev/null))
SDL_LIBS ?= $(filter-out -lmingw32 -mwindows -lSDL2main,$(shell $(PKG_CONFIG) --libs sdl2 2>/dev/null || echo -lSDL2))

OBJ = $(CPP_SRC:.cpp=.o) $(UI_SRC:.c=.o) $(LVGL_SRC:.c=.o) $(LV_DRIVERS_SRC:.c=.o)

ifeq ($(UI_BACKEND),fbdev)
BACKEND_FLAGS = -DUI_BACKEND_FBDEV
BACKEND_LIBS =
else
BACKEND_FLAGS = -DUI_BACKEND_SDL -DSDL_MAIN_HANDLED
BACKEND_LIBS = $(SDL_LIBS)
endif

INCLUDE = -Iinclude -I. -Iui $(LVGL_CFLAGS) $(LV_DRIVERS_CFLAGS) $(SDL_CFLAGS)
CFLAGS = -std=c11 -Wall $(INCLUDE) $(BACKEND_FLAGS) -DLV_CONF_INCLUDE_SIMPLE -DLV_DRV_CONF_INCLUDE_SIMPLE
CXXFLAGS = -std=c++20 -Wall $(INCLUDE) $(BACKEND_FLAGS) -DLV_CONF_INCLUDE_SIMPLE -DLV_DRV_CONF_INCLUDE_SIMPLE
LIBS = $(LVGL_LIBS) $(LV_DRIVERS_LIBS) $(BACKEND_LIBS) $(PLATFORM_LIBS) -pthread

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -g -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -g -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -g -c $< -o $@
	
clean:
	rm -f src/*.o ui/*.o ui/screens/*.o ui/components/*.o ui/images/*.o ui/fonts/*.o $(LVGL_SRC:.c=.o) $(LV_DRIVERS_SRC:.c=.o) $(TARGET)
