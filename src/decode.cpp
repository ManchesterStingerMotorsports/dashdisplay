
#include <iostream>
#include <cstring>
#include <string>
#include <linux/can.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <string>


using namespace std;

// Define a map of packet ids to the datafields expected to be in that packet
map<int, unique_ptr<DataField>> pkt_lut;


class DataField{
	public:
		string title;
		double value;
		double gain;
		double offset;
		double low_lim;
		double upp_lim;
		int    packet_id;
		int    byte_locs[2];
		string unit;
		DataField(string n_title, double n_gain, double n_offset){
			title = n_title;
			gain = n_gain;
			offset = n_offset;
		}
		void update_raw(int raw_val){
			value = raw_val * gain + offset;
		}
		
		void update_direct(double new_val){
			value = new_val;  
		}
};

class SharedData{
	private:
		vector<DataField> datapoints; 
		mutable mutex mtx;
	
	public:
		void add_point(DataField new_field){
			lock_guard<mutex> lock(mtx);
			datapoints.push_back(new_field);
		}
		
		void update_point(int val_index, int new_val){
			lock_guard<mutex> lock(mtx);
			
			datapoints.at(val_index).update_raw(new_val);
		}
		
		vector<DataField> get_points(){
			lock_guard<mutex> lock(mtx);
			return datapoints;
		}
};

class CANBus{
	public:
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
			std::strcpy(ifr.ifr_name, "can1");
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
				
				
				std::cout << "ID: " << std::hex << frame.can_id << std::endl;
				std::cout << "Data: ";
				for (int i=0; i<frame.len; i++){
					std::cout << std::hex << (int)frame.data[i] << " ";
				}
				std::cout << std::endl;
			}
			return 0;
		}
		
		// Method for waiting on and filing packets into their appropriate data field objects
		int listen(SharedData& dashData){
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
				
				
				// TODO: Update points dependent on packet id
				dashData.update_point(0, (int)frame.data[0]);
				dashData.update_point(1, (int)frame.data[2]);
			}
			
		}
};


void refresh_pkt_lut(SharedData& dashData){
	vector<DataField> points = dashData.get_points();
	pkt_lut.clear();
	for (DataField point : points){
		if (pkt_lut.count(point.packet_id) > 0){
			pkt_lut.at(point.packet_id).push_back();
		}
		else{
			vector<
			pkt_lut.insert({point.packet_id, {}});
		}
		
	}  
}



void dummy_display(SharedData& dashData){
	while(true){
		auto datapoints = dashData.get_points();
		
		system("clear");
		for (const auto& dp : datapoints){
			cout << dp.title << ": " << dp.value << endl;
		}
		
		this_thread::sleep_for(chrono::milliseconds(200));
	}
}



int main(){
	CANBus bus;
	bus.start_can();
	
	SharedData dashData;
	DataField d1 = DataField("RPM", 0.1, -101.3);
	DataField d2 = DataField("Coolant Temp", 0.1, -101.3);
	
	dashData.add_point(d1);
	dashData.add_point(d2);
	
	thread producer(&CANBus::listen, &bus, ref(dashData));
	thread consumer(dummy_display, ref(dashData));
	
	producer.join();
	consumer.join();
	
	return 0;
	}
