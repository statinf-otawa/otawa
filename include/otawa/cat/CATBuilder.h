/*
 *	CATBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
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
#ifndef OTAWA_CAT_CATBUILDER_H
#define OTAWA_CAT_CATBUILDER_H

#include <elm/assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// Extern classes
namespace dfa { class BitSet; }

class CFG;
class ContextTree;
namespace cache {
	class LBlock;
	class LBlockSet;
}	// cache

namespace cat {

// CATBuilder class
class CATBuilder: public Processor {
public:
	static Identifier<bool> NON_CONFLICT;
	static p::declare reg;
	CATBuilder(p::declare& r = reg);
	virtual void processWorkSpace(WorkSpace *fw );
private:
	dfa::BitSet *buildLBLOCKSET(cache::LBlockSet *lcache, ContextTree *root);
	void processLBlockSet(WorkSpace *fw, cache::LBlockSet *lbset);
	void setCATEGORISATION(cache::LBlockSet *lineset, ContextTree *S, int dec);
	void worst(cache::LBlock *line, ContextTree *S, cache::LBlockSet *cacheline, int dec);
};

// Properties
extern Identifier<otawa::BasicBlock *> LOWERED_CATEGORY;

} }		// otawa::cat

#endif // OTAWA_CAT_CATBUILDER_H
