/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	SequenceBasicBlock.cpp -- SequenceBasicBlock class implementation.
 */
#include "SequenceBasicBlock.h"

using namespace elm::genstruct;

namespace otawa { namespace ipet {

/**
 * @author G. Cavaignac
 * @class SequenceBasicBlock
 * This class is a sequence of basic blocks.
 * It extends the class BasicBlock, in order to be used in the standard
 * processors, and in the aggregation graph
 */

/**
 * Constructs a new sequence from a Vector
 */
SequenceBasicBlock::SequenceBasicBlock(Vector<BasicBlock*> *src)
: path(src->length()) {
	int l = src->length();
	for(int i = 0; i < l; i++)
		path.add(src->get(i));
}

/**
 * Constructs a new sequence from a Collection
 */
SequenceBasicBlock::SequenceBasicBlock(Collection<BasicBlock*> *src)
: path(src->count()) {
	IteratorInst<BasicBlock*> *iter;
	for(iter = src->visit(); !iter->ended(); iter->next())
		path.add(iter->item());
	delete iter;
	BasicBlock *first_bb = path[0];
	_head = first_bb->head();
	if(path.length() == 1){
		BasicBlock *bb = path[0];
		flags = bb->getFlags();
	}
}

/**
 * Returns the BBPath corresponding to this sequence.
 */
BBPath* SequenceBasicBlock::getBBPath(){
	return BBPath::getBBPath(&path);
}

/**
 * BasicBlock::countInstructions() overload
 */
int SequenceBasicBlock::countInstructions() const{
	int count = 0;
	int l = path.length();
	for(int i = 0; i < l; i++)
		count += path[i]->countInstructions();
	return count;
}


} } // otawa::ipet
