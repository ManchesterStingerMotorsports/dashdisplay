
#include "shareddata.h"
#include "dashboard_ui.h"

#include <csignal>
#include <memory>

#ifdef _WIN32
#include "preview_input.h"

#include <iostream>
#include <string_view>
#else
#include "datafield.h"
#include "canbus.h"
#include "canpacket.h"

#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>
#endif

using namespace std;

namespace {
	shared_ptr<SharedData> active_dash_data;

	void request_shutdown(int){
		if(active_dash_data){
			active_dash_data->ui_run = false;
		}
	}
}

#ifdef _WIN32
int main(int argc, char* argv[]){
#else
int main(){
#endif
	signal(SIGINT, request_shutdown);
	signal(SIGTERM, request_shutdown);

	shared_ptr<SharedData> dashData = make_shared<SharedData>();
	active_dash_data = dashData;

#ifdef _WIN32
	if(argc == 2 && std::string_view(argv[1]) == "--preview-input"){
		return run_preview_input(dashData);
	}
	if(argc != 1){
		cerr << "Usage: fsdash.exe [--preview-input]\n";
		return 1;
	}
#endif

#ifndef _WIN32
	system("ip link set can0 down 2>/dev/null; \
		ip link set can1 down 2>/dev/null; \
		ip link set can0 up type can bitrate 1000000 2>/dev/null; \
		ip link set can1 up type can bitrate 1000000 2>/dev/null");

	CANBus bus;
	
	// =============== Start of CAN Setup ====================
	
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
			make_shared<DataField>("Fuel Pres.", 0.0145, -14.7, 0, 2, 20, 50, "PSI"),
			make_shared<DataField>("Oil Pres.", 0.0145, -14.7, 2, 2, 5, 10, "PSI")
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
	
	vector<shared_ptr<DataField> > pk_469 = {
			make_shared<DataField>("ECU Temp.", 0.1, -273, 0, 2, 20, 50, "C"),
			};
	bus.add_packet(0x469, move(make_unique<CANPacket>(0x469, dashData, pk_469)));
	
	vector<shared_ptr<DataField> > pk_470 = {
			make_shared<DataField>("Gear", 1, 0, 7, 1, 0, 7, ""),
			};
	bus.add_packet(0x470, move(make_unique<CANPacket>(0x470, dashData, pk_470)));
			
	vector<shared_ptr<DataField> > pk_477 = {
			make_shared<DataField>("Limiter", 1, 0, 0, 2, -1, 15000, "RPM"),
			};
	bus.add_packet(0x477, move(make_unique<CANPacket>(0x477, dashData, pk_477)));

	vector<shared_ptr<DataField> > pk_3E4 = {
			make_shared<DataField>("Launch Control Active", 1, 0, 2, 1, 0, 1, "", 7),
			};
	bus.add_packet(0x3E4, move(make_unique<CANPacket>(0x3E4, dashData, pk_3E4)));
	
	// =============== End of CAN Setup ====================
	
	bool can_available = bus.start_can() == 0;
	unique_ptr<thread> producer;
	if(can_available){
		producer = make_unique<thread>(&CANBus::listen, &bus, dashData);
	}
	else{
		cerr << "CAN unavailable; running dashboard with default zero values." << endl;
	}
#endif

	int ui_result = run_dashboard_ui(dashData);

	dashData->ui_run = false;
#ifndef _WIN32
	if(producer != nullptr && producer->joinable()){
		producer->join();
	}
#endif
	return ui_result;
}


