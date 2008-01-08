#ifndef BBHGDRAWER_H_
#define BBHGDRAWER_H_

#include "BBHG.h"
#include <otawa/display/Driver.h>
#include <otawa/display/graphviz.h>

namespace otawa { namespace display {

class BBHGDrawer {
protected:
	BBHG *_bhg;
	Graph *_graph;
	bool _made;
	virtual void onInit(PropList& graph, PropList& nodes, PropList& edges);
	virtual void onNode(BBHGNode *bb, otawa::display::Node *node);
	virtual void onEdge(BBHGEdge *bhg_edge, otawa::display::Edge *display_edge);
	virtual void onCall(BBHG *bhg, display::Node *node);
	virtual void onEnd(otawa::display::Graph *graph);
	virtual void make();
	
public:
	BBHGDrawer(BBHG *bhg, Graph *graph);
	BBHGDrawer(BBHG *bhg, const PropList& props = PropList::EMPTY, Driver& driver = graphviz_driver);
	virtual void display();
};

} } // otawa::display


#endif /*BBHGDRAWER_H_*/
