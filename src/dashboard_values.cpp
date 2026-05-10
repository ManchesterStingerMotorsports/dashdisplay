#include "dashboard_values.h"

#include "datafield.h"
#include "ui.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <vector>

namespace {
	constexpr int RPM_INDEX = 0;
	constexpr int MAP_INDEX = 1;
	constexpr int THROTTLE_INDEX = 2;
	constexpr int FUEL_PRES_INDEX = 3;
	constexpr int OIL_PRES_INDEX = 4;
	constexpr int SPEED_INDEX = 5;
	constexpr int BATTERY_INDEX = 6;
	constexpr int COOLANT_TEMP_INDEX = 7;
	constexpr int AIR_TEMP_INDEX = 8;
	constexpr int OIL_TEMP_INDEX = 9;
	constexpr int GEAR_INDEX = 12;

	const char* gear_text(double value){
		static const char* gears[] = {"N", "1", "2", "3", "4", "5"};
		int idx = static_cast<int>(value);
		const int gear_count = static_cast<int>(sizeof(gears) / sizeof(gears[0]));
		if(idx < 0 || idx >= gear_count){
			return "?";
		}
		return gears[idx];
	}

	void set_label_value(lv_obj_t* label, double value, int decimals){
		if(label == nullptr){
			return;
		}

		char buffer[32];
		if(decimals <= 0){
			std::snprintf(buffer, sizeof(buffer), "%.0f", value);
		}
		else{
			std::snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
		}
		lv_label_set_text(label, buffer);
	}

	void set_limit_style(lv_obj_t* label, const std::shared_ptr<DataField>& field){
		if(label == nullptr || field == nullptr){
			return;
		}

		if(field->value > field->upp_lim){
			lv_obj_set_style_text_color(label, lv_color_hex(0xFA051A), LV_PART_MAIN | LV_STATE_DEFAULT);
		}
		else if(field->value < field->low_lim){
			lv_obj_set_style_text_color(label, lv_color_hex(0x4DA3FF), LV_PART_MAIN | LV_STATE_DEFAULT);
		}
		else{
			lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
		}
	}

	void update_field(const std::vector<std::shared_ptr<DataField>>& points, int index, lv_obj_t* label, int decimals){
		if(index < 0 || index >= static_cast<int>(points.size())){
			return;
		}

		set_label_value(label, points.at(index)->value, decimals);
		set_limit_style(label, points.at(index));
	}

	bool is_out_of_limit(const std::shared_ptr<DataField>& field){
		if(field == nullptr){
			return false;
		}
		return field->value < field->low_lim || field->value > field->upp_lim;
	}
}

void init_dashboard_values(){
	if(ui_RPMBAR != nullptr){
		lv_bar_set_range(ui_RPMBAR, 0, 14000);
	}
	if(ui_MESSAGEPANEL != nullptr){
		lv_obj_clear_state(ui_MESSAGEPANEL, LV_STATE_USER_1);
	}
	if(ui_MESSAGE != nullptr){
		lv_label_set_text(ui_MESSAGE, "OK");
	}
}

void update_dashboard_values(std::shared_ptr<SharedData> shared_data){
	if(shared_data == nullptr){
		return;
	}

	std::vector<std::shared_ptr<DataField>> points = shared_data->get_points();
	if(points.empty()){
		return;
	}

	update_field(points, RPM_INDEX, ui_RPMVAL, 0);
	update_field(points, SPEED_INDEX, ui_SPEEDMPH, 1);
	update_field(points, BATTERY_INDEX, ui_BATTVOLTAGE, 1);
	update_field(points, THROTTLE_INDEX, ui_THROTTLEPOS, 1);
	update_field(points, OIL_TEMP_INDEX, ui_OILTEMP, 0);
	update_field(points, COOLANT_TEMP_INDEX, ui_COOLANTTEMP, 0);
	update_field(points, AIR_TEMP_INDEX, ui_AIRTEMP, 0);
	update_field(points, FUEL_PRES_INDEX, ui_FUELPRES, 0);
	update_field(points, OIL_PRES_INDEX, ui_OILPRES, 0);
	update_field(points, MAP_INDEX, ui_MAPPRES, 0);

	if(GEAR_INDEX < static_cast<int>(points.size()) && ui_GEAR != nullptr){
		lv_label_set_text(ui_GEAR, gear_text(points.at(GEAR_INDEX)->value));
	}

	if(RPM_INDEX < static_cast<int>(points.size()) && ui_RPMBAR != nullptr){
		int rpm = static_cast<int>(std::clamp(points.at(RPM_INDEX)->value, 0.0, 14000.0));
		lv_bar_set_value(ui_RPMBAR, rpm, LV_ANIM_OFF);
	}

	bool warning = false;
	const int warning_fields[] = {
		BATTERY_INDEX,
		COOLANT_TEMP_INDEX,
		OIL_TEMP_INDEX,
		FUEL_PRES_INDEX,
		OIL_PRES_INDEX
	};
	for(int index : warning_fields){
		if(index >= 0 && index < static_cast<int>(points.size())){
			warning = warning || is_out_of_limit(points.at(index));
		}
	}

	if(ui_MESSAGEPANEL != nullptr){
		if(warning){
			lv_obj_add_state(ui_MESSAGEPANEL, LV_STATE_USER_1);
		}
		else{
			lv_obj_clear_state(ui_MESSAGEPANEL, LV_STATE_USER_1);
		}
	}
	if(ui_MESSAGE != nullptr){
		lv_label_set_text(ui_MESSAGE, warning ? "CHECK" : "OK");
	}
}
