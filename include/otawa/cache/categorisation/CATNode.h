#ifndef _CATNODE_H_
#define _CATNODE_H_


#include <otawa/instruction.h>
#include <otawa/cfg/BasicBlock.h>


namespace otawa {

class LBlockSet;
class LBlock;

// CCGNode class
class CATNode: public elm::inhstruct::DLNode, public ProgObject {
	
	BasicBlock *headerlblock;
	BasicBlock *headerevolution;
	bool inloop;
	bool hasheaderevolution;
	LBlock *lbl;	
	
	
public:

	//constructor
	CATNode(LBlock *lblock);
	
	// methodes
	
	void setHEADERLBLOCK(BasicBlock *hlb,bool loop);
	void setHEADEREVOLUTION(BasicBlock *hev, bool evolution);
	BasicBlock *HEADERLBLOCK(void);
	BasicBlock *HEADEREVOLUTION(void);
	bool INLOOP (void);
	bool HASHEADEREVOLUTION(void);
	LBlock *lblock(){return lbl;};
};

} // otawa
#endif //_CATNODE_H_
