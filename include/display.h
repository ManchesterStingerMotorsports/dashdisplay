#ifndef DISPLAY_H
#define DISPLAY_H

#include "shareddata.h"
#include <gtkmm.h>
#include <memory>


class DashApp : public Gtk::Window {
	public:
		DashApp(std::shared_ptr<SharedData> n_shared_data);
		Gtk::Window* get_main_window();
		
	protected:
		bool update_ui();
		
		std::shared_ptr<SharedData> shared_data;
		Glib::RefPtr<Gtk::Builder> builder;
		Gtk::Window *MainWindow;
		Gtk::Label *GearPosLabel;
		Gtk::LevelBar *RevCounterBar;
	};
	
#endif
