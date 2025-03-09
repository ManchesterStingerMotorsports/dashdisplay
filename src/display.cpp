#include "display.h"

#include <chrono>
#include <cstdlib>

bool DashApp::update_ui(){
	int random_num = rand() % 10;
	double rand2 = (rand() % 100000) / 10;
	GearPosLabel->set_text(std::to_string(random_num));
	RevCounterBar->set_value(rand2);
	return true;	
}

DashApp::DashApp(std::shared_ptr<SharedData> n_shared_data){
	shared_data = n_shared_data;
	
	builder = Gtk::Builder::create_from_file("resources/dashUI.glade");
	builder->get_widget("MainWindow", MainWindow);
	builder->get_widget("GearPosLabel", GearPosLabel);
	builder->get_widget("RevCounterBar", RevCounterBar);
	
	
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &DashApp::update_ui), 30);
	MainWindow->fullscreen();
	MainWindow->show_all();
}

Gtk::Window* DashApp::get_main_window(){ return MainWindow; }




		
