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
: CFGProcessor("otawa::ipet::Delta", Version(0, 2, 0), props){
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
	levels = props.get<int>(ID_Levels);
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
	//int nbConstraintsCreated = 0;
	//int nbObjectFunctionCreated = 0;
	System *system = getSystem(fw,cfg);
	Vector<BBPath*> bbPathList(4*cfg->bbs().count());
	for(CFG::BBIterator bb(cfg) ; bb ; bb++){
		// we create all sequences with length = 1
		bbPathList.add(BBPath::getBBPath(bb));
	}
	
	// stats
	int sum_length = 0;
	int count = 0;
	max_length = 0;
	
	
	// from 1 to nlevels+1 <=> from 0 to nlevels
	for(int i = 0 ;
		(levels && i < levels) || (bbPathList.length() > 0 && !levels)  ;
		i++){
	//while(bbPathList.length() > 0){
		Vector<BBPath*> newBBPath;
		int l = bbPathList.length();
		for(int j=0 ; j < l ; j++){
			// we search all length+1 sequences
			Vector<BBPath*> *toInsert = bbPathList[j]->nexts();
			int l2 = toInsert->length();
			for(int k = 0 ; k < l2 ; k++)
				newBBPath.add(toInsert->get(k));
			delete toInsert;
		}

		bbPathList.clear();
		l = newBBPath.length();
		for(int j=0 ; j < l ; j++){
			BBPath *bbPathPtr = newBBPath[j];
			BBPath &bbp = *bbPathPtr;

			int delta = Delta::delta(bbp, fw);
			if(bbp.tail()->countInstructions() < ID_Flush_Time(bbp)){
				bbPathList.add(bbPathPtr);
			}
			if(delta){
				// stats
				sum_length += bbp.length();
				count++;
				if(bbp.length() > max_length)
					max_length = bbp.length();
				
				ilp::Var *var = bbp.getVar(system, explicitNames); 
				Constraint *cons;
	
				// constraint S[A,B,C] <= S[A,B]
				cons = system->newConstraint(Constraint::LE);
				cons->addLeft(1,var);
				cons->addRight(1, bbp(1,bbp.l()-1)->getVar(system));
	
				// constraint S[A,B,C] <= S[B,C]
				cons = system->newConstraint(Constraint::LE);
				cons->addLeft(1,var);
				cons->addRight(1, bbp(2,bbp.l())->getVar(system));
	
				system->addObjectFunction(delta, var);
	
				//nbConstraintsCreated += 2;
				//nbObjectFunctionCreated++;
			}
			ilp::Var *var = bbp.getVar(system, explicitNames); 
			Constraint *cons;

			// constraint S[A,B,C] <= S[A,B]
			cons = system->newConstraint(Constraint::LE);
			cons->addLeft(1,var);
			cons->addRight(1, bbp(1,bbp.l()-1)->getVar(system));

			// constraint S[A,B,C] <= S[B,C]
			cons = system->newConstraint(Constraint::LE);
			cons->addLeft(1,var);
			cons->addRight(1, bbp(2,bbp.l())->getVar(system));

			system->addObjectFunction(delta, var);

			//nbConstraintsCreated += 2;
			//nbObjectFunctionCreated++;
		}
		if(count == 0)
			mean_length = 0;
		else
			mean_length = (double)sum_length / (double)count;
	}

	// dumping system on standard output
	// system->dump();

	/*cout << BBSequence::instructionsSimulated << " instructions simulated\n";
	cout << BBSequence::nbDeltasCalculated << " deltas calculated\n";
	cout << nbObjectFunctionCreated << " object function created\n";
	cout << nbConstraintsCreated << " constraints created\n";*/

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
	
	elm::Option<int> delta = bbp.get<int>(ID_Delta);
	if(!delta){
		//nbDeltasCalculated++;
		int f, o, t;
		sim::State *state = bbp.getEndingState(fw);
		f = state->cycle();
		state->flush();
		t = state->cycle();
		o = t - f;
		ID_Flush_Time(bbp) = o;
		if(bbp.length() == 2){
			delta =
				t - bbp(1,bbp.l()-1)->t(fw)
			      - bbp(2,bbp.l())->t(fw);
		}
		else{
			delta =
				t - bbp(1,bbp.l()-1)->t(fw)
			      - bbp(2,bbp.l())->t(fw)
			      + bbp(2,bbp.l()-1)->t(fw);
		}
	}
	ID_Delta(bbp) = *delta;
	return *delta;
}


/**
 * This identifier is used for forcing the depth of the Delta algorith.
 * If this identifier is not set, the depth will be adjusted automatically
 */
GenericIdentifier<int>  Delta::ID_Levels("delta.levels");

/**
 * This identifier is used for storing the delta value of a path
 */
GenericIdentifier<int> Delta::ID_Delta("delta.delta");
/**
 * This identifier is used for storing the time for the first
 * instruction to fetch after all instructions from the
 * beginning of the sequence have been fetched
 */
GenericIdentifier<int> Delta::ID_Flush_Time("delta.flush_time");

} } // otawa::ipet
