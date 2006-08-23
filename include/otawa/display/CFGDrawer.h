/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/display/CFGDrawer.h -- display::CFGDrawer class interface.
 */
#ifndef OTAWA_DISPLAY_CFGDRAWER_H
#define OTAWA_DISPLAY_CFGDRAWER_H

#include <otawa/cfg.h>
#include <otawa/display/Driver.h>
#include <otawa/display/graphviz.h>

namespace otawa { namespace display {

class CFGDrawer {
	virtual void make(CFG *cfg);
protected:
	Graph *_graph;
	virtual void onInit(PropList& graph, PropList& nodes, PropList& edges);
	virtual void onNode(otawa::BasicBlock *bb, otawa::display::Node *node);
	virtual void onEdge(otawa::Edge *cfg_edge, otawa::display::Edge *display_edge);
	
public:
	CFGDrawer(CFG *cfg, Graph *graph);
	CFGDrawer(CFG *cfg, const PropList& props = PropList::EMPTY, Driver& driver = graphviz_driver);
	virtual void display();
};

} }

#endif /*OTAWA_DISPLAY_CFGDRAWER_H*/
