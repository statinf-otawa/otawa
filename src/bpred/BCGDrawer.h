#ifndef BCGDRAWER_H_
#define BCGDRAWER_H_
#include "BCG.h"
#include <otawa/display/Driver.h>
#include <otawa/display/graphviz.h>

namespace otawa { namespace display {

class BCGDrawer {
protected:
	BCG *_bcg;
	Graph *_graph;
	bool _made;
	virtual void onInit(PropList& graph, PropList& nodes, PropList& edges);
	virtual void onNode(BCGNode *bb, otawa::display::Node *node);
	virtual void onEdge(BCGEdge *bcg_edge, otawa::display::Edge *display_edge);
	virtual void onCall(BCG *bcg, display::Node *node);
	virtual void onEnd(otawa::display::Graph *graph);
	virtual void make();
	
public:
	BCGDrawer(BCG *bcg, Graph *graph);
	BCGDrawer(BCG *bcg, const PropList& props = PropList::EMPTY, Driver& driver = graphviz_driver);
	virtual void display();
};

} } // otawa::display

#endif /*BCGDRAWER_H_*/
