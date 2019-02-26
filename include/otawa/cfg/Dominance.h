/*
 *	Dominance class interface
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
#ifndef OTAWA_CFG_DOMINANCE_H
#define OTAWA_CFG_DOMINANCE_H

#include <otawa/proc/ConcurrentCFGProcessor.h>
#include <otawa/proc/Feature.h>
#include <otawa/cfg/features.h>

namespace otawa {

// External
class BasicBlock;
class Edge;
namespace dfa { class BitSet; }

// Dominance class
class Dominance: public ConcurrentCFGProcessor, public DomInfo {
public:

	static p::declare reg;
	Dominance(void);

	bool dom(Block *b1, Block *b2) override;
	Block* idom(Block* b) override;
	bool isBackEdge(Edge *edge) override;

	void *interfaceFor(const AbstractFeature& feature) override;

	static void ensure(CFG *cfg);
	static bool dominates(Block *bb1, Block *bb2);
	static inline bool isDominated(Block *bb1, Block *bb2) { return dominates(bb2, bb1); }
	static bool isLoopHeader(Block *bb);
	//static bool isBackEdge(Edge *edge);

protected:
	void processCFG(WorkSpace *fw, CFG *cfg) override;
	void cleanup(WorkSpace *ws) override;

private:
	void markLoopHeaders(CFG *cfg);
};

// Features
extern Identifier<const dfa::BitSet *> REVERSE_DOM;

} // otawa

#endif // OTAWA_CFG_DOMINANCE_H
