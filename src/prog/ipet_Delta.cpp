/*
 *  $Id$
 *  Copyright (c) 2006, IRIT-UPS.
 *
 *  ipet_Delta.h -- Delta class implementation.
 */

#include <assert.h>
#include <elm/genstruct/Vector.h>
#include <otawa/ipet.h>
#include <otawa/ipet/Delta.h>
#include <otawa/ilp.h>


using namespace otawa::ilp;
using namespace elm::genstruct;

namespace otawa { namespace ipet {

/**
 * @class Delta
 * @author G. Cavaignac
 * This processor is used for computing time effects of pipeline, between basic blocks <br>
 * Properties accepted: <br>
 * <code>Delta::ID_Levels<int></code> : depth of the delta algorithm: Level n = delta for sequences with length n+1<br>
 * <code>IPET::ID_Explicit<bool></code> : build explicit names for sequences for ilp variables.
 * Making explicit names is time consuming. It should be activated for debugging purpose only.
 */

/**
 * Build a new delta calculator.
 * @param props Configuration properties.
 */
Delta::Delta(const PropList& props)
: CFGProcessor("otawa::ipet::Delta", Version(0, 1, 0), props),
  nlevels(ID_Levels(props)){
	explicitNames = props.get<bool>(IPET::ID_Explicit,false);
	framework = 0;
}

/**
 * Process the given CFG
 * @see CFGProcessor::processCFG()
 */
void Delta::processCFG(FrameWork* fw, CFG* cfg){
	assert(fw);
	assert(cfg);
	framework = fw;
	//int nbConstraintsCreated = 0;
	//int nbObjectFunctionCreated = 0;
	System *system = IPET::getSystem(fw,cfg);
	Vector<BBSequence*> bbsList(4*cfg->bbs().count());
	for(CFG::BBIterator bb(cfg) ; bb ; bb++){
		// we create all sequences with length = 1
		bbsList.add(getBBS(bb));
	}
	// from 1 to nlevels+1 <=> from 0 to nlevels
	for(int i = 0 ; i < nlevels ; i++){
		Vector<BBSequence*> newBBS;
		int l = bbsList.length();
		for(int j=0 ; j < l ; j++){
			// we search all length+1 sequences
			Vector<BBSequence*> *toInsert = bbsList[j]->nexts();
			int l2 = toInsert->length();
			for(int k = 0 ; k < l2 ; k++)
				newBBS.add(toInsert->get(k));
			delete toInsert;
		}

		bbsList.clear(); // clone newBBS into bbsList
		l = newBBS.length();
		for(int j=0 ; j < l ; j++){
			BBSequence *bbsptr = newBBS[j];
			bbsList.add(bbsptr); // clone newBBS into bbsList
			BBSequence &bbs = *bbsptr;

			int delta = bbs.delta();
			Var *var = bbs.getVar(system); 
			Constraint *cons;

			// constraint S[A,B,C] <= S[A,B]
			cons = system->newConstraint(Constraint::LE);
			cons->addLeft(1,var);
			cons->addRight(1, bbs(1,bbs.l()-1)->getVar(system));

			// constraint S[A,B,C] <= S[B,C]
			cons = system->newConstraint(Constraint::LE);
			cons->addLeft(1,var);
			cons->addRight(1, bbs(2,bbs.l())->getVar(system));

			system->addObjectFunction(delta, var);

			//nbConstraintsCreated += 2;
			//nbObjectFunctionCreated++;
		}
	}

	// dumping system on standard output
	// system->dump();

	/*cout << BBSequence::instructionsSimulated << " instructions simulated\n";
	cout << BBSequence::nbDeltasCalculated << " deltas calculated\n";
	cout << nbObjectFunctionCreated << " object function created\n";
	cout << nbConstraintsCreated << " constraints created\n";*/

	// cleaning
	framework = 0;
	return;
}



/**
 * Returns the sequence of length 1 composed by only the given basic block.
 * If this sequence doesn't exist, creates it.
 * @param start basic block
 * @return BBSequence corresponding to the given basic block
 */
BBSequence *Delta::getBBS(BasicBlock *start){
	TreePath<BasicBlock*,BBSequence*> *tree = ID_Tree(start);
	if(!tree){
		BBSequence *bbs = new BBSequence(this, start); 
		tree = new TreePath<BasicBlock*,BBSequence*>(start,bbs);
		ID_Tree(start) = tree;
		return bbs;
	}
	return tree->rootData();
}



/**
 * Returns the sequence composed by the given path.
 * If this sequence doesn't exist, creates it.
 * @param path vector of basic blocks composing the path
 * @return BBSequence corresponding to the given path
 */
BBSequence *Delta::getBBS(Vector<BasicBlock*> *path){
	assert(path);
	BasicBlock *bb = path->get(0);
	BBSequence *bbs;
	TreePath<BasicBlock*,BBSequence*> *tree = ID_Tree(bb);
	if(!tree){
		bbs = new BBSequence(this, path);
		tree = new TreePath<BasicBlock*,BBSequence*>(*path,bbs);
		ID_Tree(bb) = tree;
		return bbs;
	}
	elm::Option<BBSequence*> option = tree->get(*path,1);
	if(!option){
		bbs = new BBSequence(this, path);
		tree->add(path,bbs,1);
		return bbs;
	}
	return *option;
}


/**
 * This identifier is used for storing the depth of the Delta algorith
 */
GenericIdentifier<int>  Delta::ID_Levels("delta.levels",5);

/**
 * This identifier is used for storing in a BasicBlock a TreePath
 * storing all BBSequence starting from this basic block
 */
GenericIdentifier<TreePath<BasicBlock*,BBSequence*>*> Delta::ID_Tree("delta.tree",0);



} }
