/*
 *	$Id$
 *	AbstractDrawer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
#ifndef OTAWA_DISPLAY_ABSTRACT_DRAWER_H
#define OTAWA_DISPLAY_ABSTRACT_DRAWER_H

#include <elm/io.h>
#include <elm/genstruct/FragTable.h>
#include <otawa/display/Driver.h>

namespace otawa { namespace display {

using namespace elm;
using namespace elm::genstruct;

// AbstractDrawer class
class AbstractDrawer {
public:
	class Edge;
	
	// Vertex class
	class Vertex {
	public:
		Vertex(AbstractDrawer& drawer);
		virtual ~Vertex(void) { }
		ShapeStyle shape;
		virtual void configure(Output& out, ShapeStyle& shape);
		
	private:
		friend class Edge;
		friend class AbstractDrawer;
		void setup(void);
		Node *node;
	};
	
	// Edge class
	class Edge {
	public:
		Edge(AbstractDrawer& drawer, Vertex *source, Vertex *sink);
		virtual ~Edge(void) { }
		TextStyle text;
		LineStyle line;
		virtual void configure(Output& label, TextStyle& text, LineStyle& line);
		
	private:
		friend class AbstractDrawer;
		void setup(void);
		display::Edge *edge;
	};
	
	// Methods
	AbstractDrawer(void);
	AbstractDrawer(Driver& driver);
	virtual ~AbstractDrawer(void) { }
	void draw(void);
	virtual void configure(Output& caption, TextStyle& text, FillStyle& fill);
	
	// Configuration
	kind_t kind;
	String path;
	FillStyle page_fill;
	TextStyle page_text;
	
	ShapeStyle default_vertex;
	
	LineStyle default_edge_line;
	TextStyle default_edge_text;
	
private:
	Graph *graph;
	FragTable<Vertex *> vertices;
	FragTable<Edge *> edges;
};

} } // otawa::display

#endif /* OTAWA_DISPLAY_ABSTRACT_DRAWER_H */
