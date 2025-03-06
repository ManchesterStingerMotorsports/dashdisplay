
#include "display.h"

int main(int argc, char* argv[]){
	auto app = Gtk::Application::create("com.formulastudentuom.dashdisplay");
	DashApp window = DashApp();
	return app->run(window);
}

