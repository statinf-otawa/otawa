/*
 *  $Id$
 *  Copyright (c) 2006, IRIT-UPS.
 *
 *  src/prog/ipet_Delta.h -- Delta class implementation.
 */
#include <assert.h>
#include <elm/genstruct/Vector.h>
#include <elm/util/Option.h>
#include <otawa/ipet.h>
#include <otawa/ipet/Delta.h>
#include <otawa/ilp.h>
#include <otawa/sim/State.h>
#include <elm/genstruct/VectorQueue.h>

using namespace otawa::ilp;
using namespace elm::genstruct;

namespace otawa { namespace ipet {

/**
 * @author G. Cavaignac
 * @class Delta
 * This processor is used for computing time effects of pipeline, between basic
 * blocks.
 * 
 * @par Configuration Properties accepted:
 * @li @ref Delta::LEVELS: depth of the delta algorithm: Level n = delta for
 * sequences with length n.
 * @li @ref Delta::SEQ_COMPLETION: number of instructions after the basic
 * block of a sequence to stop the sequence.
 * 
 * @par Provided Feature
 * @li @ref INTERBLOCK_SUPPORT_FEATURE
 * @li @ref BB_TIME_FEATURE
 */

/**
 * Build a new delta calculator.
 * @param props Configuration properties.
 */
Delta::Delta(void)
: CFGProcessor("otawa::ipet::Delta", Version(1, 0, 0)){
	provide(BB_TIME_FEATURE);
	provide(INTERBLOCK_SUPPORT_FEATURE);
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
	levels = LEVELS(props);
	completion = SEQ_COMPLETION(props);
	explicitNames = EXPLICIT(props);
}


/**
 */
void Delta::setup(FrameWork *fw) {
	max_length = 0;
	length_sum = 0;
	length_cnt = 0;
}


/**
 */
void Delta::cleanup(FrameWork *fw) {
	if(stats) {
		MAX_LENGTH(stats) = max_length;
		if(!length_cnt)
			MEAN_LENGTH(stats) = 0;
		else
			MEAN_LENGTH(stats) = (double)length_sum / length_cnt;
	}
}


/**
 * Build the constraints for the given BB path.
 * @param system	ILP system to work on.
 * @param bbpath	BB path to build constraints for.
 */
void Delta::processBBPath(FrameWork *fw, System *system, BBPath *bbpath) {
		BBPath &bbPath = *bbpath;
		int l = bbPath.length();

		// Record statistics
		length_sum += l;
		length_cnt++;
		if(l > max_length)
			max_length = l;
		
		// Compute delta
		if(bbpath->length() == 1) {
			int time = bbpath->time(fw);
			TIME(bbpath->head()) = time;
			return;
		}
		int delta = Delta::delta(bbPath, fw);

		// Build its variable
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

		// Add the object function delta
		system->addObjectFunction(delta, var);
}


/**
 * Process the given CFG
 * @see CFGProcessor::processCFG()
 */
void Delta::processCFG(FrameWork* fw, CFG* cfg){
	assert(fw);
	assert(cfg);
	System *system = getSystem(fw,cfg);
	//Vector<BBPath*> bbPathVector(4*cfg->bbs().count());
	VectorQueue<BBPath*> to_process(4*cfg->bbs().count());

	// We create all sequences with length = 1
	for(CFG::BBIterator bb(cfg); bb; bb++){
		if(!bb->isEntry() && !bb->isExit()){
			to_process.put(BBPath::getBBPath(bb));
		}
	}
	
	// While there is something to process
	while(!to_process.isEmpty()) {
		BBPath *bbpath = to_process.get();
		/*cout << "processing [";
		for(BBPath::BBIterator bb(bbpath); bb; bb++)
			cout << bb->address() << ", ";
		cout << "]\n";*/
		
		// Generates constraints
		processBBPath(fw, system, bbpath);
		
		// one search all length+1 sequences from sequences in bbPathVector
		// and one put all these in bbPathToProcess
		/*if(truebbPath.tail()->countInstructions() < FLUSH_TIME(bbPath)){
			bbPathVector.add(bbPathPtr);
		}*/
		if(!bbpath->tail()->isCall()) {
			bool cont = false;
			if(levels)
				cont = bbpath->length() < levels;
			else {
				int local_compl = completion;
				if(!local_compl)
					local_compl = bbpath->head()->countInstructions() + 1;
				cont = bbpath->countInstructions() <
					bbpath->head()->countInstructions() + local_compl;
			}
			if(cont) {
				Vector<BBPath*> *toInsert = bbpath->nexts();
				int l2 = toInsert->length();
				for(int k = 0 ; k < l2 ; k++)
					to_process.put(toInsert->get(k));
				delete toInsert;
			}
		}	 
	}
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
GenericIdentifier<int>  Delta::LEVELS("Delta::levels", 0, ipet::NS);

/**
 * This identifier is used for storing the delta value of a path
 */
GenericIdentifier<int> Delta::DELTA("Delta::delta", 0, ipet::NS);

/**
 * This identifier is used for storing the time for the first
 * instruction to fetch after all instructions from the
 * beginning of the sequence have been fetched.
 */
GenericIdentifier<int> Delta::SEQ_COMPLETION("Delta::seq_completion", 0, ipet::NS);


/**
 * Property used to return statistics about the @ref Delta processor about
 * the maximal sequence length in instructions.
 */
GenericIdentifier<double> Delta::MAX_LENGTH("Delta::max_length", 0, ipet::NS);


/**
 * Property used to return statistics about the @ref Delta processor about
 * the mean sequence length in instructions.
 */
GenericIdentifier<double> Delta::MEAN_LENGTH("Delta::min_length", 0, ipet::NS);


/**
 * This identifier is used for storing in a BasicBlock a TreePath
 * storing all BBPath starting from this basic block
 */
GenericIdentifier<TreePath<BasicBlock*,BBPath*>*>
	Delta::TREE("Delta::tree", 0, ipet::NS);


/**
 * Feature ensure that sequences have been built to achieve the Delta
 * interblock effect support.
 * 
 * @par Properties
 * @li @ref Delta::DELTA (@ref BBPATH).
 */
Feature<Delta> DELTA_SEQUENCES_FEATURE("delta_sequences");

} } // otawa::ipet
