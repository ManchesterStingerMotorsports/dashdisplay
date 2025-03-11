
#include "display.h"
#include "shareddata.h"
#include "datafield.h"
#include "canbus.h"
#include "canpacket.h"

#include <memory>
#include <thread>
#include <vector>
#include <iostream>

using namespace std;

void dummy_display(shared_ptr<SharedData> dashData){
	
	while(true){
		vector<shared_ptr<DataField> > datapoints = dashData->get_points();
		
		system("clear");
		for (shared_ptr<DataField> dp : datapoints){
			cout << dp->title << ": " << dp->value << endl;
		}
		
		this_thread::sleep_for(chrono::milliseconds(200));
	}
	
}

int app_boot_up(shared_ptr<SharedData> shared_data){
	auto app = Gtk::Application::create("com.formulastudentuom.dashdisplay");
	DashApp window = DashApp(shared_data);
	
	auto styling = Gtk::CssProvider::create();
	styling->load_from_path("/home/fsdash/dash/resources/style.css");
	auto screen = Gdk::Screen::get_default();
	auto context = Gtk::StyleContext::create();
	
	context->add_provider_for_screen(screen, styling, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	
	return app->run(*window.get_main_window());
}


int main(int argc, char* argv[]){
	
	system("sudo ip link set can0 up type can bitrate 1000000 \
		&& sudo ip link set can1 up type can bitrate 1000000");
	
	shared_ptr<SharedData> dashData = make_shared<SharedData>();
	
	vector<shared_ptr<DataField> > pk_360 = {
			make_shared<DataField>("RPM", 1, 0, 2, "RPM"),
			make_shared<DataField>("MAP", 0.1, 0, 2, "kPa"),
			make_shared<DataField>("Throttle Pos.", 0.1, 0, 2, "%")
			};
	
	vector<shared_ptr<DataField> > pk_3E0 = {
			make_shared<DataField>("Coolant Temp.", 0.1, 0, 2, "C"),
			make_shared<DataField>("Air Temp.", 0.1, 0, 2, "C"),
			make_shared<DataField>("Fuel Temp.", 0.1, 0, 2, "C"),
			make_shared<DataField>("Oil Temp.", 0.1, 0, 2, "C")
			};
	
	CANBus bus;
	bus.start_can();
	//bus.dump_packets(50);
	bus.add_packet(0x360, move(make_unique<CANPacket>(0x360, dashData, pk_360)));
	bus.add_packet(0x3E0, move(make_unique<CANPacket>(0x3E0, dashData, pk_3E0)));
	
	
	
	
	thread producer(&CANBus::listen, &bus, dashData);
	thread consumer(app_boot_up, dashData);
	
	producer.join();
	consumer.join();
	return 0;
}


