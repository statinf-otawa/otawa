/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * src/util/util_Dominance.cpp -- Dominance class implementation.
 */

#include <otawa/util/Dominance.h>
#include <otawa/util/DFA.h>
#include <otawa/util/DFABitSet.h>
#include <otawa/cfg.h>

namespace otawa {


/**
 * Identifier of annotation containing reverse-dominance information.
 * Information is of type DFABitSet *.
 */
static Identifier ID_RevDom("otawa.util.revdom");


/**
 * DFA used for comuting the reverse domination relation. For each basic block,
 * the set of dominators is computed and hooked to the basic block. Then,
 * a simple bit test is used for testing the relation.
 */
class DominanceDFA: public DFA {
	int _size;
public:
	DominanceDFA(int size);
	
	// DFA overload
	virtual DFASet *initial(void);
	virtual DFASet *generate(BasicBlock *bb);
	virtual DFASet *kill(BasicBlock *bb);
	virtual void clear(DFASet *set);
	virtual void merge(DFASet *acc, DFASet *set);
};


/**
 * Build a new DFA for dominance relation computation.
 * @param size	Count of BB in the CFG.
 */
DominanceDFA::DominanceDFA(int size): _size(size) {
}


/**
 */
DFASet *DominanceDFA::initial(void) {
	return new DFABitSet(_size, true);
}


/**
 */
DFASet *DominanceDFA::generate(BasicBlock *bb) {
	DFABitSet *result = new DFABitSet(_size);
	result->add(bb->use<int>(CFG::ID_Index));
	return result;
}


/**
 */
DFASet *DominanceDFA::kill(BasicBlock *bb) {
	if(bb->isEntry())
		return new DFABitSet(_size, true);
	else
		return new DFABitSet(_size);
}


/**
 */
void DominanceDFA::clear(DFASet *set) {
	((DFABitSet *)set)->fill();
}


/**
 */
void DominanceDFA::merge(DFASet *acc, DFASet *set) {
	((DFABitSet *)acc)->mask((DFABitSet *)set);
}


/**
 * @class Dominance
 * This CFG processor computes and hook to the CFG the dominance relation
 * that, tehn, may be tested with @ref Dominance::dominate() function.
 */


/**
 * Test if the first basic block dominates the second one.
 * @param bb1	Dominator BB.
 * @param bb2	Dominated BB.
 * @return		True if bb1 dominates bb2.
 */
bool Dominance::dominate(BasicBlock *bb1, BasicBlock *bb2) {
	int index = bb1->use<int>(CFG::ID_Index);
	assert(index >= 0);
	DFABitSet *set = bb2->use<DFABitSet *>(ID_RevDom);
	assert(set);
	return set->contains(index);
}


/**
 * @fn bool Dominance::isDominated(BasicBlock *bb1, BasicBlock *bb2);
 * Test if the first block is dominated by the second one.
 * @param bb1	Dominated BB.
 * @param bb2	Dominator BB.
 * @return		True if bb2 dominates bb1.
 */


/**
 * Computes the domination relation.
 */
void Dominance::processCFG(FrameWork *fw, CFG *cfg) {
	DominanceDFA dfa(cfg->countBB());
	dfa.resolve(cfg, 0, &ID_RevDom);
}

} // otawa
