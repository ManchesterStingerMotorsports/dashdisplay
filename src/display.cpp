#include "display.h"
#include "datafield.h"
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>



bool DashApp::update_ui(){
	std::vector<std::shared_ptr<DataField>> iteration_data = shared_data->get_points();
	RevCounterBar->set_value(iteration_data.at(rpm_index)->value);
	int gear_pos = (int)iteration_data.at(gear_index)->value;
	if (gear_pos < (int)gear_enumeration.size()){GearPosLabel->set_text(gear_enumeration.at(gear_pos));}
	for (const auto& fr : value_layout_map){
		if(fr.second >= 0){
			std::shared_ptr<DataField> dp = iteration_data.at(fr.second);
			double display_val = dp->value;
			auto style_context = fr.first->get_style_context();
			if (display_val > dp->upp_lim){style_context->add_class("overlimit");}
			else if (display_val < dp->low_lim){style_context->add_class("underlimit");}
			else{
				style_context->remove_class("overlimit");
				style_context->remove_class("underlimit");
				}
			fr.first->set_text(std::to_string((int)display_val));
		}
	}
	return true;	
}


void DashApp::init_ui(){
	std::vector<std::shared_ptr<DataField>> init_data = shared_data->get_points();
	for (const auto& fr : value_layout_map){
		if(fr.second >= 0){
		Gtk::Container* gen_cont = fr.first->get_parent()->get_parent();
		auto frame = dynamic_cast<Gtk::Frame*>(gen_cont);
		if(frame){
			std::shared_ptr<DataField> dp = init_data.at(fr.second);
			std::string header = dp->title + " (" + dp->unit + ")";
			frame->set_label(header);
			}
		else{
			std::cout << "The retrieved parent of a label was " << typeid(*gen_cont).name() << " when it should have been a frame!" << std::endl;
			}
		}
	}
		
	}
	

DashApp::DashApp(std::shared_ptr<SharedData> n_shared_data){
	shared_data = n_shared_data;
	
	builder = Gtk::Builder::create_from_file("/home/fsdash/dash/resources/dashUI.glade");
	builder->get_widget("MainWindow", MainWindow);
	builder->get_widget("GearPosLabel", GearPosLabel);
	builder->get_widget("RevCounterBar", RevCounterBar);
	builder->get_widget("Value00", Value00);
	builder->get_widget("Value01", Value01);
	builder->get_widget("Value02", Value02);
	builder->get_widget("Value03", Value03);
	builder->get_widget("Value10", Value10);
	builder->get_widget("Value11", Value11);
	builder->get_widget("Value12", Value12);
	builder->get_widget("Value13", Value13);
	builder->get_widget("Value20", Value20);
	builder->get_widget("Value21", Value21);
	builder->get_widget("Value22", Value22);
	builder->get_widget("Value23", Value23);
	
	gear_enumeration = {"N", "1", "2", "3", "4", "5"};
	
	gear_index = 12;
	rpm_index = 0;
	
	value_layout_map = {
			{Value00, 2},
			{Value01, 5},
			{Value02, 0},
			{Value03, 6},
			{Value10, 1},
			{Value11, 7},
			{Value12, 9},
			{Value13, 8},
			{Value20, 3},
			{Value21, 4},
			{Value22, 10},
			{Value23, 13}
		};

	init_ui();
	
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &DashApp::update_ui), 35);
	MainWindow->fullscreen();
	MainWindow->show_all();
}

Gtk::Window* DashApp::get_main_window(){ return MainWindow; }




		
