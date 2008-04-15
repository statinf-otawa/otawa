/*
 *	$Id$
 *	graphviz plugin interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-07, IRIT UPS.
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
#ifndef PRIVATE_GRAPHVIZ_H
#define PRIVATE_GRAPHVIZ_H
#include <otawa/display/Driver.h>
#include <otawa/display/Graph.h>
#include <otawa/display/graphviz.h>
#include <elm/genstruct/FragTable.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/display/Plugin.h>

namespace otawa { namespace display {

/********************************* Driver *************************************/
class GraphVizDriver: public Plugin {
public:
	GraphVizDriver(void);
	virtual Graph *newGraph(
		const PropList& defaultGraphStyle = PropList::EMPTY,
		const PropList& defaultNodeStyle = PropList::EMPTY,
		const PropList& defaultEdgeStyle = PropList::EMPTY) const;
};

/*********************************** Item *************************************/
class GraphVizItem{
protected:
	/** Properties of the attached object */
	PropList _props;
	/** List of properties to print. If the key exists, include the property */
	elm::genstruct::HashTable<const AbstractIdentifier*, int> _include;
	/** List of properties to hide. If the key exists, exclude the property */
	elm::genstruct::HashTable<const AbstractIdentifier*, int> _exclude;
	/** true if the default is to include all properties */
	bool _defaultInclude;
	
	virtual String getPropertiesString();
	virtual void printOthersAttributes(elm::io::Output& out);
	virtual String attributes(const PropList& props);
	virtual bool printAttribute(elm::io::Output &out, const PropList::Iter& prop);
public:
	GraphVizItem();
	virtual ~GraphVizItem(void) { }
	virtual void setProps(const PropList& props);
	virtual String attributes() = 0;
};

/******************************** Graph Element *******************************/
// Nodes and Edges
class GraphVizGraphElement: public GraphVizItem{
protected:
	virtual ~GraphVizGraphElement(void) { }
	virtual bool printAttribute(elm::io::Output &out, const PropList::Iter& prop);
};

/******************************** Node ****************************************/
class GraphVizNode: public Node, public GraphVizGraphElement {
private:
	/** node title */
	String _title;
	/** true if the title have been set, even if it is an empty string */
	bool _hasTitle;
	/** node body */
	String _body;
	/** true if the body have been set, even if it is an empty string */
	bool _hasBody;
	/** true if the shape selected can be cutted into sub-boxes */
	bool _shapeAcceptsBody;
	/** number of the node */
	int _number;
protected:
	virtual void printOthersAttributes(elm::io::Output& out);
	virtual bool printAttribute(elm::io::Output &out, const PropList::Iter& prop);
public:
	GraphVizNode(int number);
	inline virtual int number(){return _number;}
	
	// inherited from GraphVizItem
	virtual String attributes();

	// inherited from Item:
	virtual void setProps(const PropList& props);
};

/********************************** Edge **************************************/
class GraphVizEdge: public Edge, public GraphVizGraphElement {
protected:
	/** true if the label have been set, even if it is an empty string */
	bool _hasLabel;
	/** label of the edge */
	String _label;
	/** source of the edge */
	GraphVizNode *_src;
	/** target of the edge */
	GraphVizNode *_dest;
	virtual void printOthersAttributes(elm::io::Output& out);
	virtual bool printAttribute(elm::io::Output &out, const PropList::Iter& prop);
public:
	GraphVizEdge(GraphVizNode *src, GraphVizNode *dest);
	inline virtual GraphVizNode* source(){return _src;}
	inline virtual GraphVizNode* target(){return _dest;}
	
	// inherited from GraphVizItem
	virtual String attributes();

	// inherited from Item:
	virtual void setProps(const PropList& props);
};

/********************************* Graph **************************************/
class GraphVizGraph: public Graph, public GraphVizItem {
protected:
	/** list of nodes in the graph */
	elm::genstruct::FragTable<GraphVizNode*> _nodes;
	/** list of edges in the graph */
	elm::genstruct::FragTable<GraphVizEdge*> _edges;
	/** default style for the nodes */
	PropList _default_node_style;
	/** default style for the edges */
	PropList _default_edge_style;
	/** node count for giving a different number for each new node */
	int _node_count;
	virtual void printOthersAttributes(elm::io::Output& out);
	virtual void printGraphData(elm::io::Output& out);
public:
	GraphVizGraph(
		const PropList& defaultGraphStyle = PropList::EMPTY,
		const PropList& defaultNodeStyle = PropList::EMPTY,
		const PropList& defaultEdgeStyle = PropList::EMPTY);
	
	// inherited from GraphVizItem
	virtual String attributes();
	
	// inherited from Item:
	virtual void setProps(const PropList& props);
	
	// inherited from Graph:
	virtual Node *newNode(
		const PropList& style = PropList::EMPTY,
		const PropList& props = PropList::EMPTY);
	virtual Edge *newEdge(
		Node *source,
		Node *target,
		const PropList& style = PropList::EMPTY,
		const PropList& props = PropList::EMPTY);
	virtual void display(void) throw(DisplayException);
};

/******************************************************************************/

extern String quoteSpecials(String str);
extern String quoteNewlines(String str);

} }

#endif /*PRIVATE_GRAPHVIZ_H*/
