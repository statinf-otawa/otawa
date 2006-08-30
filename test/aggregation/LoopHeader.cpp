#include "LoopHeader.h"

namespace otawa { namespace ipet {

LoopHeader::LoopHeader(BasicBlock *bb)
: _child(bb){
	_head = bb->head();
}

} } // otawa::ipet
