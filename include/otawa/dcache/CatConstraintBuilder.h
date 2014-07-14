/*
 *	dcache::CatConstraintBuilder class interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_DCACHE_CATCONSTRAINTBUILDER_H_
#define OTAWA_DCACHE_CATCONSTRAINTBUILDER_H_

#include <otawa/proc/Processor.h>
#include <otawa/prop/Identifier.h>


namespace otawa {

namespace ilp { class Var; }

namespace dcache {

class CatConstraintBuilder : public otawa::Processor {
public:
	static p::declare reg;
	CatConstraintBuilder(p::declare& r = reg);
protected:
	virtual void processWorkSpace(otawa::WorkSpace *ws);
	virtual void configure(const PropList& props);
	void collectStats(WorkSpace *ws);
private:
	bool _explicit;
};

} }		// otawa::dcache

#endif /* OTAWA_DCACHE_CATCONSTRAINTBUILDER_H_ */
