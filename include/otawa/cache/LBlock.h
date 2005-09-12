/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/LBlock.h -- interface of LBlock class.
 */

#ifndef _LBLOCK_H_
#define _LBLOCK_H_
#include <elm/genstruct/SLList.h>
#include <elm/inhstruct/DLList.h>
#include <elm/Iterator.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGNode.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ilp/Var.h>
#include <otawa/cache/categorisation/CATNode.h>
#include <string>
#include <otawa/hardware/Cache.h>

 using std::string;


namespace otawa {


class LBlockSet;
class CCGNode;
// CCGNode class

class LBlock: public elm::inhstruct::DLNode, public ProgObject {
	
	address_t lblc;
	int ident;
	BasicBlock *bblblock;
	bool nonconflit;
	ilp::Var *miss;
	ilp::Var *hit;
	ilp::Var *xi;
	int chit;
	int cmiss;
	CCGNode *ccgnod;
	CATNode *catnod; 
	
	// Private methods
	~LBlock(void){delete this;};
	friend class LBlockSet;

		

public:
	static LBlockSet *idlblockset; 
	//constructor
	LBlock(LBlockSet *graphe , address_t head , BasicBlock *bb, ilp::Var *hit1, 
		ilp::Var *miss1, ilp::Var *xi1, string tp );
	
	// methodes
	int identificateurLBLOCK (void);
	address_t addressLBLOCK (void);
	void changeSTATENONCONF(bool set);
	bool returnSTATENONCONF(void);
	BasicBlock *blockbasicLBLOCK (void);
	ilp::Var *varHIT(void);
	ilp::Var *varMISS(void);
	ilp::Var *varBB(void);
	int countLBINTRUCTION( int cycle , const Cache *cach);
	int constCHIT(void);
	int constCMISS(void);
	CCGNode *ccgnode(){return ccgnod;};
	CATNode *catnode(){return catnod;};
	
};

} // otawa


#endif //_LBLOCK_H_
