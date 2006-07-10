/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	ExecutionGraph.cpp -- control flow graph classes implementation.
 */

#include <assert.h>
#include <elm/debug.h>
#include <elm/Collection.h>
#include <otawa/exegraph/ExecutionGraph.h>
#include <elm/debug.h>
#include <otawa/util/DFABitSet.h>
#include <elm/genstruct/DLList.h>
#include <otawa/otawa.h>

using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;

namespace otawa {
	

// ---------- prologueLatestTimes

void ExecutionGraph::prologueLatestTimes(ExecutionNode *node, elm::io::Output& out_stream) {
	
	int max, count, no_contention_time, tmp;
	TimeDLNode *ptr;
	elm::inhstruct::DLList times;
	bool first;
	
	first = true;
	for (graph::Node::Predecessor pred(node) ; pred ; pred++) {
		if (first) {
			max = ((ExecutionNode *) *pred)->maxFinishTime();
			first = false;
		}
		else {
			if (((ExecutionNode *) *pred)->maxFinishTime() > max)
				max = ((ExecutionNode *) *pred)->maxFinishTime();
		}
	}
	node->setMaxReadyTime(max);
	node->setMaxStartTime(node->maxReadyTime()+node->maxLatency()-1);
	
	count = 0;
	for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
		if ( ((ExecutionNode *)*cont)->instIndex() < node->instIndex() ) { // early contender
			if ( !separated((ExecutionNode *)*cont, node, out_stream) ) {
				count++;
				TimeDLNode *new_time = new TimeDLNode((( ExecutionNode *)*cont)->maxFinishTime());
				if (times.isEmpty())
					times.addLast(new_time);
				else {
					ptr = (TimeDLNode *) times.first();
					while ( (!ptr->atEnd()) && (ptr->getValue() > new_time->getValue()) )
						ptr = (TimeDLNode *) ptr->next();
					if (ptr->atEnd())
						times.addLast(new_time); // no lower value found
					else
						ptr->insertBefore(new_time);
				}
			}
		}
	}
	if (count >= node->pipelineStage()->width()) {
		no_contention_time = node->maxStartTime()
					 + node->maxLatency() 
					 	* (count / node->pipelineStage()->width());
		count = 1;
		ptr = (TimeDLNode *) times.first();
		while ( (!ptr->atEnd()) && (count <node->pipelineStage()->width())) {
			ptr = (TimeDLNode *) ptr->next();
			count++;
		}
		max = ptr->getValue();
		if (max > no_contention_time ) 
			tmp = no_contention_time;
		else
			tmp = max;
		if (tmp > node->maxStartTime() )
			node->setMaxStartTime(tmp);
	}
	
	// empty times list
	while (!times.isEmpty())
		times.removeLast();
		
	node->setMaxFinishTime(node->maxStartTime() + node->maxLatency() );
		
	// update successors
	for (graph::Node::Successor next(node) ; next ; next++) {
		if (node->maxFinishTime() > ((ExecutionNode *)*next)->maxReadyTime())
			if (((ExecutionEdge *)(next.edge()))->type() == ExecutionEdge::SOLID)
				((ExecutionNode *)*next)->setMaxReadyTime(node->maxFinishTime());
			else // SLASHED
				((ExecutionNode *)*next)->setMaxReadyTime(node->maxStartTime());
	}
}

// ---------- bodyLatestTimes

void ExecutionGraph::bodyLatestTimes(ExecutionNode *node, elm::io::Output& out_stream) {
	
	int max, count, no_contention_time, tmp;
	TimeDLNode *ptr;
	elm::inhstruct::DLList times;
	
	node->setMaxStartTime(node->maxReadyTime());
	count = 0;		
	for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
		if ( ((ExecutionNode *)*cont)->instIndex() > node->instIndex() ) {
			// cont is a *late* contender
			if ( !separated((ExecutionNode *)*cont, node, out_stream)
					&&
					( (( ExecutionNode *)*cont)->minStartTime() < node->maxReadyTime()) ) {
				count++;
				TimeDLNode *new_time = new TimeDLNode((( ExecutionNode *)*cont)->maxFinishTime());
				if (times.isEmpty())
					times.addLast(new_time);
				else {
					ptr = (TimeDLNode *) times.first();
					while ( (!ptr->atEnd()) && (ptr->getValue() > new_time->getValue()) )
						ptr = (TimeDLNode *) ptr->next();
					if (ptr->atEnd())
						times.addLast(new_time); // no lower value found
					else
						ptr->insertBefore(new_time);
				}
			}
		}
	}
	if (count >= node->pipelineStage()->width()) {
		no_contention_time = node->maxReadyTime() + node->maxLatency() - 1;
		ptr = (TimeDLNode *) times.first();
		while ( (!ptr->atEnd()) && (count < node->pipelineStage()->width())) {
			ptr = (TimeDLNode *) ptr->next();
			count++;
		}
		max = ptr->getValue();
		if (max > no_contention_time ) 
			node->setMaxStartTime(no_contention_time);
		else
			node->setMaxStartTime(max);
	}
	while (!times.isEmpty())
		times.removeLast();
	
	count = 0;
	for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
		if ( ((ExecutionNode *)*cont)->instIndex() < node->instIndex() ) {
			if ( !separated((ExecutionNode *)*cont, node, out_stream) ) {
				count++;
				TimeDLNode *new_time = new TimeDLNode((( ExecutionNode *)*cont)->maxFinishTime());
				if (times.isEmpty())
					times.addLast(new_time);
				else {
					ptr = (TimeDLNode *) times.first();
					while ( (!ptr->atEnd()) && (ptr->getValue() > new_time->getValue()) )
						ptr = (TimeDLNode *) ptr->next();
					if (ptr->atEnd())
						times.addLast(new_time); // no lower value found
					else
						ptr->insertBefore(new_time);
				}
			}
		}
	}
	if (count >= node->pipelineStage()->width()) {
		no_contention_time = node->maxStartTime()
					 + node->maxLatency() 
					 	* (count / node->pipelineStage()->width());
		count = 1;
		ptr = (TimeDLNode *) times.first();
		while ( (!ptr->atEnd()) && (count <node->pipelineStage()->width())) {
			ptr = (TimeDLNode *) ptr->next();
			count++;
		}
		max = ptr->getValue();
		if (max > no_contention_time ) 
			tmp = no_contention_time;
		else
			tmp = max;
		if (tmp > node->maxStartTime() )
			node->setMaxStartTime(tmp);
	}
	
	// empty times list
	while (!times.isEmpty())
		times.removeLast();
		
	node->setMaxFinishTime(node->maxStartTime() + node->maxLatency() );
		
	// update successors
	for (graph::Node::Successor next(node) ; next ; next++) {
		if (node->maxFinishTime() > ((ExecutionNode *)*next)->maxReadyTime())
			if (((ExecutionEdge *)(next.edge()))->type() == ExecutionEdge::SOLID)
				((ExecutionNode *)*next)->setMaxReadyTime(node->maxFinishTime());
			else // SLASHED
				((ExecutionNode *)*next)->setMaxReadyTime(node->maxStartTime());
	}
}

// ---------- epilogueLatestTimes

void ExecutionGraph::epilogueLatestTimes(ExecutionNode *node, elm::io::Output& out_stream) {
	//to be fixed
}


// ---------- latestTimes

void ExecutionGraph::latestTimes(elm::io::Output& out_stream) {
	
	entry_node->setMaxReadyTime(0);
	for(PreorderIterator node(this, this->entry_node); node; node++) {
		if ( (((ExecutionNode *) *node)->part() == ExecutionNode::PROLOGUE) 
			|| (((ExecutionNode *) *node)->part() == ExecutionNode::BEFORE_PROLOGUE) ) {
			if(! ((ExecutionNode *)*node)->isShaded() ) {
				prologueLatestTimes(((ExecutionNode *) *node), out_stream);
			}
		
		}
		if (((ExecutionNode *) *node)->part() == ExecutionNode::BODY) {
			bodyLatestTimes(((ExecutionNode *) *node), out_stream);
		}
		
		if (((ExecutionNode *) *node)->part() == ExecutionNode::EPILOGUE) {
			// epilogueLatestTimes(((ExecutionNode *) *node), out_stream);
			bodyLatestTimes(((ExecutionNode *) *node), out_stream);
		}
		
	}
}

// ---------- earliestTimes

void ExecutionGraph::earliestTimes(elm::io::Output& out_stream) {
	
	int max, count, no_contention_time, tmp;
	TimeDLNode *ptr;
	elm::inhstruct::DLList times;
	
	entry_node->setMinReadyTime(0);
	
	// for each node in topologically sorted order
	for(PreorderIterator node(this, this->entry_node); node; node++) {
		if (((ExecutionNode *)*node)->part() <= ExecutionNode::PROLOGUE) {
			((ExecutionNode *)*node)->setMinReadyTime(-INFINITE_TIME);
			((ExecutionNode *)*node)->setMinStartTime(-INFINITE_TIME);
			((ExecutionNode *)*node)->setMinFinishTime(-INFINITE_TIME);
		}
		else {
			((ExecutionNode *)*node)->setMinStartTime(((ExecutionNode *)*node)->minReadyTime());
			count = 0;		
			for(ExecutionNode::ContenderIterator cont((ExecutionNode *)*node); cont; cont ++) {
				if ( ((ExecutionNode *)*cont)->instIndex() > ((ExecutionNode *)*node)->instIndex() ) {
					// cont is a *late* contender
					if ( !separated((ExecutionNode *)*cont, (ExecutionNode *)*node, out_stream)
							&&
							( (( ExecutionNode *)*cont)->maxStartTime() < ((ExecutionNode *)*node)->minReadyTime()) 
							&&
							( (( ExecutionNode *)*node)->minReadyTime() < ((ExecutionNode *)*cont)->minFinishTime())) {
						count++;
						TimeDLNode *new_time = new TimeDLNode((( ExecutionNode *)*cont)->minFinishTime());
						if (times.isEmpty())
							times.addLast(new_time);
						else {
							ptr = (TimeDLNode *) times.first();
							while ( (!ptr->atEnd()) && (ptr->getValue() > new_time->getValue()) )
								ptr = (TimeDLNode *) ptr->next();
							if (ptr->atEnd())
								times.addLast(new_time); // no lower value found
							else
								ptr->insertBefore(new_time);
						}
					}
				}
			}
	
			for(ExecutionNode::ContenderIterator cont((ExecutionNode *)*node); cont; cont ++) {
				if ( ((ExecutionNode *)*cont)->instIndex() < ((ExecutionNode *)*node)->instIndex() ) {
					// cont is an *early* contender
					if  ( !separated((ExecutionNode *)*cont, (ExecutionNode *)*node, out_stream) 
							&&
							((( ExecutionNode *)*cont)->maxStartTime() <= ((ExecutionNode *)*node)->minReadyTime()) 
							&&
							( (( ExecutionNode *)*node)->minReadyTime() < ((ExecutionNode *)*cont)->minFinishTime())) {
						count++;
						TimeDLNode *new_time = new TimeDLNode((( ExecutionNode *)*cont)->maxFinishTime());
						if (times.isEmpty())
							times.addLast(new_time);
						else {
							ptr = (TimeDLNode *) times.first();
							while ( (!ptr->atEnd()) && (ptr->getValue() > new_time->getValue()) )
								ptr = (TimeDLNode *) ptr->next();
							if (ptr->atEnd())
								times.addLast(new_time); // no lower value found
							else
								ptr->insertBefore(new_time);
						}
					}
				}
			}
			if (count >= ((ExecutionNode *)*node)->pipelineStage()->width()) {
				count = 1;
				ptr = (TimeDLNode *) times.first();
				while ( (!ptr->atEnd()) && (count <((ExecutionNode *)*node)->pipelineStage()->width())) {
					ptr = (TimeDLNode *) ptr->next();
					count++;
				}
				max = ptr->getValue();
				if (max > ((ExecutionNode *)*node)->minStartTime() )
					((ExecutionNode *)*node)->setMinStartTime(max);
			}
			
			// empty times list
			while (!times.isEmpty())
				times.removeLast();
				
			((ExecutionNode *)*node)->setMinFinishTime(((ExecutionNode *)*node)->minStartTime() 
							+ (( ExecutionNode *)*node)->minLatency() );
				
			// update successors
			for (graph::Node::Successor next(node) ; next ; next++) {
				if (((ExecutionNode *)*node)->minFinishTime() > ((ExecutionNode *)*next)->minReadyTime())
					if (((ExecutionEdge *)(next.edge()))->type() == ExecutionEdge::SOLID)
						((ExecutionNode *)*next)->setMinReadyTime(((ExecutionNode *)*node)->minFinishTime());
					else // SLASHED
						((ExecutionNode *)*next)->setMinReadyTime(((ExecutionNode *)*node)->minStartTime());
			}
		}		
	}
}

// ---------- shadePreds

void ExecutionGraph::shadePreds(ExecutionNode *node, elm::io::Output& out_stream) {
	int time;
	
	if (node->hasPred()) {
		for (graph::Node::Predecessor pred(node) ; pred ; pred++) {
			((ExecutionNode *) *pred)->shade();
			time = ((ExecutionNode *) node)->maxFinishTime() - ((ExecutionNode *) node)->minLatency();
			
			if ( ((ExecutionEdge *)pred.edge())->type() == ExecutionEdge::SOLID) {
				if ( time < ((ExecutionNode *) *pred)->maxFinishTime() )
					((ExecutionNode *) *pred)->setMaxFinishTime(time);
				((ExecutionNode *) *pred)->setMaxStartTime(((ExecutionNode *) *pred)->maxFinishTime() - ((ExecutionNode *) *pred)->minLatency());
				((ExecutionNode *) *pred)->setMaxReadyTime(((ExecutionNode *) *pred)->maxStartTime());
				shadePreds(((ExecutionNode *) *pred),out_stream);
			}
			else {
				if (node->maxStartTime() < ((ExecutionNode *) *pred)->maxStartTime() ) {
					((ExecutionNode *) *pred)->setMaxStartTime(node->maxStartTime());
				}
				((ExecutionNode *) *pred)->setMaxFinishTime( ((ExecutionNode *) *pred)->maxStartTime() + ((ExecutionNode *) *pred)->minLatency());
				((ExecutionNode *) *pred)->setMaxReadyTime(((ExecutionNode *) *pred)->maxStartTime());
			}
		}
	}
	
}

// ---------- shadeNodes

void ExecutionGraph::shadeNodes(elm::io::Output& out_stream) {
		
	ExecutionNode *first_body_node;
	GraphNodesListInList * inst_node_list = (GraphNodesListInList *) this->instructionsNodesLists()->first();
	bool found = false;
	while (!found && !inst_node_list->atEnd()) {
		first_body_node = ((GraphNodeInList *) inst_node_list->list()->first())->executionNode();
		if (first_body_node->part() == ExecutionNode::BODY) {
			found = true;		
		}
		else
			inst_node_list = (GraphNodesListInList *) inst_node_list->next();
	}
	first_body_node->setMinReadyTime(0);
	first_body_node->setMinStartTime(0);	
	first_body_node->setMinFinishTime(first_body_node->minLatency());
	first_body_node->setMaxReadyTime(0);
	first_body_node->setMaxStartTime(0);	
	first_body_node->setMaxFinishTime(first_body_node->maxLatency());
	shadePreds(first_body_node, out_stream);
	
}

// ---------- findPaths

PathList * findPaths(ExecutionNode * source, ExecutionNode * target, elm::io::Output& out_stream) {
	PathList *list = new PathList();
	for (graph::Node::Successor succ(source) ; succ ; succ++) {
		if ( ((ExecutionNode *) *succ) == target) {
			Path *new_path = new Path();
			new_path->addNodeFirst((ExecutionNode *) *succ);
			list->addPath(new_path);
		}
		if ( ((ExecutionNode *) *succ)->instIndex() <= target->instIndex() ) {
			PathList *new_list = findPaths(((ExecutionNode *) *succ), target, out_stream);
			for (PathList::PathIterator suffix_path(new_list) ; suffix_path ; suffix_path++) {
				((Path *) *suffix_path)->addNodeFirst(source);
				list->addPath(suffix_path);
			}
		}
	}
	return(list);
}

// ---------- minDelta

int ExecutionGraph::minDelta(elm::io::Output& out_stream) {
	if (!first_node[ExecutionNode::BODY]->hasPred())
		return(0);
	int min_all = INFINITE_TIME;
	for (graph::Node::Predecessor pred(first_node[ExecutionNode::BODY]) ; pred ; pred++) {
		PathList *path_list = findPaths( ((ExecutionNode *) *pred), last_node[ExecutionNode::PROLOGUE], out_stream);
		int max = 0;
		for(PathList::PathIterator path(path_list); path; path++) {
			int length = 0;
			for (Path::NodeIterator node((Path *)*path); node; node++) {
				length += ((ExecutionNode *)*node)->minLatency();
			}
			if (length > max)
				max = length;
		}
		if (max < min_all)
			min_all = max;
	}
	return(min_all + last_node[ExecutionNode::PROLOGUE]->pipelineStage()->minLatency());
}

// ---------- build

void ExecutionGraph::build(FrameWork *fw, Microprocessor* microprocessor, 
					elm::genstruct::DLList<Inst *> &prologue, 
					elm::genstruct::DLList<Inst *> &body, 
					elm::genstruct::DLList<Inst *> &epilogue) {

	BasicBlock *bb ;
	bool first_node, first_fu_node;
	ExecutionNode *node, *previous_node, *first_stage_node, *last_stage_node, *producing_node;
	ExecutionEdge *edge;
	int inst_index, first_inst_index, stage_index;
	int counter;
	GraphNodesListInList *node_list_in_list, *stage_node_list, *inst_node_list,  *fu_node_list;
	GraphNodeInList *node_in_list, *previous_node_in_list, *previous_cycle_node_in_list, *other_node_in_list;
	bool found;
	PipelineStage::FunctionalUnit *fu;
	instruction_category_t category = IALU; //------------------------to be fixed
	Queue *queue;
	Platform *pf = fw->platform();
	PipelineStage *stage;
	Inst *inst;
	ExecutionNode *unknown_inst;
	
	// Init rename tables
	AllocatedTable<rename_table_t> rename_tables(pf->banks().count());
	int reg_bank_count = pf->banks().count();
	for(int i = 0; i <reg_bank_count ; i++) {
		rename_tables[i].reg_bank = (RegBank *) pf->banks()[i];
		rename_tables[i].table = new AllocatedTable<ExecutionNode *>(rename_tables[i].reg_bank->count());
		for (int j=0 ; j<rename_tables[i].reg_bank->count() ; j++)
			rename_tables[i].table->set(j,NULL);
	}

	// compute the index of the first instruction (the one before the prologue)
	// The index of the first body instruction is 1.
	first_inst_index = 1;
	for (elm::genstruct::DLList<Inst *>::Iterator ins(prologue) ; ins ; ins++) {
		first_inst_index--;
	}
	if (!prologue.isEmpty())
		first_inst_index--;
	
	// create lists of nodes: one list for each pipeline stage
	GraphNodesListInList *last_stage_list = NULL;
	for (Microprocessor::PipelineIterator st(microprocessor) ; st ; st++) {
		node_list_in_list = new GraphNodesListInList(st);
		stagesNodesLists()->addLast(node_list_in_list);
		last_stage_list = node_list_in_list;
	}	
	
	// create lists of nodes: one list for each instruction
	inst_index = first_inst_index;  	
	if (!prologue.isEmpty()){
		node_list_in_list = new GraphNodesListInList(NULL, inst_index++, ExecutionNode::BEFORE_PROLOGUE);
		instructionsNodesLists()->addLast(node_list_in_list);
	}
	for(elm::genstruct::DLList<Inst *>::Iterator ins(prologue); ins; ins++) {
		node_list_in_list = new GraphNodesListInList(ins, inst_index++, ExecutionNode::PROLOGUE);
		instructionsNodesLists()->addLast(node_list_in_list);
	}
	for(elm::genstruct::DLList<Inst *>::Iterator ins(body); ins; ins++) {
		node_list_in_list = new GraphNodesListInList(ins, inst_index++, ExecutionNode::BODY);
		instructionsNodesLists()->addLast(node_list_in_list);
	}
	for(elm::genstruct::DLList<Inst *>::Iterator ins(epilogue); ins; ins++) {
		node_list_in_list = new GraphNodesListInList(ins, inst_index++, ExecutionNode::EPILOGUE);
		instructionsNodesLists()->addLast(node_list_in_list);
	}
		

	// build nodes
	// consider every pipeline stage
	stage_node_list = (GraphNodesListInList *) stagesNodesLists()->first();
	while (! stage_node_list->atEnd()) {
		stage = stage_node_list->stage();
		// consider every instruction
		inst_index = first_inst_index;  	
		inst_node_list = (GraphNodesListInList *) instructionsNodesLists()->first();
		// the instruction before the prologue (if any) is unknown => it deserves a specific treatment
		if (inst_node_list->part() == ExecutionNode::BEFORE_PROLOGUE) {
			node = new ExecutionNode(this, (PipelineStage *)stage,
									(Inst *)inst, inst_index, inst_node_list->part());
			if (stage->usesFunctionalUnits()) {
				// the category of this instruction is unknown 
				//      => assume execution with minimum/maximum latency among all functional units
				int min_latency = INFINITE_TIME, max_latency = 0;
				for (int cat=1 ; cat<INST_CATEGORY_NUMBER ; cat++) {
					fu = stage->functionalUnit(cat);
					assert(fu);
					int min_lat = 0, max_lat = 0;
					for(PipelineStage::FunctionalUnit::PipelineIterator fu_stage(fu); fu_stage; fu_stage++) {
						min_lat += ((PipelineStage *) *fu_stage)->minLatency();
						max_lat += ((PipelineStage *) *fu_stage)->maxLatency();
					}
					if (min_lat < min_latency)
						min_latency = min_lat;
					if (max_lat < max_latency)
						max_latency = max_lat;
				}
				node->setMinLatency(min_latency);
				node->setMaxLatency(max_latency);
			}
			node_in_list = new GraphNodeInList(node);
			stage_node_list->list()->addLast(node_in_list);	
			node_in_list = new GraphNodeInList(node);
			inst_node_list->list()->addLast(node_in_list);
			
			if (microprocessor->operandProducingStage() == stage) {
				// this instruction is considered as producing every register
				node->setProducesOperands(true);
				for (int b=0 ; b<reg_bank_count ; b++) {
					for (int r=0 ; r < rename_tables[b].reg_bank ->count() ; r++) {
						rename_tables[b].table->set(r,node);
					}		
				}
			}		
			inst_node_list = (GraphNodesListInList *) inst_node_list->next();
			inst_index++;
		}
		// consider the other instructions (from the prologue)
		while ( ! inst_node_list->atEnd() ) {
			inst = inst_node_list->inst();
			if (!stage->usesFunctionalUnits()) {
				node = new ExecutionNode(this, (PipelineStage *)stage, (Inst *)inst, 
										inst_index, inst_node_list->part());					
				node_in_list = new GraphNodeInList(node);
				stage_node_list->list()->addLast(node_in_list);	
				node_in_list = new GraphNodeInList(node);
				inst_node_list->list()->addLast(node_in_list);
				first_stage_node = node;
				last_stage_node = node;
			}
			else {
				fu = stage->functionalUnit(instCategory(inst));
				first_fu_node = true;
				for(PipelineStage::FunctionalUnit::PipelineIterator fu_stage(fu); fu_stage; fu_stage++) {
					node = new ExecutionNode(this, (PipelineStage *)fu_stage,
											(Inst *)inst,inst_index, inst_node_list->part());
					if (first_fu_node) {
						node_in_list = new GraphNodeInList(node);
						stage_node_list->list()->addLast(node_in_list);	
						first_fu_node = false;
						first_stage_node = node;
					}
					node_in_list = new GraphNodeInList(node);
					inst_node_list->list()->addLast(node_in_list);
					last_stage_node = node;
				}
			}
			
			if (microprocessor->operandReadingStage() == stage) {
				first_stage_node->setNeedsOperands(true);
			}				
			if (microprocessor->operandProducingStage() == stage) {
				last_stage_node->setProducesOperands(true);
			}
			
			inst_node_list = (GraphNodesListInList *) inst_node_list->next();
			inst_index++;
		}	
		stage_node_list = (GraphNodesListInList *)stage_node_list->next();
	}

		
	// search for the entry node (first inst, first stage) for each code part
	stage_node_list = (GraphNodesListInList *) stagesNodesLists()->first();
	assert(stage_node_list);
	previous_node_in_list = NULL;
	node_in_list = (GraphNodeInList *) stage_node_list->list()->first();
	setEntryNode(node_in_list->executionNode());
	while (!node_in_list->atEnd()) {
		if ((previous_node_in_list == NULL)
				|| (node_in_list->executionNode()->part() != previous_node_in_list->executionNode()->part())) {
			setFirstNode(node_in_list->executionNode()->part(), node_in_list->executionNode());
		}
		previous_node_in_list = node_in_list;
		node_in_list = (GraphNodeInList *)node_in_list->next();
	}
		
	// search for the last node (last inst, last stage) for each code part
	stage_node_list = (GraphNodesListInList *) stagesNodesLists()->last();
	assert(stage_node_list);
	previous_node_in_list = NULL;
	node_in_list = (GraphNodeInList *) stage_node_list->list()->first();
	while (!node_in_list->atEnd()) {
		if ((previous_node_in_list != NULL)
				&& (node_in_list->executionNode()->part() != previous_node_in_list->executionNode()->part())) {
			setLastNode(previous_node_in_list->executionNode()->part(), 
							previous_node_in_list->executionNode());
		}
		previous_node_in_list = node_in_list;
		node_in_list = (GraphNodeInList *)node_in_list->next();
	}
	setLastNode(previous_node_in_list->executionNode()->part(), previous_node_in_list->executionNode());
	
	
	// build edges for pipeline order and data dependencies
	inst_node_list = (GraphNodesListInList *) instructionsNodesLists()->first();
	while (!inst_node_list->atEnd()) {
		node_in_list = (GraphNodeInList *) inst_node_list->list()->first();
		previous_node_in_list = NULL;
		while (!node_in_list->atEnd()) {
			if (previous_node_in_list != NULL) {
				// edge between consecutive pipeline stages
				edge = new ExecutionEdge(previous_node_in_list->executionNode(), 
										node_in_list->executionNode(), 
										ExecutionEdge::SOLID);
			}
			if ((node_in_list->executionNode()->needsOperands()) 
				&& (node_in_list->executionNode()->instIndex() != first_inst_index) ) {
				// check for data dependencies
				const elm::genstruct::Table<hard::Register *>& reads = 
						node_in_list->executionNode()->instruction()->readRegs();
				for(int i = 0; i < reads.count(); i++) {
					for (int b=0 ; b<reg_bank_count ; b++) {
						if (rename_tables[b].reg_bank == reads[i]->bank()) {
							producing_node = rename_tables[b].table->get(reads[i]->number());
							if (producing_node != NULL) {
								// check whether there is already an edge between the two nodes
								bool exists = false;
								for (graph::Node::Predecessor pred(node_in_list->executionNode()) ; pred ; pred++) {
									if (pred == producing_node) {
										exists = true;
										break;
									}
								}
								if (!exists)
									// add an edge for data dependency
									edge = new ExecutionEdge(producing_node, node_in_list->executionNode(), 
															ExecutionEdge::SOLID);
							}
						}
					}
				}
			}
			
			// note that this instruction produces some registers
			if  ((node_in_list->executionNode()->producesOperands()) 
				&& (node_in_list->executionNode()->instIndex() != first_inst_index) ) {
				const elm::genstruct::Table<hard::Register *>& writes = 
					node_in_list->executionNode()->instruction()->writtenRegs();
				for(int i = 0; i < writes.count(); i++) {
					for (int b=0 ; b<reg_bank_count ; b++) {
						if (rename_tables[b].reg_bank == writes[i]->bank()) {
							rename_tables[b].table->set(writes[i]->number(),node_in_list->executionNode());
						}
					}
				}
			}
			
			previous_node_in_list = node_in_list;
			node_in_list = (GraphNodeInList *)node_in_list->next();			
		}
		inst_node_list = (GraphNodesListInList *) inst_node_list->next();
	}
	
	
	// build edges for program order
	stage_node_list = (GraphNodesListInList *) stagesNodesLists()->first();
	while (!stage_node_list->atEnd()) {
		// build edges for in order execution		
		if (stage_node_list->stage()->orderPolicy() == PipelineStage::IN_ORDER) {
			node_in_list = (GraphNodeInList *) stage_node_list->list()->first();
			previous_node_in_list = NULL;
			while (!node_in_list->atEnd()) {
				if (previous_node_in_list != NULL) {				  
					if (stage_node_list->stage()->width() == 1) {
						// scalar stage => draw a solid edge
						edge = new ExecutionEdge(previous_node_in_list->executionNode(), 
												node_in_list->executionNode(), ExecutionEdge::SOLID);
					}
					else {	
						// superscalar => draw a slashed edge between adjacent instructions
						edge = new ExecutionEdge(previous_node_in_list->executionNode(), 
												node_in_list->executionNode(), ExecutionEdge::SLASHED);
						// draw a solid edge to model the stage width 
						// (search for the node that should start one cycle earlier)
						counter = 1;
						previous_cycle_node_in_list = previous_node_in_list;
						while ((counter < stage_node_list->stage()->width()) 
								&& (!previous_cycle_node_in_list->atBegin())) {
							previous_cycle_node_in_list = (GraphNodeInList *) previous_cycle_node_in_list->previous();
							counter++;
						}
						if (!previous_cycle_node_in_list->atBegin()) {
							edge = new ExecutionEdge(previous_cycle_node_in_list->executionNode(), 
													node_in_list->executionNode(), ExecutionEdge::SOLID);	
						}
					}
				}
				previous_node_in_list = node_in_list;
				node_in_list = (GraphNodeInList *)node_in_list->next();
			}
		}
		
		// build edges for queues with limited capacity
		if (stage_node_list->stage()->sourceQueue() != NULL) {	
			queue = stage_node_list->stage()->sourceQueue();
			node_in_list = (GraphNodeInList *) stage_node_list->list()->first();
			while (!node_in_list->atEnd()) {
				// compute the index of the instruction that cannot be admitted
				// into the queue until the current instruction leaves it
				inst_index = node_in_list->executionNode()->instIndex();
				inst_index += stage_node_list->stage()->sourceQueue()->size();
				inst_node_list = (GraphNodesListInList *) instructionsNodesLists()->first();
				while ((!inst_node_list->atEnd()) && (inst_node_list->instIndex() != inst_index) ) {
					inst_node_list = (GraphNodesListInList *) inst_node_list->next();
				}
				if (!inst_node_list->atEnd()) {
					other_node_in_list = (GraphNodeInList *) inst_node_list->list()->first();
					// find the node when the instruction enters the queue
					while ( (!other_node_in_list->atEnd())
							&& (other_node_in_list->executionNode()->pipelineStage()->destinationQueue() != queue ) ) {
						other_node_in_list = (GraphNodeInList *) other_node_in_list->next();
					}
					if (!other_node_in_list->atEnd()) {
						// draw an edge between the node that leaves the queue and the node that enters it
						edge = new ExecutionEdge(node_in_list->executionNode(), other_node_in_list->executionNode(), ExecutionEdge::SOLID);
					}
				}
				node_in_list = (GraphNodeInList *)node_in_list->next();
			}
		}
		stage_node_list = (GraphNodesListInList *) stage_node_list->next();
	}
		
	// search for contending nodes (i.e. pairs of nodes that use the same pipeline stage)
	stage_node_list = (GraphNodesListInList *) stagesNodesLists()->first();
	while (!stage_node_list->atEnd()) {
		if (stage_node_list->stage()->orderPolicy() == PipelineStage::OUT_OF_ORDER) {
			node_in_list = (GraphNodeInList *) stage_node_list->list()->first();
			while (!node_in_list->atEnd()) {
				other_node_in_list = (GraphNodeInList *) node_in_list->next();
				while (!other_node_in_list->atEnd()) {
					if (node_in_list->executionNode()->pipelineStage() == 
							other_node_in_list->executionNode()->pipelineStage()){
						node_in_list->executionNode()->addContender(other_node_in_list->executionNode());
						other_node_in_list->executionNode()->addContender(node_in_list->executionNode());
					}
					other_node_in_list = (GraphNodeInList *) other_node_in_list->next();
				}
				node_in_list = (GraphNodeInList *) node_in_list->next();
			}
		}
		stage_node_list = (GraphNodesListInList *) stage_node_list->next();
	}	
	
}

// ---------- analyze

int ExecutionGraph::analyze(elm::io::Output& out_stream) {
	GraphNodesListInList *inst_node_list;
	GraphNodeInList *node_in_list;
	int step;

	shadeNodes(out_stream);
	initSeparated();
	do {
		latestTimes(out_stream);		
		earliestTimes(out_stream);
		step++;
	} while ((step < 10) && (!unchangedSeparated(out_stream)));
	out_stream << "Max finish time of last body node: " << lastNode(ExecutionNode::BODY)->maxFinishTime() << "\n";
	return (lastNode(ExecutionNode::BODY)->maxFinishTime() - minDelta(out_stream));
}



} // namespace otawa
