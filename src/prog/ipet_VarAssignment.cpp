/*
 *	$Id$
 *	VarAssignment class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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

#include <otawa/ipet/IPET.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ilp/Var.h>
#include <otawa/cfg.h>
#include <otawa/proc/Registry.h>
#include <otawa/ilp/System.h>

using namespace elm;
using namespace otawa::ilp;

namespace otawa {

namespace ipet {

// Registration
p::declare VarAssignment::reg = p::init("otawa::ipet::VarAssignment", Version(1, 0, 0))
	.extend<BBProcessor>()
	.require(ILP_SYSTEM_FEATURE)
	.provide(ASSIGNED_VARS_FEATURE)
	.config(EXPLICIT)
	.make<VarAssignment>();

/**
 *
 * Force the ilp variable name of an edge or basic block
 */
Identifier<String* > FORCE_NAME("otawa::FORCE_NAME", 0);


class Cleaner: public BBCleaner {
public:
	Cleaner(WorkSpace *ws): BBCleaner(ws) { }
protected:
	virtual void clean(WorkSpace *ws, CFG *cfg, Block *bb) {

		// remove var of block
		ilp::Var *v = VAR(bb);
		if(v)
			delete v;
		bb->removeProp(VAR);

		// remove var of edges
		for(Block::EdgeIter edge = bb->outs(); edge(); edge++) {
			ilp::Var *v = VAR(*edge);
			if(v)
				delete v;
			edge->removeProp(VAR);
		}
	}
};


/**
 * @class VarAssignment
 * This processor ensures that each basic block and each edge of the CFG
 * has a variable associated with a @ref ipet::VAR annotation.
 *
 * @par Configuration
 * @li @ref ipet::EXPLICIT : use explicit name (takes more to compute but
 * provides more meaningful variable names).
 * @li @ref RECURSIVE : add function names to the explicit variable names.
 *
 * @par Provided Features
 * @li @ref ASSIGNED_VARS_FEATURE
 */


void VarAssignment::setup(WorkSpace *ws) {
	sys = SYSTEM(ws);
}


/**
 */
void VarAssignment::processBB(WorkSpace *ws, CFG *cfg, Block *bb) {

	// Check BB
	if(!VAR(bb)) {
		String name = "";
		if(_explicit) {
		        if (FORCE_NAME(bb)) {
		                name = **FORCE_NAME(bb);
		        } else {
			        name = makeNodeVar(bb, cfg);
                        }
                }
		VAR(bb) = sys->newVar(name);
	}

	// Check out edges
	for(Block::EdgeIter edge = bb->outs(); edge(); edge++) {
		if(!VAR(*edge)) {
			String name = "";
			if(_explicit) {
			        if (FORCE_NAME(*edge)) {
			              name = **FORCE_NAME(*edge);
                                } else {
				      name = makeEdgeVar(*edge, cfg);
                                }
                        }
			VAR(*edge) = sys->newVar(name);
		}
	}

}

void VarAssignment::cleanup(WorkSpace *ws) {
	Cleaner *c = new Cleaner(ws);
	addCleaner(ASSIGNED_VARS_FEATURE, c);
}



/**
 * Build a new variable assignment processor.
 */
VarAssignment::VarAssignment(void)
: BBProcessor(reg), _explicit(false), _recursive(false), sys(0) {
}


/**
 */
void VarAssignment::configure(const PropList& props) {
	BBProcessor::configure(props);
	_explicit = EXPLICIT(props);
	_recursive = otawa::RECURSIVE(props);
}


/**
 * Build a node variable name.
 * @param bb	Basic block to build the variable name for.
 * @param cfg	Owner CFG.
 * @return		Basic block variable name.
 */
String VarAssignment::makeNodeVar(Block *bb, CFG *cfg) {
	ASSERT(bb);
	return _ << 'x' << bb->index() << '_' << cfg->label();
}


/**
 * Build an edge variable name.
 * @param edge	Basic block to build the variable name for.
 * @param cfg	Owner CFG.
 * @return		Basic block variable name.
 */
String VarAssignment::makeEdgeVar(Edge *edge, CFG *cfg) {
	ASSERT(edge);
	return _ 	<< "e"
				<< edge->source()->index()
				<< '_'
				<< edge->target()->index()
				<< '_'
				<< cfg->label();
}


/**
 * This feature asserts that each block and each edge has a variable
 * name asserted.
 *
 * @par Properties
 * @li @ref ipet::VAR
 */
p::feature ASSIGNED_VARS_FEATURE("otawa::ipet::ASSIGNED_VARS_FEATURE", p::make<VarAssignment>());

} } // otawa::ipet
