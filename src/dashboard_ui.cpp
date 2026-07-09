#include "dashboard_ui.h"

#include "dashboard_values.h"
#include "ui.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#ifdef UI_BACKEND_SDL
#include "lv_drivers/sdl/sdl.h"
#endif

#ifdef UI_BACKEND_FBDEV
#include "lv_drivers/display/fbdev.h"
#endif

namespace {
	constexpr int SCREEN_WIDTH = 800;
	constexpr int SCREEN_HEIGHT = 480;
	constexpr int BUFFER_LINES = 80;
	constexpr int TICK_MS = 5;
	constexpr int UI_REFRESH_MS = 35;
	constexpr int STARTUP_FULL_REDRAW_MS = 30000;

	lv_disp_draw_buf_t draw_buffer;
	lv_color_t buffer_1[SCREEN_WIDTH * BUFFER_LINES];
	lv_color_t buffer_2[SCREEN_WIDTH * BUFFER_LINES];
	lv_disp_drv_t display_driver;

	void init_display_driver(){
		lv_disp_draw_buf_init(&draw_buffer, buffer_1, buffer_2, SCREEN_WIDTH * BUFFER_LINES);
		lv_disp_drv_init(&display_driver);
		display_driver.draw_buf = &draw_buffer;
		display_driver.hor_res = SCREEN_WIDTH;
		display_driver.ver_res = SCREEN_HEIGHT;

#ifdef UI_BACKEND_SDL
		sdl_init();
		display_driver.flush_cb = sdl_display_flush;
#endif

#ifdef UI_BACKEND_FBDEV
		fbdev_init();
		display_driver.flush_cb = fbdev_flush;
#endif

		lv_disp_drv_register(&display_driver);
	}
}

int run_dashboard_ui(std::shared_ptr<SharedData> shared_data){
	lv_init();
	init_display_driver();
	ui_init();
	init_dashboard_values();

	auto startup_time = std::chrono::steady_clock::now();
	auto last_refresh = std::chrono::steady_clock::now();
	while(shared_data->ui_run){
		lv_tick_inc(TICK_MS);

		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_refresh);
		if(elapsed.count() >= UI_REFRESH_MS){
			update_dashboard_values(shared_data);
			auto startup_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startup_time);
			if(startup_elapsed.count() < STARTUP_FULL_REDRAW_MS){
				lv_obj_invalidate(lv_scr_act());
			}
			last_refresh = now;
		}

		lv_timer_handler();
		std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));
	}

	ui_destroy();
	return 0;
}
