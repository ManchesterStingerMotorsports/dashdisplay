
#include <gtkmm.h>

class DashApp : public Gtk::Window {
	public:
		DashApp();
		
	protected:
		bool update_ui();
	
		Gtk::Label ValueLabel00;
	};
