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
#include <otawa/util/GenGraph.h>
#include <otawa/exegraph/Microprocessor.h>
#include <otawa/instruction.h>
#include <elm/inhstruct/DLList.h>
#include <elm/genstruct/DLList.h>
#include <elm/util/BitVector.h>


namespace otawa { 
	
#define INFINITE_TIME 0x0FFFFFF

extern Identifier<bool> START;

class ExecutionGraph;
class ExecutionEdge;
class ExecutionNode;

typedef enum code_part_t {
	BEFORE_PROLOGUE = 0,
	PROLOGUE = 1,
	PREFIX = 2,
	BODY = 3,
	EPILOGUE = 4,
	CODE_PARTS_NUMBER  // should be the last value
} code_part_t;


class ExecutionGraphInstruction {
	Inst * _inst;
	BasicBlock *_bb;
	int _index;
	code_part_t _part;
	elm::genstruct::DLList<ExecutionNode *> _nodes;
	public:
		inline ExecutionGraphInstruction(Inst * inst, BasicBlock *bb, code_part_t part, int index);
		inline Inst * inst();
		inline code_part_t codePart();
		inline int index();
		inline void setIndex(int index);
		inline void addNode(ExecutionNode * node);
		inline void deleteNodes();
		inline ExecutionNode * firstNode();
		inline ExecutionNode * lastNode();
		inline BasicBlock * basicBlock();
		class ExecutionNodeIterator: public elm::genstruct::DLList<ExecutionNode *>::Iterator {
			public:
				inline ExecutionNodeIterator(const ExecutionGraphInstruction *inst);
			};
		
};

inline ExecutionGraphInstruction::ExecutionGraphInstruction(Inst * inst, BasicBlock *bb, code_part_t part, int index)
	: _inst(inst), _bb(bb), _part(part), _index(index) {
}

inline void ExecutionGraphInstruction::setIndex(int index) {
	_index = index;
}
inline Inst * ExecutionGraphInstruction::inst() {
	return _inst;
}
inline code_part_t ExecutionGraphInstruction::codePart() {
	return _part;
}
inline int ExecutionGraphInstruction::index() {
	return _index;
}
inline void ExecutionGraphInstruction::addNode(ExecutionNode * node) {
	_nodes.addLast(node);
}

inline ExecutionNode * ExecutionGraphInstruction::firstNode() {
	return _nodes.first();
}

inline ExecutionNode * ExecutionGraphInstruction::lastNode() {
	return _nodes.last();
}

inline BasicBlock * ExecutionGraphInstruction::basicBlock() {
	return _bb;
}

inline void ExecutionGraphInstruction::deleteNodes() {
	_nodes.clear();
}

inline ExecutionGraphInstruction::ExecutionNodeIterator::ExecutionNodeIterator(const ExecutionGraphInstruction *inst):
	elm::genstruct::DLList<ExecutionNode *>::Iterator(inst->_nodes) {
}


// -----------------------------------------------------------
// ExecutionNode class definition
// -----------------------------------------------------------

class ExecutionNode: public GenGraph<ExecutionNode, ExecutionEdge>::Node {
	friend class ExecutionGraph;
	public:
		typedef struct time_interval_t {
			int min;
			int max;
		} time_interval_t;
		
		
		int pair_index;
		bool changed;
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
		bool _has_path_to_CMI0;
		int _max_time_to_CMI0;
		int _min_time_to_IFI1;
		elm::genstruct::Vector<ExecutionNode *> contenders;
		elm::String _name;
	public:
		inline ExecutionNode(ExecutionGraph * graph, 
							PipelineStage *stage, 
							Inst *instruction, 
							int index, 
							code_part_t part);
		inline void init (ExecutionGraph * graph, 
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
		inline bool hasPathToCMI0();
		inline void setHasPathToCMI0();
		inline int maxTimeToCMI0();
		inline void setMaxTimeToCMI0(int time);
		inline int minTimeToIFI1();
		inline void setMinTimeToIFI1(int time);
		void dump(elm::io::Output& out_stream);
		void dumpLight(elm::io::Output& out_stream);
		void dumpLightTimed(elm::io::Output& out_stream);
		void dumpTime(int time, elm::io::Output& out_stream);
		inline void addContender(ExecutionNode *cont);
		inline code_part_t part(void) const;
		void dumpPart(elm::io::Output& out_stream);
		inline void shade(void);
		inline bool isShaded(void);
		inline elm::String name();
		class ContenderIterator: public elm::genstruct::Vector<ExecutionNode *>::Iterator {
			public:
				inline ContenderIterator(const ExecutionNode *node);
			};
};


 inline bool ExecutionNode::hasPathToCMI0() {
   return _has_path_to_CMI0;
 }

 inline void ExecutionNode::setHasPathToCMI0() {
   _has_path_to_CMI0 = true;
 }

 inline int ExecutionNode::maxTimeToCMI0(){
   return _max_time_to_CMI0;
 }

 inline void ExecutionNode::setMaxTimeToCMI0(int time) {
   _max_time_to_CMI0 = time;
 }
 
 inline int ExecutionNode::minTimeToIFI1(){
   return _min_time_to_IFI1;
 }

 inline void ExecutionNode::setMinTimeToIFI1(int time) {
   _min_time_to_IFI1 = time;
 }
 

// -----------------------------------------------------------
// ExecutionEdge class definition
// -----------------------------------------------------------

class ExecutionEdge: public GenGraph<ExecutionNode, ExecutionEdge>::Edge {
	
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
//		inline void init(ExecutionNode *source, ExecutionNode *target, edge_type_t type);
		inline edge_type_t type(void) const;
		void dump(elm::io::Output& out_stream);
		inline String name();
};

// -----------------------------------------------------------
// ExecutionGraph class definition
// -----------------------------------------------------------

class ExecutionGraph:  public GenGraph<ExecutionNode, ExecutionEdge>  {
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
		
		int pair_cnt;
	private:
		ExecutionNode * entry_node;
		elm::inhstruct::DLList stages_nodes_lists;
		elm::inhstruct::DLList instructions_nodes_lists;
		elm::BitVector *pairs;
		ExecutionNode * first_node[CODE_PARTS_NUMBER];
		ExecutionNode * last_node[CODE_PARTS_NUMBER];
		elm::genstruct::DLList<ExecutionGraphInstruction *> * _sequence;
		int min_delta;
		int _capacity;
		bool _times_changed;
		elm::genstruct::DLList<ExecutionNode *> early_nodes_to_IFI1;
		
	public:
		ExecutionGraph(int capacity);
		~ExecutionGraph(void);
		inline void setTimesChanged();
		inline void setEntryNode(ExecutionNode *node);
		void dump(elm::io::Output& out_stream);
		void dumpLight(elm::io::Output& out_stream);
		inline ExecutionNode * entryNode(void);
		void dotDump(elm::io::Output& dotFile, bool dump_times);
		inline elm::inhstruct::DLList * stagesNodesLists(void);
		inline elm::inhstruct::DLList * instructionsNodesLists(void);
		void initSeparated(void);
		bool unchangedSeparated();
		inline bool separated(ExecutionNode *u, ExecutionNode *v);
		void latestTimes();
//		void prologueLatestTimes(ExecutionNode *node);
		void bodyLatestTimes(ExecutionNode *node);
//		void epilogueLatestTimes(ExecutionNode *node);
//		void prologueEarliestTimes(ExecutionNode *node);
		void prologueMinTimeToCMI0(ExecutionNode *node);
		void bodyEarliestTimes(ExecutionNode *node);
//		void epilogueEarliestTimes(ExecutionNode *node, elm::io::Output& out_stream);
		void earliestTimes() ;
		int minDelta();
		void findMinPathsToIFI1(ExecutionNode * node, int length);
		void shadeNodes() ;
		void findPaths(ExecutionNode * node);
		void shadePreds(ExecutionNode *node) ;
		inline void setFirstNode(code_part_t part, ExecutionNode *node);
		inline void setLastNode(code_part_t part, ExecutionNode *node);
		inline ExecutionNode * firstNode(code_part_t part);
		inline ExecutionNode * lastNode(code_part_t part);
		void build(FrameWork *fw, Microprocessor* microprocessor, 
					elm::genstruct::DLList<ExecutionGraphInstruction *> &sequence);
		int analyze();
};

inline void ExecutionGraph::setTimesChanged() {
	_times_changed = true;
}


// -----------------------------------------------------------
// GraphNodeInList class definition
// -----------------------------------------------------------

class GraphNodeInList: public elm::inhstruct::DLNode {
	private:
		ExecutionNode *execution_node;
	public:
		inline GraphNodeInList(ExecutionNode* node);
		inline ExecutionNode * executionNode(void);
		void dump(elm::io::Output& out_stream);
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
		code_part_t code_part;
		
	public:
		inline GraphNodesListInList(PipelineStage* stage);
		inline GraphNodesListInList(Inst* inst, int index, code_part_t part);
		inline elm::inhstruct::DLList * list(void);
		inline PipelineStage * stage(void);
		inline Inst * inst(void);
		void dump(elm::io::Output& out_stream);
		inline int instIndex(void) const;
		inline int stageIndex(void) const;
		code_part_t part(void) const;
		
};

// -----------------------------------------------------------
// Path class definition
// -----------------------------------------------------------

class Path {
	private:
		elm::genstruct::DLList<ExecutionNode *> node_list;
	public:
		inline void clear();
		inline void addNodeFirst(ExecutionNode *node);
		void dump(elm::io::Output& out_stream);
		
		class NodeIterator : public elm::genstruct::DLList<ExecutionNode *>::Iterator {
			public:
				inline NodeIterator(Path *path);
		};
		
};

 inline void Path::clear() {
     while (!node_list.isEmpty()) {
         node_list.removeFirst();
   }
 }



// -----------------------------------------------------------
// PathList class definition
// -----------------------------------------------------------

class PathList {
	private:
		elm::genstruct::DLList<Path *> path_list;
	public:
		inline void clear();
		inline void addPath(Path *path);
		void dump(elm::io::Output& out_stream);
		
		class PathIterator : public elm::genstruct::DLList<Path *>::Iterator {
			public:
				inline PathIterator(PathList *list);
		};
		
};

 inline void PathList::clear() {
   Path * path;
   while (!path_list.isEmpty()) {
     path = path_list.first();
     path_list.removeFirst();
     path->clear();    
   }
 }


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
produces_operands(false), shaded(false), code_part(part), Node((otawa::graph::Graph *)graph), changed(false), 
_has_path_to_CMI0(false), _max_time_to_CMI0(0), _min_time_to_IFI1(0) {
	if (code_part >= BODY) {
		ready_time.min = 0; // needed ?
		start_time.min = 0;
		finish_time.min = stage->minLatency();
	}
	else {
		ready_time.min = -INFINITE_TIME; // needed ?
		start_time.min = -INFINITE_TIME;
		finish_time.min = -INFINITE_TIME;
	}
	ready_time.max = 0 ; 
	start_time.max = INFINITE_TIME;
	finish_time.max = INFINITE_TIME;
	latency.min = stage->minLatency();
	latency.max = stage->maxLatency();	
	StringBuffer _buffer;
	_buffer << stage->shortName() << "(I" << index << ")";
	_name = _buffer.toString();
}

// ---------- init

inline void ExecutionNode::init(ExecutionGraph * graph, 
									PipelineStage *stage, 
									Inst *instruction, 
									int index, 
									code_part_t part) {
	execution_graph = graph;
	pipeline_stage = stage;
 	inst = instruction; 
 	inst_index = index;
 	needs_operands = false;
	produces_operands = false;
	shaded = false; 
	code_part = part;
	changed = false; 
	_has_path_to_CMI0 = false;
	_max_time_to_CMI0 = 0;
	_min_time_to_IFI1 = 0;
	if (code_part >= PREFIX) {
		ready_time.min = 0; // needed ?
		start_time.min = 0;
		finish_time.min = stage->minLatency();
	}
	else {
		ready_time.min = -INFINITE_TIME; // needed ?
		start_time.min = -INFINITE_TIME;
		finish_time.min = -INFINITE_TIME;
	}
	ready_time.max = 0 ; 
	start_time.max = INFINITE_TIME;
	finish_time.max = INFINITE_TIME;
	latency.min = stage->minLatency();
	latency.max = stage->maxLatency();	
	StringBuffer _buffer;
	_buffer << stage->shortName() << "(I" << index << ")";
	_name = _buffer.toString();
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
	if(time != ready_time.min) {
		ready_time.min = time;
		changed = true;
		execution_graph->setTimesChanged();
	}
}

// ---------- maxReadyTime

inline int ExecutionNode::maxReadyTime(void) {
	return ready_time.max;
}

// ---------- setMaxreadyTime

inline void ExecutionNode::setMaxReadyTime(int time)  {
	if (time != ready_time.max) {
		ready_time.max = time;
		execution_graph->setTimesChanged();
	}
}

// ---------- minstartTime

inline int ExecutionNode::minStartTime(void) {
	return start_time.min;
}

// ---------- setMinStartTime

inline void ExecutionNode::setMinStartTime(int time)  {
	if (time != start_time.min) {
		start_time.min = time;
		execution_graph->setTimesChanged();
	}		
}

// ---------- maxStartTime

inline int ExecutionNode::maxStartTime(void) {
	return start_time.max;
}

// ---------- setMaxStartTime

inline void ExecutionNode::setMaxStartTime(int time)  {
	if (time != start_time.max) {
		start_time.max = time;
		execution_graph->setTimesChanged();
	}
}

// ---------- minFinishTime

inline int ExecutionNode::minFinishTime(void) {
	return finish_time.min;
}

// ---------- setMinFinishTime

inline void ExecutionNode::setMinFinishTime(int time)  {
	if (time != finish_time.min) {
		finish_time.min = time;
		execution_graph->setTimesChanged();
	}
}

// ---------- maxFinishTime

inline int ExecutionNode::maxFinishTime(void) {
	return finish_time.max;
}

// ---------- setMaxFinishTime

inline void ExecutionNode::setMaxFinishTime(int time)  {
	if(time != finish_time.max) {
		finish_time.max = time;
		changed = true;
		execution_graph->setTimesChanged();
	}
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

// ---------- part()

inline code_part_t ExecutionNode::part(void) const {
	return code_part;
}

// ---------- name()

inline String ExecutionNode::name(void)  {
	return _name;
}

// ---------- ContenderIterator

inline ExecutionNode::ContenderIterator::ContenderIterator(const ExecutionNode *node):
elm::genstruct::Vector<ExecutionNode *>::Iterator(node->contenders) {
}

// -----------------------------------------------------------
// ExecutionEdge functions
// -----------------------------------------------------------

// ---------- constructor

inline ExecutionEdge::ExecutionEdge(ExecutionNode *source, ExecutionNode *target, edge_type_t type)
: Edge(source,target), edge_type(type) {
}

// ---------- init

//inline void ExecutionEdge::init(ExecutionNode *source, ExecutionNode *target, edge_type_t type) {
//: Edge(source,target), edge_type(type) {
//}

// ---------- type()

inline ExecutionEdge::edge_type_t ExecutionEdge::type(void) const {
	return edge_type;
}

// ---------- name()

inline String ExecutionEdge::name(void) {
	StringBuffer buffer;
	buffer << source()->name() << "->" << target()->name();
	return buffer.toString();
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

// ---------- entryNode()

inline ExecutionNode * ExecutionGraph::entryNode(void) {
	return entry_node;
}

// ---------- setEntryNode()

inline void ExecutionGraph::setEntryNode(ExecutionNode *node) {
	entry_node = node;
}

// ---------- firstNode()

inline ExecutionNode * ExecutionGraph::firstNode(code_part_t part) {
	return first_node[part];
}
// ---------- setFirstNode()

inline void ExecutionGraph::setFirstNode(code_part_t part, ExecutionNode *node) {
	first_node[part] = node;
}

// ---------- lastNode()

inline ExecutionNode * ExecutionGraph::lastNode(code_part_t part) {
	return last_node[part];
}

// ---------- setLastNode()

inline void ExecutionGraph::setLastNode(code_part_t part, ExecutionNode *node) {
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

inline bool ExecutionGraph::separated(ExecutionNode *u, ExecutionNode *v) {
	if (v->minReadyTime() >= u->maxFinishTime())
		return true;
	if (u->minReadyTime() >= v->maxFinishTime())
		return true;
	return false;
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

inline GraphNodesListInList::GraphNodesListInList(Inst* inst, int index, code_part_t part)
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

inline code_part_t GraphNodesListInList::part(void) const {
	return code_part;
}

} // otawa

#endif // EXECUTION_GRAPH
