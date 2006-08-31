/*
 *  $Id$
 *  Copyright (c) 2006, IRIT-UPS.
 *
 *  src/prog/ipet_Delta.h -- Delta class implementation.
 */
#include <assert.h>
#include <elm/genstruct/Vector.h>
#include <otawa/ipet.h>
#include <otawa/ipet/Delta.h>
#include <otawa/ilp.h>
#include <otawa/sim/State.h>


using namespace otawa::ilp;
using namespace elm::genstruct;

namespace otawa { namespace ipet {

/**
 * @author G. Cavaignac
 * @class Delta
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
: CFGProcessor("otawa::ipet::Delta", Version(0, 3, 0), props){
	configure(props);
}

/**
 * Configures the delta calculator. Properties accepted are
 * 
 * <code>Delta::ID_Levels<int></code> : number of delta levels
 * 
 * <code>IPET::ID_Explicit<bool></code> : give explicit names for the sequences
 */
void Delta::configure(const PropList& props){
	CFGProcessor::configure(props);
	levels = props.get<int>(LEVELS);
	explicitNames = EXPLICIT(props);
}

/**
 * Process the given CFG
 * @see CFGProcessor::processCFG()
 */
void Delta::processCFG(FrameWork* fw, CFG* cfg){
	assert(fw);
	assert(cfg);
	assert(cfg->isInlined());
	
	System *system = getSystem(fw,cfg);
	Vector<BBPath*> bbPathVector(4*cfg->bbs().count());
	for(CFG::BBIterator bb(cfg) ; bb ; bb++){
		// we create all sequences with length = 1
		if(!bb->isEntry() && !bb->isExit()){
			bbPathVector.add(BBPath::getBBPath(bb));
		}
	}
	
	// stats
	int sum_length = 0;
	int count = 0;
	max_length = 0;
	
	
	// from 1 to nlevels+1 <=> from 0 to nlevels
	for(int i = 0 ;
		(levels && i < levels) || (bbPathVector.length() > 0 && !levels)  ;
		i++)
	{
		Vector<BBPath*> bbPathToProcess; // BBPaths that have to be processed
		
		// one search all length+1 sequences from sequences in bbPathVector
		// and one put all these in bbPathToProcess
		for(int j=0 ; j < bbPathVector.length() ; j++){
			Vector<BBPath*> *toInsert = bbPathVector[j]->nexts();
			int l2 = toInsert->length();
			for(int k = 0 ; k < l2 ; k++)
				bbPathToProcess.add(toInsert->get(k));
			delete toInsert;
		}

		// BBPaths processing
		bbPathVector.clear();
		for(int j=0 ; j < bbPathToProcess.length() ; j++){
			BBPath *bbPathPtr = bbPathToProcess[j];
			BBPath &bbPath = *bbPathPtr;
			int l = bbPath.length();

			int delta = Delta::delta(bbPath, fw);
			if(true/*bbPath.tail()->countInstructions() < FLUSH_TIME(bbPath)*/){
				bbPathVector.add(bbPathPtr);
			}
			
			// stats
			sum_length += l;
			count++;
			if(l > max_length)
				max_length = l;
			
			ilp::Var *var = bbPath.getVar(system, explicitNames); 
			Constraint *cons;

			// constraint S[A,B,C] <= S[A,B]
			cons = system->newConstraint(Constraint::LE);
			cons->addLeft(1, var);
			cons->addRight(1, bbPath(1, l-1)->getVar(system, explicitNames));

			// constraint S[A,B,C] <= S[B,C]
			cons = system->newConstraint(Constraint::LE);
			cons->addLeft(1, var);
			cons->addRight(1, bbPath(2, l)->getVar(system, explicitNames));
			
			// constraint S[A,B,C] >= S[A,B] - Sum(S[B,x], x != C)
			BasicBlock *bb_B;
			cons = system->newConstraint(Constraint::GE);
			cons->addLeft(1,var);
			cons->addRight(1, bbPath(1, l-1)->getVar(system, explicitNames));
			bb_B = bbPath(l-1, l-1)->head();
			Vector<BBPath*> &nexts = * BBPath::getBBPath(bb_B)->nexts();
			for(int i = 0; i < nexts.length(); i++){
				if(nexts[i]->tail() != bbPath.tail()){
					BBPath& otherbbp = *nexts[i];
					ilp::Var *otherbbp_var = otherbbp.getVar(system, explicitNames);
					cons->addRight(-1, otherbbp_var);
				}
			}

			system->addObjectFunction(delta, var);
		}
		
		// stats
		if(count == 0)
			mean_length = 0;
		else
			mean_length = (double)sum_length / (double)count;
	}
	return;
}

/**
 * Calculate the delta of the given BBPath
 * @param bbp BBPath we want to calculate the delta
 */
int Delta::delta(BBPath &bbp, FrameWork *fw){
	assert(fw);
	
	if(bbp.length() <= 1)
		return 0;
	
	elm::Option<int> delta = bbp.get<int>(DELTA);
	if(!delta){
		//nbDeltasCalculated++;
		int t = bbp.time(fw);
		int l = bbp.length();
		/*int f, o;
		sim::State *state = bbp.getEndingState(fw);
		f = state->cycle();
		state->flush();
		t = state->cycle();
		o = t - f;
		FLUSH_TIME(bbp) = o;*/
		if(bbp.length() == 2){
			delta =
				t - bbp(1,l-1)->t(fw)
			      - bbp(2,l)->t(fw);
		}
		else{
			delta =
				t - bbp(1,l-1)->t(fw)
			      - bbp(2,l)->t(fw)
			      + bbp(2,l-1)->t(fw);
		}
	}
	DELTA(bbp) = *delta;
	return *delta;
}


/**
 * This identifier is used for forcing the depth of the Delta algorith.
 * If this identifier is not set, the depth will be adjusted automatically
 */
GenericIdentifier<int>  Delta::LEVELS("delta.levels");

/**
 * This identifier is used for storing the delta value of a path
 */
GenericIdentifier<int> Delta::DELTA("ipet.delta");
/**
 * This identifier is used for storing the time for the first
 * instruction to fetch after all instructions from the
 * beginning of the sequence have been fetched
 */
GenericIdentifier<int> Delta::FLUSH_TIME("delta.flush_time");

} } // otawa::ipet
