#ifndef OTAWA_TEST_PCG_PCG_H
#define OTAWA_TEST_PCG_PCG_H

#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/CFG.h>
#include <elm/genstruct/Vector.h>
#include <elm/PreIterator.h>

#include "PCGBlock.h"
namespace otawa {
	
using namespace elm::io;

class PCG: public PropList {
	CFG * cfg;
protected:	
	genstruct::Vector<PCGBlock *> _pcgbs;
public :
	
	PCG(CFG * cfg):cfg(cfg){};
	~PCG(){};
	//void PCG_Construction(BasicBlock* bb,CFG* cfg,PCGBuilder *PCGBD);
	//void afficherPCG(FrameWork *fw,PCGBuilder * PCGBD);

	class PCGIterator: public elm::PreIterator<PCGIterator, PCGBlock *> {
		elm::genstruct::Vector<PCGBlock *>& pcgbs;
		int pos;
	public:
		inline PCGIterator(PCG *pcg):pcgbs(pcg->_pcgbs),pos(0){};
		inline bool ended(void) const {return pos>=pcgbs.length();};
		inline PCGBlock *item(void) const {return pcgbs[pos];};
		inline void next(void) {pos++;};
	};
	void addPCGBlock(PCGBlock * pcgb){_pcgbs.add(pcgb);};
	int nbPCGBlocks(void){return _pcgbs.length();};
	elm::genstruct::Vector<PCGBlock *>& pcgbs(void) {return _pcgbs;};
	CFG* getCFG(void) {return cfg;};
	
	// Hook identifier
	static Identifier<PCG *> ID;
};

inline Output& operator<<(Output& out, const PCG& pcg) {
	out << "<not printable>";
	return out;
}

}
#endif
