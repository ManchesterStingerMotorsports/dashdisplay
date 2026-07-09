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
		int start_byte;
		int length_bytes;
		int bit_index;
		std::string unit;
		
		DataField(std::string n_title, double n_gain, double n_offset, int n_start, int n_bytes, double n_low, double n_upp, std::string n_unit, int n_bit_index = -1);
		void update_raw(int raw_val);
		
	};

#endif
