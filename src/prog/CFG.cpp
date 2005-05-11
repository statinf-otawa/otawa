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
 * Constructor. Add a property to the basic block for quick retrieval of
 * the matching CFG.
 */
CFG::CFG(Code *code, BasicBlock *entry)
: ent(entry), _code(code), flags(0),
_entry(BasicBlock::FLAG_Entry), _exit(BasicBlock::FLAG_Exit) {
	assert(code && entry);
	
	// Mark entry
	ent->set<CFG *>(ID, this);
	
	// Get label
	Option<String> label = entry->get<String>(File::ID_Label);
	if(label)
		set<String>(File::ID_Label, *label);
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
	return ent->get<String>(File::ID_Label, "");
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
	
	// Add entry edge
	new Edge(&_entry, ent, EDGE_Virtual);
	
	// Explore CFG
	genstruct::Vector<BasicBlock *> ends;
	_bbs.add(&_entry);
	_entry.add<int>(ID_Index, 0);
	for(int pos = 0; pos < _bbs.length(); pos++) {
		BasicBlock *bb = _bbs[pos];
		if(bb->isReturn())
			ends.add(bb);
		for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++)
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
	for(int i = 0; i < ends.length(); i++) {
		new Edge(ends[i], &_exit, EDGE_Virtual);
	}
	_exit.add<int>(ID_Index, _bbs.length());
	_bbs.add(&_exit);
	flags |= FLAG_Scanned;
	
	// All entering edges becomes calls
	for(Iterator<Edge *> edge(ent->inEdges()); edge; edge++)
		if(!_bbs.contains(edge->source()))
			edge->toCall();
}


/**
 * @class CFG::BBIterator
 * This iterator is used for visiting all basic blocks of the CFG.
 */


/**
 * @fn CFG::BBIterator::BBIterator(const CFG *cfg);
 * Build a basic block iterator.
 * @param cfg	Used CFG.
 */


/**
 * @see elm::Iterator::ended()
 */
/*bool CFG::BBIterator::ended(void) const {
	return pos >= bbs.length();
}*/


/**
 * @see elm::Iterator::item()
 */
/*BasicBlock *CFG::BBIterator::item(void) const {
	return bbs[pos];
}*/


/**
 * @see elm::Iterator::next()
 */
/*void CFG::BBIterator::next(void) {
	BasicBlock *bb = bbs[pos];
	for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++)
		if(edge->kind() != EDGE_Call
		&& !edge->target()->isExit()
		&& !bbs.contains(edge->target()))
			bbs.add(edge->target());
	pos++;
}*/

} // namespace otawa
