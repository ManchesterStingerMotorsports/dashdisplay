#ifndef DATAFIELD_H
#define DATAFIELD_H

#include <string>


class DataField{
	public:
		std::string title;
		double value;
		double gain;
		double offset;
		double low_lim;
		double upp_lim;
		int    bytes;
		std::string unit;
		
		DataField(std::string n_title, double n_gain, double n_offset, int n_bytes, std::string n_unit);
		void update_raw(int raw_val);
		
	};

#endif
