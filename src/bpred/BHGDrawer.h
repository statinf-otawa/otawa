/*
 *	$Id$
 *	BHGDrawer class interface
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
#ifndef OTAWA_BPRED_BHGDRAWER_H_
#define OTAWA_BPRED_BHGDRAWER_H_

#include <otawa/display/Displayer.h>
#include "BHG.h"

namespace otawa { namespace bpred {

class BHGDrawer: public display::Decorator {
public:
	BHGDrawer(BHG *bhg, sys::Path path);
	virtual void display();

	void decorate(graph::DiGraph *graph, display::Text& caption, display::GraphStyle& style) const override;
	void decorate(graph::DiGraph *graph, graph::Vertex *vertex, display::Text& content, display::VertexStyle& style) const override;
	void decorate(graph::DiGraph *graph, graph::Edge *edge, display::Text& label, display::EdgeStyle& style) const override;

protected:
	virtual void onInit(display::VertexStyle& vertex_style, display::EdgeStyle& edge_style) const;
	virtual void onNode(BHGNode *bb, display::Text& content, display::VertexStyle& style) const;
	virtual void onEdge(BHGEdge *bhg_edge, display::Text& label, display::EdgeStyle& style) const;
	virtual void onCall(BHG *bhg, display::Text& label, display::GraphStyle& style) const;

	BHG *_bhg;
	bool _made;
	sys::Path _path;
};

} } // otawa::bpred

#endif /* OTAWA_BPRED_BHGDRAWER_H_*/
