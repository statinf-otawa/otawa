/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/prog/BBSequence.cpp -- implementation of basic blocs sequence (path) class.
 */

#include <assert.h>
#include <otawa/ipet/BBSequence.h>
#include <otawa/ipet.h>
#include <otawa/sim/TrivialSimulator.h>

using namespace otawa::ilp;
using namespace otawa::sim;
using namespace elm::genstruct;

namespace otawa{ namespace ipet {


/**
 * This class holds a sequence of basic blocks for calculating deltas
 * @class BBSequence
 * @author G. Cavaignac
 */

/**
 * Builds a new BBSequence (length=1) from a given basic block
 * @param delta Delta Processor it is linked to
 * @param start starting basic block
 */
BBSequence::BBSequence(Delta *delta, BasicBlock *start)
: basicBlocks(1), _length(1), ending_state(0), _delta(elm::none){
	assert(delta);
	assert(start);
	deltaProcessor = delta;
	basicBlocks.add(start);
}


/**
 * Builds a new BBSequence from a vector of basic blocks
 * @param delta Delta Processor it is linked to
 * @param path vector of the basic blocks
 */
BBSequence::BBSequence(Delta *delta, Vector<BasicBlock*> *path)
: _length(path->length()), basicBlocks(_length), ending_state(0){
	assert(delta);
	assert(path);
	deltaProcessor = delta;
	int l = path->length();
	for(int i = 0 ; i < l ; i++)
		basicBlocks.add(path->get(i));
}


/**
 * Builds a vector containing this BBSequence made longer
 * with all successors of the last basic block it holds. <br>
 * Searches in the Delta Processor if these sequences already exists
 * @return vector of the new BBSequences
 */
Vector<BBSequence*> *BBSequence::nexts(){
	Vector<BBSequence*> *nextbbs = new Vector<BBSequence*>(2);
	BasicBlock *bb = basicBlocks.top();
	Vector<BasicBlock*> bbs(basicBlocks.length()+1);
	int l = basicBlocks.length();
	for(int i=0 ; i < l ; i++)
		bbs.add(basicBlocks[i]);
	for(BasicBlock::OutIterator edge(bb) ; edge ; edge++){
		bbs.add(edge->target());
		nextbbs->add(deltaProcessor->getBBS(&bbs));
		bbs.pop();
	}
	return nextbbs;
}



/**
 * Returns the number of cycles the processor must do to cover the sequence.
 * Simulates the sequence if necessary
 * @return number of cycles
 */
int BBSequence::time(){
	int time = get<int>(IPET::ID_Time,-1);
	if(time < 0){
		time = simulate();
		IPET::ID_Time(this) = time;
	}
	return time;
}

//int BBSequence::instructionsSimulated = 0;
//int BBSequence::nbDeltasCalculated = 0;

/**
 * Launches a simulation of the sequence. If the simulator
 * supports state cloning, it uses the saved state of the
 * sub-sequences in order to simulate less instructions. <br>
 * @return number of cycles to cover the sequence
 */
int BBSequence::simulate(){
	State *state = getEndingState();
	state->flush();
	int time = state->cycle();
	delete state;
	return time;
}

/**
 * This method returns the simulator state at the end of the last instruction of this sequence.
 * If the simulator supports state cloning, it returns a copy of the state.
 * Otherwise, the sequence is simulated another time.
 * This state must be deleted after using it.
 * @return simulator state at the end of the sequence
 */
State* BBSequence::getEndingState(){
	bool simulator_state_is_clonable = true;
	if(!ending_state){
		if(_length > 1){
			ending_state = sub(1,l()-1)->getEndingState();
		}else{
			TrivialSimulator simulator;
			ending_state = simulator.instantiate(deltaProcessor->framework);
		}
		//instructionsSimulated += basicBlocks.top()->countInstructions();
		ending_state->setPC(basicBlocks.top()->address());
		ending_state->runUntilBranch();
		
	}
	if(simulator_state_is_clonable)
		return ending_state->clone();
	State *tmp = ending_state;
	ending_state = 0;
	return tmp;
}

/**
 * Calculates the delta (time effects between basic blocks) of this sequence, if never calculated. <br>
 * For sequences of length 1, the delta is 0 <br>
 * For sequences of length 2, delta = T - Ts[1..l-1] - Ts[2..l] <br>
 * For others, delta = T - Ts[1..l-1] - Ts[2..l] + Ts[2..l-1]
 * @return the value of delta
 */
int BBSequence::delta(){
	if(!_delta){
		//nbDeltasCalculated++;
		if(length() == 1)
			_delta = 0;
		else if(length() == 2)
			_delta = t() - sub(1,l()-1)->t() - sub(2,l())->t();
		else
			_delta = t() - sub(1,l()-1)->t() - sub(2,l())->t() + sub(2,l()-1)->t();
	}
	return _delta;
}

/**
 * Creates a human-readable name (or representation) for this sequence <br>
 * This name is like <code>Seq_x1_x2_x3</code> if the basic blocks have names x1, x2 and x3
 * @return string containing the sequence name
 */
String BBSequence::makeVarName(System *system){
	assert(system);
	StringBuffer buf;
	buf << "Seq_";
	for(int i=0 ; i < length() ; i++){
		if(i != 0)
			buf << '_';
		Var *var = basicBlocks[i]->get<Var*>(IPET::ID_Var,0);
		if(var) 
			buf << var->name();
		else
			buf << 'x';
	}
	return buf.toString();
}

/**
 * if this sequence doesn't have a var (for the ilp system) attached, creates a new one
 * @return Var corresponding to this sequence
 */
Var* BBSequence::getVar(System *system){
	assert(system);
	if(length() == 1)
		return IPET::getVar(system, basicBlocks[0]);
	Var *var = IPET::ID_Var(this);
	if(!var) {
		if(deltaProcessor->explicitNames){
			var = system->newVar(makeVarName(system));
		} else
			var = system->newVar();
		add(IPET::ID_Var, var);
	}
	return var;
}

/**
 * Returns a sub-sequence. Searches in the Delta Processor attached if this sequence already exists
 * @param begin index of the first basic block (first = 1)
 * @param end index of the last basic block (last = length())
 * @return sub-sequence of current sequence, from <code>begin</code> to <code>end</code> indexes
 */
BBSequence* BBSequence::sub(int begin, int end){
	assert(begin > 0);
	assert(end <= length());
	assert(begin <= end);
	Vector<BasicBlock*> bbs(end-begin+1);
	for(int i = begin-1 ; i < end ; i++)
		bbs.add(basicBlocks[i]);
	return deltaProcessor->getBBS(&bbs);
}

/**
 * tests if two BBSequences are the same or not (same basic blocks in the path) <br>
 * @param bbs BBSequence to compare
 * @return boolean : true if both sequences are the same
 * @attention Useless method : There is only 1 BBSequence alive at one time
 */
bool BBSequence::equals(BBSequence &bbs){
	if(length() != bbs.length())
		return false;
	for(int i = 0 ; i < length() ; i++)
		if(basicBlocks[i] != bbs.basicBlocks[i])
			return false;
	return true;
}


/**
 * @fn int BBSequence::t()
 * Same as BBSequence::time()<br>
 * Returns the number of cycles the processor must do to cover the sequence.
 * Simulates the sequence if necessary
 * @return number of cycles
 */

/**
 * @fn int BBSequence::l()
 * Same as BBSequence::length() <br>
 * Gives the number of BBSequences that composes this path
 * @return length of the sequence
 */

/**
 * @fn int BBSequence::length()
 * Gives the number of BBSequences that composes this path
 * @return length of the sequence
 */

/**
 * @fn bool BBSequence::operator== (BBSequence &bbs)
 * Same as equals()
 * tests if two BBSequences are the same or not (same basic blocks in the path) <br>
 * @param bbs BBSequence to compare
 * @return boolean : true if both sequences are the same
 * @attention Useless method : There is only 1 BBSequence alive at one time
 */

/**
 * @fn bool BBSequence::operator!= (BBSequence &bbs)
 * tests if two BBSequences are the same or not (same basic blocks in the path) <br>
 * @param bbs BBSequence to compare
 * @return boolean : false if both sequences are the same
 * @attention Useless method : There is only 1 BBSequence alive at one time
 */


/**
 * @fn BBSequence* BBSequence::operator() (int begin, int end)
 * Returns a sub-sequence. Searches in the Delta Processor attached if this sequence already exists
 * @param begin index of the first basic block (first = 1)
 * @param end index of the last basic block (last = length())
 * @return sub-sequence of current sequence, from <code>begin</code> to <code>end</code> indexes
 * @see sub()
 */

} }
