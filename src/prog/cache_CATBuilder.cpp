/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/cache_CATBuilder.cpp -- CATBuilder class implementation.
 */
#include <stdio.h>
#include <elm/io.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/instruction.h>
#include <otawa/cache/LBlock.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/hard/Platform.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;

namespace otawa {

/**
 */
Identifier CATBuilder::ID_NonConflict("ipet.cat.nonconflict");


/**
 */
Identifier CATBuilder::ID_Node("ipet.cat.node");


/**
 */
Identifier CATBuilder::ID_HitVar("ipet.cat.hit_var");


/**
 */
Identifier CATBuilder::ID_MissVar("ipet.cat.miss_var");


/**
 */
Identifier CATBuilder::ID_BBVar("ipet.cat.bb_var");


/**
 */
void CATBuilder::processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset) {	
	assert(fw);
	assert(cfg);
	assert(lbset);

	// Get some information
	System *system = getSystem(fw, cfg);
	
	// LBlock initialization
	for(LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {
	
		// Set a node
		lblock->add(ID_Node, new CCGNode(lblock));
		BasicBlock *bb = lblock->bb();
		if(!bb)
			continue;

		// Link BB variable
		ilp::Var *bbvar = bb->use<ilp::Var *>(VAR);
		lblock->add(ID_BBVar, bbvar);
		
		// Create x_hit variable
		ilp::Var *vhit;
		if(!_explicit)
			vhit = system->newVar();
		else {
			StringBuffer buf;
			buf << "xhit" << lblock->address() << "(" << bb->number() << ")";
			String namex = buf.toString();
			vhit = system->newVar(namex);
		}
		lblock->add(ID_HitVar, vhit);
		
		// Create x_miss variable
		ilp::Var *miss;
		if(!_explicit)
			miss = system->newVar();
		else {
			StringBuffer buf1;
			buf1 << "xmiss" << lblock->address() << "(" << bb->number() << ")";
			String name1 = buf1.toString();
			miss = system->newVar(name1);
		}
		lblock->add(ID_MissVar, miss);
	}
}


/**
 */
void CATBuilder::processCFG(FrameWork *fw, CFG *cfg) {
	assert(fw);
	assert(cfg);
	
	// Check the cache
	const hard::Cache *cache = fw->platform()->cache().instCache();
	if(!cache)
		return;
	
	// Get the l-block sets
	LBlockSet **lbsets = LBLOCKS(fw);
	if(!lbsets) {
		LBlockBuilder builder;
		builder.process(fw);
		lbsets = LBLOCKS(fw);
	}
		
	// Process the l-block sets
	for(int i = 0; i < cache->lineCount(); i++)
		processLBlockSet(fw, cfg, lbsets[i]);
}


/**
 * Create a new CATBuilder processor.
 * @param props		Configuration properties.
 */
CATBuilder::CATBuilder(const PropList& props)
: CFGProcessor("CATBuilder", Version(1, 0, 0), props), _explicit(false) {
	initialize(props);
}


/**
 * Initialize the processor.
 * @param props		Configuration properties.
 */
void CATBuilder::initialize(const PropList& props) {
	_explicit = props.get<bool>(EXPLICIT, false);
}


/**
 */
void CATBuilder::configure(const PropList& props) {
	CFGProcessor::configure(props);
	initialize(props);
}

} // otawa




