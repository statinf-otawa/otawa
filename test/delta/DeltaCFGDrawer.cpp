/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	DeltaCFGDrawer.cpp -- DeltaCFGDrawer class implementation.
 */
#include "DeltaCFGDrawer.h"
#include <otawa/ipet/Delta.h>
#include <elm/genstruct/Vector.h>

using namespace otawa::ipet;
using namespace elm::genstruct;

namespace otawa { namespace display {

DeltaCFGDrawer::DeltaCFGDrawer(CFG *cfg, const PropList& props)
: CFGDrawer(cfg, props){
}


void DeltaCFGDrawer::onEdge(otawa::Edge *cfg_edge, otawa::display::Edge *display_edge){
	Vector<BasicBlock*> bbs;
	bbs.add(cfg_edge->source());
	bbs.add(cfg_edge->target());
	BBPath *bbpath = BBPath::getBBPath(&bbs);
	if(bbpath->hasProp(Delta::DELTA)){
		int delta = Delta::DELTA(bbpath);
		Delta::DELTA(cfg_edge) = delta;
	}
	CFGDrawer::onEdge(cfg_edge,display_edge);
}


void DeltaCFGDrawer::onEnd(otawa::display::Graph *graph){
	
}


} }
