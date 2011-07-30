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


#include "ExecutionGraph.h"

using namespace  otawa;
using namespace otawa::newexegraph;




// ----------------------------------------------------------------

void ExecutionGraph::build(bool compressed_cod) {

 //   createResources();

    createNodes();
    findDataDependencies();

    addEdgesForPipelineOrder();
    addEdgesForFetch();
    addEdgesForProgramOrder();

    addEdgesForMemoryOrder();
    addEdgesForDataDependencies();

    addEdgesForQueues();
 //   findContendingNodes();

}

// ----------------------------------------------------------------

void ExecutionGraph::createNodes() {

    // consider every instruction
    for (InstIterator inst(_sequence) ; inst ; inst++)  {
		// consider every pipeline stage
		for (EGPipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {

			// create node
			EGNode *node;
			if (stage->category() != EGStage::EXECUTE) {
				node = new EGNode(this, stage, inst);
				inst->addNode(node);
				stage->addNode(node);
				if (stage->category() == EGStage::FETCH) {
					inst->setFetchNode(node);
				}
				if (!_first_node)
					_first_node = node;
				if (inst->part() == PREDECESSOR)  // FIXME
					_last_prologue_node = node;
				if (!_first_bb_node && (inst->part() == BLOCK) )
					_first_bb_node = node;
				_last_node = node;
			}
			else {
				// add FU nodes
				EGPipeline *fu = stage->findFU(inst->inst()->kind());
				ASSERTP(fu, "cannot find FU for instruction " << inst->inst()->address() << " " << inst->inst());
				int index = 0;

				for(EGPipeline::StageIterator fu_stage(fu); fu_stage; fu_stage++) {
					EGNode *fu_node = new EGNode(this, fu_stage, inst);
					inst->addNode(fu_node);
					fu_stage->addNode(fu_node);
					if (index == 0)
						inst->setExecNode(fu_node);
					index++;
				}
			}

		} // endfor each pipeline stage

    } // endfor each instruction

}


// ----------------------------------------------------------------

void ExecutionGraph::findDataDependencies() {

    otawa::hard::Platform *pf = _ws->platform();
    AllocatedTable<rename_table_t> rename_tables(pf->banks().count());
    int reg_bank_count = pf->banks().count();
    for(int i = 0; i <reg_bank_count ; i++) {
		rename_tables[i].reg_bank = (otawa::hard::RegBank *) pf->banks()[i];
		rename_tables[i].table =
			new AllocatedTable<EGNode *>(rename_tables[i].reg_bank->count());
		for (int j=0 ; j<rename_tables[i].reg_bank->count() ; j++)
			rename_tables[i].table->set(j,NULL);
    }

    // consider every instruction
    for (InstIterator inst(_sequence) ; inst ; inst++)  {
		EGNode *first_fu_node = NULL, *last_fu_node = NULL;
		for (InstNodeIterator node(inst); node ; node++){
			if (node->stage()->category() == EGStage::FU){
				if (!first_fu_node)
					first_fu_node = node;
				last_fu_node = node;
			}
		}
		// check for data dependencies
		const elm::genstruct::Table<otawa::hard::Register *>& reads = first_fu_node->inst()->inst()->readRegs();
		for(int i = 0; i < reads.count(); i++) {
			for (int b=0 ; b<reg_bank_count ; b++) {
				if (rename_tables[b].reg_bank == reads[i]->bank()) {
					EGNode *producer = rename_tables[b].table->get(reads[i]->number());
					if (producer != NULL) {
						first_fu_node->addProducer(producer);
					}
				}
			}
		}
		// fu_node is the last FU node
		const elm::genstruct::Table<otawa::hard::Register *>& writes = last_fu_node->inst()->inst()->writtenRegs();
		for(int i = 0; i < writes.count(); i++) {
			for (int b=0 ; b<reg_bank_count ; b++) {
				if (rename_tables[b].reg_bank == writes[i]->bank()) {
					rename_tables[b].table->set(writes[i]->number(),last_fu_node);
				}
			}
		}

    } // endfor each instruction

    // Free rename tables
    for(int i = 0; i <reg_bank_count ; i++)
		delete rename_tables[i].table;

}

// ----------------------------------------------------------------
void ExecutionGraph::addEdgesForPipelineOrder(){
    for (InstIterator inst(_sequence) ; inst ; inst++)  {
		for (int i=0 ; i<inst->numNodes()-1 ; i++){
			new EGEdge(inst->node(i), inst->node(i+1), EGEdge::SOLID);
		}
    }
}


/**
 * Add edge for fetch blocking, that is, edges ensuring that instruction in the same
 * block are fetched in the same and that instructions in sequence owned by different blocks
 * require two fetches to be obtained.
 *
 * For example, for a block size of 16 with fixed size instructions of 4, the instruction
 * sequence is marked with fetches bounds:
 * @li start of basic block
 * @li 0x100C	fetch
 * @li 0x1010	fetch
 * @li 0x1014
 * @li 0x1018
 * @li 0x101C
 * @li 0x1010	fetch
 * @li 0x1014
 */
void ExecutionGraph::addEdgesForFetch(){
    EGStage *fetch_stage = _microprocessor->fetchStage();

    // traverse all fetch nodes
    EGNode * first_cache_line_node = fetch_stage->firstNode();
    address_t current_cache_line = fetch_stage->firstNode()->inst()->inst()->address().address() /  _cache_line_size;
    for(int i=0 ; i<fetch_stage->numNodes()-1 ; i++) {
		EGNode *node = fetch_stage->node(i);
		EGNode *next = fetch_stage->node(i+1);

		// taken banch ?
		if (node->inst()->inst()->topAddress() != next->inst()->inst()->address()){
			// fixed by casse: topAddress() is address() + size()
			EGEdge * edge = new EGEdge(node, next, EGEdge::SOLID);
//			edge->setLatency(_branch_penalty); // taken branch penalty when no branch prediction is enabled  // FIXME
			edge = new EGEdge(first_cache_line_node, next, EGEdge::SOLID);
//			edge->setLatency(_branch_penalty);  // FIXME
		}
		else {
			new EGEdge(node, next, EGEdge::SLASHED);
		}

		// new cache line?
		//if (cache)         FIXME !!!!!!!!!!!!!!!
		address_t cache_line = next->inst()->inst()->address().address() /  _cache_line_size;
		if ( cache_line != current_cache_line){
			new EGEdge(first_cache_line_node, next, EGEdge::SOLID);
			new EGEdge(node, next, EGEdge::SOLID);
			first_cache_line_node = next;
			current_cache_line = cache_line;
		}
		//    }
    }
}


// ----------------------------------------------------------------

void ExecutionGraph::addEdgesForProgramOrder(elm::genstruct::SLList<EGStage *> *list_of_stages){

    elm::genstruct::SLList<EGStage *> *list;
    if (list_of_stages != NULL)
		list = list_of_stages;
    else {
		// if no list of stages was provided, built the default list that includes all IN_ORDER stages
		list = new  elm::genstruct::SLList<EGStage *>;
		for (EGPipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
			if (stage->orderPolicy() == EGStage::IN_ORDER){
				if (stage->category() != EGStage::FETCH){
					list->add(stage);
				}
				if (stage->category() == EGStage::EXECUTE){
					for (int i=0 ; i<stage->numFus() ; i++){
						EGStage *fu_stage = stage->fu(i)->firstStage();
						if (fu_stage->hasNodes()){
							list->add(fu_stage);
						}
					}
				}
			}
		}
    }

    for (StageIterator stage(list) ; stage ; stage++) {
		int count = 1;
		int prev = 0;
		for (int i=0 ; i<stage->numNodes()-1 ; i++){
			EGNode *node = stage->node(i);
			EGNode *next = stage->node(i+1);
			if (stage->width() == 1){
				new EGEdge(node, next, EGEdge::SOLID);
			}
			else {
				new EGEdge(node, next, EGEdge::SLASHED);
				if (count == stage->width()){
					EGNode *previous = stage->node(prev);
					new EGEdge(previous,next,EGEdge::SOLID);
					prev++;
				}
				else
					count++;
			}
		}
    }
}


// ----------------------------------------------------------------

void ExecutionGraph::addEdgesForMemoryOrder(){

    EGStage *stage = _microprocessor->execStage();
    for (int i=0 ; i<stage->numFus() ; i++) {
		EGStage *fu_stage = stage->fu(i)->firstStage();
		EGNode * previous_load = NULL;
		EGNode * previous_store = NULL;
		for (int j=0 ; j<fu_stage->numNodes() ; j++){
			EGNode *node = fu_stage->node(j);
			if (node->inst()->inst()->isLoad()) {
				if (previous_store) {// memory access are executed in order
					new EGEdge(previous_store, node, EGEdge::SOLID);
				}
				for (InstNodeIterator last_node(node->inst()); last_node ; last_node++){
					if (last_node->stage()->category() == EGStage::FU)
						previous_load = last_node;
				}
			}
			if (node->inst()->inst()->isStore()) {
				if (previous_store) {// memory access are executed in order
					new EGEdge(previous_store, node, EGEdge::SOLID);
				}
				if (previous_load) {// memory access are executed in order
					new EGEdge(previous_load, node, EGEdge::SOLID);
				}
				for (InstNodeIterator last_node(node->inst()); last_node ; last_node++){
					if (last_node->stage()->category() == EGStage::FU){
						previous_store = last_node;
					}
				}
			}
		}
    }
}

// ----------------------------------------------------------------

/**
 * Add edges for data dependencies, that is, if an instruction (a)
 * produces content of a register and instruction (b) uses this register value
 * create a solid edge between their execute stages.
 */
void ExecutionGraph::addEdgesForDataDependencies(){
    EGStage *exec_stage = _microprocessor->execStage();
    for (int j=0 ; j<exec_stage->numFus() ; j++) {
		EGStage *fu_stage = exec_stage->fu(j)->firstStage();
		for (int k=0 ; k<fu_stage->numNodes() ; k++) {
			EGNode *node = fu_stage->node(k);
			for (int p=0 ; p<node->numProducers(); p++) {
				EGNode *producer = node->producer(p);
				new EGEdge(producer, node, EGEdge::SOLID);
			}
		}
    }
}

// ----------------------------------------------------------------

void ExecutionGraph::addEdgesForQueues(){

    // build edges for queues with limited capacity */
    for (EGPipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
		EGStage * prod_stage;
		if (stage->sourceQueue() != NULL) {
			EGQueue *queue = stage->sourceQueue();
			int size = queue->size();
			prod_stage = queue->fillingStage();
			for (int i=0 ; i<stage->numNodes() - size ; i++) {
				assert(i+size < prod_stage->numNodes());
				new EGEdge(stage->node(i), prod_stage->node(i + size), EGEdge::SLASHED);
			}
		}
    }
}



// ----------------------------------------------------------------

ExecutionGraph::~ExecutionGraph() {
    for (EGPipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
		stage->deleteNodes();
		if (stage->category() == EGStage::EXECUTE) {
			for (int i=0 ; i<stage->numFus() ; i++) {
				EGPipeline *fu = stage->fu(i);
				for (EGPipeline::StageIterator fu_stage(fu); fu_stage; fu_stage++)
					fu_stage->deleteNodes();
			}
		}
    }
    for (EGSequence::InstIterator inst(_sequence) ; inst ; inst++) {
		inst->deleteNodes();
    }
}


/**
 * Manage the attribute dump.
 * Must be called before the first attribute is generated.
 */
static void dumpAttrBegin(io::Output& out, bool& first) {
	first = true;
}

/**
 * Manage the attribute dump.
 * Must be called before displaying an attribute.
 */
static void dumpAttr(io::Output& out, bool& first) {
	if(first)
		out << " [ ";
	else
		out << ", ";
	first = false;
}

/**
 * Manage the attribute dump.
 * Must be called after the last attribute has been generated.
 */
static void dumpAttrEnd(io::Output& out, bool& first) {
	if(!first)
		out << " ]";
}


// ---------------------------------------
void ExecutionGraph::dump(elm::io::Output& dotFile, const string& info) {
    int i=0;
    bool first_line = true;
    int width = 5;
    dotFile << "digraph G {\n";

    // dipsplay information if any
    if(info)
    	dotFile << "\"info\" [shape=record, label=\"{" << info << "}\"];\n";

/*    // display ressources
    dotFile << "\"legend\" [shape=record, label= \"{ ";
    for (elm::genstruct::Vector<Resource *>::Iterator res(_resources) ; res ; res++) {
		if (i == 0) {
			if (!first_line)
				dotFile << " | ";
			first_line = false;
			dotFile << "{ ";
		}
		dotFile << res->name();
		if (i < width-1){
			dotFile << " | ";
			i++;
		}
		else {
			dotFile << "} ";
			i = 0;
		}
    }
    if (i!= 0)
		dotFile << "} ";
    dotFile << "} ";
    dotFile << "\"] ; \n";*/

    // display instruction sequence
    dotFile << "\"code\" [shape=record, label= \"\\l";
    bool body = true;
    BasicBlock *bb = 0;
    for (InstIterator inst(_sequence) ; inst ; inst++) {
		if(inst->part() == BLOCK && body) {
			body = false;
			dotFile << "------\\l";
		}
    	BasicBlock *cbb = inst->basicBlock();
    	if(cbb != bb) {
    		bb = cbb;
    		dotFile << bb << "\\l";
    	}
    	dotFile << "I" << inst->index() << ": ";
		dotFile << "0x" << fmt::address(inst->inst()->address()) << ":  ";
		inst->inst()->dump(dotFile);
		dotFile << "\\l";
    }
    dotFile << "\"] ; \n";

    // edges between info, legend, code
    if(info)
    	dotFile << "\"info\" -> \"legend\";\n";
    dotFile << "\"legend\" -> \"code\";\n";

    // display nodes
    for (InstIterator inst(_sequence) ; inst ; inst++) {
		// dump nodes
		dotFile << "{ rank = same ; ";
		for (InstNodeIterator node(inst) ; node ; node++) {
			dotFile << "\"" << node->stage()->name();
			dotFile << "I" << node->inst()->index() << "\" ; ";
		}
		dotFile << "}\n";
		// again to specify labels
		for (InstNodeIterator node(inst) ; node ; node++) {
			dotFile << "\"" << node->stage()->name();
			dotFile << "I" << node->inst()->index() << "\"";
			dotFile << " [shape=record, ";
			if (node->inst()->part() == BLOCK)
				dotFile << "color=blue, ";
			dotFile << "label=\"" << node->stage()->name();
			dotFile << "(I" << node->inst()->index() << ") [" << node->latency() << "]\\l" << node->inst()->inst();
			dotFile << "| { ";
/*
			int i=0;
			int num = _resources.length();
			while (i < num) {
				int j=0;
				dotFile << "{ ";
				while ( j<width ) {
					if ( (i<num) && (j<num) ) {
						if (node->e(i))
							dotFile << node->d(i);
					}
					if (j<width-1)
						dotFile << " | ";
					i++;
					j++;
				}
				dotFile << "} ";
				if (i<num)
					dotFile << " | ";
			}
*/
			dotFile << "} ";
			dotFile << "\"] ; \n";
		}
		dotFile << "\n";
    }

    // display edges
    int group_number = 0;
    for (InstIterator inst(_sequence) ; inst ; inst++) {
		// dump edges
		for (InstNodeIterator node(inst) ; node ; node++) {
			for (Successor next(node) ; next ; next++) {
				if ( node != inst->firstNode()
					 ||
					 (node->stage()->category() != EGStage::EXECUTE)
					 || (node->inst()->index() == next->inst()->index()) ) {

					// display edges
					dotFile << "\"" << node->stage()->name();
					dotFile << "I" << node->inst()->index() << "\"";
					dotFile << " -> ";
					dotFile << "\"" << next->stage()->name();
					dotFile << "I" << next->inst()->index() << "\"";

					// display attributes
					bool first;
					dumpAttrBegin(dotFile, first);

					// latency if any
					if(next.edge()->latency()) {
						dumpAttr(dotFile, first);
						dotFile << "label=\"" << next.edge()->latency() << "\"";
					}

					// edge style
					switch( next.edge()->type()) {
					case EGEdge::SOLID:
						if (node->inst()->index() == next->inst()->index()) {
							dumpAttr(dotFile, first);
							dotFile << "minlen=4";
						}
						break;
					case EGEdge::SLASHED:
						dumpAttr(dotFile, first);
						dotFile << " style=dotted";
						if (node->inst()->index() == next->inst()->index()) {
							dumpAttr(dotFile, first);
							dotFile << "minlen=4";
						}
						break;
					default:
						break;
					}

					// dump attribute end
					dumpAttrEnd(dotFile, first);
					dotFile << ";\n";

					// group
					if ((node->inst()->index() == next->inst()->index())
						|| ((node->stage()->index() == next->stage()->index())
							&& (node->inst()->index() == next->inst()->index()-1)) ) {
						dotFile << "\"" << node->stage()->name();
						dotFile << "I" << node->inst()->index() << "\"  [group=" << group_number << "] ;\n";
						dotFile << "\"" << next->stage()->name();
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

/**
 * Build a parametric execution graph.
 * @param ws	Current workspace.
 * @param proc	Processor description.
 * @param seq	Instruction sequence to compute.
 * @param props	Other parameters.
 */
ExecutionGraph::ExecutionGraph(
	WorkSpace *ws,
	EGProc *proc,
	EGSequence *seq,
	const PropList& props
)
:	_ws(ws),
 	_microprocessor(proc),
 	_sequence(seq),
 	_first_node(NULL),
 	_first_bb_node(NULL),
 	_last_prologue_node(NULL),
 	_last_node(NULL),
 	_branch_penalty(2)
{
	_props = props;
	_cache_line_size = 1;
}


/**
 * @fn void ParExeGraph::setFetchSize(int size);
 * Set the size in bytes used to fetch instructions.
 * @param size	Size in bytes of fetched blocks.
 */


// ----------------------------------------------------------------

void ExecutionGraph::display(elm::io::Output&) {
}







