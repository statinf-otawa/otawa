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
#include <elm/PreIterator.h>
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
		PROLOGUE,			// in one of the basic block's predecessors
		BLOCK,				// in the basic block under analysis
		EPILOGUE,			// in one of the basic block's successors
		CODE_PARTS_NUMBER  	// should be the last value
	} code_part_t;


	/*
	 * class ParExeInst
	 *
	 */

	class ParExeInst {
	private:
		Inst * _inst;									// pointer to the instruction in the CFG structure
		BasicBlock *_bb;								// pointer to the basic block the instruction belongs to
		code_part_t _part;								// position of the instruction in a sequence: PROLOGUE, BLOCK under analysis, or EPILOGUE
		int _index;										// index of the instruction in the sequence
		elm::genstruct::Vector<ParExeNode *> _nodes;	// nodes in the execution graph that are related to this instruction
		ParExeNode * _fetch_node;						// fetch node related to this instruction
		ParExeNode *_exec_node;							// execution node related to this instruction
		ParExeNode *_first_fu_node;
		ParExeNode *_last_fu_node;
		elm::genstruct::Vector<ParExeInst *> _producing_insts; // list of instructions this one depends on (some of its operands are produced by those instructions)

	public:
		inline ParExeInst(Inst * inst, BasicBlock *bb, code_part_t part, int index)
			: _inst(inst), _bb(bb), _part(part), _index(index), _exec_node(NULL), _first_fu_node(NULL), _last_fu_node(NULL) {}

		// set/get information on the instruction
		inline Inst * inst()  {return _inst;}
		inline code_part_t codePart()  {return _part;}
		inline int index()  {return _index;}
		inline void setIndex(int index) {_index=index;}
		inline ParExeNode * firstNode() { return _nodes[0];}
		inline ParExeNode *node(int index) { return _nodes[index];}
		inline void setFetchNode(ParExeNode *node) { _fetch_node = node;}
		inline void setExecNode(ParExeNode *node) { _exec_node = node;}
		inline void setFirstFUNode(ParExeNode *node) { _first_fu_node = node;}
		inline void setLastFUNode(ParExeNode *node) { _last_fu_node = node;}
		inline ParExeNode * fetchNode() { return _fetch_node;}
		inline ParExeNode * execNode() { return _exec_node;}
		inline ParExeNode * firstFUNode() { return _first_fu_node;}
		inline ParExeNode * lastFUNode() { return _last_fu_node;}
		inline void addProducingInst(ParExeInst *inst) { _producing_insts.add(inst);}
		inline BasicBlock * basicBlock()  {return _bb;}

		// add/remove nodes for this instruction
		void addNode(ParExeNode * node);
		inline void removeNode(ParExeNode *node) { _nodes.remove(node); }
		inline void deleteNodes() {
			if (_nodes.length() != 0) { _nodes.clear();	}
		}

		// iterator on nodes related to the instruction
		class NodeIterator: public elm::genstruct::Vector<ParExeNode *>::Iterator {
		public:
			inline NodeIterator(const ParExeInst *inst) : elm::genstruct::Vector<ParExeNode *>::Iterator(inst->_nodes) {}
		};

		// iterator on nodes related to the instruction
		class ProducingInstIterator: public elm::genstruct::Vector<ParExeInst *>::Iterator {
		public:
			inline ProducingInstIterator(const ParExeInst *inst) : elm::genstruct::Vector<ParExeInst *>::Iterator(inst->_producing_insts) {}
		};

	};

	/*
	 * class ParExeSequence
	 *
	 */

	// a sequence (double-linked list) of ParExeInst
	class ParExeSequence : public elm::genstruct::DLList<ParExeInst *> {
	public:
		// iterator in the instructions in the sequence
		class InstIterator: public elm::genstruct::DLList<ParExeInst *>::Iterator {
		public:
			inline InstIterator(const ParExeSequence *seq) : elm::genstruct::DLList<ParExeInst *>::Iterator(*seq) {}
		};
		// number of instructions in the sequence
		inline int length() {return elm::genstruct::DLList<ParExeInst *>::count();}
	};


	/*
	 * class NodeLatency
	 *
	 */
	// a latency of a node
	class NodeLatency {												// used in ParExeGraph.h
	private:
		ParExeNode *_node;		// pointer to the node
		int _latency;			// value of the latency
	public:
		NodeLatency(ParExeNode *node, int latency) : _node(node), _latency(latency) {}
		inline ParExeNode *node(){ return _node; }
		inline int latency(){return _latency; }
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
		friend class ParExeNode;
	protected:
		WorkSpace * _ws;
		PropList _props;
		ParExeProc * _microprocessor;
		typedef struct rename_table_t {								// used in ParExeGraph.cpp
			hard::RegBank * reg_bank;
			elm::genstruct::AllocatedTable<ParExeNode *> *table;
		} rename_table_t;
		elm::genstruct::Vector<Resource *> _resources;				// resources used by the sequence of instructions

		ParExeNode *_first_node;									// first node in the graph
		ParExeNode *_last_prologue_node;																					// ====== REALLY USEFUL? (used in analyze())
		ParExeNode *_last_node;																								// ====== REALLY USEFUL? (used in analyze())
	private:
		ParExeSequence * _sequence;									// sequence of instructions related to the graph
		int _cache_line_size;																								// ====== REALLY USEFUL?
		int _capacity;																										// ====== REALLY USEFUL? (used in analyze())
		int _branch_penalty;																								// ====== REALLY USEFUL?


	public:
		ParExeGraph(WorkSpace * ws, ParExeProc *proc,  elm::genstruct::Vector<Resource *> *hw_resources, ParExeSequence *seq, const PropList& props = PropList::EMPTY);
		virtual ~ParExeGraph(void);

		// set/get information related to the graph
		inline ParExeSequence *getSequence(void) const { return _sequence; }
		inline ParExeNode * firstNode(){return _first_node;}
		virtual ParExePipeline *pipeline(ParExeStage *stage, ParExeInst *inst);
		inline int numResources() {return _resources.length();}
		inline Resource *resource(int index){return _resources[index];}
		inline int numInstructions(){return _sequence->length();}

		// build the graph
		virtual void build();
		virtual void createNodes(void);
		virtual void findDataDependencies(void);
		virtual void addEdgesForPipelineOrder(void);
		virtual void addEdgesForFetch(void);
		virtual void addEdgesForProgramOrder(elm::genstruct::SLList<ParExeStage *> *list_of_stages = NULL);
		virtual void addEdgesForMemoryOrder(void);
		virtual void addEdgesForDataDependencies(void);
		virtual void addEdgesForQueues(void);

		// compute execution time
		void findContendingNodes();
		void createSequenceResources();
		int analyze();
		void initDelays();
		void clearDelays();
		void restoreDefaultLatencies();
		void setDefaultLatencies(TimingContext *tctxt);
		void setLatencies(TimingContext *tctxt);
		void propagate();
		void analyzeContentions();
		int cost();
		int delta(ParExeNode *a, Resource *res);

		// tracing
		void dump(elm::io::Output& dotFile, const string& info = "");
		void display(elm::io::Output& output);


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
	// a node in an execution graph (ParExeGraph)
	class ParExeNode: public GenGraph<ParExeNode,ParExeEdge>::GenNode{
	private:
		ParExeStage *_pipeline_stage;								// pipeline stage to which the node is related
		ParExeInst *_inst;											// instruction to which the node is related
		int _latency;												// latency of the node
		int _default_latency;										// default latency of the node
		elm::String _name;											// name of the node (for tracing)
//		elm::genstruct::AllocatedTable<int> * _d;					// delays wrt availabilities of resources
//		elm::genstruct::AllocatedTable<bool> * _e;					// dependence on availabilities of resources
		elm::genstruct::AllocatedTable<int> * _delay;				// dependence and delays wrt availabilities of resources
	protected:
		elm::genstruct::Vector<ParExeNode *> _producers;			// nodes this one depends on (its predecessors)
		elm::genstruct::Vector<ParExeNode *> _contenders;																	// ==== STILL USEFUL?
		elm::BitVector * _possible_contenders;																				// ==== STILL USEFUL?
		elm::genstruct::DLList<elm::BitVector *> _contenders_masks_list;													// ==== STILL USEFUL?
		int _late_contenders;																								// ==== STILL USEFUL?

	public:
		inline ParExeNode(ParExeGraph *graph, ParExeStage *stage, ParExeInst *inst)
			: ParExeGraph::GenNode((otawa::graph::Graph *) graph),
			_pipeline_stage(stage), _inst(inst),  _latency(stage->latency()), _default_latency(stage->latency()){
			int num = graph->numResources();
			_delay = new elm::genstruct::AllocatedTable<int>(num);
			for (int k=0 ; k<graph->numInstructions() ; k++) {
				StringBuffer _buffer;
				_buffer << stage->name() << "(I" << inst->index() << ")";
				_name = _buffer.toString();
			}
			if (!graph->firstNode())
				graph->_first_node = this;
			if (inst->codePart() == PROLOGUE)
				graph->_last_prologue_node = this;
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
		inline int delay(int index) {return (*_delay)[index];}
		inline void setDelay(int index, int value) { (*_delay)[index] = value; }
		inline void initContenders(int size) {_possible_contenders = new BitVector(size); }									// ==== STILL USEFUL?
		inline int lateContenders(void) {return _late_contenders; }															// ==== STILL USEFUL?
		inline void setLateContenders(int num) { _late_contenders = num; }													// ==== STILL USEFUL?
		inline elm::BitVector * possibleContenders(void) { return _possible_contenders; }									// ==== STILL USEFUL?
		inline void setContender(int index) { _possible_contenders->set(index); }											// ==== STILL USEFUL?
		void buildContendersMasks();																						// ==== STILL USEFUL?
	};

	/*
	 * class ParExeEdge
	 *
	 */
	// an edge in an execution graph (ParExeGraph)
	class ParExeEdge: public GenGraph<ParExeNode,ParExeEdge>::GenEdge{
	public:
		typedef enum edge_type_t {SOLID = 1, SLASHED = 2} edge_type_t_t;
	private:
		edge_type_t _type;			// type of the edge: SOLID or SLASHED
		elm::String _name;
		int _latency;
	public:
		inline ParExeEdge(ParExeNode *source, ParExeNode *target, edge_type_t type, int latency = 0, const string& name = "")
			: ParExeGraph::GenEdge(source, target), _type(type), _name(name), _latency(latency) { ASSERT(source != target); }
		inline int latency(void) const{return _latency;}
		inline void setLatency(int latency) {_latency = latency;}
		inline edge_type_t type(void) const {return _type;}
		inline const elm::string& name(void) const {return _name;}
	};

	inline bool ParExeGraph::Predecessor::ended(void) const {return iter.ended();}
	inline ParExeNode *ParExeGraph::Predecessor::item(void) const {return iter->source();}
	inline void ParExeGraph::Predecessor::next(void) {iter.next();}
	inline ParExeEdge *ParExeGraph::Predecessor::edge(void) const {return iter;}

	inline bool ParExeGraph::Successor::ended(void) const {	return iter.ended();}
	inline ParExeNode *ParExeGraph::Successor::item(void) const {return iter->target();}
	inline void ParExeGraph::Successor::next(void) {iter.next();}
	inline ParExeEdge *ParExeGraph::Successor::edge(void) const {return iter;}


} // namespace otawa

#endif //_PAR_EXEGRAPH_H_
