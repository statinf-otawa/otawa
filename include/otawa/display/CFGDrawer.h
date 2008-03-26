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
protected:
	CFG *_cfg;
	Graph *_graph;
	bool _made;
	virtual void onInit(PropList& graph, PropList& nodes, PropList& edges);
	virtual void onNode(otawa::BasicBlock *bb, otawa::display::Node *node);
	virtual void onEdge(otawa::Edge *cfg_edge, otawa::display::Edge *display_edge);
	virtual void onCall(CFG *cfg, display::Node *node);
	virtual void onEnd(otawa::display::Graph *graph);
	virtual void make();
	
public:
	CFGDrawer(CFG *cfg, Graph *graph);
	CFGDrawer(CFG *cfg, const PropList& props = PropList::EMPTY, Driver& driver = graphviz_driver);
	virtual ~CFGDrawer(void) { }
	virtual void display();
};

} }

#endif /*OTAWA_DISPLAY_CFGDRAWER_H*/
