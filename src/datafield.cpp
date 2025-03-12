
#include "datafield.h"
#include <string>


/* Data field class which stores values against their title whilst also
 * holding relevant unit, limits and raw data scaling information
 */
DataField::DataField(std::string n_title, double n_gain, double n_offset, int n_start, int n_bytes, std::string n_unit){
	title  = n_title;
	gain   = n_gain;
	length_bytes  = n_bytes;
	start_byte = n_start;
	unit   = n_unit;
	value  = 0;
}
		
void DataField::update_raw(int raw_val){
	value = raw_val * gain + offset;
}



