

#include <string>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <memory>

class DataField{
	public:
		string title;
		double value;
		double gain;
		double offset;
		double low_lim;
		double upp_lim;
		int    bytes;
		string unit;
		
		DataField();
		void update_raw(int raw_val);
		
	};
	
class SharedData{
	private:
		vector<shared_ptr<DataField> > datapoints; 
		mutable shared_mutex data_mutex;
	
	public:
		void add_point(shared_ptr<DataField> new_field);
		void update_data(vector<shared_ptr<DataField> > n_datapoints);
		int get_dps_size();
		vector<shared_ptr<DataField> > get_points()
	};
	
class CANPacket{
	public:
		int packet_id;
		vector<int> contents_idxs;
		shared_ptr<SharedData> data;
		
		void update_dps(__u8 can_data[8]);
	};
	


