/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/cfg/CFG.h -- interface of CFG class.
 */
#ifndef EXECUTION_GRAPH_H
#define EXECUTION_GRAPH_H

#include <assert.h>
#include <elm/Collection.h>
#include <elm/genstruct/Vector.h>
#include <otawa/util/Graph.h>
#include <otawa/exegraph/Microprocessor.h>
#include <otawa/instruction.h>
#include <elm/inhstruct/DLList.h>
#include <elm/genstruct/DLList.h>


namespace otawa { 
	
#define INFINITE_TIME 0x0FFFFFF

class ExecutionGraph;
class ExecutionEdge;

// -----------------------------------------------------------
// ExecutionNode class definition
// -----------------------------------------------------------

class ExecutionNode: public graph::Node {
	friend class ExecutionGraph;
	public:
		typedef struct time_interval_t {
			int min;
			int max;
		} time_interval_t;
		
		typedef enum code_part_t {
			BEFORE_PROLOGUE = 0,
			PROLOGUE = 1,
			BODY = 2,
			EPILOGUE = 3,
			CODE_PARTS_NUMBER  // should be the last value
		} code_part_t;	
	private:
		PipelineStage * pipeline_stage;
		Inst * inst;
		int inst_index;
		time_interval_t ready_time;
		time_interval_t start_time;
		time_interval_t finish_time;
		time_interval_t latency;
		ExecutionGraph *execution_graph;
		bool needs_operands;
		bool produces_operands;
		code_part_t code_part;
		bool shaded;
		elm::genstruct::Vector<ExecutionNode *> contenders;
	public:
		inline ExecutionNode(ExecutionGraph * graph, 
							PipelineStage *stage, 
							Inst *instruction, 
							int index, 
							code_part_t part);
		inline PipelineStage * pipelineStage(void);
		inline Inst * instruction(void);
		inline int instIndex(void);
		inline int stageIndex(void);
		inline int minReadyTime(void);
		inline int maxReadyTime(void);
		inline int minStartTime(void);
		inline int maxStartTime(void);
		inline int minFinishTime(void);
		inline int maxFinishTime(void);
		inline int minLatency(void);
		inline int maxLatency(void);
		inline bool needsOperands(void);
		inline bool producesOperands(void);
		inline void setMinReadyTime(int time) ;
		inline void setMaxReadyTime(int time) ;
		inline void setMinStartTime(int time) ;
		inline void setMaxStartTime(int time) ;
		inline void setMinFinishTime(int time) ;
		inline void setMaxFinishTime(int time) ;
		inline void setMinLatency(int lat) ;
		inline void setMaxLatency(int lat) ;
		inline void setNeedsOperands(bool val);
		inline void setProducesOperands(bool val);
		inline void dump(elm::io::Output& out_stream);
		inline void dumpLight(elm::io::Output& out_stream);
		inline void dumpLightTimed(elm::io::Output& out_stream);
		inline void dumpTime(int time, elm::io::Output& out_stream);
		inline void addContender(ExecutionNode *cont);
		inline code_part_t part(void) const;
		inline void dumpPart(elm::io::Output& out_stream);
		inline void shade(void);
		inline bool isShaded(void);
		class ContenderIterator: public elm::genstruct::Vector<ExecutionNode *>::Iterator {
			public:
				inline ContenderIterator(const ExecutionNode *node);
			};
};



// -----------------------------------------------------------
// ExecutionEdge class definition
// -----------------------------------------------------------

class ExecutionEdge: public graph::Edge {
	
	public:
		typedef enum edge_type_t {	
			SOLID = 1,
			SLASHED = 2,
			NONE = 3
		} edge_type_t_t;	
		
	private:
		edge_type_t edge_type;
	
	public:
		inline ExecutionEdge(ExecutionNode *source, ExecutionNode *target, edge_type_t type);
		inline edge_type_t type(void) const;
		inline void dump(elm::io::Output& out_stream);
};

// -----------------------------------------------------------
// ExecutionGraph class definition
// -----------------------------------------------------------

class ExecutionGraph:  public graph::Graph  {
	public:
		class NodePair {
			private:
				ExecutionNode *node1;
				ExecutionNode *node2;
				bool separated;
			public:
				inline NodePair(ExecutionNode *n1, ExecutionNode *n2, bool sep);
				inline ExecutionNode *firstNode(void);
				inline ExecutionNode *secondNode(void);
				inline bool isSeparated(void);
				inline void setSeparated(bool sep);
		}; 
		typedef struct rename_table_t {
			hard::RegBank * reg_bank;
			elm::genstruct::AllocatedTable<ExecutionNode *> *table;
		} rename_table_t;
		
		
	private:
		ExecutionNode * entry_node;
		elm::inhstruct::DLList stages_nodes_lists;
		elm::inhstruct::DLList instructions_nodes_lists;
		elm::genstruct::DLList<NodePair *> pairs;
		ExecutionNode * first_node[ExecutionNode::CODE_PARTS_NUMBER];
		ExecutionNode * last_node[ExecutionNode::CODE_PARTS_NUMBER];
		
	public:
		inline ExecutionGraph(void);
		~ExecutionGraph(void);
		inline void setEntryNode(ExecutionNode *node);
		inline void dump(elm::io::Output& out_stream);
		inline void dumpLight(elm::io::Output& out_stream);
		inline ExecutionNode * entryNode(void);
		inline void dotDump(elm::io::Output& dotFile, bool dump_times);
		inline elm::inhstruct::DLList * stagesNodesLists(void);
		inline elm::inhstruct::DLList * instructionsNodesLists(void);
		inline void initSeparated(void);
		inline bool unchangedSeparated(elm::io::Output& out_stream);
		inline bool separated(ExecutionNode *u, ExecutionNode *v, elm::io::Output& out_stream);
		void latestTimes(elm::io::Output& out_stream);
		void prologueLatestTimes(ExecutionNode *node, elm::io::Output& out_stream);
		void bodyLatestTimes(ExecutionNode *node, elm::io::Output& out_stream);
		void epilogueLatestTimes(ExecutionNode *node, elm::io::Output& out_stream);
		void earliestTimes(elm::io::Output& out_stream) ;
		int minDelta(elm::io::Output& out_stream);
		void shadeNodes(elm::io::Output& out_stream) ;
		void shadePreds(ExecutionNode *node,  elm::io::Output& out_stream) ;
		inline void setFirstNode(ExecutionNode::code_part_t part, ExecutionNode *node);
		inline void setLastNode(ExecutionNode::code_part_t part, ExecutionNode *node);
		inline ExecutionNode * firstNode(ExecutionNode::code_part_t part);
		inline ExecutionNode * lastNode(ExecutionNode::code_part_t part);
		void build(FrameWork *fw, Microprocessor* microprocessor, 
					elm::genstruct::DLList<Inst *> &prologue, 
					elm::genstruct::DLList<Inst *> &body, 
					elm::genstruct::DLList<Inst *> &epilogue);
		int analyze(elm::io::Output& out_stream);
};

// -----------------------------------------------------------
// GraphNodeInList class definition
// -----------------------------------------------------------

class GraphNodeInList: public elm::inhstruct::DLNode {
	private:
		ExecutionNode *execution_node;
	public:
		inline GraphNodeInList(ExecutionNode* node);
		inline ExecutionNode * executionNode(void);
		inline void dump(elm::io::Output& out_stream);
};

// -----------------------------------------------------------
// GraphNodesListInList class definition
// -----------------------------------------------------------


class GraphNodesListInList : public elm::inhstruct::DLNode {
	private:
		elm::inhstruct::DLList node_list;
		PipelineStage * pipeline_stage;
		Inst * instruction;
		int inst_index;
		int stage_index;
		ExecutionNode::code_part_t code_part;
		
	public:
		inline GraphNodesListInList(PipelineStage* stage);
		inline GraphNodesListInList(Inst* inst, int index, ExecutionNode::code_part_t part);
		inline elm::inhstruct::DLList * list(void);
		inline PipelineStage * stage(void);
		inline Inst * inst(void);
		inline void dump(elm::io::Output& out_stream);
		inline int instIndex(void) const;
		inline int stageIndex(void) const;
		ExecutionNode::code_part_t part(void) const;
		
};

// -----------------------------------------------------------
// Path class definition
// -----------------------------------------------------------

class Path {
	private:
		elm::genstruct::DLList<ExecutionNode *> node_list;
	public:
		inline void addNodeFirst(ExecutionNode *node);
		inline void dump(elm::io::Output& out_stream);
		
		class NodeIterator : public elm::genstruct::DLList<ExecutionNode *>::Iterator {
			public:
				inline NodeIterator(Path *path);
		};
		
};

// -----------------------------------------------------------
// PathList class definition
// -----------------------------------------------------------

class PathList {
	private:
		elm::genstruct::DLList<Path *> path_list;
	public:
		inline void addPath(Path *path);
		inline void dump(elm::io::Output& out_stream);
		
		class PathIterator : public elm::genstruct::DLList<Path *>::Iterator {
			public:
				inline PathIterator(PathList *list);
		};
		
};

// -----------------------------------------------------------
// TimeDLNode class definition
// -----------------------------------------------------------

class TimeDLNode : public elm::inhstruct::DLNode {
	private:
		int value;
	public:
		inline TimeDLNode(int value);
		inline int getValue(void);
};

inline  TimeDLNode::TimeDLNode(int val) {
	value = val;
}

inline int TimeDLNode::getValue(void) {
	return value;
}



// -----------------------------------------------------------
// ExecutionNode functions
// -----------------------------------------------------------

// ---------- Constructor

inline ExecutionNode::ExecutionNode(ExecutionGraph * graph, 
									PipelineStage *stage, 
									Inst *instruction, 
									int index, 
									code_part_t part)
: execution_graph(graph), pipeline_stage(stage), inst(instruction), inst_index(index), needs_operands(false),
produces_operands(false), shaded(false), code_part(part), Node((otawa::graph::Graph *)graph) {
	ready_time.min = -INFINITE_TIME; // needed ?
	start_time.min = 0;
	finish_time.min = stage->minLatency();
	ready_time.max = 0 ; 
	start_time.max = INFINITE_TIME;
	finish_time.max = INFINITE_TIME;
	latency.min = stage->minLatency();
	latency.max = stage->maxLatency();	
}

// ---------- pipelineStage()

inline PipelineStage * ExecutionNode::pipelineStage(void) {
	return pipeline_stage;
}

// ---------- instruction()

inline Inst * ExecutionNode::instruction(void) {
	return inst;
}


// ---------- instIndex()

inline int ExecutionNode::instIndex(void) {
	return inst_index;
}

// ---------- stageIndex()

inline int ExecutionNode::stageIndex(void) {
	return pipeline_stage->index();
}

// ---------- minReadyTime

inline int ExecutionNode::minReadyTime(void) {
	return ready_time.min;
}

// ---------- setMinReadyTime

inline void ExecutionNode::setMinReadyTime(int time)  {
	ready_time.min = time;
}

// ---------- maxReadyTime

inline int ExecutionNode::maxReadyTime(void) {
	return ready_time.max;
}

// ---------- setMaxreadyTime

inline void ExecutionNode::setMaxReadyTime(int time)  {
	ready_time.max = time;
}

// ---------- minstartTime

inline int ExecutionNode::minStartTime(void) {
	return start_time.min;
}

// ---------- setMinStartTime

inline void ExecutionNode::setMinStartTime(int time)  {
	start_time.min = time;
}

// ---------- maxStartTime

inline int ExecutionNode::maxStartTime(void) {
	return start_time.max;
}

// ---------- setMaxStartTime

inline void ExecutionNode::setMaxStartTime(int time)  {
	start_time.max = time;
}

// ---------- minFinishTime

inline int ExecutionNode::minFinishTime(void) {
	return finish_time.min;
}

// ---------- setMinFinishTime

inline void ExecutionNode::setMinFinishTime(int time)  {
	finish_time.min = time;
}

// ---------- maxFinishTime

inline int ExecutionNode::maxFinishTime(void) {
	return finish_time.max;
}

// ---------- setMaxFinishTime

inline void ExecutionNode::setMaxFinishTime(int time)  {
	finish_time.max = time;
}

// ---------- minLatency

inline int ExecutionNode::minLatency(void) {
	return latency.min;
}

// ---------- setMinLatency

inline void ExecutionNode::setMinLatency(int lat)  {
	latency.min = lat;
}

// ---------- maxLatency

inline int ExecutionNode::maxLatency(void) {
	return latency.max;
}

// ---------- setMaxLatency

inline void ExecutionNode::setMaxLatency(int lat)  {
	latency.max = lat;
}

// ---------- needsOperands

inline bool ExecutionNode::needsOperands(void) {
	return needs_operands;
}

// ---------- setNeedsOperands

inline void ExecutionNode::setNeedsOperands(bool val)  {
	needs_operands = val;
}

// ---------- producesOperands

inline bool ExecutionNode::producesOperands(void) {
	return produces_operands;
}

// ---------- setProducesOperands

inline void ExecutionNode::setProducesOperands(bool val)  {
	produces_operands = val;
}

// ---------- addContender

inline void ExecutionNode::addContender(ExecutionNode *cont) {
	contenders.add(cont);
}

// ---------- shade()

inline void ExecutionNode::shade(void) {
	shaded = true;
}

// ---------- isShaded()

inline bool ExecutionNode::isShaded(void) {
	return shaded;
}

// ---------- dumpPart()

inline void ExecutionNode::dumpPart(elm::io::Output& out_stream) {
	switch(code_part) {
		case BEFORE_PROLOGUE:
			out_stream << "[BEFORE_PROLOGUE]";
			break;
		case PROLOGUE:
			out_stream << "[PROLOGUE]";
			break;
		case BODY:
			out_stream << "[BODY]";
			break;
		case EPILOGUE:
			out_stream << "[EPILOGUE]";
			break;
	}
			
}

// ---------- part()

inline ExecutionNode::code_part_t ExecutionNode::part(void) const {
	return code_part;
}

// ---------- ContenderIterator

inline ExecutionNode::ContenderIterator::ContenderIterator(const ExecutionNode *node):
elm::genstruct::Vector<ExecutionNode *>::Iterator(node->contenders) {
}

// ---------- dumpTime()

inline void ExecutionNode::dumpTime(int time, elm::io::Output& out_stream) {
	switch(time) {
		case -INFINITE_TIME:
			out_stream << "-INF";
			break;
		case INFINITE_TIME:
			out_stream << "+INF";
			break;
		default:
			if (time < -INFINITE_TIME+10)	// ----------------------------- to be fixed
				out_stream << "-INF";
			else
				if (time > INFINITE_TIME-10)
					out_stream << "-INF";
				else
					out_stream << time;
			break;
	}
}

// ---------- dump()

inline void ExecutionNode::dump(elm::io::Output& out_stream) {
	out_stream << "\t" << pipeline_stage->shortName();
	out_stream << "(I" << inst_index <<")";
	if (needs_operands)
		out_stream << "\n\t   needs operands";
	if (produces_operands)
		out_stream << "\n\t   produces operands";		
	if (this->hasPred()) {
		out_stream << "\n\t   predecessors: ";
		for (graph::Node::Predecessor pred(this) ; pred ; pred++) {
			out_stream << ((ExecutionNode *) *pred)->pipelineStage()->shortName();
			out_stream << "(I" << ((ExecutionNode *) *pred)->instIndex() <<")";
			((ExecutionEdge *) pred.edge())->dump(out_stream);
		}
	}
	if (this->hasSucc()) {
		out_stream << "\n\t   successors: ";
		for (graph::Node::Successor next(this) ; next ; next++) {
			out_stream << ((ExecutionNode *) *next)->pipelineStage()->shortName();
			out_stream << "(I" << ((ExecutionNode *) *next)->instIndex() <<")";
			((ExecutionEdge *)next.edge())->dump(out_stream);
		}
	}
	out_stream << "\n\t   contenders: ";
	for(ContenderIterator cont(this); cont; cont ++) {
		cont->dumpLight(out_stream);
	}
	out_stream << "\n\t   times: r.min="; this->dumpTime(ready_time.min, out_stream);
	out_stream << "/r.max="; this->dumpTime(ready_time.max, out_stream);
	out_stream << " - s.min="; this->dumpTime(start_time.min, out_stream);
	out_stream << "/s.max="; this->dumpTime(start_time.max, out_stream);
	out_stream << " - f.min="; this->dumpTime(finish_time.min, out_stream);
	out_stream << "/f.max="; this->dumpTime(finish_time.max, out_stream);  
}

// ---------- dumpLight()

inline void ExecutionNode::dumpLight(elm::io::Output& out_stream) {
	out_stream << pipeline_stage->shortName();
	out_stream << "(I" << inst_index <<")";
}

// ---------- dumpLightTimed()

inline void ExecutionNode::dumpLightTimed(elm::io::Output& out_stream) {
	out_stream << "\t" << pipeline_stage->shortName();
	out_stream << "(I" << inst_index <<")";
	out_stream << "["; dumpTime(ready_time.min, out_stream);
	out_stream << "/"; dumpTime(ready_time.max, out_stream);out_stream << "]";
	out_stream << "["; dumpTime(start_time.min, out_stream);
	out_stream << "/"; dumpTime(start_time.max, out_stream);out_stream << "]";
	out_stream << "["; dumpTime(finish_time.min, out_stream);
	out_stream << "/"; dumpTime(finish_time.max, out_stream);out_stream << "]";
}


// -----------------------------------------------------------
// ExecutionEdge functions
// -----------------------------------------------------------

// ---------- constructor

inline ExecutionEdge::ExecutionEdge(ExecutionNode *source, ExecutionNode *target, edge_type_t type)
: Edge(source,target), edge_type(type) {
}

// ---------- type()

inline ExecutionEdge::edge_type_t ExecutionEdge::type(void) const {
	return edge_type;
}

// ---------- dump()

inline void ExecutionEdge::dump(elm::io::Output& out_stream) {
	switch(edge_type) {
		case SOLID:
			out_stream << "[SOLID] - ";
			break;
		case SLASHED:
			out_stream << "[SLASHED] - ";
			break;
		default:
			out_stream << " - ";
			break;
	}
}


// -----------------------------------------------------------
// ExecutionGraph::NodePair functions
// -----------------------------------------------------------

// ---------- constructor

inline ExecutionGraph::NodePair::NodePair(ExecutionNode *n1, ExecutionNode *n2, bool sep) :
node1(n1), node2(n2), separated(sep){
}

// ---------- firstNode()

inline ExecutionNode * ExecutionGraph::NodePair::firstNode(void) {
	return(node1);
}

// ---------- secondNode()

inline ExecutionNode * ExecutionGraph::NodePair::secondNode(void) {
	return(node2);
}

// ---------- isSeparated()

inline bool ExecutionGraph::NodePair::isSeparated(void) {
	return(separated);
}

// ---------- setSeparated()

inline void ExecutionGraph::NodePair::setSeparated(bool sep) {
	separated = sep;
}

// -----------------------------------------------------------
// ExecutionGraph functions
// -----------------------------------------------------------

// ---------- constructor

inline ExecutionGraph::ExecutionGraph()
: entry_node(NULL) {
	for (int i=0 ; i<ExecutionNode::CODE_PARTS_NUMBER ; i++) {
		first_node[i] = NULL;
		last_node[i] = NULL;
	}
}

// ---------- initSeparated()

inline void ExecutionGraph::initSeparated() {
	int index = 0;
	NodePair *pair;
	for(NodeIterator u(this); u; u++){
		for(NodeIterator v(this); v; v++){
			pair = new NodePair((ExecutionNode *)*u, (ExecutionNode *)*v, false);
			pairs.addLast(pair);
			index++;
		}
	}	
}

// ---------- entryNode()

inline ExecutionNode * ExecutionGraph::entryNode(void) {
	return entry_node;
}

// ---------- setEntryNode()

inline void ExecutionGraph::setEntryNode(ExecutionNode *node) {
	entry_node = node;
}

// ---------- firstNode()

inline ExecutionNode * ExecutionGraph::firstNode(ExecutionNode::code_part_t part) {
	return first_node[part];
}
// ---------- setFirstNode()

inline void ExecutionGraph::setFirstNode(ExecutionNode::code_part_t part, ExecutionNode *node) {
	first_node[part] = node;
}

// ---------- lastNode()

inline ExecutionNode * ExecutionGraph::lastNode(ExecutionNode::code_part_t part) {
	return last_node[part];
}

// ---------- setLastNode()

inline void ExecutionGraph::setLastNode(ExecutionNode::code_part_t part, ExecutionNode *node) {
	last_node[part] = node;
}

// ---------- stagesNodesLists()

inline elm::inhstruct::DLList* ExecutionGraph::stagesNodesLists(void) {
	return &stages_nodes_lists;
}

// ---------- instructionsNodesLists()

inline elm::inhstruct::DLList* ExecutionGraph::instructionsNodesLists(void) {
	return &instructions_nodes_lists;
}

// ---------- separated()

inline bool ExecutionGraph::separated(ExecutionNode *u, ExecutionNode *v, elm::io::Output& out_stream) {
	if (v->minReadyTime() >= u->maxFinishTime())
		return true;
	if (u->minReadyTime() >= v->maxFinishTime())
		return true;
	return false;
}

// ---------- unchangedSeparated()

inline bool ExecutionGraph::unchangedSeparated(elm::io::Output& out_stream) {
	int index = 0;
	bool unchanged = true;
	
	for (elm::genstruct::DLList<NodePair *>::Iterator pair(this->pairs) ; pair ; pair++) {
			if ( ((NodePair *) *pair)->isSeparated() != separated(((NodePair *) *pair)->firstNode(), ((NodePair *) *pair)->secondNode(), out_stream)) {			
				unchanged = false;
				((NodePair *) *pair)->setSeparated( separated(((NodePair *) *pair)->firstNode(), ((NodePair *) *pair)->secondNode(), out_stream) );
			}
			index++;
		}
	return unchanged;
}


// ---------- dumpLight()

inline void ExecutionGraph::dumpLight(elm::io::Output& out_stream) {
	out_stream << "\tDumping the execution graph ...\n";
	out_stream << "\t\tEntry node: ";
	this->entry_node->dumpLight(out_stream);
	out_stream << " \n";
	for (int i=0 ; i<ExecutionNode::CODE_PARTS_NUMBER ; i++) {
		if (first_node[i] != NULL) {
			out_stream << "\t\t";
			first_node[i]->dumpPart(out_stream);
			out_stream << ": first_node=";
			first_node[i]->dumpLight(out_stream);
			out_stream << ", last_node=";
			last_node[i]->dumpLight(out_stream);
			out_stream << "\n";
		}
	}
}

// ---------- dump()

inline void ExecutionGraph::dump(elm::io::Output& out_stream) {
	out_stream << "\tDumping the execution graph ...\n";
	out_stream << "\t\tEntry node: ";
	this->entry_node->dumpLight(out_stream);
	out_stream << " \n";
	for (int i=0 ; i<ExecutionNode::CODE_PARTS_NUMBER ; i++) {
		if (first_node[i] != NULL) {
			out_stream << "\t\t";
			first_node[i]->dumpPart(out_stream);
			out_stream << ": first_node=";
			first_node[i]->dumpLight(out_stream);
			out_stream << ", last_node=";
			last_node[i]->dumpLight(out_stream);
			out_stream << "\n";
		}
	}
	for(NodeIterator node(this); node; node++){
		((ExecutionNode *) *node)->dump(out_stream);
		out_stream << "\n";
	}
	out_stream << "\n";
	if (this->entry_node != NULL) {
		out_stream << "\nTopological order:\n";
		for(PreorderIterator node(this, this->entry_node); node; node++) {
			out_stream << ((ExecutionNode *) *node)->pipelineStage()->shortName();
			out_stream << "(I" << ((ExecutionNode *) *node)->instIndex() <<") - ";
		}
	}
}

// ---------- dotDump()

inline void ExecutionGraph::dotDump(elm::io::Output& dotFile, bool dump_times) {
	GraphNodesListInList *stage_node_list, *inst_node_list;
	GraphNodeInList *node_in_list;
	
	dotFile << "digraph G {\n";
	inst_node_list = (GraphNodesListInList *) instructions_nodes_lists.first();
	while (!inst_node_list->atEnd()) {
		// dump nodes
		node_in_list = (GraphNodeInList *) inst_node_list->list()->first();
		dotFile << "{ rank = same ; ";
		while (!node_in_list->atEnd()) {
			dotFile << "\"" << node_in_list->executionNode()->pipelineStage()->shortName();
			dotFile << "I" << node_in_list->executionNode()->instIndex() << "\" ; ";
			node_in_list = (GraphNodeInList *) node_in_list->next();
		}
		dotFile << "}\n";
		// again to specify labels
		node_in_list = (GraphNodeInList *) inst_node_list->list()->first();
		while (!node_in_list->atEnd()) { 
			if (dump_times) {
				dotFile << "\"" << node_in_list->executionNode()->pipelineStage()->shortName();
				dotFile << "I" << node_in_list->executionNode()->instIndex() << "\"";
				dotFile << " [shape=record, ";
				if (node_in_list->executionNode()->isShaded())
					dotFile << "color=red, ";
				if (node_in_list->executionNode()->part() == ExecutionNode::BODY)
					dotFile << "color=blue, ";
				dotFile << "label=\"" << node_in_list->executionNode()->pipelineStage()->shortName();
				dotFile << "(I" << node_in_list->executionNode()->instIndex() << ") ";
				dotFile << "| { {";
				node_in_list->executionNode()->dumpTime(node_in_list->executionNode()->minReadyTime(),dotFile);
				dotFile  << "|";
				node_in_list->executionNode()->dumpTime(node_in_list->executionNode()->maxReadyTime(),dotFile);
				dotFile << "} | {";
				node_in_list->executionNode()->dumpTime(node_in_list->executionNode()->minStartTime(),dotFile);
				dotFile << "|";
				node_in_list->executionNode()->dumpTime(node_in_list->executionNode()->maxStartTime(),dotFile);
				dotFile << "} | {";
				node_in_list->executionNode()->dumpTime(node_in_list->executionNode()->minFinishTime(),dotFile);
				dotFile << "|";
				node_in_list->executionNode()->dumpTime(node_in_list->executionNode()->maxFinishTime(),dotFile);
				dotFile << "} }";		
				dotFile << "\"] ; \n";
			}
			else {
				if (node_in_list->executionNode()->isShaded()) {
					dotFile << "\"" << node_in_list->executionNode()->pipelineStage()->shortName();
					dotFile << "I" << node_in_list->executionNode()->instIndex() << "\"";
					dotFile << " [color=red] ; \n";
					
				}
				
			}
			node_in_list = (GraphNodeInList *) node_in_list->next();
		}
		dotFile << "\n";
		
		inst_node_list = (GraphNodesListInList *) inst_node_list->next();
	}
	int group_number = 0;
	inst_node_list = (GraphNodesListInList *) instructions_nodes_lists.first();
	while (!inst_node_list->atEnd()) {
		// dump edges
		node_in_list = (GraphNodeInList *) inst_node_list->list()->first();
		while (!node_in_list->atEnd()) {
				for (graph::Node::Successor next(node_in_list->executionNode()) ; next ; next++) {
					if ( (inst_node_list !=  (GraphNodesListInList *) instructions_nodes_lists.first())
						||
						(!node_in_list->executionNode()->producesOperands()) 
						|| (node_in_list->executionNode()->instIndex() == ((ExecutionNode *) *next)->instIndex()) ) {
						dotFile << "\"" << node_in_list->executionNode()->pipelineStage()->shortName();
						dotFile << "I" << node_in_list->executionNode()->instIndex() << "\"";
						dotFile << " -> ";
						dotFile << "\"" << ((ExecutionNode *) *next)->pipelineStage()->shortName();
						dotFile << "I" << ((ExecutionNode *) *next)->instIndex() << "\"";
						switch( ((ExecutionEdge *) next.edge())->type()) {
							case ExecutionEdge::SOLID:
								if (node_in_list->executionNode()->instIndex() == ((ExecutionNode *) *next)->instIndex())
									dotFile << "[minlen=4]";
								dotFile << " ;\n";
								break;
							case ExecutionEdge::SLASHED:
								dotFile << " [style=dotted";
								if (node_in_list->executionNode()->instIndex() == ((ExecutionNode *) *next)->instIndex())
									dotFile << ", minlen=4";
								dotFile << "] ;\n";
								break;	
							default:
								break;
						}	
						if ((node_in_list->executionNode()->instIndex() == ((ExecutionNode *) *next)->instIndex())
								|| ((node_in_list->executionNode()->stageIndex() == ((ExecutionNode *) *next)->stageIndex())
									&& (node_in_list->executionNode()->instIndex() == ((ExecutionNode *) *next)->instIndex()-1)) ) {
							dotFile << "\"" << node_in_list->executionNode()->pipelineStage()->shortName();
							dotFile << "I" << node_in_list->executionNode()->instIndex() << "\"  [group=" << group_number << "] ;\n";
							dotFile << "\"" << ((ExecutionNode *) *next)->pipelineStage()->shortName();
							dotFile << "I" << ((ExecutionNode *) *next)->instIndex() << "\" [group=" << group_number << "] ;\n";
							group_number++;
						}
					}
				// dump contenders
	//			for(ExecutionNode::ContenderIterator cont(node_in_list->executionNode()); cont; cont ++) {
	//				if (cont->instIndex() > node_in_list->executionNode()->instIndex()) {
	//					dotFile << node_in_list->executionNode()->pipelineStage()->shortName();
	//					dotFile << "I" << node_in_list->executionNode()->instIndex();
	//					dotFile << " -> ";
	//					dotFile << cont->pipelineStage()->shortName();
	//					dotFile << "I" << cont->instIndex();
	//					dotFile << " [style=dashed, dir=none] ; \n";
	//				}
	//			}
				}		
			node_in_list = (GraphNodeInList *) node_in_list->next();
		}
		dotFile << "\n";
		inst_node_list = (GraphNodesListInList *) inst_node_list->next();
	}
	
	dotFile << "}\n";
}


// -----------------------------------------------------------
// Path functions
// -----------------------------------------------------------

// ---------- addNodeFirst()

inline void Path::addNodeFirst(ExecutionNode *node) {
	node_list.addFirst(node);
}

// ---------- NodeIterator constructor

inline Path::NodeIterator::NodeIterator(Path *path) 
: elm::genstruct::DLList<ExecutionNode *>::Iterator(path->node_list) {
}

// ---------- dump()

inline void Path::dump(elm::io::Output& out_stream) {
	for(NodeIterator node(this); node; node++) {
		((ExecutionNode *) *node)->dumpLight(out_stream);
		out_stream << "-";
	}
}


// -----------------------------------------------------------
// PathList functions
// -----------------------------------------------------------

// ---------- addPath()

inline void PathList::addPath(Path *path) {
	path_list.addLast(path);
}

// ---------- PathIterator constructor

inline PathList::PathIterator::PathIterator(PathList *list) 
: elm::genstruct::DLList<Path *>::Iterator(list->path_list) {
}

// ---------- dump()

inline void PathList::dump(elm::io::Output& out_stream) {
	for(PathIterator path(this); path; path++) {
		((Path *) *path)->dump(out_stream);
		out_stream << "\n";
	}
}

// -----------------------------------------------------------
// GraphNodeInList functions
// -----------------------------------------------------------

// ---------- constructor

inline GraphNodeInList::GraphNodeInList(ExecutionNode* node)
: execution_node(node) {
}

// ---------- executionNode()

inline ExecutionNode * GraphNodeInList::executionNode(void) {
	return execution_node;
}

// ---------- dump()

inline void GraphNodeInList::dump(elm::io::Output& out_stream) {
	execution_node->dumpLight(out_stream);
}
		
// -----------------------------------------------------------
// GraphNodesListInList functions
// -----------------------------------------------------------


// ---------- constructors

inline GraphNodesListInList::GraphNodesListInList(PipelineStage* stage) 
: pipeline_stage(stage), instruction(NULL), stage_index(stage->index()) {
}

inline GraphNodesListInList::GraphNodesListInList(Inst* inst, int index, ExecutionNode::code_part_t part)
: instruction(inst), pipeline_stage(NULL), inst_index(index), code_part(part) {
}
		
// ---------- list()

inline elm::inhstruct::DLList * GraphNodesListInList::list(void) {
	return &node_list;
}

// ---------- stage()

inline PipelineStage * GraphNodesListInList::stage(void) {
	return pipeline_stage;
}

// ---------- inst()

inline Inst * GraphNodesListInList::inst(void) {
	return instruction;
}

// ---------- instIndex()

inline int GraphNodesListInList::instIndex(void) const {
	return inst_index;
}

// ---------- stageIndex()

inline int GraphNodesListInList::stageIndex(void) const {
	return stage_index;
}

// ---------- part()

inline ExecutionNode::code_part_t GraphNodesListInList::part(void) const {
	return code_part;
}


// ---------- dump()

inline void GraphNodesListInList::dump(elm::io::Output& out_stream) {
	if (pipeline_stage != NULL) {
		out_stream << "list of nodes for stage " << pipeline_stage->name() << "(";
		out_stream << pipeline_stage->shortName() << "): ";
	}
	else {
		if (instruction != NULL) {
			out_stream << "list of nodes for instruction ";
			instruction->dump(out_stream);
		}
		else
			out_stream << "list of nodes for ??? :";
	}
	GraphNodeInList *node = (GraphNodeInList *) node_list.first();
	if (node != NULL) {
		while (!node->atEnd()) {
			node->dump(out_stream);
			node = (GraphNodeInList *) node->next();
		}
	}
}



} // otawa

#endif // EXECUTION_GRAPH
