/*
 *	PCG class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003, IRIT UPS.
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

#include <otawa/pcg/PCG.h>

namespace otawa {

/**
 * @class PCG
 * A PCG (Program Call Graph) provides the call structure of the program.
 *
 * @ingroup cfg
 */

/**
 * Build a new PCG.
 * @param entry		Entry function.
 */
PCG::PCG(PCGBlock *entry): _entry(entry) {
}


/**
 * @class PCGBlock
 * Represents a function in the CFG.
 *
 * @ingroup cfg
 */

/**
 */
PCGBlock::PCGBlock(CFG *cfg): _cfg(cfg) {
}

/**
 */
PCGBlock::~PCGBlock(void) {
}


/**
 * @class PCFGEdge
 * Represents a call in the CFG.
 *
 * @ingroup cfg
 */

/**
 */
PCGEdge::PCGEdge(PCGBlock *caller, SynthBlock *block, PCGBlock *callee)
: graph::GenGraph<PCGBlock, PCGEdge>::GenEdge(caller, callee), _block(block) {
}

}	// otawa
