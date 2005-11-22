/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	cfg.cpp -- control flow graph classes implementation.
 */

#include <assert.h>
#include <elm/debug.h>
#include <elm/Collection.h>
#include <otawa/cfg.h>
#include <elm/debug.h>
#include <otawa/util/Dominance.h>
#include <otawa/util/DFABitSet.h>

namespace otawa {

/**
 * @class CFG
 * Control Flow Graph representation. Its entry basic block is given and
 * the graph is built using following taken and not-taken properties of the block.
 */


/**
 * Identifier used for storing and retrieving the CFG on its entry BB.
 */
id_t CFG::ID = Property::getID("otawa.cfg");


/**
 * Identifier used for storing in each basic block from the CFG its index (int).
 */
Identifier CFG::ID_Index("otawa.cfg.index");


/**
 * Identifier for marking the CFG as having the dominance relation computed.
 * Takes a boolean value that is not really used.
 */
Identifier CFG::ID_Dom("otawa.cfg.dom");


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
	DFABitSet *set = bb2->get<DFABitSet *>(Dominance::ID_RevDom, 0);
	if(!set) {
		Dominance dom;
		dom.processCFG(0, this);
		set = bb2->use<DFABitSet *>(Dominance::ID_RevDom);
	}
	
	// Test with the index
	return set->contains(bb1->use<int>(ID_Index));
}


/**
 * Constructor. Add a property to the basic block for quick retrieval of
 * the matching CFG.
 */
CFG::CFG(CodeItem *code, BasicBlock *entry)
: ent(entry), _code(code), flags(0),
_entry(BasicBlock::FLAG_Entry), _exit(BasicBlock::FLAG_Exit) {
	assert(code && entry);
	
	// Mark entry
	ent->set<CFG *>(ID, this);
	
	// Get label
	BasicBlock::InstIterator inst(entry);
	String label = inst->get<String>(File::ID_FunctionLabel, "");
	if(label)
		set<String>(File::ID_Label, label);
}


/**
 * Build an empty CFG.
 */
CFG::CFG(void): _code(0), ent(0), _entry(BasicBlock::FLAG_Entry),
_exit(BasicBlock::FLAG_Exit), flags(0) {
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
	return get<String>(File::ID_Label, "");
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
	
	// All entering edges becomes calls
	for(BasicBlock::InIterator edge(ent); edge; edge++)
		edge->toCall();
	new Edge(&_entry, ent, EDGE_Virtual);
	
	// Explore CFG
	genstruct::Vector<BasicBlock *> ends;
	_bbs.add(&_entry);
	_entry.add<int>(ID_Index, 0);
	for(int pos = 0; pos < _bbs.length(); pos++) {
		BasicBlock *bb = _bbs[pos];
		if(bb->isReturn())
			ends.add(bb);
		for(BasicBlock::OutIterator edge(bb); edge; edge++)
			if(edge->kind() != EDGE_Call) {
				BasicBlock *target = edge->target();
				if(target != ent && target->get<CFG *>(ID, 0)) {
					ends.add(bb);
					edge->toCall();
				}
				else if(!_bbs.contains(target)) {
					target->add<int>(ID_Index, _bbs.length());
					_bbs.add(target);
				}
			}
	}
	
	// Add exit edges
	_exit.add<int>(ID_Index, _bbs.length());
	_bbs.add(&_exit);
	for(int i = 0; i < ends.length(); i++)
		new Edge(ends[i], &_exit, EDGE_Virtual);
	flags |= FLAG_Scanned;
}


/**
 * Number the basic block of the CFG, that is, hook a property with ID_Index
 * identifier and the integer value of the number to each basic block. The
 * entry get the number 0 et the exit the last number.
 */
void CFG::numberBB(void) {
	for(int i = 0; i < _bbs.length(); i++)
		_bbs[i]->set<int>(ID_Index, i);
}

} // namespace otawa
