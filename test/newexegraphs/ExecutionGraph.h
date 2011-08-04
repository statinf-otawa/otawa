/*
 *	$Id$
 *	Interface to the ExecutionGraph, EGNode, EGEdge classes.
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

#ifndef _BASIC_EXEGRAPH_H_
#define _BASIC_EXEGRAPH_H_


#include <elm/string/StringBuffer.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/hard/Platform.h>
#include <otawa/graph/PreorderIterator.h>
#include <otawa/prog/WorkSpace.h>
#include "EGInst.h"
#include "EGBlockSeq.h"
#include "EGProc.h"

namespace otawa { namespace exegraph2  {

/*
 * class EGNode
 *
 */
class EGNode: public GenGraph<EGNode,EGEdge>::GenNode{
private:
	EGStage *_pipeline_stage;
	EGInst *_inst;
	int _latency;
	elm::String _name;
protected:
	elm::genstruct::Vector<EGNode *> _producers;

public:
	inline EGNode(ExecutionGraph *graph, EGStage *stage, EGInst *inst)
		: GenNode((otawa::graph::Graph *) graph),
		_pipeline_stage(stage), _inst(inst),  _latency(stage->latency()){
		StringBuffer _buffer;
		_buffer << stage->name() << "(I" << inst->index() << ")";
		_name = _buffer.toString();
	}
	~EGNode(){}
	// get information
	inline EGStage *stage()
		{return _pipeline_stage;}
	inline EGInst *inst()
		{return _inst;}
	inline elm::String name()
		{return _name;}
	inline int latency()
		{return _latency;}
	inline int numProducers()
		{return _producers.length();}
	inline EGNode *producer(int index)
		{return _producers[index];}
	// set information
	inline void setLatency(uint32_t latency)
		{_latency = latency;}
	inline void addProducer(EGNode *prod) {
		if (!_producers.contains(prod))
			_producers.add(prod);
	}
};

class EGNodeFactory {
public:
	virtual EGNode * newEGNode(ExecutionGraph *graph, EGStage *stage, EGInst *inst) = 0;
};

class EGGenericNodeFactory : public EGNodeFactory {
	EGNode * newEGNode(ExecutionGraph *graph, EGStage *stage, EGInst *inst){
		return new EGNode(graph, stage, inst);
	}
};

/*
 * class EGEdge
 *
 */
class EGEdge: public GenGraph<EGNode,EGEdge>::GenEdge{
public:
	typedef enum edge_type_t {SOLID = 1, SLASHED = 2} edge_type_t_t;
private:
	edge_type_t _type;
	elm::String _name;
	int _latency;
public:
	inline EGEdge(EGNode *source, EGNode *target, edge_type_t type, int latency=0)
		: GenEdge(source, target), _type(type), _latency(latency) {
		assert(source!=target);
		elm::StringBuffer _buffer;
		_buffer << GenEdge::source()->name() << "->" ;
		_buffer << GenEdge::target()->name();
		_name = _buffer.toString();
	}
	//~ParExeEdge();
	inline int latency()
	{return _latency;}
	inline edge_type_t type(void)
	{return _type;}
	inline elm::String name()
	{return _name;}

};

class EGEdgeFactory {
public:
	virtual EGEdge * newEGEdge(EGNode *source, EGNode *target, EGEdge::edge_type_t type, int latency=0) = 0;
};

class EGGenericEdgeFactory : public EGEdgeFactory {
	EGEdge * newEGEdge(EGNode *source, EGNode *target, EGEdge::edge_type_t type, int latency=0){
		return new EGEdge(source, target, type, latency);
	}
};

	/*
	 * class ExecutionGraph
	 *
	 */

	class ExecutionGraph: public GenGraph<EGNode, EGEdge> {
	private:
		EGInstSeq _inst_seq;
		EGProc *_proc;
		EGNode *_first_node;
		EGNode *_first_bb_node;
		EGNode *_last_prologue_node;
		EGNode *_last_node;
		elm::genstruct::Vector<EGNode *> _unknown_nodes_list;
		elm::genstruct::Vector<EGEdge *> _unknown_edges_list;


	public:
		ExecutionGraph(WorkSpace *ws, EGBlockSeq *block_seq);
		~ExecutionGraph();

		inline EGNode * firstNode()
			{return _first_node;}
		inline void setFirstNode(EGNode *node)
			{_first_node = node;}
		inline void setLastPredNode(EGNode *node)
			{_last_prologue_node = node;}
		inline EGNode *firstBlockNode()
			{return _first_bb_node;}
		inline void setFirstBlockNode(EGNode *node)
			{_first_bb_node = node;}
		inline void setLastNode(EGNode *node)
			{_last_node = node;}
		inline EGInstSeq * instSeq()
			{return &_inst_seq;}
		inline EGProc * proc()
			{return _proc;}
		inline void addUnknownNode(EGNode *node)
			{ _unknown_nodes_list.add(node);}
		inline void addUnknownEdge(EGEdge *edge)
			{ _unknown_edges_list.add(edge);}
		class PreorderIterator: public graph::PreorderIterator<ExecutionGraph> {
		public:
			inline PreorderIterator(ExecutionGraph * graph)
				: graph::PreorderIterator<ExecutionGraph>(*graph, graph->firstNode()) {}
		};

		class Predecessor: public PreIterator<Predecessor, EGNode *> {
		public:
			inline Predecessor(const EGNode* node): iter(node) { }
			inline bool ended(void) const
				{ return iter.ended();}
			inline EGNode *item(void) const
				{return iter->source();}
			inline void next(void)
				{iter.next(); }
			inline EGEdge *edge(void) const
				{return iter;}
		private:
			GenGraph<EGNode,EGEdge>::InIterator iter;
		};

		class Successor: public PreIterator<Successor, EGNode *> {
		public:
			inline Successor(const EGNode* node): iter(node) {}
			inline bool ended(void) const
				{return iter.ended();}
			inline EGNode *item(void) const
				{return iter->target();}
			inline void next(void)
				{iter.next();}
			inline EGEdge *edge(void) const
				{return iter;}
		private:
			GenGraph<EGNode,EGEdge>::OutIterator iter;
		};

		class UnknownNodeIterator: public elm::genstruct::Vector<EGNode *>::Iterator {
		public:
			inline UnknownNodeIterator(const ExecutionGraph *graph):
				elm::genstruct::Vector<EGNode *>::Iterator(graph->_unknown_nodes_list){}
		};

		class UnknownEdgeIterator: public elm::genstruct::Vector<EGEdge *>::Iterator {
		public:
			inline UnknownEdgeIterator(const ExecutionGraph *graph):
				elm::genstruct::Vector<EGEdge *>::Iterator(graph->_unknown_edges_list){}
		};


	};


} // namespace exegraph2
} // namespace otawa

#endif //_BASIC_EXEGRAPH_H_




