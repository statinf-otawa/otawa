#ifndef OTAWA_IPET_SEQUENCEBASICBLOCK_H
#define OTAWA_IPET_SEQUENCEBASICBLOCK_H

#include <otawa/cfg.h>
#include <otawa/ipet/BBPath.h>
#include <elm/Collection.h>
#include <elm/genstruct/Vector.h>

namespace otawa { namespace ipet {

class SequenceBasicBlock : public BasicBlock{
protected:
	elm::genstruct::Vector<BasicBlock*> path;
public:
	SequenceBasicBlock(elm::genstruct::Vector<BasicBlock*> *src);
	SequenceBasicBlock(elm::Collection<BasicBlock*> *src);
	//address_t address() {return path[0]->address();}
	virtual int countInstructions(void) const;
	virtual inline IteratorInst<Inst*> *visit();
	//size_t size();
	//size_t getBlockSize();
	BBPath* getBBPath(void);
	
	// Instruction Iterator
	class InstIterator: PreIterator<InstIterator, Inst*>{
		IteratorInst<Inst*> *cur_iterator;
		elm::genstruct::Vector<BasicBlock*> &path;
		int next_bb;
		//Inst *inst;
	public:
		inline InstIterator(SequenceBasicBlock *sbb);
		inline ~InstIterator();
		inline bool ended(void) const;
		inline Inst *item(void) const;
		inline void next(void);
	};
};

inline IteratorInst<Inst*> *SequenceBasicBlock::visit(){
	InstIterator iter(this);
	return new IteratorObject<InstIterator, Inst *>(iter);
}

inline SequenceBasicBlock::InstIterator::InstIterator(SequenceBasicBlock *sbb)
: next_bb(1), path(sbb->path){
	assert(sbb);
	cur_iterator = path[0]->visit();
}

inline SequenceBasicBlock::InstIterator::~InstIterator(){
	//delete cur_iterator;
}

inline bool SequenceBasicBlock::InstIterator::ended(void) const {
	return next_bb >= path.length() && cur_iterator->ended();
}

inline Inst *SequenceBasicBlock::InstIterator::item(void) const {
	return cur_iterator->item();
}

inline void SequenceBasicBlock::InstIterator::next(void) {
//	if(!cur_iterator->ended()){
	cur_iterator->next();
	if(cur_iterator->ended() && next_bb < path.length()){
		delete cur_iterator;
		cur_iterator = path[next_bb++]->visit();
	}
//	}
}


} } // otawa::ipet

#endif /*OTAWA_IPET_SEQUENCEBASICBLOCK_H*/
