/*
 *	$Id$
 *	BBHGDrawer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef BBHGDRAWER_H_
#define BBHGDRAWER_H_

#include "BBHG.h"
#include <otawa/display/Driver.h>
#include <otawa/display/graphviz.h>

namespace otawa { namespace bpred {

class BBHGDrawer {
protected:
	BBHG *_bhg;
	display::Graph *_graph;
	bool _made;
	virtual void onInit(PropList& graph, PropList& nodes, PropList& edges);
	virtual void onNode(BBHGNode *bb, otawa::display::Node *node);
	virtual void onEdge(BBHGEdge *bhg_edge, otawa::display::Edge *display_edge);
	virtual void onCall(BBHG *bhg, display::Node *node);
	virtual void onEnd(otawa::display::Graph *graph);
	virtual void make();
	
public:
	BBHGDrawer(BBHG *bhg, display::Graph *graph);
	BBHGDrawer(BBHG *bhg, const PropList& props = PropList::EMPTY, display::Driver& driver = display::graphviz_driver);
	virtual void display();
};

} } // otawa::bpred


#endif /*BBHGDRAWER_H_*/
