
#include "shareddata.h"

#include <mutex>
#include <iostream>


/* Class for managing a thread safe data structure keeping a vector
 * of pointers to DataField objects
 */
 
 
SharedData::SharedData(){
	ui_run.store(true);
	}
 
void SharedData::add_point(std::shared_ptr<DataField> new_field){
	std::unique_lock<std::shared_mutex> lock(data_mutex);
	//std::cout << "DF PTR was " << sizeof(new_field) << " bytes" << std::endl;
	datapoints.push_back(new_field);
}
		
/*Implements double buffering for data renewal
 * The producer (in this case the CAN bus) prepares a new vector
 * of points (updating only those which are modified). This is 
 * then swapped with the existing set minimising lock time
 */
void SharedData::update_data(std::vector<std::shared_ptr<DataField> > n_datapoints){
	std::unique_lock<std::shared_mutex> lock(data_mutex);
	datapoints.swap(n_datapoints);
}

int SharedData::get_dps_size(){
	return datapoints.size();
}

std::vector<std::shared_ptr<DataField> > SharedData::get_points(){
	std::shared_lock<std::shared_mutex> lock(data_mutex);
	//std::cout << "Data vector occupied " << sizeof(datapoints) << " bytes" << std::endl;
	return datapoints;
}
