#ifndef CANBUS_H
#define CANBUS_H

#include "canpacket.h"
#include "shareddata.h"

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <map>
#include <memory>

class CANBus{
	public:
		std::map<int, std::unique_ptr<CANPacket> > packet_lut;
		int sock;
		bool sock_init = false;
		struct sockaddr_can addr;
		struct ifreq ifr;
		
		CANBus();
		~CANBus();
		int start_can();
		int dump_packets(int quantity);
		int listen(std::shared_ptr<SharedData> shared_data);
		void add_packet(int packet_id, std::unique_ptr<CANPacket> pkt_ptr);
	};

#endif
