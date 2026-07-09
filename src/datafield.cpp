
#include "datafield.h"
#include <string>
#include <iostream>


/* Data field class which stores values against their title whilst also
 * holding relevant unit, limits and raw data scaling information
 */
DataField::DataField(std::string n_title, double n_gain, double n_offset, int n_start, int n_bytes, double n_low, double n_upp, std::string n_unit, int n_bit_index){
	title  = n_title;
	gain   = n_gain;
	offset = n_offset;
	start_byte = n_start;
	length_bytes  = n_bytes;
	bit_index = n_bit_index;
	low_lim = n_low;
	upp_lim = n_upp;
	unit   = n_unit;
	value  = 0;
	//std::cout << "Initialised DF: " << title << std::endl;
}
		
void DataField::update_raw(int raw_val){
	value = raw_val * gain + offset;
}



