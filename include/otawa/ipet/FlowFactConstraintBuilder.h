/*
 *	ipet::FlowFactConstraintBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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
#ifndef OTAWA_IPET_IPET_FLOW_FACT_CONSTRAINT_BUILDER_H
#define OTAWA_IPET_IPET_FLOW_FACT_CONSTRAINT_BUILDER_H

#include <otawa/proc/Feature.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa { 

using namespace elm;

// Externals
namespace ilp { class System; }

namespace ipet {
	
// FlowFactConstraintBuilder class
class FlowFactConstraintBuilder: public BBProcessor {
public:
	static p::declare reg;
	FlowFactConstraintBuilder(p::declare& r = reg);
	virtual void configure(const PropList& props = PropList::EMPTY);

protected:
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *bb);
	virtual void setup(WorkSpace *ws);
private:
	ilp::System *system;
	bool _explicit;
};

} } // otawa::ipet

#endif // OTAWA_IPET_IPET_FLOW_FACT_CONSTRAINT_BUILDER_H
