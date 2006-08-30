#ifndef OTAWA_IPET_LOOPHEADER_H
#define OTAWA_IPET_LOOPHEADER_H

#include <otawa/cfg/BasicBlock.h>
#include <iostream>

namespace otawa { namespace ipet {

class LoopHeader: public EndBasicBlock {
	BasicBlock *_child;
public:
	LoopHeader(BasicBlock *bb);
	inline BasicBlock* child(void){return _child;}
	virtual inline IteratorInst<Inst *> *visit(void);
	virtual int countInstructions(void) const {return 0;}
	
private:
	// Instructions Iterator
	class InstIterator: PreIterator<InstIterator, Inst*>{
	public:
		inline InstIterator(void){}
		inline bool ended(void) const {return true;}
		inline Inst *item(void) const {return 0;}
		inline void next(void){}
	};
};

inline IteratorInst<Inst*> *LoopHeader::visit(void){
	InstIterator iter;
	return new IteratorObject<InstIterator, Inst *>(iter);;
}

} } // otawa::ipet

#endif /*OTAWA_IPET_LOOPHEADER_H*/
