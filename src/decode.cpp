
#include <linux/can.h>
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


class SharedData{
	private:
		vector<shared_ptr<DataField> > datapoints; 
		mutable shared_mutex data_mutex;
	
	public:
		void add_point(shared_ptr<DataField> new_field){
			unique_lock<shared_mutex> lock(data_mutex);
			datapoints.push_back(new_field);
		}
		
		void update_data(vector<shared_ptr<DataField> > n_datapoints){
			unique_lock<shared_mutex> lock(data_mutex);
			datapoints.swap(n_datapoints);
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
		vector<shared_ptr<DataField> > contents;
		shared_ptr<SharedData> data;
	
		CANPacket(int n_packet_id, shared_ptr<SharedData> n_data, vector<shared_ptr<DataField> > n_contents){
			packet_id = n_packet_id;
			contents  = n_contents;
			data      = n_data;
			for (shared_ptr<DataField> dp : n_contents){
				data->add_point(dp);
			}
		}
		
		void update_dps(__u8 can_data[8]){
			vector<shared_ptr<DataField> > new_data;
			vector<shared_ptr<DataField> > old_data = data->get_points();
			for (shared_ptr<DataField> df : old_data){
				if(contents.count
			} 
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


void dummy_display(shared_ptr<SharedData> dashData){
	/*
	while(true){
		vector<shared_ptr<DataField> > datapoints = dashData->get_points();
		
		system("clear");
		for (shared_ptr<DataField> dp : datapoints){
			cout << dp->title << ": " << dp->value << endl;
		}
		
		this_thread::sleep_for(chrono::milliseconds(200));
	}
	*/
}



int main(){
	
	system("sudo ip link set can0 up type can bitrate 1000000 \
		&& sudo ip link set can1 up type can bitrate 1000000");
	
	shared_ptr<SharedData> dashData = make_shared<SharedData>();
	
	vector<shared_ptr<DataField> > pk_360 = {
			make_shared<DataField>("RPM", 1, 0, 2, "RPM"),
			make_shared<DataField>("MAP", 0.1, 0, 2, "kPa"),
			make_shared<DataField>("Throttle Pos.", 0.1, 0, 2, "%")
			};
	
	
	CANBus bus;
	bus.start_can();
	//bus.dump_packets(50);
	bus.add_packet(0x360, move(make_unique<CANPacket>(0x360, dashData, pk_360)));
	
	thread producer(&CANBus::listen, &bus, dashData);
	thread consumer(dummy_display, dashData);
	
	producer.join();
	consumer.join();
	return 0;
	}
