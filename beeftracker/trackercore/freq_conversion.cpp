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

#include "freq_conversion.h"


Uint32 get_period(Uint16 note,Sint32 speed,bool linear) {

	if (linear) {

		return Tables::get_linear_period(note,speed);
	} else {

		return Tables::get_old_period(note,speed);
	}
}


Uint32 get_frequency(Uint32 period, bool linear) {

	if (linear) {

		return Tables::get_linear_frequency(period);
	} else {

		return Tables::get_old_frequency(period);
	}
}


