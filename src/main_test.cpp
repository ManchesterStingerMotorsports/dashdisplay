

#include <linux/can.h>
#include <gtkmm.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <string>
#include <iostream>
#include <cstring>


using namespace std;

/* Data field class which stores values against their title whilst also
 * holding relevant unit, limits and raw data scaling information
 */
class DataField{
	public:
		string title;
		double value;
		double gain;
		double offset;
		double low_lim;
		double upp_lim;
		int    bytes;
		string unit;
		
		DataField(string n_title, double n_gain, double n_offset, int n_bytes, string n_unit){
			title  = n_title;
			gain   = n_gain;
			offset = n_offset;
			bytes  = n_bytes;
			unit   = n_unit;
		}
		
		void update_raw(int raw_val){
			value = raw_val * gain + offset;
		}
		
		void update_direct(double new_val){
			value = new_val;  
		}
};

/* Class for managing a thread safe data structure keeping a vector
 * of pointers to DataField objects
 */
class SharedData{
	private:
		vector<shared_ptr<DataField> > datapoints; 
		mutable shared_mutex data_mutex;
	
	public:
		void add_point(shared_ptr<DataField> new_field){
			unique_lock<shared_mutex> lock(data_mutex);
			datapoints.push_back(new_field);
		}
		
		/*Implements double buffering for data renewal
		 * The producer (in this case the CAN bus) prepares a new vector
		 * of points (replacting only those which are modified). This is 
		 * then swapped with the existing set minimising lock time
		 */
		void update_data(vector<shared_ptr<DataField> > n_datapoints){
			unique_lock<shared_mutex> lock(data_mutex);
			datapoints.swap(n_datapoints);
		}
		
		int get_dps_size(){
			return datapoints.size();
		}
		
		vector<shared_ptr<DataField> > get_points(){
			shared_lock<shared_mutex> lock(data_mutex);
			return datapoints;
		}
};


/* Structure defining the data field objects associated with a packet
 * Separate from the individual CAN struct which contain specific,
 * individual frames
 * The sequence that the packet contents is in DOES MATTER
 */
class CANPacket{
	public:
		int packet_id;
		vector<int> contents_idxs;
		shared_ptr<SharedData> data;
	
		/* Packets maintain a vector of indexes to the DataField pointer
		 * in the SharedData object relating to the dfs in this packet
		 */
		CANPacket(int n_packet_id, shared_ptr<SharedData> n_data, vector<shared_ptr<DataField> > n_contents){
			packet_id = n_packet_id;
			data      = n_data;
			for (shared_ptr<DataField> dp : n_contents){
				data->add_point(dp);
				contents_idxs.push_back(data->get_dps_size() - 1);
			}
		}
		
		void update_dps(__u8 can_data[8]){
			vector<shared_ptr<DataField> > new_data;
			vector<shared_ptr<DataField> > old_data = data->get_points();
			for (size_t i=0; i<old_data.size(); i++){
				new_data.push_back(old_data.at(i));
			}
			int array_consumed = 0;
			for (int i : contents_idxs){
				shared_ptr<DataField> updated_field = make_shared<DataField>(*old_data.at(i));
				int nd = 0;
				for (int b=0; b<updated_field->bytes; b++){
					nd = (nd << 8) | can_data[array_consumed + b];
				}
				updated_field->update_raw(nd);
				array_consumed += updated_field->bytes;
				new_data.at(i) = updated_field;
			}
			
			data->update_data(new_data);
		}
};


class CANBus{
	public:
		map<int, unique_ptr<CANPacket> > packet_lut;
		int sock;
		bool sock_init = false;
		struct sockaddr_can addr;
		struct ifreq ifr;
		CANBus(){
		}
		~CANBus(){
			close(sock);
		}
		
		int start_can(){
			// Create socket
			sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
			if(sock < 0){
				perror("Couldn't create CAN socket!");
				return 1;
			}
			
			// Establish parameters of the CAN interface we'd like to bind to the socket
			strcpy(ifr.ifr_name, "can1");
			if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0){
				perror("Couldn't open CAN interface using ioctl!");
				return 1;
			}
			addr.can_ifindex = ifr.ifr_ifindex;
			addr.can_family = AF_CAN;  
			
			// Bind the socket and linux CAN interface together
			bind(sock, (struct sockaddr *)&addr, sizeof(addr));
			
			sock_init = true;
			return 0;
		}
		
		
		// Utility Function to wait on and log a specified number of packets
		int dump_packets(int quantity){
			
			if(not sock_init){
				perror("Socket not initialised! Has start_can been run?");
				return 1;
			}
			for(int i=0; i<quantity; i++){
				struct can_frame frame;
				ssize_t nbytes = read(sock, &frame, sizeof(frame));
				
				if(nbytes < 0){
					perror("CAN raw socket read had no bytes");
					return 1;
				}  
				if(nbytes < (int)sizeof(struct can_frame)){
					perror("CAN raw socket read had incomplete frame");
					return 1;
				}
				
				
				cout << "ID: " << hex << frame.can_id << endl;
				cout << "Data: ";
				for (int i=0; i<frame.len; i++){
					cout << hex << (int)frame.data[i] << " ";
				}
				cout << endl;
			}
			return 0;
		}
		
		// Method for waiting on and filing packets into their appropriate data field objects
		int listen(shared_ptr<SharedData> shared_data){
			if(not sock_init){
				perror("Socket not initialised! Has start_can been run?");
				return 1;
			}
			
			while(true){
				struct can_frame frame;
				ssize_t nbytes = read(sock, &frame, sizeof(frame));
				
				if(nbytes < 0){
					perror("CAN raw socket read had no bytes");
					return 1;
				}  
				if(nbytes < (int)sizeof(struct can_frame)){
					perror("CAN raw socket read had incomplete frame");
					return 1;
				}
				
				// Check if the packet received is one we're looking for
				if (packet_lut.count((int)frame.can_id) > 0){
					packet_lut.at((int)frame.can_id)->update_dps(frame.data);
				}
			}
			
		}
		
		void add_packet(int packet_id, unique_ptr<CANPacket> pkt_ptr){
			packet_lut[packet_id] = move(pkt_ptr);
		}
};

class DashApp : public Gtk::Window {
	public:
		DashApp(shared_ptr<SharedData> n_shared_data);
		Gtk::Window* get_main_window();
		
	protected:
		bool update_ui();
		
		shared_ptr<SharedData> shared_data;
		Glib::RefPtr<Gtk::Builder> builder;
		Gtk::Window *MainWindow;
		Gtk::Label *GearPosLabel;
		Gtk::Label *Label00;
		Gtk::LevelBar *RevCounterBar;
	};


bool DashApp::update_ui(){
	vector<shared_ptr<DataField> > dps = shared_data->get_points();
	double rpm = dps.at(0)->value;
	RevCounterBar->set_value(rpm);
	Label00->set_text(to_string(rpm));
	return true;	
}

DashApp::DashApp(shared_ptr<SharedData> n_shared_data){
	builder = Gtk::Builder::create_from_file("dashUI.glade");
	builder->get_widget("MainWindow", MainWindow);
	builder->get_widget("GearPosLabel", GearPosLabel);
	builder->get_widget("RevCounterBar", RevCounterBar);
	builder->get_widget("01Value1", Label00);
	
	
	shared_data = n_shared_data;
	
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &DashApp::update_ui), 30);
	
	MainWindow->show_all();
}

Gtk::Window* DashApp::get_main_window(){ return MainWindow; }



int app_boot_up(shared_ptr<SharedData> shared_data){
auto app = Gtk::Application::create("com.formulastudentuom.dashdisplay");
DashApp window = DashApp(shared_data);

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
