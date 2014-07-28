/*
 *	$Id$
 *	Interface to the ParExeInst, ParExeSequence, ParExeGraph, ParExeNode, ParExeEdge classes.
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

#ifndef _PAR_EXEGRAPH_H_
#define _PAR_EXEGRAPH_H_

#include <otawa/graph/GenGraph.h>
#include <elm/string/StringBuffer.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/parexegraph/ParExeProc.h>
#include <otawa/parexegraph/Resource.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/hard/Platform.h>
#include <otawa/parexegraph/ParExeProc.h>
#include <otawa/graph/PreorderIterator.h>
#include <otawa/util/Dominance.h>
#include <otawa/cache/cat2/CachePenalty.h>

namespace otawa { 
	
	class ParExeNode;
	class ParExeEdge;
	class ParExeGraph;

	class ParExeException: public otawa::Exception {
	public:
		inline ParExeException(string message): otawa::Exception(message) { }
	};

	typedef enum code_part_t {
		BEFORE_PROLOGUE = 0,
		PROLOGUE = 1,
		BODY = 2,
		EPILOGUE = 3,
		CODE_PARTS_NUMBER  // should be the last value
	} code_part_t;

 
	/*
	 * class ParExeInst
	 *
	 */

	class ParExeInst {				
	private:
		Inst * _inst;
		BasicBlock *_bb;
		code_part_t _part;                           
		int _index;
		elm::genstruct::Vector<ParExeNode *> _nodes;
		ParExeNode * _fetch_node;
		ParExeNode *_exec_node;

	public:
		inline ParExeInst(Inst * inst, BasicBlock *bb, code_part_t part, int index) 
			: _inst(inst), _bb(bb), _part(part), _index(index) {}

		inline Inst * inst()  {return _inst;}
		inline code_part_t codePart()  {return _part;}
		inline int index()  {return _index;}
		inline void setIndex(int index) {_index=index;}
		inline ParExeNode * firstNode() { return _nodes[0];}
		inline ParExeNode * lastNode() { return _nodes[_nodes.length()-1];}
		inline bool hasNodes() { return (_nodes.length() != 0);}
		inline int numNodes() { return _nodes.length();}
		inline ParExeNode *node(int index) { return _nodes[index];}
		inline void setFetchNode(ParExeNode *node) { _fetch_node = node;}
		inline void setExecNode(ParExeNode *node) { _exec_node = node;}
		inline ParExeNode * fetchNode() { return _fetch_node;}
		inline ParExeNode * execNode() { return _exec_node;}
		inline BasicBlock * basicBlock()  {return _bb;}

		inline void addNode(ParExeNode * node)  { _nodes.add(node);}

		inline void removeNode(ParExeNode *node) { _nodes.remove(node); }

		inline void addAfter(ParExeNode *pos, ParExeNode *node) {
			int p = _nodes.indexOf(pos);
			ASSERT(p >= 0);
			_nodes.insert(p + 1, node);
		}

		inline void deleteNodes() {
				if (_nodes.length() != 0) { _nodes.clear();	// FIXME: is there anything else to do?
		} }

		class NodeIterator: public elm::genstruct::Vector<ParExeNode *>::Iterator {
		public:
			inline NodeIterator(const ParExeInst *inst)
				: elm::genstruct::Vector<ParExeNode *>::Iterator(inst->_nodes) {}
		};
	};
 
	/* 
	 * class ParExeSequence
	 * 
	 */

	class ParExeSequence : public elm::genstruct::DLList<ParExeInst *> {
	public:
		class InstIterator: public elm::genstruct::DLList<ParExeInst *>::Iterator {
		public:
			inline InstIterator(const ParExeSequence *seq)
				: elm::genstruct::DLList<ParExeInst *>::Iterator(*seq) {}
		};
		inline int length()
		{return elm::genstruct::DLList<ParExeInst *>::count();}
	};


	/*
	 * class NodeLatency
	 *
	 */

	class NodeLatency {
	private:
		ParExeNode *_node;
		int _latency;
	public:
	NodeLatency(ParExeNode *node, int latency)
		: _node(node), _latency(latency) {}
		inline ParExeNode *node()
		{ return _node; }
		inline int latency()
		{return _latency; }
	};

	// -- class TimingContext ----------------------------------------------------------------------------------------

	class TimingContext {
	private:
		elm::genstruct::SLList<NodeLatency *> _node_latencies_list;
		BasicBlock *_header[2];
		CachePenalty::cache_penalty_type_t _type;
	public:
		TimingContext(){
			_header[0] = NULL;
			_header[1] = NULL;
		}
		TimingContext(BasicBlock *h0, BasicBlock *h1=NULL){
			_header[0] = h0;
			_header[1] = h1;
		}

		class NodeLatencyIterator: public elm::genstruct::SLList<NodeLatency *>::Iterator {
		public:
			inline NodeLatencyIterator(const TimingContext& tctxt)
				: elm::genstruct::SLList<NodeLatency *>::Iterator(tctxt._node_latencies_list) {}
		};

		TimingContext(TimingContext *ctxt){
			for (NodeLatencyIterator nl(*ctxt) ; nl ; nl++){
				NodeLatency *new_nl = new NodeLatency(nl->node(), nl->latency());
				_node_latencies_list.addLast(new_nl);
			}
			_header[0] = ctxt->_header[0];
			_header[1] = ctxt->_header[1];
			_type = ctxt->_type; 
		}
		~TimingContext() { 
			while (!_node_latencies_list.isEmpty()){
				NodeLatency * nl = _node_latencies_list.first();
				_node_latencies_list.removeFirst();
				delete nl;
			}
		}
		inline void addNodeLatency(NodeLatency *nl)
		{ _node_latencies_list.addLast(nl); }
		inline bool isEmpty()
		{ return _node_latencies_list.isEmpty(); }
		inline void setHeaders(BasicBlock *h0, BasicBlock *h1=NULL){
			_header[0] = h0;
			_header[1] = h1;
		}
		inline BasicBlock *header(int index)
		{ return _header[index]; }
		inline void setType(CachePenalty::cache_penalty_type_t type)
		{ _type = type; }
		inline CachePenalty::cache_penalty_type_t type()
		{ return _type; }

	};

 
	/*
	 * class ParExeGraph
	 *
	 */

	class ParExeGraph: public GenGraph<ParExeNode, ParExeEdge> {
		friend class InstIterator;
	protected:
		WorkSpace * _ws;
		PropList _props;
		ParExeProc * _microprocessor;
		typedef struct rename_table_t {
			hard::RegBank * reg_bank;
			elm::genstruct::AllocatedTable<ParExeNode *> *table;
		} rename_table_t;
		elm::genstruct::Vector<Resource *> _resources;

		ParExeNode *_first_node;
		ParExeNode *_first_bb_node;
		ParExeNode *_last_prologue_node;
		ParExeNode *_last_node;
	private:
		ParExeSequence * _sequence;
		int _cache_line_size;
		int _capacity;
		int _branch_penalty;

    
	public:
		ParExeGraph(WorkSpace * ws, ParExeProc *proc, ParExeSequence *seq, const PropList& props = PropList::EMPTY);
		~ParExeGraph();
		inline void setFetchSize(int size) { _cache_line_size = size; }
		inline void setBranchPenalty(int penalty) { _branch_penalty = penalty; }
		inline ParExeSequence *getSequence(void) const { return _sequence; }
      
		// graph building
		virtual void build(bool compressed_code=false);
		virtual void createNodes(void);
		virtual void findDataDependencies(void);
		virtual void addEdgesForPipelineOrder(void);
		virtual void addEdgesForFetch(void);
		virtual void addEdgesForFetchWithDecomp(void);
		virtual void addEdgesForProgramOrder(elm::genstruct::SLList<ParExeStage *> *list_of_stages = NULL);
		virtual void addEdgesForMemoryOrder(void);
		virtual void addEdgesForDataDependencies(void);
		virtual void addEdgesForQueues(void);
		virtual ParExePipeline *pipeline(ParExeStage *stage, ParExeInst *inst);

		// time computation
		void findContendingNodes();
		void createResources();
		int analyze();
		void initDelays();
		void clearDelays();
		void restoreDefaultLatencies();
		void setDefaultLatencies(TimingContext *tctxt);
		void setLatencies(TimingContext *tctxt);
		void propagate();
		void analyzeContentions();
		int cost();
		int Delta(ParExeNode *a, Resource *res);
		void dump(elm::io::Output& dotFile, const string& info = "");
		void display(elm::io::Output& output);

		inline int numResources()
			{return _resources.length();}
		inline Resource *resource(int index)
			{return _resources[index];}
		inline int numInstructions()
			{return _sequence->length();}
		inline ParExeNode * firstNode()
				{return _first_node;}
    
		class InstIterator : public ParExeSequence::InstIterator {
		public:
			inline InstIterator(const ParExeSequence *sequence)
				: ParExeSequence::InstIterator(sequence) {}
			inline InstIterator(const ParExeGraph *graph)
				: ParExeSequence::InstIterator(graph->_sequence) {}
		};
		class InstNodeIterator : public ParExeInst::NodeIterator {
		public:
			inline InstNodeIterator(const ParExeInst *inst)
				: ParExeInst::NodeIterator(inst) {}
		};   
		class StageIterator : public elm::genstruct::SLList<ParExeStage *>::Iterator {
		public:
			inline StageIterator(const SLList<ParExeStage *>  *list)
				: elm::genstruct::SLList<ParExeStage *>::Iterator(*list) {}
		};

		class StageNodeIterator : public ParExeStage::NodeIterator {
		public:
			inline StageNodeIterator(const ParExeStage *stage)
				: ParExeStage::NodeIterator(stage) {}
		};

		class PreorderIterator: public graph::PreorderIterator<ParExeGraph> {
		public:
			inline PreorderIterator(ParExeGraph * graph)
				: graph::PreorderIterator<ParExeGraph>(*graph, graph->firstNode()) {}
		}; 

		class Predecessor: public PreIterator<Predecessor, ParExeNode *> {
		public:
			inline Predecessor(const ParExeNode* node): iter(node) { }
			inline bool ended(void) const;
			inline ParExeNode *item(void) const;
			inline void next(void);
			inline ParExeEdge *edge(void) const;
		private:
			GenGraph<ParExeNode,ParExeEdge>::InIterator iter;
		};
		
		class Successor: public PreIterator<Successor, ParExeNode *> {
		public:
			inline Successor(const ParExeNode* node): iter(node) {}
			inline bool ended(void) const;
			inline ParExeNode *item(void) const;
			inline void next(void);
			inline ParExeEdge *edge(void) const;
		private:
			GenGraph<ParExeNode,ParExeEdge>::OutIterator iter;
		};
	

	};

	/*
	 * class ParExeNode
	 *
	 */
	class ParExeNode: public GenGraph<ParExeNode,ParExeEdge>::GenNode{
	private:
		ParExeStage *_pipeline_stage;           
		ParExeInst *_inst;                        
		int _latency;
		int _default_latency;
		elm::String _name;
		elm::genstruct::AllocatedTable<int> * _d;
		elm::genstruct::AllocatedTable<bool> * _e;
		elm::genstruct::AllocatedTable<bool> * _contention_dep;
	protected:
		elm::genstruct::Vector<ParExeNode *> _producers;
		elm::genstruct::Vector<ParExeNode *> _contenders;
		elm::BitVector * _possible_contenders;
		elm::genstruct::DLList<elm::BitVector *> _contenders_masks_list;
		int _late_contenders;
   
	public:
		inline ParExeNode(ParExeGraph *graph, ParExeStage *stage, ParExeInst *inst)
			: ParExeGraph::GenNode((otawa::graph::Graph *) graph), 
			_pipeline_stage(stage), _inst(inst),  _latency(stage->latency()), _default_latency(stage->latency()){
			int num = graph->numResources();
			_d = new elm::genstruct::AllocatedTable<int>(num);
			_e = new elm::genstruct::AllocatedTable<bool>(num);
			for (int i=0 ; i<num; i++){
				(*_e)[i] = false;
				(*_d)[i] = 0;
			}
			_contention_dep =  new elm::genstruct::AllocatedTable<bool>(graph->numInstructions());
			for (int k=0 ; k<graph->numInstructions() ; k++) {
				(*_contention_dep)[k] = false;
				StringBuffer _buffer;
				_buffer << stage->name() << "(I" << inst->index() << ")";
				_name = _buffer.toString();
 
			}
		}

		inline ParExeStage *stage(void) { return _pipeline_stage; }
		inline ParExeInst *inst(void) { return _inst; }
		inline int latency() { return _latency; }
		inline void setDefaultLatency(int lat) { _default_latency = lat; _latency = lat; }
		inline void restoreDefaultLatency(void) { _latency = _default_latency; }
		inline void setLatency(int latency) { _latency = latency; }
		inline void addProducer(ParExeNode *prod) { if (!_producers.contains(prod)) _producers.add(prod); }
		inline int numProducers(void) { return _producers.length(); }
		inline ParExeNode *producer(int index) { return _producers[index]; }
		inline void addContender(ParExeNode *cont) { _contenders.add(cont); }
		inline elm::genstruct::DLList<elm::BitVector *>* contendersMasksList() {return &_contenders_masks_list;}
		inline elm::String name(void) { return _name; }
		inline int d(int index) { return (*_d)[index]; }
		inline bool e(int index) { return (*_e)[index]; }
		inline void setD(int index, int value) { (*_d)[index] = value; }
		inline void setE(int index, bool value) { (*_e)[index] = value; }
		inline void setContentionDep(int index)  { (*_contention_dep)[index] = true; }
		inline void initContenders(int size) {_possible_contenders = new BitVector(size); }
		inline int lateContenders(void) {return _late_contenders; }
		inline void setLateContenders(int num) { _late_contenders = num; }
		inline elm::BitVector * possibleContenders(void) { return _possible_contenders; }
		inline void setContender(int index) { _possible_contenders->set(index); }
		void buildContendersMasks();
	};

	/*
	 * class ParExeEdge
	 *
	 */
	class ParExeEdge: public GenGraph<ParExeNode,ParExeEdge>::GenEdge{
	public:
		typedef enum edge_type_t {SOLID = 1, SLASHED = 2} edge_type_t_t;
	private:
		edge_type_t _type;
		elm::String _name;
		int _latency;
	public:
		inline ParExeEdge(ParExeNode *source, ParExeNode *target, edge_type_t type, int latency = 0, const string& name = "")
			: ParExeGraph::GenEdge(source, target), _type(type), _latency(latency), _name(name) { ASSERT(source != target); }
		inline int latency(void) const{return _latency;}
		inline void setLatency(int latency) {_latency = latency;}
		inline edge_type_t type(void) const {return _type;}
		inline const elm::string& name(void) const {return _name;}
	};





	inline bool ParExeGraph::Predecessor::ended(void) const { 
		return iter.ended(); 
	}
	inline ParExeNode *ParExeGraph::Predecessor::item(void) const { 
		return iter->source(); 
	}
	inline void ParExeGraph::Predecessor::next(void) { 
		iter.next(); 
	}
	inline ParExeEdge *ParExeGraph::Predecessor::edge(void) const { 
		return iter; 
	}

	inline bool ParExeGraph::Successor::ended(void) const { 
		return iter.ended(); 
	}
	inline ParExeNode *ParExeGraph::Successor::item(void) const { 
		return iter->target(); 
	}
	inline void ParExeGraph::Successor::next(void) { 
		iter.next(); 
	}
	inline ParExeEdge *ParExeGraph::Successor::edge(void) const { 
		return iter; 
	}

 
} // namespace otawa

#endif //_PAR_EXEGRAPH_H_
