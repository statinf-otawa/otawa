/*
 *	EventCollector class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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
#ifndef OTAWA_ETIME_EVENTCOLLECTOR_H_
#define OTAWA_ETIME_EVENTCOLLECTOR_H_

#include <otawa/etime/features.h>

namespace otawa { namespace etime {

/**
 * Collects variables linking events with blocks in ILP.
 */
class EventCollector {
public:

	EventCollector(Event *event): imprec(0), evt(event) { }
	typedef enum {
		PREFIX_OFF = 0,
		PREFIX_ON = 1,
		BLOCK_OFF = 2,
		BLOCK_ON = 3,
		SIZE = 4
	} case_t;

	inline Event *event(void) const { return evt; }

	void contribute(case_t c, ilp::Var *var);
	void make(ilp::System *sys);

private:
	inline bool isOn(case_t c);

	t::uint32 imprec;
	Event *evt;
	List<ilp::Var *> vars[SIZE];
};

} }		// otawa::etime

#endif /* OTAWA_ETIME_EVENTCOLLECTOR_H_ */
