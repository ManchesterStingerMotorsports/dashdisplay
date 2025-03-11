#ifndef DISPLAY_H
#define DISPLAY_H

#include "shareddata.h"
#include <gtkmm.h>
#include <memory>
#include <map>


class DashApp : public Gtk::Window {
	public:
		DashApp(std::shared_ptr<SharedData> n_shared_data);
		Gtk::Window* get_main_window();
		
	protected:
		bool update_ui();
		
		void init_ui();
		
		std::shared_ptr<SharedData> shared_data;
		std::map<Gtk::Label*, int> value_layout_map; 
		
		int gear_index;
		int rpm_index;
		
		Glib::RefPtr<Gtk::Builder> builder;
		Gtk::Window *MainWindow;
		Gtk::Label *GearPosLabel;
		Gtk::LevelBar *RevCounterBar;
		
		Gtk::Label *Value00;
		Gtk::Label *Value01;
		Gtk::Label *Value02;
		Gtk::Label *Value03;
		Gtk::Label *Value10;
		Gtk::Label *Value11;
		Gtk::Label *Value12;
		Gtk::Label *Value13;
		Gtk::Label *Value20;
		Gtk::Label *Value21;
		Gtk::Label *Value22;
		Gtk::Label *Value23;
		
	};
	
#endif
