
#include <gtkmm.h>

class DashApp : public Gtk::Window {
	public:
		DashApp();
		Gtk::Window* get_main_window();
		
	protected:
		bool update_ui();
		
		Glib::RefPtr<Gtk::Builder> builder;
		Gtk::Window *MainWindow;
		Gtk::Label *GearPosLabel;
		Gtk::LevelBar *RevCounterBar;
	};
