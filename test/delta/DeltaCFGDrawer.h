/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	DeltaCFGDrawer.h -- DeltaCFGDrawer class interface.
 */
#ifndef DELTACFGDRAWER_H
#define DELTACFGDRAWER_H

#include <otawa/display/CFGDrawer.h>

namespace otawa { namespace display {

class DeltaCFGDrawer: public CFGDrawer{
protected:
	virtual void onEdge(otawa::Edge *cfg_edge, otawa::display::Edge *display_edge);
	virtual void onEnd(otawa::display::Graph *graph);
public:
	DeltaCFGDrawer(CFG *cfg, const PropList& props = PropList::EMPTY);
};

} }

#endif /*DELTACFGDRAWER_H*/
