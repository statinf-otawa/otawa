/*
 *	ConsBuilder processor interface
 *	Copyright (c) 2011, IRIT UPS.
 *
 *	This file is part of OTAWA
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
#ifndef BRANCH_CONSBUILDER_H_
#define BRANCH_CONSBUILDER_H_

#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/features.h>
#include <otawa/hard/BHT.h>
#include <otawa/ilp/expr.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/prog/WorkSpace.h>

#include "features.h"
#include "BranchBuilder.h"

namespace otawa { namespace branch {
	
using namespace elm;

// OnlyConsBuilder processor
class OnlyConsBuilder: public BBProcessor {
public:
	static p::declare reg;
	OnlyConsBuilder(p::declare& r = reg);
	virtual void configure(const PropList &props);
protected:
	virtual void setup(WorkSpace *ws);
	virtual void processBB(otawa::WorkSpace*, CFG *cfg, Block *bb);
private:
	void genAlwaysDefault(ilp::model& model, ilp::var x_mp, BasicBlock *bb);
	void genAlwaysHit(ilp::model& model, ilp::var x_mp, BasicBlock *bb);
	void genFirstUnknown(ilp::model& model, ilp::var x_mp, BasicBlock *bb);
	void genNotClassified(ilp::model& model, ilp::var x_mp, BasicBlock *bb);

	bool _explicit;
	hard::BHT *bht;
};

// ConsBuilder processor
class ConsBuilder: public BBProcessor {
public:
	static p::declare reg;
	ConsBuilder(p::declare& r = reg);
	virtual void processBB(otawa::WorkSpace*, CFG *cfg, Block *bb);
};

} }		// otawa::branch

#endif
