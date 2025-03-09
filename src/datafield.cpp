
#include "datafield.h"
#include <string>


/* Data field class which stores values against their title whilst also
 * holding relevant unit, limits and raw data scaling information
 */
DataField::DataField(std::string n_title, double n_gain, double n_offset, int n_bytes, std::string n_unit){
	title  = n_title;
	gain   = n_gain;
	offset = n_offset;
	bytes  = n_bytes;
	unit   = n_unit;
}
		
void DataField::update_raw(int raw_val){
	value = raw_val * gain + offset;
}



