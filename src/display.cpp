#include "display.h"
#include "datafield.h"
#include <chrono>
#include <cstdlib>
#include <vector>
#include <string>



bool DashApp::update_ui(){
	std::vector<std::shared_ptr<DataField>> iteration_data = shared_data->get_points();
	RevCounterBar->set_value(iteration_data.at(rpm_index)->value);
	//GearPosLabel->set_text
	for (const auto& fr : value_layout_map){
		std::shared_ptr<DataField> dp = iteration_data.at(fr.second);
		fr.first->set_text(std::to_string(dp->value));
	}
	return true;	
}


/*
void DashApp::init_ui(){
	std::vector<std::shared_ptr<DataField>> init_data = shared_data->get_points();
	for (const auto& fr : frame_layout_map){
		std::shared_ptr<DataField> dp = init_data.at(fr.second);
		std::string header = dp->title + " (" + dp->unit + ")";
		fr.first->set_label(header);
		}
	}
*/	

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
	
	
	gear_index = 0;
	rpm_index = 0;
	
	value_layout_map = {
			{Value00, 0},
			{Value01, 1},
			{Value02, 2},
			{Value03, 3},
			{Value10, 4},
			{Value11, 5},
			{Value12, 6},
			{Value13, 0},
			{Value20, 0},
			{Value21, 0},
			{Value22, 0},
			{Value23, 0}
		};
	
	//init_ui();
	
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &DashApp::update_ui), 1000);
	MainWindow->fullscreen();
	MainWindow->show_all();
}

Gtk::Window* DashApp::get_main_window(){ return MainWindow; }




		
