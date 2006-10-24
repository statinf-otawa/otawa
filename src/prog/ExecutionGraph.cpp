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

#define dumpFile elm::cout



using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;

namespace otawa {

GenericIdentifier<bool> START("otawa.exegraph.start", false);

//START(inst) = true;

//if(START(inst))



// ---------- prologueLatestTimes

void ExecutionGraph::prologueLatestTimes(ExecutionNode *node) {
	
	int parv;
	parv = node->pipelineStage()->width();
	elm::genstruct::AllocatedTable<int> times(parv);
	
	/* v.ready.latest = MAX{u|u->v}(u.finish.latest);
	 * v.ready.latest = MAX(v.ready.latest, CM(I-p).ready.latest);
	 */
	bool first = true;
	int max;
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
	if (lastNode(BEFORE_PROLOGUE)
		&&
	    (lastNode(BEFORE_PROLOGUE)->maxReadyTime() > max)){
		max = lastNode(BEFORE_PROLOGUE)->maxReadyTime();
	}
	node->setMaxReadyTime(max);
	
	/* v.start.latest = v.ready.latest + max_lat(v) - 1; */
	
	node->setMaxStartTime(node->maxReadyTime() + node->maxLatency() - 1);
	
	/* Searly = early_cont(v);
	 * if (|Searly| >= parv) then
	 *     tmp = MAX{u| u in Searly}(u.finish.latest);
	 *     tmp = MIN(tmp, v.start.latest + |Searly|/parv x max_lat(v));
	 *     v.start.latest = MAX(tmp, v.start.latest);
	 * endif
	 * v.finish.latest = v.start.latest + max_lat(v);
	 */
	 
	int count = 0, index = 0;
	int min, pos;
	for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
		if ( cont->instIndex() < node->instIndex() ) {
			// cont is an *early* contender
			count++;
			if (index < parv) {
				// less than parv values have been registered
				times[index++] = cont->maxFinishTime();
			}
			else {
				// if cont->maxFinishTime() > min_value, replace min_value
				min = times[0];
				pos = 0;
				for (int i=1 ; i<index ; i++) {
					if (times[i] < min) {
						min = times[i];
						pos = i;
					}
				}
				if (cont->maxFinishTime() > min)
					times[pos] = cont->maxFinishTime();
			}
		}
	}
	
	if (count >= parv) {
		int worst_case_delay = node->maxStartTime() + node->maxLatency() * count / parv;
		int max_delay = times.get(0);
		int tmp;
		for (int i=1 ; i<index ; i++) {
			if (times[i] < max_delay) {
				max_delay = times[i];
			}
		}
		if (max_delay > worst_case_delay ) 
			tmp = worst_case_delay;
		else
			tmp = max_delay;
		if (tmp > node->maxStartTime())
			node->setMaxStartTime(tmp);
	}
	node->setMaxFinishTime(node->maxStartTime() + node->maxLatency());
	
	/* foreach immediate successor w of v do
	 * 		if slashed edge
	 * 			w.ready.latest = MAX(w.ready.latest, v.start.latest);
	 * 		if solid edge
	 * 			w.ready.latest = MAX(w.ready.latest, v.finish.latest
	 */
	for (Successor next(node) ; next ; next++) {
		if (next.edge()->type() == ExecutionEdge::SLASHED) {
			if (node->maxStartTime() > next->maxReadyTime())	
				next->setMaxReadyTime(node->maxStartTime());
		}
		else {// SOLID 
			if (node->maxFinishTime() > next->maxReadyTime())
				next->setMaxReadyTime(node->maxFinishTime());
		}
	}
	
		
}

// ---------- bodyLatestTimes

void ExecutionGraph::bodyLatestTimes(ExecutionNode *node) {
	
	int parv;
	parv = node->pipelineStage()->width();
	elm::genstruct::AllocatedTable<int> times(parv) ;
	
	/* node.start.latest = node.ready.latest */
	node->setMaxStartTime(node->maxReadyTime());
	
	/* Slate = {u / u in late_contenders(v)
	 * 				&& !separated(u,v)
	 * 				&& u.start.earliest < v.ready.latest };
	 * max_delay = the n-teenth maximum value of maxFinishTime() in Slate;
	 * worst_case_delay = v.ready.latest + max_lat(v) - 1;
	 * if (|Slate| >= parv) then
	 * 		v.start.latest = MIN(max_delay, worst_case_delay);
	 */
	int index = 0, count = 0;
	int min, pos;	
	for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
		if ( cont->instIndex() > node->instIndex() ) {
			// cont is a *late* contender
			if ( !separated(cont, node)
					&&
					( cont->minStartTime() < node->maxReadyTime()) ) {
				count++;
				if (index < parv) {
					// less than parv values have been registered
					times[index++] = cont->maxFinishTime();
				}
				else {
					// if cont->maxFinishTime() > min_value, replace min_value
					min = times[0];
					pos = 0;
					for (int i=1 ; i<index ; i++) {
						if (times[i] < min) {
							min = times[i];
							pos = i;
						}
					}
					if (cont->maxFinishTime() > min)
						times[pos] = cont->maxFinishTime();
				}
			}
		}
	}
	
	if (count >= parv) {
		int worst_case_delay = node->maxReadyTime() + node->maxLatency() - 1;
		int max_delay = times[0];
		for (int i=1 ; i<index ; i++) {
			if (times[i] < max_delay) {
				max_delay = times[i];
			}
		}
		if (max_delay > worst_case_delay ) 
			node->setMaxStartTime(worst_case_delay);
		else
			node->setMaxStartTime(max_delay);
	}
	
	
	/* Searly = {u / u in late_contenders(v)
	 * 				&& !separated(u,v) };
	 * max_delay = the n-teenth maximum value of maxFinishTime() in Slate;
	 * worst_case_delay = v.start.latest + |Searly|/parv x max_lat(v);
	 * if (|Searly| >= parv) then
	 * 		tmp = MIN(max_delay, worst_case_delay);
	 * 		v.start.latest = MAX(tmp, v.start.latest);
	 */
	
	index = 0;
	count = 0;
	for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
		if ( cont->instIndex() < node->instIndex() ) {
			// cont is an *early* contender
			if ( !separated(cont, node) ) {
				count++;
				if (index < parv)
					// less than parv values have been registered
					times[index++] = cont->maxFinishTime();
				else {
					// if cont->maxFinishTime() > min_value, replace min_value
					min = times[0];
					pos = 0;
					for (int i=1 ; i<index ; i++) {
						if (times[i] < min) {
							min = times[i];
							pos = i;
						}
					}
					if (cont->maxFinishTime() > min)
						times[pos] = cont->maxFinishTime();
				}
			}
		}
	}
	
	if (count >= parv) {
		int worst_case_delay = node->maxStartTime() + node->maxLatency() * (count / parv);
		int max_delay = times[0];
		for (int i=1 ; i<index ; i++) {
			if (times[i] < max_delay) {
				max_delay = times[i];
			}
		}
		int tmp;
		if (max_delay > worst_case_delay ) 
			tmp = worst_case_delay;
		else
			tmp = max_delay;
		if (tmp > node->maxStartTime())
			node->setMaxStartTime(tmp);
	}
	
	/* v.finish.latest = v.start.latest + max_lat(v); */
		
	node->setMaxFinishTime(node->maxStartTime() + node->maxLatency() );
		
	/* foreach immediate successor w of v do
	 * 		if slashed edge
	 * 			w.ready.latest = MAX(w.ready.latest, v.start.latest);
	 * 		if solid edge
	 * 			w.ready.latest = MAX(w.ready.latest, v.finish.latest
	 */
	for (Successor next(node) ; next ; next++) {
		if (next.edge()->type() == ExecutionEdge::SLASHED) {
			if (node->maxStartTime() > next->maxReadyTime())	
				next->setMaxReadyTime(node->maxStartTime());
		}
		else {// SOLID 
			if (node->maxFinishTime() > next->maxReadyTime())
				next->setMaxReadyTime(node->maxFinishTime());
		}
	}
}

// ---------- epilogueLatestTimes

void ExecutionGraph::epilogueLatestTimes(ExecutionNode *node) {
	//to be fixed
}

// ---------- bodyEarliestTimes

void ExecutionGraph::bodyEarliestTimes(ExecutionNode *node) {
	
	int parv;
	parv = node->pipelineStage()->width();
	elm::genstruct::AllocatedTable<int> times(parv) ;
	
	/* node.start.earliest = node.ready.earliest */
	node->setMinStartTime(node->minReadyTime());

	/* Slate = {u / u in late_contenders(v)
	 * 				&& !separated(u,v)
	 * 				&& u.start.latest < v.ready.earliest
	 * 				&& v.ready.earliest < u.finish.earliest };
	 * Searly = {u / u in early_contenders(v)
	 * 				&& !separated(u,v)
	 * 				&& u.start.latest <= v.ready.earliest
	 * 				&& v.ready.earliest < u.finish.earliest };
	 * S = Searly UNION Slate;
	 * max_delay = the n-teenth maximum value of minFinishTime() in S;
	 * if (|Slate| >= parv) then
	 * 		v.start.earliest = MAX(max_delay, v.ready.latest);
	 */
	int index = 0, count = 0;
	int min, pos;	
	for(ExecutionNode::ContenderIterator cont(node); cont; cont ++) {
		if (  ( (cont->instIndex() > node->instIndex())
				&&
				!separated(cont, node)
				&&
				(cont->maxStartTime() < node->minReadyTime())
				&&
				(node->minReadyTime() < cont->minFinishTime()) )
			||
			  ( (cont->instIndex() < node->instIndex())
			  	&&
			  	!separated(cont, node)
			  	&&
			  	(cont->maxStartTime() <= node->minReadyTime())
			  	&&
			  	node->minReadyTime() < cont->minFinishTime() ) ) {
			  		
			count++;
			if (index < parv)
				// less than parv values have been registered
				times[index++] = cont->maxFinishTime();
			else {
				// if cont->maxFinishTime() > min_value, replace min_value
				min = times[0];
				pos = 0;
				for (int i=1 ; i<index ; i++) {
					if (times[i] < min) {
						min = times[i];
						pos = i;
					}
				}
				if (cont->maxFinishTime() > min)
					times[pos] = cont->maxFinishTime();
			}
		}
	}

	if (count >= parv) {
		int max_delay = times[0];	
		for (int i=1 ; i<index ; i++) {
			if (times[i] < max_delay) {
				max_delay = times[i];
			}
		}
		if (max_delay > node->minReadyTime() ) 
			node->setMinStartTime(max_delay);
		else
			node->setMinStartTime(node->minReadyTime());
	}
	
	
	/* v.finish.earliest = v.start.earliest + min_lat(v); */
		
	node->setMinFinishTime(node->minStartTime() + node->minLatency() );
		
	/* foreach immediate successor w of v do
	 * 		if slashed edge
	 * 			w.ready.latest = MAX(w.ready.latest, v.start.latest);
	 * 		if solid edge
	 * 			w.ready.latest = MAX(w.ready.latest, v.finish.latest
	 */

	for (Successor next(node) ; next ; next++) {

		if (next.edge()->type() == ExecutionEdge::SLASHED) {
		  if (node->minStartTime() > next->minReadyTime()){
		    	next->setMinReadyTime(node->minStartTime());

		  }
		}
		else {// SOLID 
		  if (node->minFinishTime() > next->minReadyTime()){
				next->setMinReadyTime(node->minFinishTime());

		  }
		}
	}
}

// ---------- latestTimes

void ExecutionGraph::latestTimes() {
	
	firstNode(BODY)->setMaxReadyTime(0);
	for(PreorderIterator node(this, this->entry_node); node; node++) {
		if ( (node->part() == PROLOGUE) 
			|| (node->part() == BEFORE_PROLOGUE) ) {
			if(! node->isShaded() ) {
				prologueLatestTimes(node);
			}
		
		}
		if (node->part() == BODY) {
			bodyLatestTimes(node);
		}
		
		if (node->part() == EPILOGUE) {
			// epilogueLatestTimes(((ExecutionNode *) *node));
			bodyLatestTimes(node);
		}
		
	}
}

// ---------- earliestTimes

void ExecutionGraph::earliestTimes() {
	
	firstNode(BODY)->setMinReadyTime(0);
	for(PreorderIterator node(this, this->entry_node); node; node++) {
		if ( node->part() <= PROLOGUE) {
			node->setMinReadyTime(-INFINITE_TIME);
			node->setMinStartTime(-INFINITE_TIME);
			node->setMinFinishTime(-INFINITE_TIME);
		}
		else {
			bodyEarliestTimes(node);
		}

	}
}



// ---------- shadePreds

void ExecutionGraph::shadePreds(ExecutionNode *node) {
	int time;
	
	if (node->hasPred()) {
		for (Predecessor pred(node) ; pred ; pred++) {
			if (pred.edge()->type() == ExecutionEdge::SOLID) {
				pred->shade();
				time = node->maxFinishTime() - node->minLatency();
				
//				if ( pred.edge()->type() == ExecutionEdge::SOLID) {
				if ( time < pred->maxFinishTime() ) {
					pred->setMaxFinishTime(time);
					pred->setMaxStartTime(pred->maxFinishTime() - pred->minLatency());
					pred->setMaxReadyTime(pred->maxStartTime());
					shadePreds(pred);
				}
//				}
//				else {
//					if (node->maxStartTime() < pred->maxStartTime() ) {
//						pred->setMaxStartTime(node->maxStartTime());
//					}
//					pred->setMaxFinishTime(pred->maxStartTime() + pred->minLatency());
//					pred->setMaxReadyTime(pred->maxStartTime());
//				}
			}
		}
	}
	
}


// ---------- shadeNodes

void ExecutionGraph::shadeNodes() {
		
	ExecutionNode *first_body_node = first_node[BODY];
	first_body_node->setMinReadyTime(0);
	first_body_node->setMinStartTime(0);	
	first_body_node->setMinFinishTime(first_body_node->minLatency());
	first_body_node->setMaxReadyTime(0);
	first_body_node->setMaxStartTime(0);	
	first_body_node->setMaxFinishTime(first_body_node->maxLatency());
	shadePreds(first_body_node);
	
}

// ---------- findPaths

PathList * findPaths(ExecutionNode * source, ExecutionNode * target) {
	PathList *list = new PathList();
	for (ExecutionGraph::Successor succ(source) ; succ ; succ++) {
		if(succ == target) {
			Path *new_path = new Path();
			new_path->addNodeFirst(succ);
			list->addPath(new_path);
		}
		if(succ->instIndex() <= target->instIndex()) {
			PathList *new_list = findPaths(succ, target);
			for (PathList::PathIterator suffix_path(new_list) ; suffix_path ; suffix_path++) {
				suffix_path->addNodeFirst(source);
				list->addPath(suffix_path);
			}
		}
	}
	return(list);
}

// ---------- minDelta

int ExecutionGraph::minDelta() {
	if (!first_node[BODY]->hasPred())
		return(0);
	int min_all = INFINITE_TIME;
	for (Predecessor pred(first_node[BODY]) ; pred ; pred++) {
		PathList *path_list = findPaths(pred, last_node[PROLOGUE]);
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


/**
 * Build the graph for the given sequence.
 * @param fw				Used framework.
 * @param microprocessor	Current microprocessor.
 * @param sequence			Selected sequence.
 */
void ExecutionGraph::build(
	FrameWork *fw,
	Microprocessor* microprocessor, 
	elm::genstruct::DLList<ExecutionGraphInstruction *> &sequence
) {
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
			for(genstruct::Vector<PipelineStage::FunctionalUnit *>::Iterator
			fu(stage->getFUs()); fu; fu++)
				for (PipelineStage::FunctionalUnit::PipelineIterator fu_stage(fu); fu_stage; fu_stage++)
					fu_stage->deleteNodes();
		}
	}
	for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(sequence) ; inst ; inst++)
		inst->deleteNodes();
	
	// build nodes
	// consider every pipeline stage
	code_part_t current_code_part = BEFORE_PROLOGUE;
	for (Microprocessor::PipelineIterator stage(microprocessor) ; stage ; stage++) {
		
		// consider every instruction
		for (elm::genstruct::DLList<ExecutionGraphInstruction *>::Iterator inst(sequence) ;
		inst ; inst++)  {
			
			// the instruction before the prologue (if any) is unknown
			// => it deserves a specific treatment
			if (inst->codePart() == BEFORE_PROLOGUE) {
				
				ExecutionNode * node = new ExecutionNode(this,
					(PipelineStage *)stage,
					inst->inst(),
					inst->index(),
					inst->codePart());
				if (stage->usesFunctionalUnits()) {
					
					// the category of this instruction is unknown 
					//      => assume execution with minimum/maximum latency among all functional units
					int min_latency = INFINITE_TIME, max_latency = 0;
					for(genstruct::Vector<PipelineStage::FunctionalUnit *>::Iterator
					fu(stage->getFUs()); fu; fu++) {
						int min_lat = 0, max_lat = 0;
						for(PipelineStage::FunctionalUnit::PipelineIterator fu_stage(fu);
						fu_stage; fu_stage++) {
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
					PipelineStage::FunctionalUnit *fu = stage->findFU(inst->inst()->kind()); 
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
	
	ExecutionNode *node = lastNode(BEFORE_PROLOGUE);
	if (node) {
		// the instruction before the prologue is assumed to produce every register
		// this instruction is guaranted to be executed when its last node is finished
		for (int b=0 ; b<reg_bank_count ; b++) {
			for (int r=0 ; r < rename_tables[b].reg_bank ->count() ; r++) {
				rename_tables[b].table->set(r,node);
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
//				LOG( dumpFile << "\tEdge between consecutive pipeline stages: " << edge->name() << ")\n";)
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
								if (!exists) {
									// add an edge for data dependency
									ExecutionEdge *edge = new ExecutionEdge(producing_node, node, ExecutionEdge::SOLID);
//									LOG( dumpFile << "\tEdge for data dependancy: " << edge->name()<< ")\n";)	
								}
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
			ExecutionNode * previous = NULL;	// !!WARNING!!
			if (stage->width() == 1) {
				// scalar stage
				for (PipelineStage::ExecutionNodeIterator node(stage) ; node ; node++) {
					if (previous != NULL) {				  
						// scalar stage => draw a solid edge
						ExecutionEdge * edge = new ExecutionEdge(previous, node, ExecutionEdge::SOLID);
//						LOG(dumpFile << "\tEdge for program order (scalar): " << edge->name() << "\n";)
					}
					previous = node;
				}
			}
			else {
				elm::genstruct::DLList<ExecutionNode *> previous_nodes;
				for (PipelineStage::ExecutionNodeIterator node(stage) ; node ; node++) {			
					// superscalar => draw a slashed edge between adjacent instructions
					if(previous)
						ExecutionEdge *edge = new ExecutionEdge(previous, node, ExecutionEdge::SLASHED);
//					LOG(dumpFile << "\tSlashed edge between successive instructions (superscalar stage): " << edge->name() << ")\n";)
					
					// draw a solid edge to model the stage width 
					if (previous_nodes.count() == stage->width()) {
						ExecutionEdge *edge = new ExecutionEdge(previous_nodes.first(), node, ExecutionEdge::SOLID);
//						LOG(dumpFile << "\tEdge between successive instructions (superscalar stage): " << edge->name() << ")\n";)
						previous_nodes.removeFirst();	
					}
					previous_nodes.addLast(node);
					previous = node;
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
						ExecutionEdge *edge = new ExecutionEdge(node, waiting_node, ExecutionEdge::SLASHED);
//						LOG( dumpFile << "\tEdge for queue capacity: " << edge->name()<< ")\n";)
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
//							LOG( 	dumpFile << "\t" << node1->name();
//									dumpFile << " contends with " << node2->name() << "\n";
//								)	
							//node2->addContender(node1);
						}
					}
				}
			}
			else {
				/*for (int i=0 ; i<INST_CATEGORY_NUMBER ; i++) {
					PipelineStage *fu_stage = stage->functionalUnit(i)->firstStage();*/
				for(genstruct::Vector<PipelineStage::FunctionalUnit *>::Iterator
				fu(stage->getFUs()); fu; fu++) {
					PipelineStage *fu_stage = fu->firstStage();
					
					for (PipelineStage::ExecutionNodeIterator node1(fu_stage) ; node1 ; node1++) {
						for (PipelineStage::ExecutionNodeIterator node2(fu_stage) ; node2 ; node2++) {
							if ((ExecutionNode *)node1 != (ExecutionNode *)node2) {
								node1->addContender(node2);
//								LOG( 	dumpFile << "\t" << node1->name();
//									dumpFile << " contends with " << node2->name() << "\n";
//								)	
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
int ExecutionGraph::analyze() {
	int step = 0;

	shadeNodes();
	initSeparated();
	do {
		latestTimes();		
		earliestTimes();
		step++;
	} while ((step < 10) && (!unchangedSeparated()));
//	LOG(elm::cout << "Max finish time of last body node: " << lastNode(BODY)->maxFinishTime() << "\n");
	return (lastNode(BODY)->maxFinishTime() - minDelta());
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

bool ExecutionGraph::unchangedSeparated() {
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
				bool sep = separated(first, node);
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
				bool sep = separated(node, second);
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
