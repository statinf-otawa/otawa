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

class DataAccessBuilder;
class FetchBuilder;

class StandardEventBuilder: public BBProcessor {
public:
	static p::declare reg;
	StandardEventBuilder(p::declare& r = reg);

protected:
	void configure(const PropList& props) override;
	void setup(WorkSpace *ws) override;
	void processBB(WorkSpace *ws, CFG *cfg, Block *bb) override;
	void cleanup(WorkSpace *ws) override;

private:
	void handleVariableBranchPred(BasicBlock *bb, Block *wbb);

	const hard::Memory *mem;
	const hard::CacheConfiguration *caches;
	const hard::BHT *bht;
	bool has_branch;
	bool _explicit;
	FetchBuilder *fetch;
	DataAccessBuilder *data;
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_STANDARDEVENTBUILDER_CPP_ */
