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
#include "EGInst.h"
#include "EGProc.h"
#include "TAATimingInfo.h"

namespace otawa { namespace newexegraph  {

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
	TimingInfo *_timing;
protected:
	elm::genstruct::Vector<EGNode *> _producers;
	elm::genstruct::Vector<EGNode *> _contenders;
	elm::BitVector * _possible_contenders;
	elm::genstruct::DLList<elm::BitVector *> _contenders_masks_list;
	int _late_contenders;

public:
	inline EGNode(ExecutionGraph *graph, EGStage *stage, EGInst *inst)
		: GenNode((otawa::graph::Graph *) graph),
		_pipeline_stage(stage), _inst(inst),  _latency(stage->latency()){
	}
	~EGNode(){}
	// get information
	inline EGStage *stage()
		{return _pipeline_stage;}
	inline EGInst *inst()
		{return _inst;}
	inline int latency()
		{return _latency;}
	inline elm::String name()
		{return _name;}
	inline TimingInfo * timing()
		{return _timing;}
	inline int numProducers()
		{return _producers.length();}
	inline EGNode *producer(int index)
		{return _producers[index];}
	// set information
	inline void setTiming(TimingInfo *timing)
		{_timing = timing;}
	inline void addProducer(EGNode *prod) {
		if (!_producers.contains(prod))
			_producers.add(prod);
	}
};

class EGNodeFactory {
public:
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

	/*
	 * class ExecutionGraph
	 *
	 */

	class ExecutionGraph: public GenGraph<EGNode, EGEdge> {
		friend class InstIterator;
	protected:
		WorkSpace * _ws;
		PropList _props;
		EGProc * _microprocessor;
		typedef struct rename_table_t {
			otawa::hard::RegBank * reg_bank;
			elm::genstruct::AllocatedTable<EGNode *> *table;
		} rename_table_t;
//		elm::genstruct::Vector<Resource *> _resources;
	private:
		EGNodeFactory * _node_factory;
		EGSequence * _sequence;
		EGNode *_first_node;
		EGNode *_first_bb_node;
		EGNode *_last_prologue_node;
		EGNode *_last_node;
		int _cache_line_size;
		int _capacity;
		int _branch_penalty;


	public:
		ExecutionGraph(WorkSpace * ws, EGProc *proc, EGSequence *seq, EGNodeFactory *node_factory, const PropList& props = PropList::EMPTY);
		~ExecutionGraph();
		inline void setFetchSize(int size) { _cache_line_size = size; }
		inline void setBranchPenalty(int penalty) { _branch_penalty = penalty; }

		void build(bool compressed_code=false);
		void createNodes();
		void findDataDependencies();
		void addEdgesForPipelineOrder();
		void addEdgesForFetch();
		void addEdgesForFetchWithDecomp();
		void addEdgesForProgramOrder(elm::genstruct::SLList<EGStage *> *list_of_stages = NULL);
		void addEdgesForMemoryOrder();
		void addEdgesForDataDependencies();
		void addEdgesForQueues();
		void findContendingNodes();
		void createResources();
		void dump(elm::io::Output& dotFile, const string& info = "");
		void display(elm::io::Output& output);

		inline int numInstructions()
			{return _sequence->length();}
		inline EGNode * firstNode()
				{return _first_node;}

		class InstIterator : public EGSequence::InstIterator {
		public:
			inline InstIterator(const EGSequence *sequence)
				: EGSequence::InstIterator(sequence) {}
			inline InstIterator(const ExecutionGraph *graph)
				: EGSequence::InstIterator(graph->_sequence) {}
		};
		class InstNodeIterator : public EGInst::NodeIterator {
		public:
			inline InstNodeIterator(const EGInst *inst)
				: EGInst::NodeIterator(inst) {}
		};
		class StageIterator : public elm::genstruct::SLList<EGStage *>::Iterator {
		public:
			inline StageIterator(const SLList<EGStage *>  *list)
				: elm::genstruct::SLList<EGStage *>::Iterator(*list) {}
		};

		class StageNodeIterator : public EGStage::NodeIterator {
		public:
			inline StageNodeIterator(const EGStage *stage)
				: EGStage::NodeIterator(stage) {}
		};

		class PreorderIterator: public graph::PreorderIterator<ExecutionGraph> {
		public:
			inline PreorderIterator(ExecutionGraph * graph)
				: graph::PreorderIterator<ExecutionGraph>(*graph, graph->firstNode()) {}
		};

		class Predecessor: public PreIterator<Predecessor, EGNode *> {
		public:
			inline Predecessor(const EGNode* node): iter(node) { }
			inline bool ended(void) const;
			inline EGNode *item(void) const;
			inline void next(void);
			inline EGEdge *edge(void) const;
		private:
			GenGraph<EGNode,EGEdge>::InIterator iter;
		};

		class Successor: public PreIterator<Successor, EGNode *> {
		public:
			inline Successor(const EGNode* node): iter(node) {}
			inline bool ended(void) const;
			inline EGNode *item(void) const;
			inline void next(void);
			inline EGEdge *edge(void) const;
		private:
			GenGraph<EGNode,EGEdge>::OutIterator iter;
		};


	};




	inline bool ExecutionGraph::Predecessor::ended(void) const {
		return iter.ended();
	}
	inline EGNode *ExecutionGraph::Predecessor::item(void) const {
		return iter->source();
	}
	inline void ExecutionGraph::Predecessor::next(void) {
		iter.next();
	}
	inline EGEdge *ExecutionGraph::Predecessor::edge(void) const {
		return iter;
	}

	inline bool ExecutionGraph::Successor::ended(void) const {
		return iter.ended();
	}
	inline EGNode *ExecutionGraph::Successor::item(void) const {
		return iter->target();
	}
	inline void ExecutionGraph::Successor::next(void) {
		iter.next();
	}
	inline EGEdge *ExecutionGraph::Successor::edge(void) const {
		return iter;
	}


} // namespace newexegraph
} // namespace otawa

#endif //_BASIC_EXEGRAPH_H_




