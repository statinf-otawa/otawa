/*
 *	SubCFGBuilder analyzer implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009-18, IRIT UPS.
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
p::declare SubCFGBuilder::reg = p::init("otawa::SubCFGBuilder", Version(3, 0, 0))
	.require(COLLECTED_CFG_FEATURE)
	.base(CFGTransformer::reg)
	.maker<SubCFGBuilder>();


/**
 * Provide the start address of a sub-CFG to extract (inclusive).
 * Only one start address can be given. If not provided, the start is assumed to be
 * the first address of the CFG.
 *
 * In configuration of, @ref otawa::SubCFGBuilder.
 *
 * @ingroup cfg
 */
p::id<Address> CFG_START("otawa::CFG_START");

/**
 * Provide the stop address of a sub-CFG to extract (exclusive).
 * Several stop address can be given. If not provided, the stop is assumed to be the last address
 * of the blocks preceding the exit of the current CFG.
 *
 * In configuration of, @ref otawa::SubCFGBuilder.
 *
 * @ingroup cfg
 */
p::id<Address> CFG_STOP("otawa::CFG_STOP", false);


// Internal work
static p::id<bool> IS_START("", false);
static p::id<bool> IS_STOP("", false);
static p::id<Address> ADDR("");
static p::id<bool> IS_ON_FORWARD_PATH("", false);
static p::id<bool> IS_ON_BACKWARD_PATH("", false);


/**
 * @class SubCFGBuilder
 * Build a sub-CFG starting at the given @ref otawa::CFG_START address and
 * ending at the multiple @ref otawa::CFG_STOP addresses.
 *
 * Note that, in this version, @ref otawa::CFG_START and @ref otawa::CFG_STOP addresses must be part
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
	VectorQueue<Block *> todo;
	todo.put(_start_bb);
	while(todo) {
		Block *bb = todo.get();
		IS_ON_FORWARD_PATH(*bb) = true;
		if(!IS_STOP(*bb))
			for(BasicBlock::EdgeIter next = bb->outs(); next; next++) {
				Block *nextbb = next->sink();
				if(!IS_ON_FORWARD_PATH(*nextbb) && !todo.contains(nextbb))
					todo.put(nextbb);
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
		if(!IS_START(*bb))
			for(Block::EdgeIter prev = bb->ins(); prev ; prev++){
				Block *prevbb = prev->source();
				if (!IS_ON_BACKWARD_PATH(*prevbb) && !todo.contains(prevbb))
					todo.put(prevbb);
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

	// record the start tag
	if(start.isNull()) {
		_start_bb = cfg->entry()->outs()->sink();
		start = _start_bb->address();
	}
	else {
		bool done = false;
		for(auto b = cfg->blocks(); b; b++) {
			if(!b->isBasic())
				continue;
			BasicBlock *bb = b->toBasic();
			if(bb->address() <= start && start < bb->address() + bb->size()) {
				_start_bb = bb;
				done = true;
				break;
			}
		}
		if(!done)
			throw ProcessorException(*this, "cannot find the start address!");
	}
	IS_START(_start_bb) = true;
	if(isVerbose())
		log << "\tstart at " << start << io::endl;

	// record the stop tags
	if(!stops)
		for(auto prev = cfg->exit()->ins(); prev; prev++) {
			IS_STOP(prev->source()) = true;
			if(prev->source()->isBasic()) {
				BasicBlock *bb = prev->source()->toBasic();
				ADDR(bb) = bb->topAddress();
				stops.add(bb->topAddress());
			}
			_stop_bbs.add(prev->source());
		}
	else {
		bool done = false;
		for(auto b = cfg->blocks(); b; b++) {
			if(!b->isBasic())
				continue;
			BasicBlock *bb = b->toBasic();
			for(auto stop = *stops; stop; stop++)
				if(bb->address() <= *stop && *stop < bb->address() + bb->size()) {
					IS_STOP(bb) = true;
					ADDR(bb) = stop;
					_stop_bbs.add(bb);
					done = true;
					break;
				}
		}
		if(!done)
			throw ProcessorException(*this, "cannot find any stop address!");
	}
	if(isVerbose())
		for(auto a = *stops; a; a++)
			log << "\tstop at " << *a << io::endl;

	// start flood analysis
	floodForward();
	floodBackward();

	// make all virtual BB
	FragTable<Block *> orgs;
	for(CFG::BlockIter v = cfg->blocks(); v; v++) {

		// is the block contained in sub-CFG?
		if(v->isEnd())
			continue;
		if(!(IS_ON_FORWARD_PATH(v) && IS_ON_BACKWARD_PATH(v)))
			continue;

		// cutting required?
		if(v->isBasic() && (IS_START(v) || IS_STOP(v))) {
			BasicBlock *bb = v->toBasic();

			// determine start address
			Address start_addr = bb->address();
			if(IS_START(bb))
				start_addr = start;

			// determine stop address
			Address stop_addr = ADDR(bb);
			if(stop_addr.isNull())
				stop_addr = bb->topAddress();

			// build the split block
			Vector<Inst *> is;
			auto i = bb->insts();
			while(i->address() < start_addr)
				i++;
			while(i && i->address() < stop_addr) {
				is.add(i);
				i++;
			}
			BasicBlock *vbb = CFGTransformer::build(is.detach());
			map(bb, vbb);
			orgs.add(v);
		}

		// just duplicate
		else {
			CFGTransformer::transform(v);
			orgs.add(v);
		}
	}

	// build the virtual edges
	for(FragTable<Block *>::Iter src(orgs); src; src++) {
		Block *vsrc = get(src);

		// manage start
		if(IS_START(src)) {
			build(maker.entry(), vsrc, 0);
			IS_START(src).remove();
		}

		// manage stop
		if(IS_STOP(src)) {
			build(vsrc, maker.exit(), 0);
			IS_STOP(src).remove();
			ADDR(src).remove();
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
 * This feature ensure that a part of the current has been extracted
 * as a sub-CFG based on address provided by two properties:
 *	* @ref otawa::CFG_START (only one allowed)
 *	* @ref otawa::CFG_STOP (several allowed)
 *
 * This addresses must match instructions of the task entry function and
 * all basic blocks on the paths from start addresses to stop addresses
 * are included in the sub-CFG.
 *
 * @par Configuration
 *	* @ref otawa::CFG_START (only one allowed)
 *	* @ref otawa::CFG_STOP (several allowed)
 *
 * @par Default processor
 * 	* @ref otawa::SubCFGBuilder
 *
 * @ingroup cfg
 */
p::feature SPLIT_CFG("otawa::SPLIT_CFG", p::make<SubCFGBuilder>());

}	// otawa
