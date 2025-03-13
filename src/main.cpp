
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
		
	CANBus bus;
	
	
	/*To add packets to the packet listener, create a vector of shared_ptr<DataField> for the packet
	 * For each data field in the packet, create a shared pointer to a new DataField obj.
	 * make_shared<DataField>({Title}, {Gain}, {Offset}, {Start Byte}, {Length in bytes}, {Lower Alarm}, {Upper Alarm}, {Unit})
	 */
	
	vector<shared_ptr<DataField> > pk_360 = {
			make_shared<DataField>("RPM", 1, 0, 0, 2, -1, 13000, "RPM"),
			make_shared<DataField>("MAP", 0.1, 0, 2, 2, -1000, 1000, "kPa (abs)"),
			make_shared<DataField>("Throttle Pos.", 0.1, 0, 4, 2, -1, 101, "%")
			};			
	bus.add_packet(0x360, move(make_unique<CANPacket>(0x360, dashData, pk_360)));
			
	vector<shared_ptr<DataField> > pk_361 = {
			make_shared<DataField>("Fuel Pres.", 0.0145, -101.3, 0, 2, 20, 50, "PSI"),
			make_shared<DataField>("Oil Pres.", 0.1, -101.3, 2, 2, 0, 500, "kPa")
			};
	bus.add_packet(0x361, move(make_unique<CANPacket>(0x361, dashData, pk_361)));
			
	vector<shared_ptr<DataField> > pk_370 = {
			make_shared<DataField>("Speed", 0.0621, 0, 0, 2, -1, 100, "MPH"),
			};
	bus.add_packet(0x370, move(make_unique<CANPacket>(0x370, dashData, pk_370)));
			
	vector<shared_ptr<DataField> > pk_372 = {
			make_shared<DataField>("Battery Volts", 0.1, 0, 0, 2, 10.5, 14.5, "V"),
			};
	bus.add_packet(0x372, move(make_unique<CANPacket>(0x372, dashData, pk_372)));
	
	vector<shared_ptr<DataField> > pk_3E0 = {
			make_shared<DataField>("Coolant Temp.", 0.1, -273, 0, 2, 0, 95, "C"),
			make_shared<DataField>("Air Temp.", 0.1, -273, 2, 2, -10, 50, "C"),
			make_shared<DataField>("Oil Temp.", 0.1, -273, 6, 2, 90, 105, "C")
			};
	bus.add_packet(0x3E0, move(make_unique<CANPacket>(0x3E0, dashData, pk_3E0)));
			
	vector<shared_ptr<DataField> > pk_3EB = {
			make_shared<DataField>("IGN Angle", 0.1, 0, 4, 2, -180, 180, "DEG")
			};
	bus.add_packet(0x3EB, move(make_unique<CANPacket>(0x3EB, dashData, pk_3EB)));
			
	vector<shared_ptr<DataField> > pk_477 = {
			make_shared<DataField>("Limiter", 1, 0, 0, 2, -1, 15000, "RPM"),
			};
	bus.add_packet(0x477, move(make_unique<CANPacket>(0x477, dashData, pk_477)));

	bus.start_can();
	
	thread producer(&CANBus::listen, &bus, dashData);
	thread consumer(app_boot_up, dashData);
	
	producer.join();
	consumer.join();
	return 0;
}


