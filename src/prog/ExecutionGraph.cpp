/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	ExecutionGraph.cpp -- ExecutionGraph classes implementation.
 */

#include <assert.h>
#include <elm/debug.h>
#include <elm/Collection.h>
#include <otawa/exegraph/ExecutionGraph.h>
#include <elm/debug.h>
#include <otawa/util/DFABitSet.h>
#include <elm/genstruct/DLList.h>
#include <otawa/otawa.h>

//#define DO_CHECK
#if defined(NDEBUG) || !defined(DO_CHECK)
#	define CHECK(c)
#else
#	define CHECK(c)	c
#endif

#define DO_LOG
#if defined(NDEBUG) || !defined(DO_LOG)
#	define LOG(c)
#else
#	define LOG(c) c
#endif


using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;

namespace otawa {

GenericIdentifier<bool> START("otawa.exegraph.start", false);

//START(inst) = true;

//if(START(inst))



// ---------- prologueLatestTimes

void ExecutionGraph::prologueLatestTimes(ExecutionNode *node, elm::io::Output& out_stream) {
	
	int max, count, no_contention_time, tmp;
	TimeDLNode *ptr;
	elm::inhstruct::DLList times;
	bool first;
	
	first = true;
	for (ExecutionGraph::Predecessor pred(node) ; pred ; pred++) {
		if (first) {
			max = pred->maxFinishTime();
			first = false;
		}
		else {
			if (pred->maxFinishTime() > max)
				max = pred->maxFinishTime();
		}
	}
	node->setMaxReadyTime(max);
	node->setMaxStartTime(node->maxReadyTime() + node->maxLatency()-1);
	
	count = 0;
	for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
		if ( cont->instIndex() < node->instIndex() ) { // early contender
			if ( !separated(cont, node, out_stream) ) {
				count++;
				TimeDLNode *new_time = new TimeDLNode(cont->maxFinishTime());
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
	while (!times.isEmpty()) {
		TimeDLNode *node = (TimeDLNode *) times.last();
		times.removeLast();
		delete node;
	}
		
	node->setMaxFinishTime(node->maxStartTime() + node->maxLatency() );
		
	// update successors
	for (Successor next(node) ; next ; next++) {
		if (node->maxFinishTime() > next->maxReadyTime())
			if (next.edge()->type() == ExecutionEdge::SOLID)
				next->setMaxReadyTime(node->maxFinishTime());
			else // SLASHED
				next->setMaxReadyTime(node->maxStartTime());
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
		if ( cont->instIndex() > node->instIndex() ) {
			// cont is a *late* contender
			if ( !separated(cont, node, out_stream)
					&&
					( cont->minStartTime() < node->maxReadyTime()) ) {
				count++;
				TimeDLNode *new_time = new TimeDLNode(cont->maxFinishTime());
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
	while (!times.isEmpty()) {
		TimeDLNode *node = (TimeDLNode *) times.last();
		times.removeLast();
		delete node;
	}
	
	count = 0;
	for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
		if ( cont->instIndex() < node->instIndex() ) {
			if ( !separated(cont, node, out_stream) ) {
				count++;
				TimeDLNode *new_time = new TimeDLNode(cont->maxFinishTime());
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
	while (!times.isEmpty()) {
		TimeDLNode *node = (TimeDLNode *) times.last();
		times.removeLast();
		delete node;
	}
		
	node->setMaxFinishTime(node->maxStartTime() + node->maxLatency() );
		
	// update successors
	for (Successor next(node) ; next ; next++) {
		if (node->maxFinishTime() > next->maxReadyTime())
			if (next.edge()->type() == ExecutionEdge::SOLID)
				next->setMaxReadyTime(node->maxFinishTime());
			else // SLASHED
				next->setMaxReadyTime(node->maxStartTime());
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
		if ( (node->part() == PROLOGUE) 
			|| (node->part() == BEFORE_PROLOGUE) ) {
			if(! node->isShaded() ) {
				prologueLatestTimes(node, out_stream);
			}
		
		}
		if (node->part() == BODY) {
			bodyLatestTimes(node, out_stream);
		}
		
		if (node->part() == EPILOGUE) {
			// epilogueLatestTimes(((ExecutionNode *) *node), out_stream);
			bodyLatestTimes(node, out_stream);
		}
		
	}
}

// ---------- earliestTimes

void ExecutionGraph::earliestTimes(elm::io::Output& out_stream) {
	
	int max, count, no_contention_time, tmp;
	TimeDLNode *ptr;
	elm::inhstruct::DLList times;
	
	/**
	 * HINT : to improve the efficiency, just use an array with the PARv greater
	 * earliest_finish(u). 
	 */
	
	// earliest_ready(IF1) = 0
	entry_node->setMinReadyTime(0);
	
	// foreach v in topologic_order do
	for(PreorderIterator node(this, this->entry_node); node; node++) {
		if (node->part() <= PROLOGUE) {
			node->setMinReadyTime(-INFINITE_TIME);
			node->setMinStartTime(-INFINITE_TIME);
			node->setMinFinishTime(-INFINITE_TIME);
		}
		else {
			
			// earliest_start(v) = earliest_ready(v)
			node->setMinStartTime(node->minReadyTime());
			count = 0;
			
			/* Slate = {u / u in late_contenders(v)
			 * 			&&  ! separated(u, v)
			 * 			&&  latest_start(u) < earliest_ready(v)
			 * 			&&  earliest_ready(v) < earliest_finish(u) }
			 */
			for(ExecutionNode::ContenderIterator cont(node); cont; cont ++)
				if ( cont->instIndex() > node->instIndex() ) {
					// cont is a *late* contender
					if(!separated(cont, node, out_stream)
					&& (cont->maxStartTime() < node->minReadyTime()) 
					&& (node->minReadyTime() < cont->minFinishTime())) {
						count++;
						TimeDLNode *new_time = new TimeDLNode(cont->minFinishTime());
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
	
			/* Searly = {u / u in early_contenders(u) 
			 * 			&&	 ! separated(u, v)
			 * 			&&	 latest_start(u) <= earliest_ready(v)
			 * 			&&	 earliest_ready(v) < arliest_finish(u) }
			 */
			for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
				if(cont->instIndex() < node->instIndex()) {
					// cont is an *early* contender
					if (!separated(cont, node, out_stream) 
					&& (cont->maxStartTime() <= node->minReadyTime()) 
					&& (node->minReadyTime() < cont->minFinishTime())) {
						count++;
						TimeDLNode *new_time = new TimeDLNode(cont->maxFinishTime());
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
			
			// S = Slate union Searly
			// if |S| > par(v) then
			if (count >= node->pipelineStage()->width()) {
				count = 1;
				ptr = (TimeDLNode *) times.first();
				while ( (!ptr->atEnd()) && (count < node->pipelineStage()->width())) {
					ptr = (TimeDLNode *) ptr->next();
					count++;
				}
				max = ptr->getValue();
				if (max > node->minStartTime() )
					node->setMinStartTime(max);
			}
			
			// empty times list
			while (!times.isEmpty())
				times.removeLast();
			
			// earliest_finish(v) = earliest_start(v) + min_lat(v)
			node->setMinFinishTime(node->minStartTime() + node->minLatency() );
				
			/* foreach k in immediate_successor(i)
			 * 	earliest_ready(k) = max(earliest_ready(k), earliest_finish(i))
			 */
			for(Successor next(node) ; next ; next++) {
				if(node->minFinishTime() > next->minReadyTime())
					if(next.edge()->type() == ExecutionEdge::SOLID)
						next->setMinReadyTime(node->minFinishTime());
					else // SLASHED
						next->setMinReadyTime(node->minStartTime());
			}
		}		
	}
}

// ---------- shadePreds

void ExecutionGraph::shadePreds(ExecutionNode *node, elm::io::Output& out_stream) {
	int time;
	
	if (node->hasPred()) {
		for (Predecessor pred(node) ; pred ; pred++) {
			pred->shade();
			time = node->maxFinishTime() - node->minLatency();
			
			if ( pred.edge()->type() == ExecutionEdge::SOLID) {
				if ( time < pred->maxFinishTime() ) {
					pred->setMaxFinishTime(time);
					pred->setMaxStartTime(pred->maxFinishTime() - pred->minLatency());
					pred->setMaxReadyTime(pred->maxStartTime());
					shadePreds(pred, out_stream);
				}
			}
			else {
				if (node->maxStartTime() < pred->maxStartTime() ) {
					pred->setMaxStartTime(node->maxStartTime());
				}
				pred->setMaxFinishTime(pred->maxStartTime() + pred->minLatency());
				pred->setMaxReadyTime(pred->maxStartTime());
			}
		}
	}
	
}


// ---------- shadeNodes

void ExecutionGraph::shadeNodes(elm::io::Output& out_stream) {
		
	ExecutionNode *first_body_node = first_node[BODY];
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
	for (ExecutionGraph::Successor succ(source) ; succ ; succ++) {
		if(succ == target) {
			Path *new_path = new Path();
			new_path->addNodeFirst(succ);
			list->addPath(new_path);
		}
		if(succ->instIndex() <= target->instIndex()) {
			PathList *new_list = findPaths(succ, target, out_stream);
			for (PathList::PathIterator suffix_path(new_list) ; suffix_path ; suffix_path++) {
				suffix_path->addNodeFirst(source);
				list->addPath(suffix_path);
			}
		}
	}
	return(list);
}

// ---------- minDelta

int ExecutionGraph::minDelta(elm::io::Output& out_stream) {
	if (!first_node[BODY]->hasPred())
		return(0);
	int min_all = INFINITE_TIME;
	for (Predecessor pred(first_node[BODY]) ; pred ; pred++) {
		PathList *path_list = findPaths(pred, last_node[PROLOGUE], out_stream);
		int max = 0;
		for(PathList::PathIterator path(path_list); path; path++) {
			int length = 0;
			for (Path::NodeIterator node(path); node; node++) {
				length += node->minLatency();
			}
			if (length > max)
				max = length;
		}
		if (max < min_all)
			min_all = max;
	}
	return(min_all + last_node[PROLOGUE]->pipelineStage()->minLatency());
}



// ---------- build

void ExecutionGraph::build(FrameWork *fw, Microprocessor* microprocessor, 
					elm::genstruct::DLList<ExecutionGraphInstruction *> &sequence) {
	this->_sequence = &sequence;
	// Init rename tables
	Platform *pf = fw->platform();
	AllocatedTable<rename_table_t> rename_tables(pf->banks().count());
	int reg_bank_count = pf->banks().count();
	for(int i = 0; i <reg_bank_count ; i++) {
		rename_tables[i].reg_bank = (RegBank *) pf->banks()[i];
		rename_tables[i].table = new AllocatedTable<ExecutionNode *>(rename_tables[i].reg_bank->count());
		for (int j=0 ; j<rename_tables[i].reg_bank->count() ; j++)
			rename_tables[i].table->set(j,NULL);
	}

	// clear node queues for instructions and for pipeline stages
	for (Microprocessor::PipelineIterator stage(microprocessor) ; stage ; stage++) {
		stage->deleteNodes();
		if (stage->usesFunctionalUnits()) {
			for (int i=0 ; i<INST_CATEGORY_NUMBER ; i++) {
				PipelineStage::FunctionalUnit * fu = stage->functionalUnit(i);
				for (PipelineStage::FunctionalUnit::PipelineIterator fu_stage(fu) ; fu_stage ; fu_stage++) {
					fu_stage->deleteNodes();
				}
			}
		}
	}
	for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(sequence) ; inst ; inst++) {
		inst->deleteNodes();
	}
	
	// build nodes
	// consider every pipeline stage
	code_part_t current_code_part = BEFORE_PROLOGUE;
	for (Microprocessor::PipelineIterator stage(microprocessor) ; stage ; stage++) {
		// consider every instruction
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(sequence) ; inst ; inst++)  {
			// the instruction before the prologue (if any) is unknown => it deserves a specific treatment
			if (inst->codePart() == BEFORE_PROLOGUE) {
				ExecutionNode * node = new ExecutionNode(this, (PipelineStage *)stage,
									inst->inst(), inst->index(), inst->codePart());
				if (stage->usesFunctionalUnits()) {
					// the category of this instruction is unknown 
					//      => assume execution with minimum/maximum latency among all functional units
					int min_latency = INFINITE_TIME, max_latency = 0;
					for (int cat=1 ; cat<INST_CATEGORY_NUMBER ; cat++) {
						PipelineStage::FunctionalUnit *fu = stage->functionalUnit(cat);
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
				inst->addNode(node);
				stage->addNode(node);
			
				if (microprocessor->operandProducingStage() == stage) {
					// this instruction is considered as producing every register
					//node->setProducesOperands(true);
					for (int b=0 ; b<reg_bank_count ; b++) {
						for (int r=0 ; r < rename_tables[b].reg_bank ->count() ; r++) {
							rename_tables[b].table->set(r,node);
						}		
					}
				}
				setFirstNode(BEFORE_PROLOGUE,inst->firstNode());
				setLastNode(current_code_part, inst->lastNode());
			}
			else {
				if (!stage->usesFunctionalUnits()) {
					ExecutionNode * node = new ExecutionNode(this, (PipelineStage *)stage, inst->inst(), 
										inst->index(), inst->codePart());
					inst->addNode(node);
					stage->addNode(node);	
					if (microprocessor->operandReadingStage() == stage) {
						node->setNeedsOperands(true);
					}				
					if (microprocessor->operandProducingStage() == stage) {
						node->setProducesOperands(true);
					}			
				}
				else {
					PipelineStage::FunctionalUnit *fu = stage->functionalUnit(instCategory(inst->inst()));
					bool first_fu_node = true;
					ExecutionNode * node;
					for(PipelineStage::FunctionalUnit::PipelineIterator fu_stage(fu); fu_stage; fu_stage++) {
						node = new ExecutionNode(this, (PipelineStage *)fu_stage,
											inst->inst(), inst->index(), inst->codePart());
						inst->addNode(node);
						fu_stage->addNode(node);
						if (first_fu_node) {
							stage->addNode(node);
							if (microprocessor->operandReadingStage() == stage) {
								node->setNeedsOperands(true);
							}	
							first_fu_node = false;
						}
					}
					if (microprocessor->operandProducingStage() == stage) {
						node->setProducesOperands(true);
					}
				}
				if (inst->codePart() != current_code_part) {
					current_code_part = inst->codePart();
					setFirstNode(current_code_part,inst->firstNode());	 
				}
				setLastNode(current_code_part, inst->lastNode());
			}	
		}
	}
	
	setEntryNode(sequence.first()->firstNode());
	
	// build edges for pipeline order and data dependencies
	for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(sequence) ; inst ; inst++)  {
		ExecutionNode * previous = NULL;
		for (ExecutionGraphInstruction::ExecutionNodeIterator node(*inst) ; node ; node++) {
			if (previous != NULL) {
				// edge between consecutive pipeline stages
				ExecutionEdge *edge = new ExecutionEdge(previous, node, ExecutionEdge::SOLID);
			}
			previous = node;
			if (node->needsOperands()) {
				// check for data dependencies
				const elm::genstruct::Table<hard::Register *>& reads = node->instruction()->readRegs();
				for(int i = 0; i < reads.count(); i++) {
					for (int b=0 ; b<reg_bank_count ; b++) {
						if (rename_tables[b].reg_bank == reads[i]->bank()) {
							ExecutionNode *producing_node = rename_tables[b].table->get(reads[i]->number());
							if (producing_node != NULL) {
								// check whether there is already an edge between the two nodes
								bool exists = false;
								for (Predecessor pred(node) ; pred ; pred++) {
									if (pred == producing_node) {
										exists = true;
										break;
									}
								}
								if (!exists)
									// add an edge for data dependency
									ExecutionEdge *edge = new ExecutionEdge(producing_node, node, ExecutionEdge::SOLID);
							}
						}
					}
				}
			}
			
			// note that this instruction produces some registers
			if  (node->producesOperands()) {
				const elm::genstruct::Table<hard::Register *>& writes = 
					node->instruction()->writtenRegs();
				for(int i = 0; i < writes.count(); i++) {
					for (int b=0 ; b<reg_bank_count ; b++) {
						if (rename_tables[b].reg_bank == writes[i]->bank()) {
							rename_tables[b].table->set(writes[i]->number(),node);
						}
					}
				}
			}
		}
	}
	
	
	// build edges for program order
	for (Microprocessor::PipelineIterator stage(microprocessor) ; stage ; stage++) {
		if (stage->orderPolicy() == PipelineStage::IN_ORDER) {
			ExecutionNode * previous = NULL;
			if (stage->width() == 1) {
				// scalar stage
				for (PipelineStage::ExecutionNodeIterator node(stage) ; node ; node++) {
					if (previous != NULL) {				  
						// scalar stage => draw a solid edge
						ExecutionEdge * edge = new ExecutionEdge(previous, node, ExecutionEdge::SOLID);
					}
					previous = node;
				}
			}
			else {
				elm::genstruct::DLList<ExecutionNode *> previous_nodes;
				for (PipelineStage::ExecutionNodeIterator node(stage) ; node ; node++) {			
					// superscalar => draw a slashed edge between adjacent instructions
					ExecutionEdge *edge = new ExecutionEdge(previous, node, ExecutionEdge::SLASHED);
					// draw a solid edge to model the stage width 
					if (previous_nodes.count() == stage->width()) {
						ExecutionEdge *edge = new ExecutionEdge(previous_nodes.first(), node, ExecutionEdge::SOLID);
						previous_nodes.removeFirst();	
					}
					previous_nodes.addLast(node);
				}
			}
		}
		
		// build edges for queues with limited capacity
		if (stage->sourceQueue() != NULL) {	
			Queue *queue = stage->sourceQueue();
			PipelineStage * prod_stage;
			for (Microprocessor::PipelineIterator st(microprocessor) ; st ; st++) {
				if (st->destinationQueue() == queue) {
					prod_stage = st;
					break;
				}
			}
			for (PipelineStage::ExecutionNodeIterator node(stage) ; node ; node++) {
				// compute the index of the instruction that cannot be admitted
				// into the queue until the current instruction leaves it
				int index = node->instIndex() + stage->sourceQueue()->size();
				for (PipelineStage::ExecutionNodeIterator waiting_node(prod_stage) ; waiting_node ; waiting_node++) {
					if (waiting_node->instIndex() == index) {
						ExecutionEdge *edge = new ExecutionEdge(node, waiting_node, ExecutionEdge::SOLID);
						break;
					}
				}
			}
		}
	}
		
	// search for contending nodes (i.e. pairs of nodes that use the same pipeline stage)
	for (Microprocessor::PipelineIterator stage(microprocessor) ; stage ; stage++) {
		if (stage->orderPolicy() == PipelineStage::OUT_OF_ORDER) {
			if (!stage->usesFunctionalUnits()) {
				for (PipelineStage::ExecutionNodeIterator node1(stage) ; node1 ; node1++) {
					for (PipelineStage::ExecutionNodeIterator node2(stage) ; node2 ; node2++) {
						if ((ExecutionNode *)node1 != (ExecutionNode *)node2) {
							node1->addContender(node2);
							node2->addContender(node1);
						}
					}
				}
			}
			else {
				for (int i=0 ; i<INST_CATEGORY_NUMBER ; i++) {
					PipelineStage *fu_stage = stage->functionalUnit(i)->firstStage();
					for (PipelineStage::ExecutionNodeIterator node1(fu_stage) ; node1 ; node1++) {
						for (PipelineStage::ExecutionNodeIterator node2(fu_stage) ; node2 ; node2++) {
							if ((ExecutionNode *)node1 != (ExecutionNode *)node2) {
								node1->addContender(node2);
								//node2->addContender(node1);
							}
						}
					}
				}
			}
		}
	}	
	
	// Free rename tables
	for(int i = 0; i <reg_bank_count ; i++)
		delete rename_tables[i].table;
}

// ---------- analyze

int ExecutionGraph::analyze(elm::io::Output& out_stream) {
//	GraphNodesListInList *inst_node_list;
//	GraphNodeInList *node_in_list;
	int step;

	//cout << "Shade\n";
	/*io::OutFileStream file("test.dot");
	io::Output file_out(file);
	dotDump(file_out, false);*/
	shadeNodes(out_stream);
	initSeparated();
	//cout << "IP loop\n";
	do {
		//cout << "Next Step\n";
		latestTimes(out_stream);		
		earliestTimes(out_stream);
		step++;
	} while ((step < 10) && (!unchangedSeparated(out_stream)));
	LOG(out_stream << "Max finish time of last body node: " << lastNode(BODY)->maxFinishTime() << "\n");
	return (lastNode(BODY)->maxFinishTime() - minDelta(out_stream));
}



/**
 */
ExecutionGraph::~ExecutionGraph(void) {
	delete pairs;
}


// ---------- dumpPart()
void ExecutionNode::dumpPart(elm::io::Output& out_stream) {
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


// ---------- dumpTime()

void ExecutionNode::dumpTime(int time, elm::io::Output& out_stream) {
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

void ExecutionNode::dump(elm::io::Output& out_stream) {
	out_stream << "\t" << pipeline_stage->shortName();
	out_stream << "(I" << inst_index <<")";
	if (needs_operands)
		out_stream << "\n\t   needs operands";
	if (produces_operands)
		out_stream << "\n\t   produces operands";		
	if (this->hasPred()) {
		out_stream << "\n\t   predecessors: ";
		for (ExecutionGraph::Predecessor pred(this) ; pred ; pred++) {
			out_stream << pred->pipelineStage()->shortName();
			out_stream << "(I" << pred->instIndex() <<")";
			pred.edge()->dump(out_stream);
		}
	}
	if (this->hasSucc()) {
		out_stream << "\n\t   successors: ";
		for (ExecutionGraph::Successor next(this) ; next ; next++) {
			out_stream << next->pipelineStage()->shortName();
			out_stream << "(I" << next->instIndex() <<")";
			next.edge()->dump(out_stream);
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

void ExecutionNode::dumpLight(elm::io::Output& out_stream) {
	out_stream << pipeline_stage->shortName();
	out_stream << "(I" << inst_index <<")";
}

// ---------- dumpLightTimed()

void ExecutionNode::dumpLightTimed(elm::io::Output& out_stream) {
	out_stream << "\t" << pipeline_stage->shortName();
	out_stream << "(I" << inst_index <<")";
	out_stream << "["; dumpTime(ready_time.min, out_stream);
	out_stream << "/"; dumpTime(ready_time.max, out_stream);out_stream << "]";
	out_stream << "["; dumpTime(start_time.min, out_stream);
	out_stream << "/"; dumpTime(start_time.max, out_stream);out_stream << "]";
	out_stream << "["; dumpTime(finish_time.min, out_stream);
	out_stream << "/"; dumpTime(finish_time.max, out_stream);out_stream << "]";
}


// ---------- dump()

void ExecutionEdge::dump(elm::io::Output& out_stream) {
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

// ---------- constructor

ExecutionGraph::ExecutionGraph()
: entry_node(NULL) {
	for (int i=0 ; i<CODE_PARTS_NUMBER ; i++) {
		first_node[i] = NULL;
		last_node[i] = NULL;
	}
}

// ---------- initSeparated()

void ExecutionGraph::initSeparated() {
	pair_cnt = 0;
	for(NodeIterator u(this); u; u++)
		u->pair_index = pair_cnt++;
	pairs = new BitVector(pair_cnt * pair_cnt);
}


// ---------- unchangedSeparated()

bool ExecutionGraph::unchangedSeparated(elm::io::Output& out_stream) {
	bool unchanged = true;
	/*int sep_cnt = 0, node_cnt = 0;
	static int change_cnt = 0;
	static double sep_ratio_sum = 0, node_ratio_sum = 0;*/
	
	
	for(NodeIterator node(this); node; node++)
		if(node->changed) {
			node->changed = false;
			//node_cnt++;
		
			// Horizontal
			for(NodeIterator first(this); *first != *node; first++) {
				int index = (first->pair_index * pair_cnt) + node->pair_index;
				bool sep = separated(first, node, out_stream);
				if(pairs->bit(index) != sep) {
					unchanged = false;
					pairs->set(index, sep);
					//sep_cnt++;
				}
			}
			
			// Vertical
			NodeIterator second(node);
			for(second++; second; second++) {
				int index = node->pair_index * pair_cnt + second->pair_index;
				bool sep = separated(node, second, out_stream);
				if(pairs->bit(index) != sep) {
					unchanged = false;
					pairs->set(index, sep);
					//sep_cnt++;
				}
			}
		}
	

	/*double sep_ratio = (double)sep_cnt / (pair_cnt * pair_cnt / 2);
	double node_ratio = (double)node_cnt / pair_cnt;
	sep_ratio_sum += sep_ratio;
	node_ratio_sum += node_ratio;
	change_cnt++;	
	cout << "changed "
		 <<  "SEP = " << (sep_ratio * 100) << " (" << (sep_ratio_sum / change_cnt * 100) << ") "
		 <<  "NODE = " << (node_ratio * 100) << " (" << (node_ratio_sum / change_cnt * 100) << ")\n";*/
	return unchanged;
}


// ---------- dumpLight()

void ExecutionGraph::dumpLight(elm::io::Output& out_stream) {
//	out_stream << "\tDumping the execution graph ...\n";
//	out_stream << "\t\tEntry node: ";
//	this->entry_node->dumpLight(out_stream);
//	out_stream << " \n";
//	for (int i=0 ; i<CODE_PARTS_NUMBER ; i++) {
//		if (first_node[i] != NULL) {
//			out_stream << "\t\t";
//			first_node[i]->dumpPart(out_stream);
//			out_stream << ": first_node=";
//			first_node[i]->dumpLight(out_stream);
//			out_stream << ", last_node=";
//			last_node[i]->dumpLight(out_stream);
//			out_stream << "\n";
//		}
//	}
}


// ---------- dump()

void ExecutionGraph::dump(elm::io::Output& out_stream) {
//	out_stream << "\tDumping the execution graph ...\n";
//	out_stream << "\t\tEntry node: ";
//	this->entry_node->dumpLight(out_stream);
//	out_stream << " \n";
//	for (int i=0 ; i<CODE_PARTS_NUMBER ; i++) {
//		if (first_node[i] != NULL) {
//			out_stream << "\t\t";
//			first_node[i]->dumpPart(out_stream);
//			out_stream << ": first_node=";
//			first_node[i]->dumpLight(out_stream);
//			out_stream << ", last_node=";
//			last_node[i]->dumpLight(out_stream);
//			out_stream << "\n";
//		}
//	}
//	for(NodeIterator node(this); node; node++){
//		node->dump(out_stream);
//		out_stream << "\n";
//	}
//	out_stream << "\n";
//	if (this->entry_node != NULL) {
//		out_stream << "\nTopological order:\n";
//		for(PreorderIterator node(this, this->entry_node); node; node++) {
//			out_stream << node->pipelineStage()->shortName();
//			out_stream << "(I" << node->instIndex() <<") - ";
//		}
//	}
}



// ---------- dotDump()

void ExecutionGraph::dotDump(elm::io::Output& dotFile, bool dump_times) {
	GraphNodesListInList *stage_node_list, *inst_node_list;
	GraphNodeInList *node_in_list;
	
	dotFile << "digraph G {\n";
	for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*_sequence) ; inst ; inst++) {
		// dump nodes
		dotFile << "{ rank = same ; ";
		for (ExecutionGraphInstruction::ExecutionNodeIterator node(inst) ; node ; node++) {
			dotFile << "\"" << node->pipelineStage()->shortName();
			dotFile << "I" << node->instIndex() << "\" ; ";
		}
		dotFile << "}\n";
		// again to specify labels
		for (ExecutionGraphInstruction::ExecutionNodeIterator node(inst) ; node ; node++) {
			if (dump_times) {
				dotFile << "\"" << node->pipelineStage()->shortName();
				dotFile << "I" << node->instIndex() << "\"";
				dotFile << " [shape=record, ";
				if (node->isShaded())
					dotFile << "color=red, ";
				if (node->part() == BODY)
					dotFile << "color=blue, ";
				dotFile << "label=\"" << node->pipelineStage()->shortName();
				dotFile << "(I" << node->instIndex() << ") ";
				dotFile << "| { {";
				node->dumpTime(node->minReadyTime(),dotFile);
				dotFile  << "|";
				node->dumpTime(node->maxReadyTime(),dotFile);
				dotFile << "} | {";
				node->dumpTime(node->minStartTime(),dotFile);
				dotFile << "|";
				node->dumpTime(node->maxStartTime(),dotFile);
				dotFile << "} | {";
				node->dumpTime(node->minFinishTime(),dotFile);
				dotFile << "|";
				node->dumpTime(node->maxFinishTime(),dotFile);
				dotFile << "} }";		
				dotFile << "\"] ; \n";
			}
			else {
				if (node->isShaded()) {
					dotFile << "\"" << node->pipelineStage()->shortName();
					dotFile << "I" << node->instIndex() << "\"";
					dotFile << " [color=red] ; \n";	
				}
				
			}
		}
		dotFile << "\n";
	}
	
						
	int group_number = 0;
	for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(*_sequence) ; inst ; inst++) {	
		// dump edges
		for (ExecutionGraphInstruction::ExecutionNodeIterator node(inst) ; node ; node++) {
			for (Successor next(node) ; next ; next++) {
				if ( node != inst->firstNode()
						||
						(!node->producesOperands()) 
						|| (node->instIndex() == ((ExecutionNode *) *next)->instIndex()) ) {				
					dotFile << "\"" << node->pipelineStage()->shortName();
					dotFile << "I" << node->instIndex() << "\"";
					dotFile << " -> ";
					dotFile << "\"" << ((ExecutionNode *) *next)->pipelineStage()->shortName();
					dotFile << "I" << ((ExecutionNode *) *next)->instIndex() << "\"";
					switch( ((ExecutionEdge *) next.edge())->type()) {
						case ExecutionEdge::SOLID:
							if (node->instIndex() == ((ExecutionNode *) *next)->instIndex())
								dotFile << "[minlen=4]";
							dotFile << " ;\n";
							break;
						case ExecutionEdge::SLASHED:
							dotFile << " [style=dotted";
							if (node->instIndex() == ((ExecutionNode *) *next)->instIndex())
								dotFile << ", minlen=4";
							dotFile << "] ;\n";
							break;	
						default:
							break;
					}	
					if ((node->instIndex() == ((ExecutionNode *) *next)->instIndex())
							|| ((node->stageIndex() == ((ExecutionNode *) *next)->stageIndex())
								&& (node->instIndex() == ((ExecutionNode *) *next)->instIndex()-1)) ) {
						dotFile << "\"" << node->pipelineStage()->shortName();
						dotFile << "I" << node->instIndex() << "\"  [group=" << group_number << "] ;\n";
						dotFile << "\"" << ((ExecutionNode *) *next)->pipelineStage()->shortName();
						dotFile << "I" << ((ExecutionNode *) *next)->instIndex() << "\" [group=" << group_number << "] ;\n";
						group_number++;
					}
				}
				// dump contenders
	//			for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
	//				if (cont->instIndex() > node->instIndex()) {
	//					dotFile << node->pipelineStage()->shortName();
	//					dotFile << "I" << node->instIndex();
	//					dotFile << " -> ";
	//					dotFile << cont->pipelineStage()->shortName();
	//					dotFile << "I" << cont->instIndex();
	//					dotFile << " [style=dashed, dir=none] ; \n";
	//				}
	//			}
			}
		}
		dotFile << "\n";
	}
	
	dotFile << "}\n";
}


// ---------- dump()

inline void Path::dump(elm::io::Output& out_stream) {
	for(NodeIterator node(this); node; node++) {
		node->dumpLight(out_stream);
		out_stream << "-";
	}
}


// ---------- dump()

inline void PathList::dump(elm::io::Output& out_stream) {
	for(PathIterator path(this); path; path++) {
		path->dump(out_stream);
		out_stream << "\n";
	}
}


// ---------- dump()

inline void GraphNodesListInList::dump(elm::io::Output& out_stream) {
//	if (pipeline_stage != NULL) {
//		out_stream << "list of nodes for stage " << pipeline_stage->name() << "(";
//		out_stream << pipeline_stage->shortName() << "): ";
//	}
//	else {
//		if (instruction != NULL) {
//			out_stream << "list of nodes for instruction ";
//			instruction->dump(out_stream);
//		}
//		else
//			out_stream << "list of nodes for ??? :";
//	}
//	GraphNodeInList *node = (GraphNodeInList *) node_list.first();
//	if (node != NULL) {
//		while (!node->atEnd()) {
//			node->dump(out_stream);
//			node = (GraphNodeInList *) node->next();
//		}
//	}
}

} // namespace otawa
