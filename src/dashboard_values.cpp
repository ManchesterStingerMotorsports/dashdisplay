#include "dashboard_values.h"

#include "datafield.h"
#include "ui.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
	constexpr const char* RPM_FIELD = "RPM";
	constexpr const char* THROTTLE_FIELD = "Throttle Pos.";
	constexpr const char* FUEL_PRES_FIELD = "Fuel Pres.";
	constexpr const char* OIL_PRES_FIELD = "Oil Pres.";
	constexpr const char* SPEED_FIELD = "Speed";
	constexpr const char* BATTERY_FIELD = "Battery Volts";
	constexpr const char* COOLANT_TEMP_FIELD = "Coolant Temp.";
	constexpr const char* OIL_TEMP_FIELD = "Oil Temp.";
	constexpr const char* GEAR_FIELD = "Gear";
	constexpr const char* LAUNCH_CONTROL_FIELD = "Launch Control Active";
	constexpr double TEMP_AMBER_MIN = 95.0;
	constexpr double TEMP_RED_MIN = 100.0;

	using FieldMap = std::unordered_map<std::string, std::shared_ptr<DataField>>;

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

	void set_temperature_style(lv_obj_t* label, double value){
		if(label == nullptr){
			return;
		}

		lv_obj_clear_state(label, LV_STATE_USER_1 | LV_STATE_USER_2);
		if(value > TEMP_RED_MIN){
			lv_obj_add_state(label, LV_STATE_USER_2);
			lv_obj_set_style_text_color(label, lv_color_hex(0xFA051A), LV_PART_MAIN | LV_STATE_DEFAULT);
		}
		else if(value >= TEMP_AMBER_MIN){
			lv_obj_add_state(label, LV_STATE_USER_1);
			lv_obj_set_style_text_color(label, lv_color_hex(0xFB9B02), LV_PART_MAIN | LV_STATE_DEFAULT);
		}
		else{
			lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
		}
	}

	FieldMap map_fields(const std::vector<std::shared_ptr<DataField>>& points){
		FieldMap fields;
		for(const auto& point : points){
			if(point != nullptr){
				fields[point->title] = point;
			}
		}
		return fields;
	}

	std::shared_ptr<DataField> find_field(const FieldMap& fields, const char* title){
		auto it = fields.find(title);
		if(it == fields.end()){
			return nullptr;
		}
		return it->second;
	}

	void update_field(const FieldMap& fields, const char* title, lv_obj_t* label, int decimals){
		std::shared_ptr<DataField> field = find_field(fields, title);
		if(field == nullptr){
			return;
		}

		set_label_value(label, field->value, decimals);
		if(label != nullptr){
			lv_obj_clear_state(label, LV_STATE_USER_1 | LV_STATE_USER_2);
			lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
		}
	}

	void update_temperature_field(const FieldMap& fields, const char* title, lv_obj_t* label){
		std::shared_ptr<DataField> field = find_field(fields, title);
		if(field == nullptr){
			return;
		}

		set_label_value(label, field->value, 0);
		set_temperature_style(label, field->value);
	}

	bool is_temperature_red(const FieldMap& fields, const char* title){
		std::shared_ptr<DataField> field = find_field(fields, title);
		return field != nullptr && field->value > TEMP_RED_MIN;
	}

	bool is_active(const FieldMap& fields, const char* title){
		std::shared_ptr<DataField> field = find_field(fields, title);
		return field != nullptr && field->value > 0.5;
	}

	void clear_message_state(){
		if(ui_MESSAGEPANEL != nullptr){
			lv_obj_clear_state(ui_MESSAGEPANEL, LV_STATE_USER_1 | LV_STATE_USER_2);
		}
		if(ui_HOTMESSAGE != nullptr){
			lv_obj_clear_state(ui_HOTMESSAGE, LV_STATE_USER_1);
		}
		if(ui_LCMESSAGE != nullptr){
			lv_obj_clear_state(ui_LCMESSAGE, LV_STATE_USER_1);
		}
	}
}

void init_dashboard_values(){
	if(ui_rpmbar != nullptr){
		lv_bar_set_range(ui_rpmbar, 0, 14000);
	}
	clear_message_state();
}

void update_dashboard_values(std::shared_ptr<SharedData> shared_data){
	if(shared_data == nullptr){
		return;
	}

	std::vector<std::shared_ptr<DataField>> points = shared_data->get_points();
	if(points.empty()){
		return;
	}

	FieldMap fields = map_fields(points);

	update_field(fields, RPM_FIELD, ui_RPMVAL, 0);
	update_field(fields, SPEED_FIELD, ui_SPEEDMPH, 1);
	update_field(fields, BATTERY_FIELD, ui_BATTVOLTAGE, 1);
	update_field(fields, THROTTLE_FIELD, ui_THROTTLEPOS, 1);
	update_temperature_field(fields, OIL_TEMP_FIELD, ui_OILTEMP);
	update_temperature_field(fields, COOLANT_TEMP_FIELD, ui_COOLANTTEMP);
	update_field(fields, FUEL_PRES_FIELD, ui_FUELPRESSURE, 0);
	update_field(fields, OIL_PRES_FIELD, ui_OILPRESSURE, 0);

	std::shared_ptr<DataField> gear = find_field(fields, GEAR_FIELD);
	if(gear != nullptr && ui_GEAR != nullptr){
		lv_label_set_text(ui_GEAR, gear_text(gear->value));
	}

	std::shared_ptr<DataField> rpm_field = find_field(fields, RPM_FIELD);
	if(rpm_field != nullptr && ui_rpmbar != nullptr){
		int rpm = static_cast<int>(std::clamp(rpm_field->value, 0.0, 14000.0));
		lv_bar_set_value(ui_rpmbar, rpm, LV_ANIM_OFF);
	}

	bool temp_red = is_temperature_red(fields, COOLANT_TEMP_FIELD) || is_temperature_red(fields, OIL_TEMP_FIELD);
	bool launch_control = is_active(fields, LAUNCH_CONTROL_FIELD);

	if(ui_MESSAGEPANEL != nullptr){
		lv_obj_clear_state(ui_MESSAGEPANEL, LV_STATE_USER_1 | LV_STATE_USER_2);
		if(temp_red){
			lv_obj_add_state(ui_MESSAGEPANEL, LV_STATE_USER_1);
		}
		else if(launch_control){
			lv_obj_add_state(ui_MESSAGEPANEL, LV_STATE_USER_2);
		}
	}
	if(ui_HOTMESSAGE != nullptr){
		if(temp_red){
			lv_obj_add_state(ui_HOTMESSAGE, LV_STATE_USER_1);
		}
		else{
			lv_obj_clear_state(ui_HOTMESSAGE, LV_STATE_USER_1);
		}
	}
	if(ui_LCMESSAGE != nullptr){
		if(launch_control && !temp_red){
			lv_obj_add_state(ui_LCMESSAGE, LV_STATE_USER_1);
		}
		else{
			lv_obj_clear_state(ui_LCMESSAGE, LV_STATE_USER_1);
		}
	}
}
