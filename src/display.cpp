#include "display.h"
#include <chrono>
#include <cstdlib>


bool DashApp::update_ui(){
	int random_num = rand() % 101;
	ValueLabel00.set_text(std::to_string(random_num));
	return true;	
}

DashApp::DashApp(){
	
	ValueLabel00.set_text("Hello World!");
	add(ValueLabel00);
	
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &DashApp::update_ui), 100);
	
	show_all();
}




		
