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

#include <otawa/cfg/features.h>
#include <otawa/pcg/PCG.h>

namespace otawa {

/**
 * @defgroup pcg Program Call Graph
 *
 * This module is built upon the @ref cfg group and provides a high-level
 * program representation of the CFG calls.
 *
 * A Program Call Graph (PCG) is a directed graph where vertices
 * (@ref PCGBlock) are the CFG and edges exists between vertex F and vertex G
 * if F calls G. If F contains several calls to G, the PCG will contain
 * as many edges. The PCG has a unique vertex without predecessors that
 * corresponds to the entry function of the task.
 *
 * Beside the global synthetic view provided by the PCG, they provides
 * some facilities as recursivity analysis (@ref RECURSIVITY_ANALYSIS).
 */

/**
 * @class PCG
 * A PCG (Program Call Graph) provides the call structure of the program.
 *
 * @ingroup pcg
 */

/**
 * @class PCGBlock
 * Represents a function in the CFG.
 *
 * @ingroup pcg
 */

/**
 * Build a PCG block. The cfg parameter can be null creating an undefined
 * PCG block. This may represent a missing function (partial link edition)
 * or a function that is voluntary kept out of the analysis.
 * @param cfg	CFG in the PCG (or null pointer).
 */
PCGBlock::PCGBlock(CFG *cfg): _cfg(cfg) {
}

/**
 */
PCGBlock::~PCGBlock(void) {
}

/**
 * Test if the function is undefined.
 * @return	Return true if the function is undefined, false else.
 */
bool PCGBlock::isUndef() const {
	return _cfg == nullptr;
}

/**
 * Get the address of the called function.
 * @return	Function address or null pointer if the function is undefined.
 */
address_t PCGBlock::address() const {
	if(_cfg == nullptr)
		return Address::null;
	else
		return _cfg->address();
}

/**
 * Get the name of the called function or "<undef>" for undefined call.
 * @return	Function name.
 */
String PCGBlock::name() const {
	if(_cfg == nullptr)
		return "<undef>";
	else
		return _cfg->label();
}

/**
 * Get the CFG corresponding to the current call. This CFG may be a null
 * pointer for an undefined function.
 * @return	Corresponding CFG.
 */
CFG *PCGBlock::cfg() const {
	return _cfg;
}


/**
 * @class PCFGEdge
 * Represents a call in the CFG.
 *
 * @ingroup cfg
 */

/**
 */
PCGEdge::PCGEdge(SynthBlock *block): _block(block) {
}

/**
 * @fn SynthBlock *PCGEdge::call(void) const;
 * Get the synthetic block representing the call.
 * @return	Corresponding synthetic block.
 */

/**
 * @fn PCGBlock *PCGEdge::caller(void) const;
 * Get the caller subprogram.
 * @return	Caller sub-program.
 */

/**
 * @fn PCGBlock *PCGEdge::callee(void) const;
 * Get the callee subprogram.
 * @return Callee subprogram.
 */


///
io::Output& operator<<(io::Output& out, PCGBlock *b) {
	out << "C(";
	if(b->isUndef())
		out << b->getName();
	else {
		if(CONTEXT(b->cfg()).exists())
			out << *CONTEXT(b->cfg()) << "/";
		out << b->getName();
	}
	out << ")";
	return out;
}

///
io::Output& operator<<(io::Output& out, PCGEdge *b) {
	out << b->source() << " -> " << b ->sink();
	return out;
}

}	// otawa
