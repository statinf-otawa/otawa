/*
 *	StandardEventBuilder class interface
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
#ifndef OTAWA_ETIME_STANDARDEVENTBUILDER_CPP_
#define OTAWA_ETIME_STANDARDEVENTBUILDER_CPP_

#include <otawa/proc/BBProcessor.h>
#include <otawa/branch/features.h>
#include "features.h"

namespace otawa {

// pre-declarations
namespace hard {
	class Bank;
	class BHT;
	class CacheConfiguration;
	class Memory;
}

namespace etime {

class StandardEventBuilder: public BBProcessor {
public:
	static p::declare reg;
	StandardEventBuilder(p::declare& r = reg);

protected:
	virtual void configure(const PropList& props);
	virtual void setup(WorkSpace *ws);
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);

private:
	void handleVariableBranchPred(BasicBlock *bb, BasicBlock *wbb);
	ot::time costOf(Address addr, bool write = false);

	const hard::Memory *mem;
	const hard::CacheConfiguration *cconf;
	const hard::BHT *bht;
	bool has_il1, has_dl1;
	bool has_branch;
	const hard::Bank *bank;
	bool _explicit;
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_STANDARDEVENTBUILDER_CPP_ */
