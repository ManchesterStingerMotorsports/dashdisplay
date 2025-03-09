



#include <gtkmm.h>
#include <memory>



class DashApp : public Gtk::Window {
	public:
		DashApp(shared_ptr<SharedData> n_shared_data);
		Gtk::Window* get_main_window();
		
	protected:
		bool update_ui();
		
		shared_ptr<SharedData> shared_data;
		Glib::RefPtr<Gtk::Builder> builder;
		Gtk::Window *MainWindow;
		Gtk::Label *GearPosLabel;
		Gtk::LevelBar *RevCounterBar;
	};
