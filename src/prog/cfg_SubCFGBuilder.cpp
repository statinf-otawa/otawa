/*
 *	$Id$
 *	SubCFGBuilder analyzer implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <elm/data/FragTable.h>
#include <elm/data/VectorQueue.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/SubCFGBuilder.h>
#include <otawa/dfa/AbsIntLite.h>
#include <otawa/prog/WorkSpace.h>
#include <elm/util/BitVector.h>

using namespace elm;
using namespace otawa::dfa;

namespace otawa {

/**
 */
SubCFGBuilder::SubCFGBuilder(void)
: CFGTransformer(reg), _start_bb(0), maker(0), cfg(0) {
}

/**
 */
p::declare SubCFGBuilder::reg = p::init("otawa::SubCFGBuilder", Version(2, 0, 0))
	.require(COLLECTED_CFG_FEATURE)
	.base(CFGTransformer::reg)
	.maker<SubCFGBuilder>();


Identifier<bool> IS_START("", false);
Identifier<bool> IS_STOP("", false);
Identifier<bool> IS_ON_FORWARD_PATH("", false);
Identifier<bool> IS_ON_BACKWARD_PATH("", false);


/**
 * @class SubCFGBuilder
 * Build a sub-CFG starting at the given @ref otawa::START address and
 * ending at the multiple @ref otawa::STOP addresses.
 *
 * Note that, in this version, START and STOP addresses must be part
 * of the same CFG (original CFG or virtualized CFG).
 *
 * @par Required features
 * @li @ref otawa::COLLECTED_CFG_FEATURE
 *
 * @par Provided features
 * @li @ref otawa::COLLECTED_CFG_FEATURE
 *
 * @par Configuration
 * @li @ref otawa::CFG_START
 * @li @ref otawa::CFG_STOP
 */


/**
 */
void SubCFGBuilder::configure(const PropList &props) {
	Processor::configure(props);
	start = CFG_START(props);
	stops.clear();
	for(Identifier<Address>::Getter stop(props, CFG_STOP); stop; stop++)
		stops.add(stop);
}

/**
 * Flood forward start reachable marker.
 */
void SubCFGBuilder::floodForward(void) {
	genstruct::VectorQueue<Block *> todo;
	todo.put(_start_bb);
	while (todo){
		Block *bb = todo.get();
		IS_ON_FORWARD_PATH(*bb) = true;
		if (!IS_STOP(*bb)){
			for(BasicBlock::EdgeIter next = bb->outs(); next; next++){
				Block *nextbb = next->target();
				if(!IS_ON_FORWARD_PATH(*nextbb) && !todo.contains(nextbb))
					todo.put(nextbb);
			}
		}
	}
}

/**
 * Flood backward stop reachable markers.
 */
void SubCFGBuilder::floodBackward() {
	VectorQueue<Block *> todo;
	for(Vector<Block *>::Iter stop(_stop_bbs); stop; stop++)
		todo.put(stop);
	while (todo){
		Block *bb = todo.get();
		IS_ON_BACKWARD_PATH(*bb) = true;
		if (!IS_START(*bb)){
			for(Block::EdgeIter prev = bb->ins(); prev ; prev++){
				Block *prevbb = prev->source();
				if (!IS_ON_BACKWARD_PATH(*prevbb) && !todo.contains(prevbb))
					todo.put(prevbb);
			}
		}
	}
}

/**
 */
void SubCFGBuilder::transform(CFG *cfg, CFGMaker& maker) {

	// special process only for entry CFG
	if(cfg != entry()) {
		CFGTransformer::transform(cfg, maker);
		return;
	}

	// marker start and stops
	if(!start){
		Block::EdgeIter next = cfg->entry()->outs();
		_start_bb = next->target();
	}
	else
		_start_bb = NULL;  // needs to scan CFG
	if (!stops){
		Block::EdgeIter prev = cfg->exit()->ins();
		ASSERTP(prev, "Error! No predecessor for cfg->exit() -- you need to specify a stop address");
		_stop_bbs.add(prev->source());
	}
	else
		_stop_bbs.clear();   // needs to scan CFG

	//scan CFG
	for(CFG::BlockIter b = cfg->blocks(); b; b++) {
		if(!b->isBasic())
			continue;
		BasicBlock *bb = b->toBasic();
		if(bb->address() <= start && start < bb->address() + bb->size()){
			IS_START(bb) = true;
			_start_bb = bb;
		}
		for(Vector<Address>::Iter stop(stops); stop; stop++)
			if(bb->address() <= *stop && *stop < bb->address() + bb->size()) {
				IS_STOP(bb) = true;
				_stop_bbs.add(bb);
				break;
			}
	}

	// start flood analysis
	floodForward();
	floodBackward();

	// make all virtual BB
	FragTable<Block *> orgs;
	for(CFG::BlockIter bb = cfg->blocks(); bb; bb++) {

		// remove non-useful blocks
		if(bb->isEnd())
			continue;
		if(!(IS_ON_FORWARD_PATH(*bb) && IS_ON_BACKWARD_PATH(*bb)))
			continue;

		// build the new basic block
		CFGTransformer::transform(*bb);
		//bbs.put(bb, vbb);
		orgs.add(bb);
	}

	// build the virtual edges
	for(FragTable<Block *>::Iter src(orgs); src; src++) {
		Block *vsrc = get(src);

		// manage start
		if(IS_START(src)) {
			build(maker.entry(), vsrc, 0);
			src->removeProp(IS_START);
		}

		// manage stop
		if(IS_STOP(src)) {
			build(vsrc, maker.exit(), 0);
			src->removeProp(IS_STOP);
			continue;
		}

		// manage successors
		for(Block::EdgeIter edge = src->outs(); edge; edge++)
			if(orgs.contains(edge->sink())){
				Block *vtarget = get(edge->target());
				build(vsrc, vtarget, 0);
			}
	}
}


/**
 * Configuration of @ref otawa::SubCFGBuilder specifying the start of the sub-CFG
 * to process. If not provided, the start is assumed to be the ENTRY of the CFG.
 */
Identifier<Address> CFG_START("otawa::CFG_START");


/**
 * Configuration of @ref otawa::SubCFGBuilder specifying a end of the sub-CFG
 * to process. Several properties of this type are accepted.
 * If not provided, the stop is assumed to be the EXIT of the original CFG.
 */
Identifier<Address> CFG_STOP("otawa::CFG_STOP");

}	// otawa

