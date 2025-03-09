#ifndef CANPACKET_H
#define CANPACKET_H


#include "shareddata.h"
#include <vector>
#include <memory>
#include <linux/types.h>

class CANPacket{
	public:
		int packet_id;
		std::vector<int> contents_idxs;
		std::shared_ptr<SharedData> data;
		
		CANPacket(int n_packet_id, std::shared_ptr<SharedData> n_data, std::vector<std::shared_ptr<DataField> > n_contents);
		void update_dps(__u8 can_data[8]);
	};

#endif
