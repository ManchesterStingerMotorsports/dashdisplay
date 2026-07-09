
#include "canpacket.h"
#include <iostream>

/* Structure defining the data field objects associated with a packet
 * Separate from the individual CAN struct which contain specific,
 * individual frames
 */

/* Packets maintain a vector of indexes to the DataField pointer
 * in the SharedData object relating to the dfs in this packet
 */
CANPacket::CANPacket(int n_packet_id, std::shared_ptr<SharedData> n_data, std::vector<std::shared_ptr<DataField> > n_contents){
	packet_id = n_packet_id;
	data      = n_data;
	//std::cout << std::hex << n_packet_id << " created successfully" << std::endl;
	for (std::shared_ptr<DataField> dp : n_contents){
		data->add_point(dp);
		contents_idxs.push_back(data->get_dps_size() - 1);
	}
}

void CANPacket::update_dps(__u8 can_data[8]){
	std::vector<std::shared_ptr<DataField> > new_data;
	std::vector<std::shared_ptr<DataField> > old_data = data->get_points();
	for (size_t i=0; i<old_data.size(); i++){
		new_data.push_back(old_data.at(i));
	}
	for (int i : contents_idxs){
		std::shared_ptr<DataField> updated_field = std::make_shared<DataField>(*old_data.at(i));
		
		int nd = 0;
		if(updated_field->bit_index >= 0){
			nd = (can_data[updated_field->start_byte] >> updated_field->bit_index) & 0x01;
		}
		else{
			// Big-endian decode
			for (int b=0; b < updated_field->length_bytes; b++){
				nd = (nd << 8) | can_data[updated_field->start_byte + b];
			}
		}
		
		updated_field->update_raw(nd);
		new_data.at(i) = updated_field;
	}
	
	data->update_data(new_data);
}

