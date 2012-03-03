/*
 *	$Id$
 *	BranchBuilder processor interface
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
#ifndef BRANCH_BRANCHBUILDER_H_
#define BRANCH_BRANCHBUILDER_H_

#include <otawa/cfg/features.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/cfg/BasicBlock.h>

#include "BranchProblem.h"

namespace otawa { namespace branch {
	
using namespace elm;

// features
typedef enum category_t {
	ALWAYS_D,
	ALWAYS_H,
	FIRST_UNKNOWN,
	NOT_CLASSIFIED
} category_t;

extern Identifier<category_t> CATEGORY;
extern Identifier<BasicBlock*> HEADER;
extern SilentFeature CATEGORY_FEATURE;

// processor
class BranchBuilder : public otawa::Processor {
public:
	static proc::declare reg;
	BranchBuilder(void);

protected:
	virtual void processWorkSpace(otawa::WorkSpace*);
	
private:
	void categorize(BasicBlock *bb, BranchProblem::Domain *dom, BasicBlock* &header, category_t &cat);
};

} }		// otawa::branch

#endif
