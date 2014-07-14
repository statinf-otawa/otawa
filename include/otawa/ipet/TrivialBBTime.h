/*
 *	TrivialBBTime processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
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
#ifndef OTAWA_IPET_TRIVIALBBTIME_H
#define OTAWA_IPET_TRIVIALBBTIME_H

#include <elm/assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {

// TrivialBBTime class
class TrivialBBTime: public BBProcessor {
public:
	static p::declare reg;
	TrivialBBTime(p::declare& r = reg);

protected:
	virtual void configure(const PropList& props);
	void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);

private:
	unsigned dep;
};

// Configuration Properties
extern Identifier<unsigned> PIPELINE_DEPTH;

} } // otawa::ipet

#endif // OTAWA_IPET_TRIVIALBBTIME_H
