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
: CFGTransformer(reg), _start(location_t(nullptr, nullptr)) /*maker(0), cfg(0)*/ {
}

/**
 */
p::declare SubCFGBuilder::reg = p::init("otawa::SubCFGBuilder", Version(3, 0, 0))
	.require(COLLECTED_CFG_FEATURE)
	.base(CFGTransformer::reg)
	.maker<SubCFGBuilder>();


// Internal work
static p::id<bool> IS_START("", false);
static p::id<bool> IS_STOP("", false);
static p::id<Inst *> ADDR("");
static p::id<bool> IS_ON_FORWARD_PATH("", false);
static p::id<bool> IS_ON_BACKWARD_PATH("", false);


/**
 */
io::Output& operator<<(io::Output& out, const location_t& loc) {
	out << loc.snd->address() << " (BB " << loc.fst->index() << ")";
	return out;
}


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

	// look for start
	_start = otawa::LOCATION_START(props);
	if(_start.fst == nullptr)
		_start_addr = CFG_START(props);

	// look for stops
	_stops.clear();
	_stop_addrs.clear();
	for(Identifier<location_t>::Getter stop(props, LOCATION_STOP); stop(); stop++)
		_stops.add(*stop);
	for(Identifier<Address>::Getter stop(props, CFG_STOP); stop(); stop++)
		_stop_addrs.add(*stop);
}

/**
 * Flood forward start reachable marker.
 */
void SubCFGBuilder::floodForward(void) {
	VectorQueue<Block *> todo;
	todo.put(_start.fst);
	while(todo) {
		Block *bb = todo.get();
		IS_ON_FORWARD_PATH(*bb) = true;
		if(!IS_STOP(*bb))
			for(BasicBlock::EdgeIter next = bb->outs(); next(); next++) {
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
	for(auto stop = *_stops; stop(); stop++)
		todo.put((*stop).fst);
	while (todo){
		Block *bb = todo.get();
		IS_ON_BACKWARD_PATH(*bb) = true;
		if(!IS_START(*bb))
			for(Block::EdgeIter prev = bb->ins(); prev(); prev++){
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
	if(!_start_addr.isNull()) {
		bool done = false;
		for(auto b = cfg->blocks(); b(); b++) {
			if(!b->isBasic())
				continue;
			BasicBlock *bb = b->toBasic();
			if(bb->address() <= _start_addr && _start_addr < bb->topAddress()) {
				for(auto i = bb->insts(); i(); i++)
					if(i->address() <= _start_addr && _start_addr < i->topAddress()) {
						_start = location_t(bb, *i);
						done = true;
						break;
					}
				if(done)
					break;
			}
		}
		if(!done)
			throw ProcessorException(*this, _ << "cannot the start address " << _start_addr);
	}
	else if(_start.fst == nullptr)
		_start = location_t(cfg->entry()->outs()->sink()->toBasic(), cfg->first());
	IS_START(_start.fst) = true;
	if(isVerbose())
		log << "\tstart at " << _start << io::endl;

	// record the stop tags
	if(_stop_addrs) {
		for(auto b = cfg->blocks(); b(); b++) {
			if(!b->isBasic())
				continue;
			BasicBlock *bb = b->toBasic();
			for(auto stop = *_stop_addrs; stop(); stop++)
				if(bb->address() <= *stop && *stop < bb->topAddress()) {
					for(auto i = bb->insts(); i(); i++)
						if(i->address() <= *stop && *stop < bb->topAddress()) {
							_stops.add(location_t(bb, *i));
							break;
						}
					break;
				}
			}
	}
	else if(!_stops)
		for(auto e = cfg->exit()->ins(); e(); e++)
			_stops.add(location_t(e->source()->toBasic(), e->source()->toBasic()->last()));
	if(!_stops)
		throw ProcessorException(*this, "cannot find any stop address!");
	for(auto stop = *_stops; stop(); stop++) {
		IS_STOP((*stop).fst) = true;
		ADDR((*stop).fst) = (*stop).snd;
		if(isVerbose())
			log << "\tstop at " << *stop << io::endl;
	}

	// start flood analysis
	floodForward();
	floodBackward();

	// make all virtual BB
	FragTable<Block *> orgs;
	for(CFG::BlockIter v = cfg->blocks(); v(); v++) {

		// is the block contained in sub-CFG?
		if(v->isEnd())
			continue;
		if(!(IS_ON_FORWARD_PATH(*v) && IS_ON_BACKWARD_PATH(*v)))
			continue;

		// cutting required?
		if(v->isBasic() && (IS_START(*v) || IS_STOP(*v))) {
			BasicBlock *bb = v->toBasic();

			// determine start address
			location_t start(bb, bb->first());
			if(IS_START(*v))
				start = _start;

			// determine stop address
			location_t stop(bb, bb->last());
			if(IS_STOP(bb))
				stop.snd = ADDR(bb);

			log << "DEBUG: splitting " << start << " - " << stop << io::endl;

			// build the split block
			Vector<Inst *> is;
			auto i = bb->insts();
			while(*i != start.snd) {
				ASSERT(i);
				i++;
			}
			while(*i != stop.snd) {
				ASSERT(i());
				is.add(*i);
				i++;
			}
			is.add(*i);
			BasicBlock *vbb = CFGTransformer::build(is.detach());
			map(bb, vbb);
			orgs.add(*v);
		}

		// just duplicate
		else {
			CFGTransformer::transform(*v);
			orgs.add(*v);
		}
	}

	// build the virtual edges
	for(FragTable<Block *>::Iter src(orgs); src(); src++) {
		Block *vsrc = get(*src);

		// manage start
		if(IS_START(*src)) {
			build(maker.entry(), vsrc, 0);
			IS_START(*src).remove();
		}

		// manage stop
		if(IS_STOP(*src)) {
			build(vsrc, maker.exit(), 0);
			IS_STOP(*src).remove();
			ADDR(*src).remove();
			continue;
		}

		// manage successors
		for(Block::EdgeIter edge = src->outs(); edge(); edge++)
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
 *	* @ref otawa::CFG_START
 *	* @ref otawa::LOCATION_START
 *	* @ref otawa::CFG_STOP (several allowed)
 *	* @ref otawa::LOCATION_STOP (several allowed)
 *
 * Only one otawa::CFG_START or otawa::LOCATION_START can be defined.
 *
 * @par Default processor
 * 	* @ref otawa::SubCFGBuilder
 *
 * @ingroup cfg
 */
p::feature SPLIT_CFG("otawa::SPLIT_CFG", p::make<SubCFGBuilder>());


/**
 * Provide the start address of a sub-CFG to extract (inclusive).
 * Only one start address can be given. If not provided, the start is assumed to be
 * the first address of the CFG.
 *
 * @par Feature
 * 	* otawa::SPLIT_CFG
 *
 * @ingroup cfg
 */
p::id<Address> CFG_START("otawa::CFG_START");

/**
 * Provide the stop address of a sub-CFG to extract (exclusive).
 * Several stop address can be given. If not provided, the stop is assumed to be the last address
 * of the blocks preceding the exit of the current CFG.
 *
 * @par Feature
 * 	* otawa::SPLIT_CFG
 *
 * @ingroup cfg
 */
p::id<Address> CFG_STOP("otawa::CFG_STOP", false);


/**
 * Provide the start location of a sub-CFG to extract (inclusive).
 * Only one start location can be given. If not provided, the start
 * location is assumed to be the first address of the CFG.
 *
 * @par Feature
 * 	* otawa::SPLIT_CFG
 *
 * @ingroup cfg
 */
p::id<location_t> LOCATION_START("otawa::LOCATION_START", location_t(nullptr, nullptr));

/**
 * Provide the stop location of a sub-CFG to extract (inclusive).
 * Several stop locations can be given. If not provided, the stop location
 * is assumed to be the last instruction of the blocks preceding the exit
 * of the current CFG.
 *
 * @par Feature
 * 	* otawa::SPLIT_CFG
 *
 * @ingroup cfg
 */
p::id<location_t> LOCATION_STOP("otawa::LOCATION_STOP", location_t(nullptr, nullptr));

}	// otawa
