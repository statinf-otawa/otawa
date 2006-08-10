/*
 *  $Id$
 *  Copyright (c) 2006, IRIT-UPS.
 *
 *  src/prog/ipet_BBPath.h -- BBPath class implementation.
 */
#include <otawa/ipet/BBPath.h>
#include <otawa/ipet.h>
#include <otawa/sim/TrivialSimulator.h>

using namespace elm::genstruct;
using namespace otawa::ilp;
using namespace otawa::sim;

namespace otawa { namespace ipet {

/**
 * @author G. Cavaignac
 * @class BBPath
 * This class represents a sequence of basic blocks, making a
 * basic block path.
 * BBPath are linked with others, and linked to basic blocks, and
 * they are unique. One can't have two instances of the same path.
 * It is possible to get a simulated time for this sequence
 */

/**
 * This identifier is used for storing in a BasicBlock a TreePath
 * storing all BBPath starting from this basic block
 */
GenericIdentifier<TreePath<BasicBlock*,BBPath*>*> BBPath::ID_Tree("pathmanager.tree",0);


/**
 * Builds a new BBPath (length=1) from a given basic block
 * @param framework FrameWork where one can get a simulator, or IPET::ID_Explicit
 * @param start starting basic block
 */
BBPath::BBPath(BasicBlock *start)
: basicBlocks(1), _length(1), ending_state(0){
	assert(start);
	basicBlocks.add(start);
}


/**
 * Builds a new BBPath from a vector of basic blocks
 * @param framework FrameWork where one can get a simulator, or IPET::ID_Explicit
 * @param path vector of the basic blocks
 */
BBPath::BBPath(Vector<BasicBlock*> *path)
: _length(path->length()), basicBlocks(_length), ending_state(0){
	assert(path);
	int l = path->length();
	for(int i = 0 ; i < l ; i++){
		basicBlocks.add(path->get(i));
	}
}


/**
 * Destroys the BBPath
 */
BBPath::~BBPath(){
	if(!ending_state){
		delete ending_state;
	}
}


/**
 * Returns the path of length 1 composed by only the given basic block.
 * If this path doesn't exist, creates it.
 * @param start basic block
 * @return BBPath corresponding to the given basic block
 */
BBPath *BBPath::getBBPath(BasicBlock *start){
	TreePath<BasicBlock*,BBPath*> *tree = ID_Tree(start);
	if(!tree){
		BBPath *bbp = new BBPath(start); 
		tree = new TreePath<BasicBlock*,BBPath*>(start,bbp);
		ID_Tree(start) = tree;
		return bbp;
	}
	return tree->rootData();
}



/**
 * Returns the path composed by the given path.
 * If this path doesn't exist, creates it.
 * @param path vector of basic blocks composing the path
 * @return BBPath corresponding to the given path
 */
BBPath *BBPath::getBBPath(Vector<BasicBlock*> *path_vector){
	assert(path_vector);
	BasicBlock *bb = path_vector->get(0);
	BBPath *bbpath;
	TreePath<BasicBlock*,BBPath*> *tree = ID_Tree(bb);
	if(!tree){
		bbpath = new BBPath(path_vector);
		tree = new TreePath<BasicBlock*,BBPath*>(*path_vector, bbpath);
		ID_Tree(bb) = tree;
		return bbpath;
	}
	elm::Option<BBPath*> option = tree->get(*path_vector,1);
	if(!option){
		bbpath = new BBPath(path_vector);
		tree->add(path_vector, bbpath, 1);
		return bbpath;
	}
	return *option;
}



/**
 * Builds a vector containing this BBSequence made longer
 * with all successors of the last basic block it holds. <br>
 * Searches in the Delta Processor if these sequences already exists
 * @return vector of the new BBSequences
 */
Vector<BBPath*> *BBPath::nexts(){
	Vector<BBPath*> *nextbbp = new Vector<BBPath*>(2);
	BasicBlock *bb = basicBlocks.top();
	Vector<BasicBlock*> bbp(basicBlocks.length()+1);
	
	int l = basicBlocks.length();
	for(int i=0 ; i < l ; i++){
		bbp.add(basicBlocks[i]);
	}
	
	for(BasicBlock::OutIterator edge(bb) ; edge ; edge++){
		bbp.add(edge->target());
		nextbbp->add(getBBPath(&bbp));
		bbp.pop();
	}
	
	return nextbbp;
}




IteratorInst<BasicBlock*> *BBPath::visit(void){
	BBIterator iter(this);
	return new elm::IteratorObject<BBIterator, BasicBlock *>(iter);
}

MutableCollection<BasicBlock *> *BBPath::empty(void){
	return 0;
}



/**
 * Returns the number of cycles the processor must do to cover the sequence.
 * Simulates the sequence if necessary
 * @return number of cycles
 */
int BBPath::time(FrameWork *fw){
	int time = get<int>(IPET::ID_Time,-1);
	if(time < 0){
		time = simulate(fw);
		IPET::ID_Time(this) = time;
	}
	return time;
}


int BBPath::countInstructions(){
	int count = 0;
	for(int i = 0; i < _length; i++){
		count += basicBlocks[i]->countInstructions();
	}
	return count;
}

/**
 * Launches a simulation of the sequence. If the simulator
 * supports state cloning, it uses the saved state of the
 * sub-sequences in order to simulate less instructions. <br>
 * @return number of cycles to cover the sequence
 */
int BBPath::simulate(FrameWork *fw){
	State *state = getEndingState(fw);
	state->flush();
	int time = state->cycle();
	delete state;
	//cerr << "Simulated time for " << makeVarName() << " = " << time << "\n";
	return time;
}


int BBPath::instructions_simulated = 0;


/**
 * This method returns the simulator state at the end of the last instruction of this sequence.
 * If the simulator supports state cloning, it returns a copy of the state.
 * Otherwise, the sequence is simulated another time.
 * This state must be deleted after using it.
 * @return simulator state at the end of the sequence
 */
State* BBPath::getEndingState(FrameWork *fw){
	bool simulator_state_is_clonable = true;
	if(!ending_state || fw != last_framework_used){
		last_framework_used = fw;
		if(_length > 1){
			ending_state = sub(1,l()-1)->getEndingState(fw);
		}
		else {
			TrivialSimulator simulator;
			ending_state = simulator.instantiate(fw);
		}
		BasicBlock &bb = *basicBlocks.top();
		for(Iterator<Inst*> inst(bb.visit()); inst; inst++){
			ending_state->setPC(inst);
			ending_state->step();
			instructions_simulated ++;
		}
	}
	if(simulator_state_is_clonable){
		return ending_state->clone();
	}
	State *tmp = ending_state;
	ending_state = 0;
	return tmp;
}

/**
 * Creates a human-readable name (or representation) for this sequence <br>
 * This name is like <code>Seq_x1_x2_x3</code> if the basic blocks have names x1, x2 and x3
 * @return string containing the sequence name
 */
String BBPath::makeVarName(){
	StringBuffer buf;
	buf << "Seq_";
	for(int i=0 ; i < length() ; i++){
		if(i != 0){
			buf << '_';
		}
		ilp::Var *var = basicBlocks[i]->get<ilp::Var*>(IPET::ID_Var,0);
		if(var && !var->name().isEmpty()){ 
			buf << var->name();
		}
		else {
			buf << "xx";
		}
	}
	return buf.toString();
}

/**
 * if this sequence doesn't have a var (for the ilp system) attached, creates a new one
 * @return Var corresponding to this sequence
 */
ilp::Var* BBPath::getVar(System *system, bool explicit_names){
	assert(system);
	if(length() == 1){
		return IPET::getVar(system, basicBlocks[0]);
	}
	ilp::Var *var = IPET::ID_Var(this);
	if(!var) {
		if(explicit_names){
			var = system->newVar(makeVarName());
		}
		else {
			var = system->newVar();
		}
		set(IPET::ID_Var, var);
	}
	return var;
}

/**
 * Returns a sub-path. Searches if this path already exists
 * @param begin index of the first basic block (first = 1)
 * @param end index of the last basic block (last = length())
 * @return sub-path of current path, from <code>begin</code> to <code>end</code> indexes
 */
BBPath* BBPath::sub(int begin, int end){
	assert(begin > 0);
	assert(end <= length());
	assert(begin <= end);
	Vector<BasicBlock*> bbs(end-begin+1);
	for(int i = begin-1 ; i < end ; i++) {
		bbs.add(basicBlocks[i]);
	}
	return getBBPath(&bbs);
}

/**
 * tests if two BBSequences are the same or not (same basic blocks in the path) <br>
 * @param bbs BBSequence to compare
 * @return boolean : true if both sequences are the same
 */
/*bool BBSequence::equals(BBSequence &bbs){
	if(length() != bbs.length())
		return false;
	for(int i = 0 ; i < length() ; i++)
		if(basicBlocks[i] != bbs.basicBlocks[i])
			return false;
	return true;
}*/


/**
 * @fn int BBPath::t()
 * Same as BBPath::time()<br>
 * Returns the number of cycles the processor must do to cover the path.
 * Simulates the path if necessary
 * @return number of cycles
 */

/**
 * @fn int BBPath::l()
 * Same as BBPath::length() <br>
 * Gives the number of basic blocks that composes this path
 * @return length of the path
 */

/**
 * @fn int BBPath::length()
 * Gives the number of basic blocks that composes this path
 * @return length of the path
 */





} } // otawa::ipet



