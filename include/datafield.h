#ifndef DATAFIELD_H
#define DATAFIELD_H

#include <string>


class DataField{
	public:
		std::string title;
		double value;
		double gain;
		double offset;
		int low_lim;
		int upp_lim;
		int start_byte;
		int length_bytes;
		std::string unit;
		
		DataField(std::string n_title, double n_gain, double n_offset, int n_start, int n_bytes, int n_low, int n_upp, std::string n_unit);
		void update_raw(int raw_val);
		
	};

#endif
