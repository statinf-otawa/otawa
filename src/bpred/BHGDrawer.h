#ifndef BHGDRAWER_H_
#define BHGDRAWER_H_
#include "BHG.h"
#include <otawa/display/Driver.h>
#include <otawa/display/graphviz.h>

namespace otawa { namespace display {

class BHGDrawer {
protected:
	BHG *_bhg;
	Graph *_graph;
	bool _made;
	virtual void onInit(PropList& graph, PropList& nodes, PropList& edges);
	virtual void onNode(BHGNode *bb, otawa::display::Node *node);
	virtual void onEdge(BHGEdge *bhg_edge, otawa::display::Edge *display_edge);
	virtual void onCall(BHG *bhg, display::Node *node);
	virtual void onEnd(otawa::display::Graph *graph);
	virtual void make();
	
public:
	BHGDrawer(BHG *bhg, Graph *graph);
	BHGDrawer(BHG *bhg, const PropList& props = PropList::EMPTY, Driver& driver = graphviz_driver);
	virtual void display();
};

} } // otawa::display

#endif /*BHGDRAWER_H_*/
