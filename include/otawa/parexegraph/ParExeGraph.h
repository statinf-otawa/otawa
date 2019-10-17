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

#include <elm/PreIterator.h>
#include <elm/string/StringBuffer.h>
#include <elm/data/List.h>
#include <elm/data/BiDiList.h>
#include <otawa/base.h>
#include <otawa/cache/cat2/CachePenalty.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/hard/Memory.h>
#include <otawa/hard/Platform.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/parexegraph/ParExeProc.h>
#include <otawa/parexegraph/Resource.h>
#include "../ograph/GenGraph.h"
#include "../ograph/PreorderIterator.h"

namespace otawa {
	extern Identifier<int> DEFAULT_BRANCH_PENALTY;

	class ParExeNode;
	class ParExeEdge;
	class ParExeGraph;

	class ParExeException: public otawa::Exception {
	public:
		ParExeException(string message);
	};

	class NoMemFU: public ParExeException {
	public:
		NoMemFU();
	};

	typedef enum code_part {
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
		Vector<ParExeNode *> _nodes;	// nodes in the execution graph that are related to this instruction
		ParExeNode * _fetch_node;						// fetch node related to this instruction
		ParExeNode *_exec_node;							// execution node related to this instruction
		ParExeNode *_first_fu_node;
		ParExeNode *_last_fu_node;
		Vector<ParExeInst *> _producing_insts; // list of instructions this one depends on (some of its operands are produced by those instructions)

	public:
		inline ParExeInst(Inst * inst, BasicBlock *bb, code_part_t part, int index)
			: _inst(inst), _bb(bb), _part(part), _index(index), _fetch_node(NULL), _exec_node(NULL), _first_fu_node(NULL), _last_fu_node(NULL) {}

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
		class NodeIterator: public Vector<ParExeNode *>::Iter {
		public:
			inline NodeIterator(const ParExeInst *inst) : Vector<ParExeNode *>::Iter(inst->_nodes) {}
		};
		inline NodeIterator nodes(void) const { return NodeIterator(this); }

		// iterator on nodes related to the instruction
		class ProducingInstIterator: public Vector<ParExeInst *>::Iter {
		public:
			inline ProducingInstIterator(const ParExeInst *inst) : Vector<ParExeInst *>::Iter(inst->_producing_insts) {}
		};

	};

	/*
	 * class ParExeSequence
	 *
	 */

	// a sequence (double-linked list) of ParExeInst
	class ParExeSequence : public BiDiList<ParExeInst *> {
	public:
		// iterator in the instructions in the sequence
		class InstIterator: public BiDiList<ParExeInst *>::Iter {
		public:
			inline InstIterator(const ParExeSequence *seq) : BiDiList<ParExeInst *>::Iter(*seq) {}
		};
		// number of instructions in the sequence
		inline int length() {return BiDiList<ParExeInst *>::count();}
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
		List<NodeLatency *> _node_latencies_list;
		Block *_header[2];
		CachePenalty::cache_penalty_type_t _type;
	public:
		TimingContext(){
			_header[0] = NULL;
			_header[1] = NULL;
			_type = CachePenalty::MISS;
		}
		TimingContext(Block *h0, Block *h1=NULL){
			_header[0] = h0;
			_header[1] = h1;
			_type = CachePenalty::MISS;
		}

		class NodeLatencyIterator: public List<NodeLatency *>::Iter {
		public:
			inline NodeLatencyIterator(const TimingContext& tctxt)
				: List<NodeLatency *>::Iter(tctxt._node_latencies_list) {}
		};

		TimingContext(TimingContext *ctxt){
			for (NodeLatencyIterator nl(*ctxt) ; nl() ; nl++){
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
		inline Block *header(int index)
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

	class ParExeGraph: public ograph::GenGraph<ParExeNode, ParExeEdge>, Monitor {
		friend class InstIterator;
		friend class ParExeNode;
	protected:
		WorkSpace * _ws;
		PropList _props;
		ParExeProc * _microprocessor;
		typedef struct rename_table_t {								// used in ParExeGraph.cpp
			hard::RegBank * reg_bank;
			AllocArray<ParExeNode *> *table;
		} rename_table_t;
		Vector<Resource *> _resources;				// resources used by the sequence of instructions

		ParExeNode *_first_node;									// first node in the graph
		ParExeNode *_last_prologue_node;																					// ====== REALLY USEFUL? (used in analyze())
		ParExeNode *_last_node;																								// ====== REALLY USEFUL? (used in analyze())
		int _cache_line_size;																								// ====== REALLY USEFUL?
		const hard::Memory *mem;
		int _branch_penalty;																							// ====== REALLY USEFUL?

	private:
		ParExeSequence * _sequence;									// sequence of instructions related to the graph
		int _capacity;																										// ====== REALLY USEFUL? (used in analyze())
		bool _explicit;


		inline string comment(string com)
			{ if(_explicit) return com; else return ""; }

	public:
		ParExeGraph(WorkSpace * ws, ParExeProc *proc,  Vector<Resource *> *hw_resources, ParExeSequence *seq, const PropList& props = PropList::EMPTY);
		virtual ~ParExeGraph(void);
		inline void setExplicit(bool ex) { _explicit = ex; }
		inline bool getExplicit(void) const { return _explicit; }

		// set/get information related to the graph
		inline ParExeSequence *getSequence(void) const { return _sequence; }
		inline ParExeProc *getMicroprocessor(void) const { return _microprocessor; }
		inline ParExeNode * firstNode(){return _first_node;}
		inline ParExeNode *lastNode(){return _last_node;}
		inline ParExeNode *lastPrologueNode(){return _last_prologue_node;}
		virtual ParExePipeline *pipeline(ParExeStage *stage, ParExeInst *inst);
		inline int numResources() {return _resources.length();}
		inline Resource *resource(int index){return _resources[index];}
		inline int numInstructions(){return _sequence->length();}
		inline void setLastNode(ParExeNode *node) { _last_node = node; }

		// build the graph
		virtual void build();
		virtual void createNodes(void);
		virtual void findDataDependencies(void);
		virtual void addEdgesForPipelineOrder(void);
		virtual void addEdgesForFetch(void);
		virtual void addEdgesForProgramOrder(List<ParExeStage *> *list_of_stages = NULL);
		virtual void addEdgesForMemoryOrder(void);
		virtual void addEdgesForDataDependencies(void);
		virtual void addEdgesForQueues(void);

		// compute execution time
		void findContendingNodes();
		void createSequenceResources();
		RegResource *newRegResource(const hard::Register *r);
		int analyze();
		virtual void initDelays();
		void clearDelays();
		void restoreDefaultLatencies();
		void setDefaultLatencies(TimingContext *tctxt);
		void setLatencies(TimingContext *tctxt);
		virtual void propagate();
		void analyzeContentions();
		int cost();
		virtual int delta(ParExeNode *a, Resource *res);

		// tracing
		void dump(elm::io::Output& dotFile, const string& info = "", const string& custom = "");
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
		class StageIterator : public List<ParExeStage *>::Iter {
		public:
			inline StageIterator(const List<ParExeStage *>  *list)
				: List<ParExeStage *>::Iter(*list) {}
		};

		class StageNodeIterator : public ParExeStage::NodeIterator {
		public:
			inline StageNodeIterator(const ParExeStage *stage)
				: ParExeStage::NodeIterator(stage) {}
		};

		class PreorderIterator: public ograph::PreorderIterator<ParExeGraph> {
		public:
			inline PreorderIterator(ParExeGraph * graph)
				: ograph::PreorderIterator<ParExeGraph>(*graph, graph->firstNode()) {}
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

	protected:
		virtual void customDump(io::Output& out);

	};

	/*
	 * class ParExeNode
	 *
	 */
	// a node in an execution graph (ParExeGraph)
	class ParExeNode: public ograph::GenGraph<ParExeNode,ParExeEdge>::GenNode{
	private:
		ParExeStage *_pipeline_stage;		// pipeline stage to which the node is related
		ParExeInst *_inst;					// instruction to which the node is related
		int _latency;						// latency of the node
		int _default_latency;				// default latency of the node
		elm::String _name;					// name of the node (for tracing)
//		AllocArray<int> * _d;				// delays wrt availabilities of resources
//		AllocArray<bool> * _e;				// dependence on availabilities of resources
		Vector<int> * _delay;				// dependence and delays wrt availabilities of resources
	protected:
		Vector<ParExeNode *> _producers;			// nodes this one depends on (its predecessors)
		Vector<ParExeNode *> _contenders;																	// ==== STILL USEFUL?
		BitVector * _possible_contenders;																				// ==== STILL USEFUL?
		BiDiList<elm::BitVector *> _contenders_masks_list;													// ==== STILL USEFUL?
		int _late_contenders;																								// ==== STILL USEFUL?

	public:
		inline ParExeNode(ParExeGraph *graph, ParExeStage *stage, ParExeInst *inst)
		:	ParExeGraph::GenNode(graph),
			_pipeline_stage(stage),
			_inst(inst),
			_latency(stage->latency()),
			_default_latency(stage->latency()),
			_possible_contenders(nullptr),
			_late_contenders(0)
		{
			int num = graph->numResources();
			_delay = new Vector<int>(num);
			StringBuffer _buffer;
			_buffer << stage->name() << "(I" << inst->index() << ")";
			_name = _buffer.toString();
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
		inline BiDiList<BitVector *>* contendersMasksList() {return &_contenders_masks_list;}
		inline elm::String name(void) { return _name; }
		inline int delay(int index) {return (*_delay)[index];}
		inline int delayLength() { return _delay->length(); }
		inline void setDelay(int index, int value) { 
			while (index >= _delay->length()) _delay->add(-1);
			(*_delay)[index] = value;
		}
		inline void initContenders(int size) {_possible_contenders = new BitVector(size); }									// ==== STILL USEFUL?
		inline int lateContenders(void) {return _late_contenders; }															// ==== STILL USEFUL?
		inline void setLateContenders(int num) { _late_contenders = num; }													// ==== STILL USEFUL?
		inline elm::BitVector * possibleContenders(void) { return _possible_contenders; }									// ==== STILL USEFUL?
		inline void setContender(int index) { _possible_contenders->set(index); }											// ==== STILL USEFUL?
		void buildContendersMasks();																						// ==== STILL USEFUL?
		inline virtual void customDump(io::Output& out) { }
	};

	/*
	 * class ParExeEdge
	 *
	 */
	// an edge in an execution graph (ParExeGraph)
	class ParExeEdge: public ograph::GenGraph<ParExeNode,ParExeEdge>::GenEdge{
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
		inline virtual void customDump(io::Output& out) { }
	};

	inline bool ParExeGraph::Predecessor::ended(void) const {return iter.ended();}
	inline ParExeNode *ParExeGraph::Predecessor::item(void) const {return iter->source();}
	inline void ParExeGraph::Predecessor::next(void) {iter.next();}
	inline ParExeEdge *ParExeGraph::Predecessor::edge(void) const {return *iter;}

	inline bool ParExeGraph::Successor::ended(void) const {	return iter.ended();}
	inline ParExeNode *ParExeGraph::Successor::item(void) const {return iter->target();}
	inline void ParExeGraph::Successor::next(void) {iter.next();}
	inline ParExeEdge *ParExeGraph::Successor::edge(void) const {return *iter;}


} // namespace otawa

#endif //_PAR_EXEGRAPH_H_
