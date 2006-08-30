#include "SequenceBasicBlock.h"

using namespace elm::genstruct;

namespace otawa { namespace ipet {

SequenceBasicBlock::SequenceBasicBlock(Vector<BasicBlock*> *src)
: path(src->length()) {
	int l = src->length();
	for(int i = 0; i < l; i++)
		path.add(src->get(i));
}

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

BBPath* SequenceBasicBlock::getBBPath(){
	return BBPath::getBBPath(&path);
}

int SequenceBasicBlock::countInstructions() const{
	int count = 0;
	int l = path.length();
	for(int i = 0; i < l; i++)
		count += path[i]->countInstructions();
	return count;
}


} } // otawa::ipet
