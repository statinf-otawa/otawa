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
#include <elm/Collection.h>
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
Identifier<CFG *> ENTRY("otawa::entry", 0);


/**
 * Identifier used for storing in each basic block from the CFG its index.
 * Also used for storing each CFG's index. 
 *
 * @par Hooks
 * @li @ref BasicBlock
 * @li @ref CFG
 */
Identifier<int> INDEX("otawa::index", -1);


/**
 * @class CFG
 * Control Flow Graph representation. Its entry basic block is given and
 * the graph is built using following taken and not-taken properties of the block.
 */


/**
 * Test if the first BB dominates the second one.
 * @param bb1	Dominator BB.
 * @param bb2	Dominated BB.
 * @return		True if bb1 dominates bb2.
 */
bool CFG::dominates(BasicBlock *bb1, BasicBlock *bb2) {
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
}


/**
 * Constructor. Add a property to the basic block for quick retrieval of
 * the matching CFG.
 */
CFG::CFG(Segment *seg, BasicBlock *entry)
:	_seg(seg),
	ent(entry),
	flags(0),
	_entry(BasicBlock::FLAG_Entry),
	_exit(BasicBlock::FLAG_Exit)
{
	assert(seg && entry);
	
	// Mark entry
	ENTRY(ent) = this;
	
	// Get label
	BasicBlock::InstIterator inst(entry);
	String label = FUNCTION_LABEL(inst);
	if(label)
		LABEL(this) = label;
}


/**
 * Build an empty CFG.
 */
CFG::CFG(void):
	_seg(0),
	ent(0),
	 flags(0),
	_entry(BasicBlock::FLAG_Entry),
	_exit(BasicBlock::FLAG_Exit) {
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
 * Get the CFG name, that is, the label associated with the entry of the CFG.
 * @return	CFG label or an empty string.
 */
String CFG::label(void) {
	return LABEL(this);
}


/**
 * Get the address of the first instruction of the CFG.
 * @return	Return address of the first instruction.
 */
address_t CFG::address(void) {
	return ent->address();
}


/**
 * @see elm::datastruct::Collection::visit()
 */
IteratorInst<BasicBlock *> *CFG::visit(void) {
	BBIterator iter(this);
	return new elm::IteratorObject<BBIterator, BasicBlock *>(iter);
}


/**
 * @see elm::Collection::empty()
 */
MutableCollection<BasicBlock *> *CFG::empty(void) {
	return 0;
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
	//cerr << "begin CFG::scan()\n";
	
	// Experimental code

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
		BasicBlock *vbb = new VirtualBasicBlock(bb);
		_bbs.add(vbb);
		map.put(bb, vbb);
		ASSERTP(map.exists(bb), "not for " << bb->address());

		// !!DEBUG!!
		/*if(bb->address() == Address(0x8a98))
			cerr << "1 BB " << (void *)bb  << " at " << bb->address() << io::endl;*/
		
		for(BasicBlock::OutIterator edge(bb); edge; edge++) {

			// !!DEBUG!!
			/*cerr << edge->source()->address() << " => ";
			if(!edge->target())
				cerr << "unknown";
			else
				cerr << edge->target()->address();
			cerr << io::endl;*/
			
			if(edge->target() && edge->kind() != Edge::CALL)
				todo.put(edge->target());
		}
	}
	
	/*for(map_t::ItemIterator vbb(map); vbb; vbb++)
		cerr << (void *)vbb.key() << " -> " << (void *)vbb << io::endl;*/ 
	
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
		
		// !!DEBUG!!
		/*if(bb->address() == Address(0x8a98))
			cerr << "2 BB " << (void *)bb  << " at " << bb->address() << io::endl;*/
		
		for(BasicBlock::OutIterator edge(bb); edge; edge++) {
			
			// !!DEBUG!!
			/*cerr << edge->source()->address() << " -> ";
			if(!edge->target())
				cerr << "unknown";
			else
				cerr << edge->target()->address();
			cerr << io::endl;*/
			
			// A call
			if(edge->kind() == Edge::CALL) {
				Edge *vedge = new Edge(vbb, edge->target(), Edge::CALL);
				vedge->toCall();
				//cerr << vbb->address() << ": call" << io::endl;
			}
			
			// Pending edge
			else if(!edge->target()) {
				new Edge(vbb, 0, edge->kind());
				//cerr << vbb->address() << ": pending edge" << io::endl;
			}
			
			// Possibly a not explicit call
			else {
				ASSERT(edge->target());
				BasicBlock *vtarget = map.get(edge->target(), 0);
				if(vtarget)	{	// simple branch
					new Edge(vbb, vtarget, edge->kind());
					//cerr << vbb->address() << ": to " << vtarget->address() << io::endl;
				}
				else {		// calling jump to a function
					new Edge(vbb, edge->target(), Edge::CALL);
					//cerr << vbb->address() << ": calling jump" << io::endl;
					vbb->flags |= BasicBlock::FLAG_Call;
					new Edge(vbb, &_exit, Edge::VIRTUAL);
					//cerr << vbb->address() << ": exit" << io::endl;
				}
			}

		}	
	}
	_bbs.add(&_exit);
	
	
#	if 0
	// All entering edges becomes calls
	for(BasicBlock::InIterator edge(ent); edge; edge++)
		edge->toCall();		// !!BUG!!
	new Edge(&_entry, ent, EDGE_Virtual);
	
	// Explore CFG
	genstruct::Vector<BasicBlock *> ends;
	_bbs.add(&_entry);
	_entry._cfg = this;
	INDEX(_entry) = 0;
	for(int pos = 0; pos < _bbs.length(); pos++) {
		BasicBlock *bb = _bbs[pos];
		bb->_cfg = this;
		if(bb->isReturn())
			ends.add(bb);
		for(BasicBlock::OutIterator edge(bb); edge; edge++) {
			if(edge->kind() == EDGE_Call) {
				bool not_taken = false;
				for(BasicBlock::OutIterator edge(bb); edge; edge++)
					if(edge->kind() != Edge::CALL) {
						not_taken = true;
						break;
					}
				if(!not_taken)
					ends.add(bb);
			}
			else {
				BasicBlock *target = edge->target();
				if(target != ent && ENTRY(target)) {
					ends.add(bb);
					edge->toCall();
				}
				else if(!_bbs.contains(target)) {
					INDEX(target) = _bbs.length();
					_bbs.add(target);
				}
			}
		}
	}
	
	// Check for entering dead code
	Vector<Edge *> cut;
	for(int i = 0; i < _bbs.length(); i++) {
		BasicBlock *bb = _bbs[i];
		for(BasicBlock::InIterator edge(bb); edge; edge++)
			if(!_bbs.contains(edge->source()))
				cut.add(edge);
		for(int j = 0; j < cut.length(); j++)
			bb->removeInEdge(cut[j]);
		cut.clear();
	}
	
	// Add exit edges
	INDEX(_exit) = _bbs.length();
	_exit._cfg = this;
	_bbs.add(&_exit);
	for(int i = 0; i < ends.length(); i++) {
		new Edge(ends[i], &_exit, EDGE_Virtual);
	}
#	endif

	// Number the BB
	for(int i = 0; i < _bbs.length(); i++) {
		// !!DEBUG!!
		/*if(_bbs[i]->address() == Address(0x8a9c)) {
			BasicBlock *bb = _bbs[i];
			cerr << bb << io::endl;
		}*/
		INDEX(_bbs[i]) = i;
		_bbs[i]->_cfg = this;
	}
	flags |= FLAG_Scanned;
	//cerr << "end CFG::scan()\n";
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


// GenericIdentifier<CFG *>::print Specialization
template <>
void Identifier<CFG *>::print(elm::io::Output& out, const Property& prop) const {
	out << "cfg(" << get(prop)->label() << ")";
}


/**
 */
CFG::~CFG(void) {
	for(int i = 1; i < _bbs.length() - 1; i++)
		delete _bbs[i];
}

} // namespace otawa
