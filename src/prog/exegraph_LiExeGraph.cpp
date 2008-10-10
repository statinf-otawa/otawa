/*
 *	$Id$
 *	exegraph module implementation
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

#include <otawa/exegraph/LiExeGraph.h>
#include <otawa/graph/PreorderIterator.h>

using namespace  otawa;

void LiExeGraph::dump(elm::io::Output& dotFile) {
	
	dotFile << "digraph G {\n";
	for (InstIterator inst(_sequence) ; inst ; inst++) {
		// dump nodes
		dotFile << "{ rank = same ; ";
		for (InstNodeIterator node(inst) ; node ; node++) {
			dotFile << "\"" << node->pipelineStage()->shortName();
			dotFile << "I" << node->inst()->index() << "\" ; ";
		}
		dotFile << "}\n";
		// again to specify labels
		for (InstNodeIterator node(inst) ; node ; node++) {
			dotFile << "\"" << node->pipelineStage()->shortName();
			dotFile << "I" << node->inst()->index() << "\"";
			dotFile << " [shape=record, ";
			if (node->inst()->codePart() == BODY)
				dotFile << "color=blue, ";
			if (node->isShaded())
				dotFile << "color=red, ";
			dotFile << "label=\"" << node->pipelineStage()->shortName();
			dotFile << "(I" << node->inst()->index() << ") ";
			dotFile << "| { {" << timeName(node->time(READY,MIN));
			dotFile  << "|" << timeName(node->time(READY,MAX));
			dotFile << "} | {" << timeName(node->time(START,MIN));
			dotFile << "|" << timeName(node->time(START,MAX));
			dotFile << "} | {" << timeName(node->time(FINISH,MIN));
			dotFile << "|" << timeName(node->time(FINISH,MAX));
				dotFile << "} }";		
			dotFile << "\"] ; \n";
		}
		dotFile << "\n";
	}
	
						
	int group_number = 0;
	for (InstIterator inst(_sequence) ; inst ; inst++) {	
		// dump edges
		for (InstNodeIterator node(inst) ; node ; node++) {
			for (Successor next(node) ; next ; next++) {
				if ( node != inst->firstNode()
						||
						(!node->producesOperands()) 
						|| (node->inst()->index() == next->inst()->index()) ) {				
					dotFile << "\"" << node->pipelineStage()->shortName();
					dotFile << "I" << node->inst()->index() << "\"";
					dotFile << " -> ";
					dotFile << "\"" << next->pipelineStage()->shortName();
					dotFile << "I" << next->inst()->index() << "\"";
					switch( next.edge()->type()) {
						case ExeEdge::SOLID:
							if (node->inst()->index() == next->inst()->index())
								dotFile << "[minlen=4]";
							dotFile << " ;\n";
							break;
						case ExeEdge::SLASHED:
							dotFile << " [style=dotted";
							if (node->inst()->index() == next->inst()->index())
								dotFile << ", minlen=4";
							dotFile << "] ;\n";
							break;	
						default:
							break;
					}	
					if ((node->inst()->index() == next->inst()->index())
							|| ((node->pipelineStage()->index() == next->pipelineStage()->index())
								&& (node->inst()->index() == next->inst()->index()-1)) ) {
						dotFile << "\"" << node->pipelineStage()->shortName();
						dotFile << "I" << node->inst()->index() << "\"  [group=" << group_number << "] ;\n";
						dotFile << "\"" << next->pipelineStage()->shortName();
						dotFile << "I" << next->inst()->index() << "\" [group=" << group_number << "] ;\n";
						group_number++;
					}
				}
			}
		}
		dotFile << "\n";
	}
	
	dotFile << "}\n";
}

int LiExeGraph::analyze() {
	int step = 0;
	shadeNodes(firstNode(BODY));
	do {
		_times_changed = false;
		latestTimes();		
		earliestTimes();
		step++;
	} while ((step < 5) && (_times_changed));
	
	if (lastNode(PROLOGUE)) {
		int delta = INFINITE_TIME;
		int delay;
		for (Predecessor pred(firstNode(BODY)) ; pred ; pred++) {
			if (maxTimeToNode(pred, lastNode(PROLOGUE), &delay)) {
				// did not want to take the min latency of pred into account
				delay = delay - pred->latency( MIN);
				if (delay < delta) {
					delta = delay;
				}
			}
		}
		delta += lastNode(PROLOGUE)->latency( MIN);
		return(lastNode(BODY)->time( FINISH, MAX) - delta);
	}
	else {
		return lastNode(BODY)->time( FINISH, MAX);
	}
}

bool LiExeGraph::maxTimeToNode(LiExeNode *source, LiExeNode *target, int *time) {
	*time = 0;
	bool reached = false;
	int target_index = target->inst()->index();
	for (Successor next(source) ; next ; next++) {
		if ( next == target ) {
			reached = true;
		}
		else {
			if (next->inst()->index() <= target_index) {
				int next_time;
				if (maxTimeToNode(next,target,&next_time)) {
					reached = true;
					if (next_time > *time)
						*time = next_time;
				}
			}
		}
	}
	*time += source->latency( MIN);
	return reached;
}


void LiExeGraph::latestTimes() {
	LiExeNode * last_prologue_node = lastNode(PROLOGUE);
	LiExeNode *first_body_node;
	first_body_node = (LiExeNode *) _first_node[BODY];
	if (last_prologue_node) {
		first_body_node->setTime(READY, MIN, 0);
		first_body_node->setTime(START, MIN, 0);
		first_body_node->setTime(FINISH, MIN, first_body_node->latency(MIN));
		first_body_node->setTime(READY, MAX, 0);
		first_body_node->setTime(START, MAX, 0);
		first_body_node->setTime(FINISH, MAX, first_body_node->latency(MAX));
		shadeNodes(first_body_node);
	}
	first_body_node->setTime( READY, MAX,0);
	for(PreorderIterator node(this, this->_entry_node); node; node++) {
		nodeLatestTimes(node);
	}
}

void LiExeGraph::earliestTimes() {
	_first_node[BODY]->setTime(READY,MIN,0);
	for(PreorderIterator node(this, this->_entry_node); node; node++) {
		if (node->inst()->codePart() >= BODY)
			nodeEarliestTimes(node);
	}
	
}

void LiExeGraph::shadeNodes(LiExeNode *node) {
	
	if (node->hasPred()) {
		for (Predecessor pred(node) ; pred ; pred++) {
			pred->shade();
			if ( pred.edge()->type() == ExeEdge::SOLID) {
				if ( pred->time(FINISH,MAX) > node->time(READY, MAX)) {
					pred->setTime(FINISH,MAX, node->time( READY,  MAX));
					pred->setTime(START,MAX, pred->time(FINISH,MAX) - pred->latency(MIN));
					pred->setTime(READY,MAX, pred->time(START,MAX));			
				}
			}
			else {
				if (node->time(START,MAX) < pred->time(START,MAX) ) {
					pred->setTime(START,MAX, node->time(START,MAX));
					pred->setTime(FINISH,MAX, pred->time(START,MAX) + pred->latency(MAX));
					pred->setTime(READY,MAX, pred->time(START,MAX));
				}
			}
			shadeNodes(pred);				
		}
	}
}

void LiExeGraph::nodeLatestTimes(LiExeNode *node) {
	if (node->isShaded())
		return;
	int parv;
	parv = node->pipelineStage()->width();
	elm::genstruct::AllocatedTable<int> times(parv) ;
	
	/* node.start.latest = node.ready.latest */
	node->setTime(START,MAX, node->time(READY,MAX));
	
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
	for(LiExeNode::ContenderIterator cont(node); cont; cont ++) {
		if ( cont->inst()->index() > node->inst()->index() ) {
			// cont is a *late* contender
			if (!separated(cont, node)
					&&
					(cont->time(START,MIN) < node->time(READY,MIN)) ) {
				count++;
				if (index < parv) {
					// less than parv values have been registered
					times[index++] = cont->time(FINISH,MAX);
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
					if (cont->time(FINISH,MAX) > min)
						times[pos] = cont->time(FINISH,MAX);
				}
			}
		}
	}
	if (count >= parv) {
		int worst_case_start_time = node->time(READY,MAX) + node->latency(MAX) - 1;
		int max_start_time = times[0];
		for (int i=1 ; i<index ; i++) {
			if (times[i] < max_start_time) {
				max_start_time = times[i];
			}
		}
		if (max_start_time < worst_case_start_time ) 
			worst_case_start_time = max_start_time;
		node->setTime(START,MAX, worst_case_start_time);
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
	for( LiExeNode::ContenderIterator cont(node); cont; cont ++) {
		if ( cont->inst()->index() < node->inst()->index() ) {
			// cont is an *early* contender
			if ( !separated(cont, node) ) {
				count++;
				if (index < parv)
					// less than parv values have been registered
					times[index++] = cont->time(FINISH, MAX);
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
					if (cont->time(FINISH,MAX) > min)
						times[pos] = cont->time(FINISH,MAX);
				}
			}
		}
	}
	
	if(lastNode(BEFORE_PROLOGUE)) { // node might have contenders before the prologue (they finish one cycle before CM(I-p))
		int possible_contenders = -node->inst()->index();
		int finish_time = lastNode(BEFORE_PROLOGUE)->time(FINISH,MAX) - 1;
		bool stop = false;
		while ((possible_contenders > 0) && !stop) {
			if (index < parv)
				// less than parv values have been registered
				times[index++] = finish_time;
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
				if (finish_time > min)
					times[pos] = finish_time;
				else
					stop = true; // all contenders before the prologue have the same max finish time
			}
			possible_contenders--;
		}
	}
	
	if (count >= parv) {
		int worst_case_start_time =  node->time(START,MAX) + node->latency(MAX) * (count / parv);
		int max_start_time = times[0];
		for (int i=1 ; i<index ; i++) {
			if (times[i] < max_start_time) {
				max_start_time = times[i];
			}
		}
	if (max_start_time < worst_case_start_time ) 
			worst_case_start_time = max_start_time;
		if (worst_case_start_time  > node->time(START,MAX)) {
			node->setTime(START,MAX, worst_case_start_time);
		}
	}
	
	/* v.finish.latest = v.start.latest + max_lat(v); */
		
	node->setTime(FINISH,MAX, node->time(START,MAX)+node->latency(MAX) );
			
	/* foreach immediate successor w of v do
	 * 		if slashed edge
	 * 			w.ready.latest = MAX(w.ready.latest, v.start.latest);
	 * 		if solid edge
	 * 			w.ready.latest = MAX(w.ready.latest, v.finish.latest
	 */
	 
	for (Successor next(node) ; next ; next++) {
		if (next.edge()->type() == ExeEdge::SLASHED) {
			if (node->time(START,MAX) > next->time(READY,MAX))	
				next->setTime(READY,MAX, node->time(START,MAX));
		}
		else {// SOLID 
			if (node->time(FINISH,MAX) > next->time(READY,MAX))
				next->setTime(READY,MAX, node->time(FINISH,MAX));
		}
	}
}

void LiExeGraph::nodeEarliestTimes(LiExeNode *node) {
	int parv;
	parv = node->pipelineStage()->width();
	elm::genstruct::AllocatedTable<int> times(parv) ;
	
	/* node.start.earliest = node.ready.earliest */
	node->setTime(START,MIN, node->time(READY,MIN));

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
	for(LiExeNode::ContenderIterator cont(node); cont; cont ++) {
		if (  ( (cont->inst()->index() > node->inst()->index())
				&&
				!separated(cont, node)
				&&
				(cont->time(START,MAX) < node->time(READY,MIN))
				&&
				(node->time(READY,MIN) < cont->time(FINISH,MIN)) )
			||
			  ( (cont->inst()->index() < node->inst()->index())
			  	&&
			  	!separated(cont, node)
			  	&&
			  	(cont->time(START,MAX) <= node->time(READY,MIN))
			  	&&
			  	node->time(READY,MIN) < cont->time(FINISH,MIN) ) ) {
			  		
			count++;
			if (index < parv)
				// less than parv values have been registered
				times[index++] = cont->time(FINISH,MAX)
				;
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
				if (cont->time(FINISH,MAX) > min)
					times[pos] = cont->time(FINISH,MAX);
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
		if (max_delay > node->time(READY,MIN) ) 
			node->setTime(START,MIN, max_delay);
		else
			node->setTime(START,MIN, node->time(READY,MIN));
	}
	
	
	/* v.finish.earliest = v.start.earliest + min_lat(v); */
		
	node->setTime(FINISH,MIN, node->time(START,MIN) + node->latency(MIN) );
		
	/* foreach immediate successor w of v do
	 * 		if slashed edge
	 * 			w.ready.latest = MAX(w.ready.latest, v.start.latest);
	 * 		if solid edge5       14

	 * 			w.ready.latest = MAX(w.ready.latest, v.finish.latest
	 */

	for (Successor next(node) ; next ; next++) {

		if (next.edge()->type() == ExeEdge::SLASHED) {
		  if (node->time(START,MIN) > next->time(READY,MIN)){
		    	next->setTime(READY,MIN, node->time(START,MIN));

		  }
		}
		else {// SOLID 
		  if (node->time(FINISH,MIN) > next->time(READY,MIN)){
				next->setTime(READY,MIN, node->time(FINISH,MIN));

		  }
		}
	}
}


bool LiExeGraph::separated(LiExeNode *u, LiExeNode *v) {
	if (v->time(READY,MIN) >= u->time(FINISH,MAX))
		return true;
	if (u->time(READY,MIN) >= v->time(FINISH,MAX))
		return true;
	return false;
}

/**
 * @class LiExeNode : public ExeGraph<LiExeNode>::ExeNode
 * @brief An execution graph node with special features to be handled
 * by the algorithm proposed by Li et al. ("X. Li, A. Roychoudhury, T. Mitra, 
 * Modeling out-of-order processors for WCET analysis, Real-Time Systems, 34(3), 2006").
 */
 
/**
 * @fn LiExeNode::LiExeNode(ExeGraph<LiExeNode> *graph, PipelineStage<LiExeNode> *stage, ExeInst<LiExeNode> *inst)
 * Builds an execution node tuned for Li's algorithm.
 * @param	graph	The execution graph that contains the node.
 * @p	aram	stage	The pipeline stage the node is related to.
 * @param	inst	The instruction the node is related to.
 */
  
/**
 * @fn LiExeNode::time(ExeGraph<LiExeNode>::time_step_t step, ExeGraph<LiExeNode>::time_bound_t bound)
 * @param	step	Time step (READY, START, FINISH)
 * @param	bound 	Time bound (MIN, MAX)
 * @return	The node time
 */
   
/**
 * @fn LiExeNode::setTime(ExeGraph<LiExeNode>::time_step_t step, ExeGraph<LiExeNode>::time_bound_t bound, int time)
 * @param	step	Time step (READY, START, FINISH)
 * @param	bound 	Time bound (MIN, MAX)
 * @param	time	The node new time value.
 * Sets the new node time value.
 */
   	
/**
 * @fn LiExeNode::shade
 * Marks the node as "shaded" (see Li's algorithm)
 */
   	 
/**
 * @fn LiExeNode::isShaded
 * @return Whether the node is shaded or not.
 */
   	  
/** @class LiExeGraph
 * @brief An execution graph to be solved by Li's algorithm ("X. Li, A. Roychoudhury, T. Mitra, 
 * Modeling out-of-order processors for WCET analysis, Real-Time Systems, 34(3), 2006").
 */
 
/**
  * LiExeGraoh::LiExeGraph(FrameWork *fw,  Microprocessor<LiExeNode> * microprocessor, ExeSequence<LiExeNode> *sequence)	
  * Constructor.
  * @param	microprocessor	The description of the target microprocessor
  * @param	sequence	The sequence of instructions to be analyzed.
  */
  
/**	LiExeGraph::setTimesChanged() 
 * Marks that node times have changed
 */
 
/**
 * LiExeGraph::dump(elm::io::Output& dotFile)
 * Dumps the graph in dot format.
 * @param	dotFile	The output file (dot format)
 */
 
/**
 * LiExeGraph::analyze()
 * Computes the sequence execution time using Li's algorithm.
 * @return	The execution time.
 */
