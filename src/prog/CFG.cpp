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

namespace otawa {

/**
 * @class CFG
 * Control Flow Graph representation. Its entry basic block is given and
 * the graph is built using following taken and not-taken properties of the block.
 */

/**
 * Identifier used for storing and retrieving the CFG on its entry BB.
 */
id_t CFG::ID = Property::getID("otawa.CFG");

/**
 * Constructor. Add a property to the basic block for quick retrieval of
 * the matching CFG.
 */
CFG::CFG(Code *code, BasicBlock *entry): ent(entry), _code(code) {
	assert(code && entry);
	ent->set<CFG *>(ID, this);
	Option<String> label = entry->get<String>(File::ID_Label);
	if(label)
		set<String>(File::ID_Label, *label);
}


/**
 * @fn BasicBlock *CFG::entry(void) const;
 * Get the entry basic block of the CFG.
 * @return Entry basic block.
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
IteratorInst<BasicBlock *> *CFG::visit(void) const {
	return new BBIterator(this);
}


/**
 * @see elm::Collection::empty()
 */
MutableCollection<BasicBlock *> *CFG::empty(void) const {
	return 0;
}


/**
 * @class CFG::BBIterator
 * This iterator is used for visiting all basic blocks of the CFG.
 */


/**
 * @fn CFG::BBIterator::BBIterator(CFG *cfg);
 * Build a basic block iterator.
 * @param cfg	Used CFG.
 */


/**
 * @see elm::Iterator::ended()
 */
bool CFG::BBIterator::ended(void) const {
	return pos >= bbs.length();
}


/**
 * @see elm::Iterator::item()
 */
BasicBlock *CFG::BBIterator::item(void) const {
	return bbs[pos];
}


/**
 * @see elm::Iterator::next()
 */
void CFG::BBIterator::next(void) {
	BasicBlock *bb = bbs[pos];
	for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++)
		if(!bbs.contains(edge->target()))
			bbs.add(edge->target());
	pos++;
}

} // namespace otawa
