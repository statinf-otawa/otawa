/*
 *	BasicGraphBBTime interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2012 IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_BASICGRAPHBBTIME_H_
#define OTAWA_BASICGRAPHBBTIME_H_

#include <otawa/parexegraph/GraphBBTime.h>
#include <otawa/parexegraph/ParExeGraph.h>

namespace otawa {

class BasicGraphBBTime: public GraphBBTime<ParExeGraph> {
public:
	static p::declare reg;
	BasicGraphBBTime(AbstractRegistration& r = reg);
};

}	// otawa

#endif /* OTAWA_BASICGRAPHBBTIME_H_ */
