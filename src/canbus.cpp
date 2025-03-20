
#include "canbus.h"


#include <sys/socket.h>
#include <sys/ioctl.h>
#include <cstring>
#include <unistd.h>
#include <iostream>


CANBus::CANBus(){
}

CANBus::~CANBus(){
	close(sock);
}


int CANBus::start_can(){
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
int CANBus::dump_packets(int quantity){
	
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
int CANBus::listen(std::shared_ptr<SharedData> shared_data){
	if(not sock_init){
		perror("Socket not initialised! Has start_can been run?");
		return 1;
	}
	
	while(shared_data->ui_run){
		
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
	std::cout << "Finished listening" << std::endl;
	return 0;
}

void CANBus::add_packet(int packet_id, std::unique_ptr<CANPacket> pkt_ptr){
	packet_lut[packet_id] = std::move(pkt_ptr);
}
