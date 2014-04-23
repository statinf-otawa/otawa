/*
 *	dcache::WCETFunctionBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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

#include <otawa/dcache/WCETFunctionBuilder.h>
#include <otawa/hard/Memory.h>
#include <otawa/ipet.h>

namespace otawa { namespace dcache {

/**
 * @class WCETFunctionBuilder
 * Default implementation of feature @ref WCET_FUNCTION_FEATURE.
 *
 * @par Provided Features
 * @li @ref WCET_FUNCTION_FEATURE
 *
 * @par Required Features
 * @li @ref CONSTRAINTS_FEATURE
 */

p::declare WCETFunctionBuilder::reg = p::init("otawa::dcache::WCETFunctionBuilder", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<WCETFunctionBuilder>()
	.provide(WCET_FUNCTION_FEATURE)
	.require(CONSTRAINTS_FEATURE);

/**
 */
WCETFunctionBuilder::WCETFunctionBuilder(p::declare& r): BBProcessor(r), sys(0), mem(0), worst_read(0), worst_write(0) {
}


/**
 */
void WCETFunctionBuilder::setup(WorkSpace *ws) {

	// get information
	sys = ipet::SYSTEM(ws);
	ASSERT(sys);
	mem = hard::MEMORY(ws);
	ASSERT(mem);

	// look for worst times
	worst_read = mem->worstReadAccess();
	worst_write = mem->worstWriteAccess();
}


/**
 */
void WCETFunctionBuilder::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	if(bb->isEnd())
		return;

	// process each access
	Pair<int, BlockAccess *> blocks = DATA_BLOCKS(bb);
	for(int i = 0; i < blocks.fst; i++) {

		// get variable
		ilp::Var *var = dcache::MISS_VAR(blocks.snd[i]);
		ASSERT(var);

		// reador write
		bool write;
		switch(blocks.snd[i].action()) {
		case BlockAccess::LOAD:		write = false; break;
		case BlockAccess::STORE:	write = true; break;
		default:					continue;
		}

		// compute latency
		const hard::Bank *bank;
		ot::time lat;
		switch(blocks.snd[i].kind()) {
		case BlockAccess::ANY:
			lat = write ? worst_write : worst_read;
			break;
		case BlockAccess::BLOCK:
			bank = mem->get(blocks.snd[i].block().address());
			ASSERT(bank);
			lat = write ? bank->writeLatency() : bank->latency();
			break;
		case BlockAccess::RANGE:
			bank = mem->get(blocks.snd[i].first());
			ASSERT(bank);
			lat = write ? bank->writeLatency() : bank->latency();
			break;
		default:
			ASSERT(0);
			break;
		}

		// add x_i^miss * t^miss to the objective function
		sys->addObjectFunction(lat, var);
	}
}


/**
 * This feature provide an easy and naive to add data cache time to the WCET computation
 * by adding to the objectif function representing the WCET the miss time multiplied
 * by the number of occurences of the miss.
 */
p::feature WCET_FUNCTION_FEATURE("otawa::dcache::WCET_FUNCTION_FEATURE", new Maker<WCETFunctionBuilder>());

} }	// otawa::dcache


