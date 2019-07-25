/*
 *	$Id$
 *	StatInfo class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef OTAWA_STATS_STATINFO_H_
#define OTAWA_STATS_STATINFO_H_

#include <otawa/prop/Identifier.h>
#include <elm/data/Vector.h>
#include "StatCollector.h"

namespace otawa {

class WorkSpace;

class StatInfo {
public:
	static Identifier<StatInfo *> ID;

	static void add(WorkSpace *ws, StatCollector& stats);
	static void remove(WorkSpace *ws, StatCollector& stats);

	~StatInfo(void);

	class Iter: public Vector<StatCollector *>::Iter {
	public:
		Iter(WorkSpace *ws);
	};

	static const Vector<StatCollector *>& get(WorkSpace *ws);

	private:
	Vector<StatCollector *> stats;
};

}	// otawa

#endif /* OTAWA_STATS_STATINFO_H_ */
