#include <elm/Iterator.h>
#include <otawa/sim/BBPathDriver.h>

using namespace elm;

namespace otawa { namespace sim {



BBPathDriver::BBPathDriver(ipet::BBPath& bbpath)
: bb_iter(bbpath.bbs().visit()), ended(false){
	inst_iter = new Iterator<Inst*>(bb_iter.item()->visit());
	if(bb_iter.ended()){
		ended = true;
	}
	else{
		bb_iter.next();
	}
}

BBPathDriver::~BBPathDriver(){
	delete inst_iter;
}

Inst* BBPathDriver::firstInstruction(State &state){
	return nextInstruction(state, 0);
}

Inst* BBPathDriver::nextInstruction(State &state, Inst *inst){
	if(ended){
		return 0;
	}
	while(inst_iter->ended() && !bb_iter.ended()){
		delete inst_iter;
		inst_iter = new Iterator<Inst*>(bb_iter.item()->visit());
		bb_iter.next();
	}
	if(inst_iter->ended() && bb_iter.ended()){
		ended = true;
		return 0;
	}
	else{
		Inst* inst = inst_iter->item();
		inst_iter->next();
		return inst;
	}
}

void BBPathDriver::terminateInstruction(State &state, Inst *inst){
}

} }
