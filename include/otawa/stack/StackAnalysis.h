/*
 *	$Id$
 *	StackAnalysis process interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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

#ifndef OTAWA_STACK_STACKANALYSIS_H_
#define OTAWA_STACK_STACKANALYSIS_H_

#include <otawa/stack/features.h>
#include <otawa/proc/Processor.h>

namespace otawa {

namespace hard { class Register; }

// StackAnalysis class
class StackAnalysis: public Processor {
public:
	typedef Pair<const hard::Register *, Address> init_t;
	static Identifier<init_t> INITIAL;

	static p::declare reg;
	StackAnalysis(p::declare& r = reg);
	virtual void configure(const PropList &props);

protected:
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	Vector<init_t> inits;
};

}	// otawa

#endif /* OTAWA_STACK_STACKANALYSIS_H_ */
