/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 * 
 * otawa/prog/CATNode.h -- CATNode class interface.
 */
#ifndef OTAWA_PROG_CATNODE_H
#define OTAWA_PROG_CATNODE_H


#include <otawa/instruction.h>
#include <otawa/cfg/BasicBlock.h>


namespace otawa {

class LBlockSet;
class LBlock;

// CCGNode class
class CATNode: public elm::inhstruct::DLNode, public ProgObject {
	
	BasicBlock *headerlblock;
	//BasicBlock *headerevolution;
	bool inloop;
	//bool hasheaderevolution;
	LBlock *lbl;	
	
	
public:

	//constructor
	CATNode(LBlock *lblock);
	
	// methodes
	
	void setHEADERLBLOCK(BasicBlock *hlb,bool loop);
	//void setHEADEREVOLUTION(BasicBlock *hev, bool evolution);
	BasicBlock *HEADERLBLOCK(void);
	//BasicBlock *HEADEREVOLUTION(void);
	bool INLOOP (void);
	//bool HASHEADEREVOLUTION(void);
	LBlock *lblock(){return lbl;};
};

} // otawa

#endif // OTAWA_PROG_CATNODE_H
