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

#ifndef VARIABLES_EDIT_H
#define VARIABLES_EDIT_H


#include "trackercore/player_data.h"
#include <QHBoxLayout>
#include <QGroupBox>
#include "interface__QT/helpers/property_bridge_edit.h"
#include "trackercore/song.h"
/**
 *
 * Juan Linietsky
 **/
class Variables_Edit : public QHBoxLayout {

	Q_OBJECT


	struct Variables {

		Q3VBox *vbox;

		Q3GroupBox *speed_group;
		Q_Property_Bridge_Int_CSpinButon * prop_tempo;
		Q_Property_Bridge_Int_CSpinButon * prop_speed;

		Q3GroupBox *hilight_group;

		Q_Property_Bridge_Int_CSpinButon * prop_hilight_major;
		Q_Property_Bridge_Int_CSpinButon * prop_hilight_minor;


		Q3GroupBox *modifiers_group;

		Q_Property_Bridge_Int_CSpinButon * prop_global_volume;
		Q_Property_Bridge_Int_CSpinButon * prop_mixing_volume;
		Q_Property_Bridge_Int_CSpinButon * prop_separation;


		Q3GroupBox *compatibility_group;

		Q_Property_Bridge_Bool * prop_old_effects;
		Q_Property_Bridge_Bool * prop_compatible_gxx;

		Q_Property_Bridge_Bool * prop_stereo;
		Q_Property_Bridge_Bool * prop_linear_slides;

		Q3GroupBox *message_group;

		Q_Property_Bridge_String *prop_name;
		Q_Property_Bridge_String_Multiline *prop_message;

	} variables;

        Song *song;
	Player_Data *player;




public slots:


signals:

	void update_instrument_list_needed();

public:

	void set_player(Player_Data *p_player) { player=p_player; }
	void set_song(Song *p_song);

	Variables_Edit(QWidget *p_parent);
	~Variables_Edit();
};

#endif
