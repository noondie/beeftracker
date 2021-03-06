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

#ifndef ENVELOPE_POINT_EDITOR_H
#define ENVELOPE_POINT_EDITOR_H

#include "common/os/mutex_lock.h"
#include <qwidget.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QEvent>
#include "common/components/audio/envelope.h"
#include <vector>
	using std::vector;

/**
 *
 * Juan Linietsky
 **/
class Envelope_Point_Editor : public QWidget {

Q_OBJECT

	enum {

		MIN_DRAG_DISTANCE=8 // min drag distance
	};

	float zoom;
	int font_height_spacing;


	void paintEvent( QPaintEvent * p_event );

	Envelope *envelope;

	struct Grab {

		bool moving_point;
		int node;
		int x,y;
	} grab;

	bool adjust_zoom_to_window();
	void paint_envelope_points(QPainter &painter);
	bool event ( QEvent * e );
	void move_mouse_to_selected_node();

	bool recalc_pending;

	QFont display_font;

	vector<bool> position_cache;
	vector<bool> old_position_cache;

	void draw_envelope_pos(int p_x,QPainter &painter);
        Mutex_Lock * envelope_lock;

public:
signals:

	void envelopeChanged();

public:

        void set_envelope_lock(Mutex_Lock * p_envelope_lock) { envelope_lock=p_envelope_lock; };

	void update_position_list(const vector<int>& p_positions);

	void set_envelope(Envelope *p_envelope);
	Envelope_Point_Editor(QWidget *p_parent);

	~Envelope_Point_Editor();



};

#endif
