/*
 *	icache features
 *	Copyright (c) 2016, IRIT UPS.
 *
 *	This file is part of OTAWA
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <otawa/icache/features.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg/features.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Memory.h>

namespace otawa { namespace icache {

using namespace elm;


/**
 * @class Access
 * Represents an access to the instruction cache. This access may be
 * classified:
 * @li NONE - convenient value representing no access,
 * @li FETCH - a cache block is actually accessed,
 * @li PREFETCH - a cache is possibly accessed (result of a speculative prefetch policy).
 *
 * For both FETCH and PREFETCH, the @ref address() is meaningful while for NONE,
 * it is always null.
 *
 * To get the the access of a program, the feature @ref ACCESSES_FEATURE must be required.
 *
 * @ingroup icache
 */


io::Output& operator<<(io::Output& out, const Access& acc) {
	static cstring labels[] = {
		"no access",
		"fetch",
		"prefetch"
	};
	out << labels[acc.kind()];
	if(acc.kind() != NONE)
		out << " to " << acc.address() << " from I(" << acc.instruction()->address() << ")";
	return out;
}


/**
 */
class AccessBuilder: public BBProcessor {
public:
	static p::declare reg;
	AccessBuilder(void): BBProcessor(reg), icache(0), mem(0) { }

protected:

	void setup(WorkSpace *ws) override {
		const hard::CacheConfiguration *conf = hard::CACHE_CONFIGURATION_FEATURE.get(ws);
		ASSERT(conf);
		icache = conf->instCache();
		if(!icache && logFor(LOG_FUN))
			log << "\tno instruction cache available; nothing to do.\n";
		mem = hard::MEMORY_FEATURE.get(ws);
		ASSERT(mem);
	}

	void processWorkSpace(WorkSpace *ws) override {
		if(icache)
			BBProcessor::processWorkSpace(ws);
	}

	void processBB(WorkSpace *ws, CFG *cfg, Block *b) override {
		if(!b->isBasic())
			return;
		BasicBlock *bb = b->toBasic();

		// is it cached (assuming a BB does not span over several banks)?
		const hard::Bank *bank = mem->get(bb->address());
		if(!bank->isCached()) {
			warn(_ << "no bank contains " << bb);
			return;
		}

		// split the BB in block accesses
		// ASSUMPTION: a bundle cannot be bigger than 1 cache block!
		BasicBlock::BundleIter i(bb);
		Address last_block;
		Inst *last_bundle = 0;
		BasicBlock::BundleIter bundle(bb);

		// first bundle of the block (no last block)
		if(bundle()) {
			// First bundle access
			accs.add(icache::Access(icache::FETCH, (*bundle).first(), (*bundle).address()));
			last_block = icache->round((*bundle).address());
			last_bundle = (*bundle).first();
			bundle++;
			if(logFor(LOG_BLOCK))
				log << "\t\t\t" << accs.top() << io::endl;
		}

		// process following bundle
		for(; bundle(); ++bundle) {

			// Last bundle crossed cache block boundary
			if(icache->round((*bundle).address() - 1) != last_block) {
				last_block = icache->round((*bundle).address() - 1);
				accs.add(icache::Access(icache::FETCH, last_bundle, last_block));
				if (logFor(LOG_BLOCK))
					log << "\t\t\t" << accs.top() << io::endl;
			}

			// Bundle starts in a new block
			if(icache->round((*bundle).address()) != last_block) {
				last_block = icache->round((*bundle).address());
				accs.add(icache::Access(icache::FETCH, (*bundle).first(), last_block));
				if (logFor(LOG_BLOCK))
					log << "\t\t\t" << accs.top() << io::endl;
			}

			last_bundle = (*bundle).first();
		}

		// Last bundle of the BB crossed cache block boundary
		if(last_bundle && icache->round(bb->topAddress() - 1) != last_block) {
			accs.add(icache::Access(icache::FETCH, last_bundle, icache->round(bb->last()->address())));
			if (logFor(LOG_BLOCK))
				log << "\t\t\t" << accs.top() << io::endl;
		}

		// build the property
		ACCESSES(bb) = accs;
		accs.clear();
	}

private:
	const hard::Cache *icache;
	const hard::Memory *mem;
	Vector<Access> accs;
};

p::declare AccessBuilder::reg = p::init("otawa::icache::AccessBuilder", Version(1, 0, 0))
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.provide(ACCESSES_FEATURE)
	.provide(BLOCK_ACCESSES_FEATURE)
	.make<AccessBuilder>()
	.extend<BBProcessor>();


/**
 */
class EdgeAccessBuilder: public BBProcessor {
public:
	static p::declare reg;
	EdgeAccessBuilder(void): BBProcessor(reg), icache(0), mem(0) { }

protected:

	void setup(WorkSpace *ws) override {
		const hard::CacheConfiguration *conf = hard::CACHE_CONFIGURATION_FEATURE.get(ws);
		ASSERT(conf);
		icache = conf->instCache();
		if(!icache && logFor(LOG_FUN))
			log << "\tno instruction cache available; nothing to do.\n";
		mem = hard::MEMORY_FEATURE.get(ws);
		ASSERT(mem);
	}

	void processWorkSpace(WorkSpace *ws) override {
		if(icache)
			BBProcessor::processWorkSpace(ws);
	}

	void processBB(WorkSpace *ws, CFG *cfg, Block *b) override {
		if(!b->isBasic())
			return;
		BasicBlock *bb = b->toBasic();

		// is it cached?
		// ASSUMPTION: a BB does not span over several banks
		const hard::Bank *bank = mem->get(bb->address());
		if(!bank->isCached()) {
			warn(_ << "no bank contains " << bb);
			return;
		}

		// split the BB in block accesses
		// ASSUMPTION: a bundle cannot be bigger than 1 cache block!
		Address last_block;
		Inst *last_bundle = 0;
		BasicBlock::BundleIter bundle(bb);

		// first bundle of the block (no last block)
		if(bundle()) {
			// First bundle access
			accs.add(icache::Access(icache::FETCH, (*bundle).first(), (*bundle).address()));
			last_block = icache->round((*bundle).address());
			last_bundle = (*bundle).first();
			bundle++;
			if(logFor(LOG_BLOCK))
				log << "\t\t\t" << accs.top() << io::endl;
		}

		// process following bundle
		for(; bundle(); ++bundle) {

			// Last bundle crossed cache block boundary
			if(icache->round((*bundle).address() - 1) != last_block) {
				last_block = icache->round((*bundle).address() - 1);
				accs.add(icache::Access(icache::FETCH, last_bundle, last_block));
				if (logFor(LOG_BLOCK))
					log << "\t\t\t" << accs.top() << io::endl;
			}

			// Bundle starts in a new block
			if(icache->round((*bundle).address()) != last_block) {
				last_block = icache->round((*bundle).address());
				accs.add(icache::Access(icache::FETCH, (*bundle).first(), last_block));
				if (logFor(LOG_BLOCK))
					log << "\t\t\t" << accs.top() << io::endl;
			}

			last_bundle = (*bundle).first();
		}

		// Last bundle of the BB crossed cache block boundary
		if(last_bundle && icache->round(bb->topAddress() - 1) != last_block) {
			accs.add(icache::Access(icache::FETCH, last_bundle, icache->round(bb->last()->address())));
			if (logFor(LOG_BLOCK))
				log << "\t\t\t" << accs.top() << io::endl;
		}

		// build the property
		for(Block::EdgeIter e = bb->ins(); e(); e++)
			ACCESSES(*e) = accs;
		accs.clear();
	}

private:
	const hard::Cache *icache;
	const hard::Memory *mem;
	Vector<Access> accs;
};

p::declare EdgeAccessBuilder::reg = p::init("otawa::icache::EdgeAccessBuilder", Version(1, 0, 0))
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.provide(ACCESSES_FEATURE)
	.provide(EDGE_ACCESSES_FEATURE)
	.make<EdgeAccessBuilder>()
	.extend<BBProcessor>();


/**
 * This feature ensures that information about the instruction
 * cache behavior has been added to the CFG using
 * @ref ACCESSES property.
 *
 * @par Properties
 * @li @ref ACCESSES
 */
p::feature ACCESSES_FEATURE("otawa::icache::ACCESSES_FEATURE", AccessBuilder::reg);



/**
 * This feature ensures that information about the instruction
 * cache behavior has been added to the CFG using
 * @ref ACCESSES property located on the edges.
 *
 * This feature implies the avaiability of @ref ACCESSES_FEATURE.
 *
 * @par Properties
 * @li @ref ACCESSES
 */
p::feature EDGE_ACCESSES_FEATURE("otawa::icache::EDGE_ACCESSES_FEATURE", EdgeAccessBuilder::reg);


/**
 * This feature ensures that information about the instruction
 * cache behavior has been added to the CFG using
 * @ref ACCESSES property located on the blocks.
 *
 * This feature implies the avaiability of @ref ACCESSES_FEATURE.
 *
 * @par Properties
 * @li @ref ACCESSES
 */
p::feature BLOCK_ACCESSES_FEATURE("otawa::icache::BLOCK_ACCESSES_FEATURE", AccessBuilder::reg);



/**
 * This property defines the instruction caches performed by a basic block
 * or when an edge is traversed (mainly to support prefetching existing on some
 * microprocessors).
 *
 * @par Hooks
 * @li @ref BasicBlock -- accessed memories.
 * @li @ref Edge -- to support prefetching before a branch is performed.
 */
p::id<Bag<Access> > ACCESSES("otawa::icache::ACCESSES");

} }	// otawa::icache
