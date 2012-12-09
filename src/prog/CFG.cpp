/*
 *	$Id$
 *	CFG class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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

#include <assert.h>
#include <elm/debug.h>
#include <otawa/cfg.h>
#include <elm/debug.h>
#include <otawa/util/Dominance.h>
#include <otawa/dfa/BitSet.h>
#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/VectorQueue.h>

using namespace elm::genstruct;

namespace otawa {

/**
 * Identifier used for storing and retrieving the CFG on its entry BB.
 */
Identifier<CFG *> ENTRY("otawa::ENTRY", 0);


/**
 * Identifier used for storing in each basic block from the CFG its index.
 * Also used for storing each CFG's index.
 *
 * @par Hooks
 * @li @ref BasicBlock
 * @li @ref CFG
 */
Identifier<int> INDEX("otawa::INDEX", -1);


/**
 * @class CFG
 * Control Flow Graph representation. Its entry basic block is given and
 * the graph is built using following taken and not-taken properties of the block.
 */


/**
 * Format the display of the given address relativelt to the given CFG.
 * @param addr	Address to format.
 * @return		Formatted address.
 */
string CFG::format(const Address& addr) {
	string lab = LABEL(this);
	if(!lab)
		return _ << addr;
	else {
		t::int32 off = addr - address();
		if(off >= 0)
			return _ << lab << " + 0x" << io::hex(off);
		else
			return _ << lab << " - 0x" << io::hex(-off);
	}
}


/**
 * Test if the first BB dominates the second one.
 * @param bb1	Dominator BB.
 * @param bb2	Dominated BB.
 * @return		True if bb1 dominates bb2.
 */
/*bool CFG::dominates(BasicBlock *bb1, BasicBlock *bb2) {
	assert(bb1);
	assert(bb2);

	// Look for reverse-dominating annotation
	dfa::BitSet *set = REVERSE_DOM(bb2);
	if(!set) {
		Dominance dom;
		dom.processCFG(0, this);
		set = REVERSE_DOM(bb2);
	}

	// Test with the index
	return set->contains(INDEX(bb1));
}*/


/**
 * Constructor. Add a property to the basic block for quick retrieval of
 * the matching CFG.
 */
CFG::CFG(Segment *seg, BasicBlock *entry):
	flags(0),
	_entry(BasicBlock::FLAG_Entry),
	_exit(BasicBlock::FLAG_Exit),
	_seg(seg),
	ent(entry)
{
	assert(seg && entry);

	// Get label
	BasicBlock::InstIter inst(entry);
	String label = FUNCTION_LABEL(inst);
	if(label)
		LABEL(this) = label;
}


/**
 * Build an empty CFG.
 */
CFG::CFG(void):
	 flags(0),
	_entry(BasicBlock::FLAG_Entry),
	_exit(BasicBlock::FLAG_Exit),
	_seg(0),
	ent(0)
{
}


/**
 * @fn BasicBlock *CFG::entry(void);
 * Get the entry basic block of the CFG.
 * @return Entry basic block.
 */


/**
 * @fn BasicBlock *CFG::exit(void);
 * Get the exit basic block of the CFG.
 * @return Exit basic block.
 */


/**
 * @fn Code *CFG::code(void) const;
 * Get the code containing the CFG.
 * @return Container code.
 */


/**
 * Get some label to identify the CFG.
 * @return	CFG label or any other identification way.
 */
String CFG::label(void) {
	if(!ent) {
		BasicBlock::OutIterator out(entry());
		if(!out)
			return "";
		ent = out->target();
	}
	string id = LABEL(this);
	if(!id) {
		Inst *first = ent->firstInst();
		if(first) {
			id = FUNCTION_LABEL(first);
			if(!id)
				id = LABEL(first);
		}
		if(!id)
			id = _ << "__0x" << ent->address();
	}
	return id;
}


/**
 * Get the address of the first instruction of the CFG.
 * @return	Return address of the first instruction.
 */
Address CFG::address(void) {
	if(!ent) {
		BasicBlock::OutIterator edge(entry());
		if(edge)
			ent = edge->target();
		if(!ent)
			return Address::null;
	}
	return ent->address();
}


/**
 * @fn elm::Collection<BasicBlock *>& CFG::bbs(void);
 * Get an iterator on basic blocks of the CFG.
 * @return	Basic block iterator.
 */


/**
 * Scan the CFG for finding exit and builds virtual edges with entry and exit.
 * For memory-place and time purposes, this method is only called when the CFG
 * is used (call to an accessors method).
 */
void CFG::scan(void) {

	// Prepare data
	typedef HashTable<BasicBlock *, BasicBlock *> map_t;
	map_t map;
	VectorQueue<BasicBlock *> todo;
	todo.put(ent);

	// Find all BB
	_bbs.add(&_entry);
	while(todo) {
		BasicBlock *bb = todo.get();
		ASSERT(bb);

		// second case : calling jump to a function
		if(map.exists(bb) || (bb != ent && ENTRY(bb)))
			continue;

		// !!TODO!! Add a CFGChecker !
		// unknown target ?
		/*if(bb->isTargetUnknown()) {
			Inst *last = 0;
			for(BasicBlock::InstIter inst(bb); inst; inst++)
				last = inst;
			throw otawa::Exception(_ << "can not build CFG of " << label() << " because "
				"there is an instruction with unknown target at " << format(last->address()));
		}*/

		// build the virtual BB
		BasicBlock *vbb = new VirtualBasicBlock(bb);
		_bbs.add(vbb);
		map.put(bb, vbb);
		ASSERTP(map.exists(bb), "not for " << bb->address());

		// resolve targets
		for(BasicBlock::OutIterator edge(bb); edge; edge++) {
			ASSERT(edge->target());
			if(edge->kind() != Edge::CALL)
				todo.put(edge->target());
		}
	}

	// Relink the BB
	BasicBlock *vent = map.get(ent, 0);
	ASSERT(vent);
	new Edge(&_entry, vent, Edge::VIRTUAL);
	for(bbs_t::Iterator vbb(_bbs); vbb; vbb++) {
		if(vbb->isEnd())
			continue;
		BasicBlock *bb = ((VirtualBasicBlock *)*vbb)->bb();
		if(bb->isReturn())
			new Edge(vbb, &_exit, Edge::VIRTUAL);

		for(BasicBlock::OutIterator edge(bb); edge; edge++) {

			// A call
			if(edge->kind() == Edge::CALL) {
				Edge *vedge = new Edge(vbb, edge->target(), Edge::CALL);
				vedge->toCall();
			}

			// Pending edge
			else if(!edge->target()) {
				new Edge(vbb, 0, edge->kind());
			}

			// Possibly a not explicit call
			else {
				ASSERT(edge->target());
				BasicBlock *vtarget = map.get(edge->target(), 0);
				if(vtarget)	// simple branch
					new Edge(vbb, vtarget, edge->kind());
				else {		// calling jump to a function
					new Edge(vbb, edge->target(), Edge::CALL);
					vbb->flags |= BasicBlock::FLAG_Call;
					new Edge(vbb, &_exit, Edge::VIRTUAL);
				}
			}

		}
	}
	_bbs.add(&_exit);

	// Number the BB
	for(int i = 0; i < _bbs.length(); i++) {
		INDEX(_bbs[i]) = i;
		_bbs[i]->_cfg = this;
	}
	flags |= FLAG_Scanned;

}


/**
 * Number the basic block of the CFG, that is, hook a property with ID_Index
 * identifier and the integer value of the number to each basic block. The
 * entry get the number 0 et the exit the last number.
 */
void CFG::numberBB(void) {
	for(int i = 0; i < _bbs.length(); i++)
		INDEX(_bbs[i]) = i;
}


/**
 */
CFG::~CFG(void) {

	// remove edges
	for(int i = 0; i < _bbs.length() - 1; i++) {
		BasicBlock *bb = _bbs[i];
		while(true) {
			BasicBlock::OutIterator edge(bb);
			if(edge)
				delete *edge;
			else
				break;
		}
	}

	// remover basic blocks
	for(int i = 1; i < _bbs.length() - 1; i++)
		delete _bbs[i];
}


/**
 * Get the first basic block of the CFG.
 * @return	First basic block.
 */
BasicBlock *CFG::firstBB(void) {
	if(!(flags & FLAG_Scanned))
		scan();
	return _bbs[1];
}


/**
 * Get the first instruction of the CFG.
 * @return	First instruction of the CFG.
 */
Inst *CFG::firstInst(void) {
	if(!(flags & FLAG_Scanned))
		scan();
	BasicBlock *bb = firstBB();
	BasicBlock::InstIter inst(bb);
	return *inst;
}


/**
 * Print a reference for the CFG.
 * @param out	Output stream.
 */
void CFG::print(io::Output& out) {
	out << label();
}

} // namespace otawa
