/***************************************************************************
    This file is part of the CheeseTronic Music Tools
    url                  : http://reduz.com.ar/cheesetronic
    copyright            : (C) 2003 by Juan Linietsky
    email                : coding@reduz.com.ar
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//
//
// C++ Implementation: cpp
//
// Description:
//
//
// Author: Juan Linietsky <coding@reduz.com.ar>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "components/midi/midi_client_manager.h"
#include "interface.h"
//#include "interface__QT/icons/icon_audio_config.xpm"
#include "interface__QT/icons/icon_envelope.xpm"
#include "interface__QT/icons/icon_patterns.xpm"
#include "interface__QT/icons/icon_sample.xpm"
#include "interface__QT/icons/icon_tempo.xpm"
#include "interface__QT/icons/icon_variables.xpm"
#include "interface__QT/icons/icon_vumeter.xpm"

#include "components/audio/sound_driver_manager.h"

#include <cctype>
#include <QFileDialog>
#include <qmessagebox.h>
#include <QKeyEvent>
#include <QPixmap>
#include <QEvent>


Saver::Error  Interface::save_song()
{

	FILE *f=fopen(tracker.song.variables.filename.c_str(),"rb");
	if (!f) { //file doesnt exist, cant do direct save

		return save_song_as();
	}

        if (tracker.format_manager.save_module(tracker.song.variables.filename.c_str())) {

		QMessageBox::warning( this, "Oops!","Error saving song!","Ok");
		return Saver::SAVE_ERROR;
	}

	return Saver::SAVE_OK;

}

Saver::Error  Interface::save_song_as()
{

	QString s=QString::null;

#ifdef CYGWIN_ENABLED

	s = Q3FileDialog::getOpenFileName( QString::null, "Module Files (*.ct *.it)", this );


#else
	Q3FileDialog* fd = new Q3FileDialog( this, "Save Song", TRUE );
	fd->setMode( Q3FileDialog::AnyFile );
	fd->setFilter( "Cheese Tracker (*.ct)" );
	fd->addFilter( "Impulse Tracker 2.14 (*.it)" );
	fd->addFilter( "Songs (*.ct *.it)" );

	fd->setDir(tracker.song.variables.filename.c_str());

	if ( fd->exec() == QDialog::Accepted )
		s = fd->selectedFile();

#endif
	delete fd;



	if (s==QString::null)
		return Saver::SAVE_OK; //nothing selected



	FILE *f=fopen(s.ascii(),"rb");
	if (f) { //file exists
		fclose(f);
		if ( QMessageBox::warning( this, "Question:","File exists! Overwrite?","Yes", "No") ) {

//			//printf("Leaving file alone!\n");
			return Saver::SAVE_OK;
		}
	}

        if (tracker.format_manager.save_module(s.ascii())) {

		QMessageBox::warning( this, "Oops!","Error saving song!","Ok");
		return Saver::SAVE_ERROR;
	}

	tracker.song.variables.filename=s.ascii();

	return Saver::SAVE_OK;
}

string Interface::get_song_name()
{

	return tracker.song.variables.name;
}
Interface::PageList Interface::get_current_page()
{
	return (PageList)currentPageIndex();
}


void Interface::update_pattern_editor()
{
	pattern_editor->reupdate_components();
}

Tracker_Instance * Interface::get_tracker()
{
	return &tracker;
}

void Interface::play_song()
{
	tracker.player.play_start_song();
}

void Interface::stop_song()
{
	tracker.player.play_stop();
	tracker.rt_keyboard.instrument_stop_all();
	tracker.rt_keyboard.sample_stop_all();
}
void Interface::play_pattern()
{
	tracker.player.play_start_pattern(tracker.editor.get_current_pattern());
}

void Interface::play_pattern_from_cursor()
{
	tracker.player.play_start_pattern(tracker.editor.get_current_pattern(),tracker.editor.get_cursor_y());
}

void Interface::play_from_mark()
{
	tracker.player.play_start(tracker.editor.get_marked_pattern(),-1,tracker.editor.get_marked_row());
}
void Interface::play_from_order()
{
	tracker.player.play_start_song_from_order(tracker.editor.orderlist_get_cursor_y());
}

void Interface::play_from_cursor()
{
	// Bugfix Sun Mar 25 04:56:51 EDT 2007
	//
	// Play from the cursor on the Order & Defaults (F11)
	// page when that is the page being viewed by the
	// user.

	if(get_current_page() == Page__Order_And_Defaults) {
		play_from_order();
		return;
	}

	// Otherwise, play from the first order where the pattern
	// being shown in the Pattern Editor (F2) is used.

	int from_order;

	from_order=tracker.song.find_pattern_in_orderlist(tracker.editor.get_current_pattern());

	if (from_order!=-1) {

		tracker.player.play_start(-1,from_order,tracker.editor.get_cursor_y());

	} else {

		tracker.player.play_start(tracker.editor.get_current_pattern(),-1,tracker.editor.get_cursor_y());
	}
}


void Interface::keyReleaseEvent ( QKeyEvent * e )
{
        switch (currentPageIndex()) {

		case Page__Instrument_Editor: {

			if (e->isAutoRepeat())
				break;
			//printf("UNPRESS!\n");

			int key=e->ascii();
			Uint8 note=tracker.editor.get_note_from_key(key);
			if(note != EMPTY_FIELD) {
				tracker.rt_keyboard.instrument_set(tracker.editor.get_instrument_mask());
				tracker.rt_keyboard.instrument_stop_key(note);
			}
		} break;
/*
		case Page__Sample_Editor: {

			if (e->isAutoRepeat())
				break;
			//printf("UNPRESS!\n");

			int key=e->ascii();
			if ((key>='a') && (key<='z')) key-='a'-'A';

			if ( ((key>='A') && (key<='Z')) || ((key>='0') && (key<='9')) ) {
				Uint8 note=tracker.editor.get_note_from_key(key);
				tracker.rt_keyboard.sample_stop_key(note);
			}
		} break;*/
	};

	QTabWidget::keyReleaseEvent(e);

}

//stupid stupid Qt. DONT EAT THE KEYPRESSES DAMN

bool Interface::eventFilter( QObject *o, QEvent *e )
{
	if (e->type()!=QEvent::KeyPress) {

		return QTabWidget::eventFilter(o,e);
	}

	if ( (o==instrument_editor->instrument_list) || (o==sample_editor->sample_list) )  {

		QKeyEvent *k=(QKeyEvent*)e;
		keyPressEvent(k);
		return (tracker.editor.get_note_from_key(k->ascii()) != EMPTY_FIELD);

	}

	return QTabWidget::eventFilter(o,e);
}


void Interface::keyPressEvent ( QKeyEvent * e )
{
	if(e->state() & (Qt::AltButton|Qt::ShiftButton|Qt::ControlButton)) {
		return;
	}

	switch (e->key()) {

		case Qt::Key_F2: {

                	setCurrentPage(Page__Pattern_Editor);
			pattern_editor->focus_pattern_edit_widget();
		} break;
		case Qt::Key_F3: {

                	setCurrentPage(Page__Sample_Editor);
		} break;
		case Qt::Key_F4: {

                	setCurrentPage(Page__Instrument_Editor);
		} break;
		case Qt::Key_F11: {

                	setCurrentPage(Page__Order_And_Defaults);
		} break;
		case Qt::Key_F12: {

                	setCurrentPage(Page__Variables);
		} break;
		case Qt::Key_Asterisk : {

			Editor::set_default_octave(Editor::get_default_octave()+1);
		} break;
		case Qt::Key_Slash : {
			Editor::set_default_octave(Editor::get_default_octave()-1);
		} break;
	}

        switch (currentPageIndex()) {

		case Page__Instrument_Editor: {


			if (e->isAutoRepeat())
				break;

			//printf("PRESS !\n");
			int key=e->ascii();
			Uint8 note=tracker.editor.get_note_from_key(key);
			if(note != EMPTY_FIELD) {
				tracker.rt_keyboard.instrument_set(tracker.editor.get_instrument_mask_value());
				tracker.rt_keyboard.instrument_press_key(note,64);
			}
		} break;
		case Page__Sample_Editor: {


			if (e->isAutoRepeat())
				break;
			int key=e->ascii();
			Uint8 note=tracker.editor.get_note_from_key(key);
			if(note != EMPTY_FIELD) {
				Sample_Data * sample=tracker.song.get_sample( sample_editor->get_selected_sample_index() )?&tracker.song.get_sample( sample_editor->get_selected_sample_index() )->data:NULL;
				if (sample==NULL) break;

				tracker.rt_keyboard.sample_stop_key(note);
				tracker.rt_keyboard.sample_set(sample);
				tracker.rt_keyboard.sample_press_key(note);
				//printf("starting note %i\n",note);
			}
		} break;
	};

	QTabWidget::keyPressEvent(e);
}


int Interface::widget_timer_interval=20; //20 miliseconds?
static const int midi_timer_interval=10; //20 miliseconds?
Int_Property_Bridge Interface::timer_interval_bridge("Repaint Interval (msecs):",&widget_timer_interval,20,1000);

void Interface::midi_check_timer()
{
	MIDI_Client_Manager *mcm=MIDI_Client_Manager::get_singleton_instance();
	if (!mcm)
		return;

	MIDI_Event event;

	while (mcm->pop_midi_event(&event)) {


		if ((event.type!=MIDI_Event::NOTE_ON) && (event.type!=MIDI_Event::NOTE_OFF))
			continue;

		switch (currentPageIndex()) {

			case Page__Pattern_Editor: {

				if ((event.type!=MIDI_Event::NOTE_ON) || (event.data.note.note>=Note::NOTES))
					break;


				int previous_cursor_x=tracker.editor.get_cursor_x();
				int previous_cursor_y=tracker.editor.get_cursor_y();
				tracker.editor.insert_note_value_at_cursor(event.data.note.note);
				tracker.player.play_note(previous_cursor_x,tracker.song.get_pattern(tracker.editor.get_current_pattern())->get_note(previous_cursor_x,previous_cursor_y));
				pattern_editor->get_pattern_edit()->update();

			} break;
			case Page__Instrument_Editor: {

				if (event.type==MIDI_Event::NOTE_ON) {
					tracker.rt_keyboard.instrument_set(instrument_editor->get_selected_instrument());
					tracker.rt_keyboard.instrument_press_key(event.data.note.note,(int)event.data.note.velocity*64/127);
				} else {
					tracker.rt_keyboard.instrument_set(instrument_editor->get_selected_instrument());
					tracker.rt_keyboard.instrument_stop_key(event.data.note.note);
				}
			} break;

			case Page__Sample_Editor: {

				if (event.type==MIDI_Event::NOTE_ON) {
					Sample_Data * sample=tracker.song.get_sample( sample_editor->get_selected_sample_index() )?&tracker.song.get_sample( sample_editor->get_selected_sample_index() )->data:NULL;
					if (sample==NULL) break;

					tracker.rt_keyboard.sample_stop_key(event.data.note.note);
					tracker.rt_keyboard.sample_set(sample);
					tracker.rt_keyboard.sample_press_key(event.data.note.note);

				} else {

					//tracker.rt_keyboard.sample_stop_key(event.data.note.note);
				}
			} break;
		};

	}

}

void Interface::update_song_widgets()
{
	pattern_editor->configure(&tracker.song,&tracker.editor,&tracker.player);
	sample_editor->set_song(&tracker.song);
	instrument_editor->set_song(&tracker.song);
	instrument_editor->set_editor(&tracker.editor);
	order_and_defaults_editor->set_song(&tracker.song);
	order_and_defaults_editor->set_editor(&tracker.editor);
	variables_editor->set_song(&tracker.song);

	instrument_editor->set_selected_instrument(0);
	sample_editor->set_selected_sample(0);
	sample_editor->update_selected_sample();
	buffers_editor->update();
}

Loader::Error Interface::open_song(string p_name)
{
	Sound_Driver_Manager::get_singleton_instance()->get_variables_lock()->grab(__FILE__, __LINE__);
	Loader::Error res=tracker.format_manager.load_module(p_name.c_str());
	Sound_Driver_Manager::get_singleton_instance()->get_variables_lock()->release();
	tracker.song.variables.filename=p_name;
	
	

	if (!res) { //load success!
                update_song_widgets();
	}

	return res;
}

void Interface::selected_sample_in_editor(int p_which)
{
	instrument_editor->set_selected_instrument(tracker.song.find_sample_in_instrument(p_which));
}
void Interface::selected_instrument_in_pattern(int p_which)
{
	instrument_editor->set_selected_instrument(p_which);
}

void Interface::update_sample_editor_request()
{
	sample_editor->update_samples();
	sample_editor->update_selected_sample();
}

void Interface::widget_update_timer()
{
	if (last_timer_interval!=widget_timer_interval) {
		//just in case the timer changed interval
		timer->changeInterval(widget_timer_interval);
		last_timer_interval=widget_timer_interval;
	}

	tracker.player.get_voice_status_info(voice_status_info);

	pattern_editor->timer_callback();
	sample_editor->timer_callback();
	instrument_editor->timer_callback();
	order_and_defaults_editor->timer_callback();

	if (tracker.player.get_play_mode()!=Player_Data::PLAY_NOTHING) {
		pattern_editor->get_pattern_edit()->update_info_areas(tracker.player.get_current_row(),tracker.player.get_current_pattern());
		order_and_defaults_editor->get_orderlist()->set_current_playing_order( tracker.player.get_current_order() );

	} else {
		pattern_editor->get_pattern_edit()->update_info_areas(-1,-1);
		order_and_defaults_editor->get_orderlist()->set_current_playing_order( -1 );
	}

}
void Interface::update_instrument_mode_callback()
{
	instrument_editor->update();
}

void Interface::play_next_order()
{
	tracker.player.goto_next_order();
}
void Interface::play_previous_order()
{
	tracker.player.goto_previous_order();
}

Interface::Interface(QWidget *p_widget) : QTabWidget(p_widget)
{
	pattern_editor = new  Pattern_Edit_Widget(this);
	addTab(pattern_editor,QPixmap((const char**)icon_patterns_xpm) ,"Patterns");
	pattern_editor ->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	QObject::connect( pattern_editor, SIGNAL(instrument_changed(int)), this, SLOT(selected_instrument_in_pattern(int)) );


	sample_editor = new Sample_Edit(this);
	addTab(sample_editor,QPixmap((const char**)icon_sample_xpm),"Samples");
	sample_editor->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	QObject::connect( sample_editor, SIGNAL(selected_sample_signal(int)), this, SLOT(selected_sample_in_editor(int)) );

	instrument_editor = new Instrument_Edit(this);
	addTab(instrument_editor,QPixmap((const char**)icon_envelope_xpm),"Instruments");
	instrument_editor ->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

	order_and_defaults_editor = new Order_And_Defaults_Editor(this);
	addTab(order_and_defaults_editor,QPixmap((const char**)icon_variables_xpm),"Order && Defaults");
	order_and_defaults_editor->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));


	variables_editor = new Variables_Edit(this);

	addTab(variables_editor,QPixmap((const char**)icon_tempo_xpm),"Variables");
	variables_editor->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        buffers_editor = new Mixer_Effects_Manager(this);
	buffers_editor->set_mixer(&tracker.mixer);
	addTab(buffers_editor,QPixmap((const char**)icon_vumeter_xpm),"Buffers");

	timer=new QTimer(this);
	QObject::connect( timer, SIGNAL(timeout()), this, SLOT(widget_update_timer()) );
	timer->start(widget_timer_interval,false);
	last_timer_interval=widget_timer_interval;

	timer_midi=new QTimer(this);
	QObject::connect( timer_midi, SIGNAL(timeout()), this, SLOT(midi_check_timer()) );
	timer_midi->start(midi_timer_interval,false);

	tracker.song.variables.name="New Song";
	update_song_widgets();


	instrument_editor->instrument_list->installEventFilter(this);
	sample_editor->sample_list->installEventFilter(this);
	tracker.player.reset(); //needs to be reset
	tracker.player.play_stop(); //needs to be reset
	tracker.rt_keyboard.instrument_stop_all();
	tracker.rt_keyboard.sample_stop_all();
	tracker.player.reset(); //needs to be reset
	tracker.player.play_stop(); //needs to be reset

	sample_editor->set_voice_status_info(&voice_status_info);
	sample_editor->set_rt_keyboard(&tracker.rt_keyboard);
	sample_editor->set_player(&tracker.player);
	sample_editor->set_file_format_manager(&tracker.format_manager);

	instrument_editor->set_voice_status_info(&voice_status_info);
	instrument_editor->set_player(&tracker.player);
	instrument_editor->set_file_format_manager(&tracker.format_manager);

	variables_editor->set_player(&tracker.player);

	QObject::connect(instrument_editor,SIGNAL(update_sample_editor()),this,SLOT(update_sample_editor_request()));
	QObject::connect(variables_editor,SIGNAL(update_instrument_list_needed()),this,SLOT(update_instrument_mode_callback()));


}

Interface::~Interface()
{
}
