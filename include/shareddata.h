#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include "datafield.h"
#include <memory>
#include <shared_mutex>
#include <vector>

class SharedData{
	private:
		std::vector<std::shared_ptr<DataField> > datapoints; 
		mutable std::shared_mutex data_mutex;
	
	public:
		void add_point(std::shared_ptr<DataField> new_field);
		void update_data(std::vector<std::shared_ptr<DataField> > n_datapoints);
		int get_dps_size();
		std::vector<std::shared_ptr<DataField> > get_points();
	};

#endif
