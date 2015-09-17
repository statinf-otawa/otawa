#ifndef OTAWA_TEST_PCG_PCGBLOCK_H
#define OTAWA_TEST_PCG_PCGBLOCK_H

#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/CFG.h>
#include <elm/genstruct/Vector.h>
#include <elm/PreIterator.h>

namespace otawa {
class PCGBlock: public PropList {
	BasicBlock *bb;
	elm::genstruct::Vector<PCGBlock *> ins, outs;
	CFG* cfg;
public:
	inline PCGBlock(BasicBlock *bb,CFG* cfg):bb(bb),cfg(cfg){};
	virtual inline ~PCGBlock(void){};
	inline void addInLink(PCGBlock *pcgb) { ins.add(pcgb); };
	inline void addOutLink(PCGBlock *pcgb) { outs.add(pcgb); };
	inline BasicBlock* getBB(void){return bb;};
	inline address_t getAddress(){return cfg->address();};
	inline String getName(){return cfg->label();};
	inline elm::genstruct::Vector<PCGBlock*>& getFathers(){return ins;};
	inline elm::genstruct::Vector<PCGBlock*>& getSons(){return outs;};
	inline CFG *getCFG(void) const { return cfg; }
	
	class PCGBlockInIterator: public elm::PreIterator<PCGBlockInIterator, PCGBlock *> {
		elm::genstruct::Vector<PCGBlock *>& ins;
		int pos;
	public:
		inline PCGBlockInIterator(PCGBlock *pcgb): ins(pcgb->ins), pos(0){};
		inline bool ended(void) const {return pos>=ins.length();};
		inline PCGBlock *item(void) const {return ins[pos];};
		inline void next(void) {pos++;};
	};
	
	class PCGBlockOutIterator: public elm::PreIterator<PCGBlockOutIterator, PCGBlock *> {
		elm::genstruct::Vector<PCGBlock *>& outs;
		int pos;
	public:
		inline PCGBlockOutIterator(PCGBlock* pcgb):outs(pcgb->outs),pos(0){};
		inline bool ended(void) const {return pos>=outs.length();};
		inline PCGBlock *item(void) const {return outs[pos];};
		inline void next(void){pos++;};
	};
};

}

#endif
