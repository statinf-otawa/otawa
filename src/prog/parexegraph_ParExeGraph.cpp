
#include <elm/string.h>
#include <otawa/parexegraph/ParExeGraph.h>

namespace otawa {

/**
 * @defgroup peg Parametric Execution Graph
 *
 * Parametric Execution Graph, or parexegraph, is a way to compute execution of code blocks. A parexegraph
 * is a graph whose nodes represents the crossing of an instruction in the microprocessor stage
 * and the edges the time constraints of this crossing.
 *
 * @section Principles
 * This graph is built for a sequence of instructions, whose the execution time is required, and a prefix sequence
 * to take into account the overlapping effect of the pipeline. Depending on the actual microprocessor, the longer
 * is the prefix sequencer, better are the obtained result. The cost of the sequence is obtained after assigning to each
 * node the last starting date and computing the difference between the last stage of the last instruction of the sequence
 * and the last stage of the last instruction of the prefix sequence. The date assignment to each node is performed under
 * the constraint of contention in the use of resources of the microprocessor (stage parallelism, queues, functional units, etc).
 * Below is an example of such a parametric execution graph:
 *
 * @image html parexegraph-sample.png
 *
 * As a basis, the pipeline microprocessor is taken from the platform processor description (possibly coming
 * from an external XML file or specially built). As the model used to describe microprocessor is relatively simple
 * and does not allow to describe any type of microprocessor, the use of parexegraph may require to customize the way
 * the graph is built.
 *
 * @section Customization
 *
 * Basically, an execution graph is represented by a @ref ParExeGraph class whose provide also the computation
 * of the cost of a sequence. To compute the cost fore each block a program, for example basic block, a dedicated
 * processor like @ref GraphBBTime has to be invoked. As the execution graphs needs to be customized, this analyzer
 * takes a generic parameter defining the actual execution graph to use. To perform the computation in the standard way,
 * you have either to implemened @ref GraphBBTime with @ref ParExeGraph as the parametric argument:
 * @code
 * GraphBBTime<ParExeGraph> bbtime;
 * bbtime.process();
 * @endcode
 *
 * Or to use an analyzer as @ref BasicGraphBBTime that already provides such an implementation.
 *
 * Yet, if your hardware does not fits the standard OTAWA processor model, you may to customize the way the graphs are built.
 *
 * First, build a model as close as possible in the processor model of OTAWA and use a way to pass it to pass it to the WCET
 * building system. This may be done using the configuration properties (@ref PROCESSOR_PATH, @ref PROCESSOR_ELEMENT or @ref PROCESSOR)
 * or using an OTAWA script (in the platform section).
 *
 * Then, you provide your own parametric execution graph implementation by overloading the @ref ParExeGraph class
 * and pass as parametric argument to @ref GraphBBTime. To build the graph, the function @ref ParExeGraph::build() is called
 * that, in turn, calls the following methods:
 * @li void ParExeGraph::createNodes(void) -- creates the nodes of the graph,
 * @li void ParExeGraph::addEdgesForPipelineOrder(void) -- add edges to follow stage pipeline order,
 * @li void ParExeGraph::addEdgesForFetch(void) -- add edges to support fetching effects (according to the instruction cache as a default),
 * @li void ParExeGraph::addEdgesForProgramOrder(elm::genstruct::SLList<ParExeStage *> *list_of_stages) -- add edges to represent program order,
 * @li void ParExeGraph::addEdgesForMemoryOrder(void) -- add edges to support memory access order,
 * @li void ParExeGraph::addEdgesForDataDependencies(void) -- add edges for data dependencies,
 * @li void ParExeGraph::addEdgesForQueues(void) -- add edges to represent access to queues.
 *
 * All these methods are virtual and can be overridden to implements the specific behaviour of your hardware.
 *
 * @section Structure
 *
 * A parametric execution graph, @ref ParExeGraph, is made of nodes, @ref ParExeNode, and of edges, @ref ParExeEdge linking two nodes.
 * An edge may be solid, @ref ParExeEdge::SLASHED, representing a possible startup at the same time, or @ref ParExeEdge::SOLID,
 * representing a sequence in the startup (at least one cycle). The time passed in a node or on a edge may be customized.
 * A node is the join of an instruction, @ref ParExeInst, executed on a particular pipeline stage, @par ParExeStage.
 *
 * Basically,the parametric execution graph is generated from a processor description, @ref ParExeProc, whose most interesting
 * paer is the @ref ParExePipeline that contains two collections:
 * @li a collection of queues, @ref ParExeQueue, representing the queues inside the processor,
 * @li a list of stages, @ref ParExeStage, representing the stages in order in the pipeline.
 *
 * In turn, if the @ref ParExeStage is an execution stage, it is composed of @ref ParExePipeline that contains
 * functional units represented by @ref ParExeStage.
 *
 * To build the graph, a user has to pass @ref ParExeSequence, made of @ref ParExeInst, that represents, in order,
 * the concatenation of prefix and processed sequences. The @ref ParExeInst are marked according to their
 * ownership to the prefix or the processed sequence (type @ref ParExeInst::code_part_t).
 */


/**
 * @class ParExeGraph
 * Representation of a parametric execution graph (@ref peg).
 * @ingroup peg
 */

/**
 * Compute the cost, in cycles, of the current graph. The cost is the difference between
 * the execution in the commit stage of the last instruction of the considered instruction
 * and of the last prefix instruction.
 * @return	Cost for the current graph.
 */
int ParExeGraph::analyze() {

	clearDelays();
    initDelays();

    _capacity = 0;
    for(ParExeProc::QueueIterator queue(_microprocessor); queue; queue++){
		_capacity = queue->size();		//FIXME: capacity should be the size of the queue where instructions can be in conflict to access to FUs
    }

    propagate();
    analyzeContentions();

    //   for (int i=0 ; i<RES_TYPE_NUM ; i++) {                       // FIXME: useful?
    //     res_dep[i] = false;
    //     res_dep_impact[i] = 0;
    //   }

    if (_last_prologue_node)
		return(cost());	 
    else
		return (_last_node->d(0));  // resource 0 is BLOCK_START
}

// --------------------------------------------------------------------------------------------------

int ParExeGraph::cost() {
    int _cost = 0;
    int _start_cost = 0;
    int offset;
    ParExeNode *a = _last_node;
    for (elm::genstruct::Vector<Resource *>::Iterator res(_resources) ; res ; res++) {
		offset = 0;
		if (res->type() != Resource::INTERNAL_CONFLICT) {
			int r = res->index();
			if (res->type() == Resource::QUEUE) {
				StageResource * upper_bound = ((QueueResource *)(*res))->upperBound();
				int u = upper_bound->index();
				if ((a->e(r)) && (a->e(u))){
					if (a->d(r) <= a->d(u)
						+ (_microprocessor->pipeline()->numStages() - upper_bound->stage()->index())) {
						continue;
					}
				}
			}
			int tmp = Delta(a,res);
			if (res->type() == Resource::BLOCK_START) {
				_start_cost = tmp;
				_cost = tmp;
			}
			else {
				if (tmp > _cost) {
					_cost = tmp;
				}
			}
		}
    }
    return(_cost);
}

// -----------------------------------------------------------------
void ParExeGraph::propagate() {
    for (PreorderIterator node(this); node; node++) {
		if (node != _first_node) {
			for (Predecessor pred(node) ; pred ; pred++) {
				int _latency;
				if ( pred.edge()->type() == ParExeEdge::SOLID) {
					_latency = pred->latency() + pred.edge()->latency();
				}
				else {
					_latency = 0;	
				}
				for (elm::genstruct::Vector<Resource *>::Iterator resource(_resources) ; resource ; resource++) {
					int index = resource->index();
					if (pred->e(index)) {
						node->setE(index, true);
						int _delay = pred->d(index) + _latency;
						if (_delay > node->d(index)) {
							node->setD(index, _delay);
						}
					}
				}
			}
		}
    }
}

// --------------------------------------------------------------------------------------------------
int ParExeGraph::Delta(ParExeNode *a, Resource *res) {
	int r = res->index();
	if (!a->e(r))
		return (0);
	ParExeNode *lp = _last_prologue_node;

	int default_lp = lp->d(numResources() - 1);
	if(res->type() == Resource::STAGE)
		default_lp += _microprocessor->pipeline()->numStages() - ((StageResource *)(res))->stage()->index();
	else if(res->type() == Resource::QUEUE) {
		StageResource * upper_bound = ((QueueResource *) (res))->upperBound();
		int u = upper_bound->index();
		if(lp->e(u)) {
			int tmp = lp->d(u) + (_microprocessor->pipeline()->numStages() - upper_bound->stage()->index());
			if (tmp < default_lp)
				default_lp = tmp;
		}
	}

	int delta;
	if (lp->e(r))
		delta = a->d(r) - lp->d(r);
	else
		delta = a->d(r) - default_lp;

	for (elm::genstruct::Vector<Resource *>::Iterator resource(_resources); resource; resource++) {
		if (resource->type() == Resource::INTERNAL_CONFLICT) {
			int s = resource->index();
			ParExeNode * S = ((InternalConflictResource *)*resource)->node();
			if (a->e(s) && S->e(r)) {
				if (lp->e(s)) {
					int tmp = a->d(s) - lp->d(s);
					if (tmp > delta)
						delta = tmp;
				} // end: is lp depends on S

				else { //lp does not depend on S
					for (elm::genstruct::DLList<elm::BitVector *>::Iterator mask(*(S->contendersMasksList())); mask; mask++) {
						int tmp = a->d(s);
						tmp += (((mask->countBits() + S->lateContenders()) / S->stage()->width()) * S->latency());
						int tmp2 = 0;

						// mask is null == no early contenders
						if(mask->countBits() == 0) {
							int tmp3;
							if (!lp->e(r))
								tmp3 = tmp + S->d(r) - default_lp;
							else
								tmp3 = tmp + S->d(r) - lp->d(r);
							if (tmp3 > delta)
								delta = tmp3;
						}

						// mask is not null
						else {

							// get the conflicting resource
							for(elm::BitVector::OneIterator one(**mask); one; one++) {
								ParExeNode *C = S->stage()->node(one.item());
								int c = -1;
								for (elm::genstruct::Vector<Resource *>::Iterator ic(_resources); ic; ic++)
									if(ic->type() == Resource::INTERNAL_CONFLICT
									&& ((InternalConflictResource *) *ic)->node() == C)
										c = ((InternalConflictResource *) *ic)->index();
								ASSERT(c != -1);
								ASSERT(lp->e(c));
								if (lp->d(c) > tmp2)
									tmp2 = lp->d(c);
							} // end: foreach one in mask

							// fix the date for the ressource
							if (lp->e(r)) {
								if (lp->d(r) - S->d(r) > tmp2)
									tmp2 = lp->d(r) - S->d(r);
							}
							else {
								int tmp4 = lp->d(numResources() - 1) - S->d(r);
								if (res->type() == Resource::STAGE)
									tmp4 += _microprocessor->pipeline()->numStages() - ((StageResource *) (res))->stage()->index();
								if (tmp4 > tmp2)
									tmp2 = default_lp - S->d(r);
							}

							tmp2 = tmp - tmp2;
							if (tmp2 > delta)
								delta = tmp2;
						} // if mask not null
					}
				}
			}
		} // if resource is INTERNAL_CONFLICT
	} // end: foreach resource
	return (delta);
}

// --------------------------------------------------------------------------------------------------

void ParExeGraph::analyzeContentions() {

    for(ParExePipeline::StageIterator st(_microprocessor->pipeline()); st; st++){
		if (st->orderPolicy() == ParExeStage::OUT_OF_ORDER) {
			for (int i=0 ; i<st->numFus() ; i++) {
				ParExeStage* stage = st->fu(i)->firstStage();
				for (int j=0 ; j<stage->numNodes() ; j++) {
					ParExeNode *node = stage->node(j);
					bool stop = false;
					int num_possible_contenders = 0;
					if (node->latency() > 1)
						num_possible_contenders = 1; // possible late contender
					int num_early_contenders = 0;
	  
					int index = 0;
					int size = stage->numNodes();
					node->initContenders(size);		// TODO for several call to apply, possible memory leak
					stop = false;
					for (int k=0 ; k<stage->numNodes() ; k++) {
						ParExeNode *cont = stage->node(k);
						if (cont->inst()->index() >= node->inst()->index())
							stop = true;
						else {
							if (cont->inst()->index() >= node->inst()->index() - _capacity ) {
								// if cont finishes surely before node, it is not contemp
								// if cont is ready after node, it is not contemp	
								bool finishes_before = true;
								bool ready_after = true;
								for (int r=0 ; r<numResources() ; r++) {
									Resource *res = resource(r);
									if ((res->type() != Resource::INTERNAL_CONFLICT) 
										&& 
										((res->type() != Resource::EXTERNAL_CONFLICT))) {
										if (cont->e(r)) {
											if (!node->e(r)) {
												finishes_before = false;
											}
											else {
												// 						int contention_delay = 
												// 						    ((cont->lateContenders() + cont->possibleContenders()->countBits()) / stage->width()) 
												// 						    * node->latency();
												if (1 /*node->d(r) < cont->d(r) + cont->latency() + cont_contention_delay*/)
													finishes_before = false;
											}
										}
										if (node->e(r)) {
											if (!cont->e(r))
												ready_after = false;
											else {
												int node_contention_delay = (num_possible_contenders / stage->width()) * node->latency();
												if (cont->d(r) <= node->d(r) + node_contention_delay)
													ready_after = false;
											}
										}
									}
									if (!finishes_before && !ready_after){
										num_possible_contenders++; 
										if (_last_prologue_node && (cont->inst()->index() <= _last_prologue_node->inst()->index())) {
											num_early_contenders++;
											node->setContender(index);    
										}
										break;
									}
								}
							}
						}
						index++;
					} // end: foreach possible contender
					node->setLateContenders(num_possible_contenders - num_early_contenders);
					node->buildContendersMasks();
				} // end: foreach node of the stage
			} //end: foreach functional unit
		}
    } // end: foreach stage
}

// -- initDelays ------------------------------------------------------------------------------------------------

void ParExeGraph::initDelays() {
    int index = 0;
    for (elm::genstruct::Vector<Resource *>::Iterator res(_resources) ; res ; res++) {
		switch ( res->type() ) {
		case Resource::BLOCK_START: {
			ParExeNode * node = _first_node;
			node->setE(index,true);
		}
			break;
		case Resource::STAGE: {
			ParExeStage * stage = ((StageResource *) *res)->stage();
			int slot = ((StageResource *) *res)->slot();
			ParExeNode * node = stage->node(slot);
			if (node) {
				node->setE(index,true);
			}
		}
			break;
		case Resource::QUEUE : {
			ParExeQueue *queue = ((QueueResource *) *res)->queue();
			ParExeStage * stage = queue->fillingStage();
			int slot = ((QueueResource *) *res)->slot();
			ParExeNode * node = stage->node(slot);
			if (node) {
				node->setE(index,true);
			}
		}
			break;
		case Resource::REG : {
			for (RegResource::UsingInstIterator inst( (RegResource *) *res ) ; inst ; inst++) {
				for (InstNodeIterator node(inst) ; node ; node++) {
					if (node->stage()->category() == ParExeStage::EXECUTE) {
						node->setE(index,true);
					}
				}
			}
		}
			break;
		case Resource::EXTERNAL_CONFLICT:{
			ParExeInst * inst = ((ExternalConflictResource *) *res)->instruction();
			for (InstNodeIterator node(inst) ; node ; node++) {
				if ( (node->stage()->category() == ParExeStage::EXECUTE) 
					 && 
					 (node->stage()->orderPolicy() == ParExeStage::OUT_OF_ORDER) ) {
					node->setE(index,true);
					node->setContentionDep(inst->index());
				}
			}
		}
			break;	
		case Resource::INTERNAL_CONFLICT: {
			ParExeInst * inst = ((InternalConflictResource *) *res)->instruction();
			for (InstNodeIterator node(inst) ; node ; node++) {
				if ( (node->stage()->category() == ParExeStage::EXECUTE)
					 && 
					 (node->stage()->orderPolicy() == ParExeStage::OUT_OF_ORDER) ) {
					node->setE(index,true);
					((InternalConflictResource *) *res)->setNode(node);
				}
			}
		}
			break;
		case Resource::RES_TYPE_NUM:
			break;
		}
		index++;
    }
}

// -- clearDelays ------------------------------------------------------------------------------------------------

void ParExeGraph::clearDelays() {
    for (PreorderIterator node(this); node; node++) {
		for (elm::genstruct::Vector<Resource *>::Iterator resource(_resources) ; resource ; resource++) {
			int index = resource->index();
			node->setE(index, false);
			node->setD(index, 0);
		}
    }
}

// -- restoreDefaultLatencies ------------------------------------------------------------------------------------------------

void ParExeGraph::restoreDefaultLatencies(){
    for (PreorderIterator node(this); node; node++) {
		node->restoreDefaultLatency();
    }
}

// -- setDefaultLatencies ------------------------------------------------------------------------------------------------

void ParExeGraph::setDefaultLatencies(TimingContext *tctxt){
    for (TimingContext::NodeLatencyIterator nl(*tctxt) ; nl ; nl++){
		nl->node()->setDefaultLatency(nl->latency());
    }
}

// -- setLatencies ------------------------------------------------------------------------------------------------

void ParExeGraph::setLatencies(TimingContext *tctxt){
    for (TimingContext::NodeLatencyIterator nl(*tctxt) ; nl ; nl++){
		nl->node()->setLatency(nl->latency());
    }
}
// --------------------------------------------
void ParExeNode::buildContendersMasks(){
    if (_possible_contenders->countBits() == 0) {
		elm::BitVector *mask = new elm::BitVector(_possible_contenders->size());
		_contenders_masks_list.addLast(mask);
    }
    for (elm::BitVector::OneIterator one(*_possible_contenders) ; one ; one++) {
		if (_contenders_masks_list.isEmpty()) {
			elm::BitVector *mask = new elm::BitVector(_possible_contenders->size());
			ASSERT(mask->size() == _possible_contenders->size());
			_contenders_masks_list.addLast(mask);
			mask = new elm::BitVector(_possible_contenders->size());
			ASSERT(mask->size() == _possible_contenders->size());
			mask->set(one.item());
			_contenders_masks_list.addLast(mask);
		}
		else {
			elm::genstruct::DLList<elm::BitVector *> new_masks;
			for (elm::genstruct::DLList<elm::BitVector *>::Iterator mask(_contenders_masks_list) ;
				 mask ; mask++) {
				ASSERT(mask->size() == _possible_contenders->size());
				elm::BitVector *new_mask = new elm::BitVector(**mask);
				ASSERT(new_mask->size() == _possible_contenders->size());
				new_mask->set(one.item());
				new_masks.addLast(new_mask);
			}
			for (elm::genstruct::DLList<elm::BitVector *>::Iterator new_mask(new_masks) ;
				 new_mask ; new_mask++)
				_contenders_masks_list.addLast(new_mask);
		}
    }
}



// ----------------------------------------------------------------

void ParExeGraph::createResources(){
  
    int resource_index = 0;
    bool is_ooo_proc = false;
  
    // build the start resource
    StartResource * new_resource = new StartResource((elm::String) "start", resource_index++);
    _resources.add(new_resource);
	
    // build resource for stages and FUs
    for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
		if (stage->category() != ParExeStage::EXECUTE) {     
			for (int i=0 ; i<stage->width() ; i++) {
				StringBuffer buffer;
				buffer << stage->name() << "[" << i << "]";
				StageResource * new_resource = new StageResource(buffer.toString(), stage, i, resource_index++);
				_resources.add(new_resource);
			}
		}
		else { // EXECUTE stage
			if (stage->orderPolicy() == ParExeStage::IN_ORDER) {
				for (int i=0 ; i<stage->numFus() ; i++) {
					ParExePipeline * fu = stage->fu(i);
					ParExeStage *fu_stage = fu->firstStage();
					for (int j=0 ; j<fu_stage->width() ; j++) {
						StringBuffer buffer;
						buffer << fu_stage->name() << "[" << j << "]";
						StageResource * new_resource = new StageResource(buffer.toString(), fu_stage, j, resource_index++);
						_resources.add(new_resource);
					}
				}
			}
			else
				is_ooo_proc = true;
		}
    }

    // build resources for queues
    for (ParExeProc::QueueIterator queue(_microprocessor) ; queue ; queue++) {
		int num = queue->size();
		if (num > _sequence->count())
			num = _sequence->count();
		for (int i=0 ; i<num ; i++) {
			StringBuffer buffer;
			buffer << queue->name() << "[" << i << "]";

			// find emptying stage
			/*int _i = 0, _empty_i;
			for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
				if (stage == queue->emptyingStage())
					_empty_i = _i;
				_i++;
			}*/

			//
			StageResource * upper_bound;
			//int upper_bound_offset;
			for (elm::genstruct::Vector<Resource *>::Iterator resource(_resources) ; resource ; resource++) {
				if (resource->type() == Resource::STAGE) {
					if (((StageResource *)(*resource))->stage() == queue->emptyingStage()) {
						if (i < queue->size() - ((StageResource *)(*resource))->stage()->width() - 1) {
							if (((StageResource *)(*resource))->slot() == ((StageResource *)(*resource))->stage()->width()-1) {
								upper_bound = (StageResource *) (*resource);
								//upper_bound_offset = (queue->size() - i) / ((StageResource *)(*resource))->stage()->width();
							}
						}
						else {
							if (((StageResource *)(*resource))->slot() == i - queue->size() + ((StageResource *)(*resource))->stage()->width()) {
								upper_bound = (StageResource *) (*resource);
								//upper_bound_offset = 0;
							}
						}
					}
				}
			}
			ASSERT(upper_bound);

			// build the queue resource
			QueueResource * new_resource = new QueueResource(buffer.toString(), queue, i, resource_index++, upper_bound);
			_resources.add(new_resource);
		}
    }

    // get the list of registers
    otawa::hard::Platform *pf = _ws->platform();
    AllocatedTable<Resource::input_t> inputs(pf->banks().count());
    int reg_bank_count = pf->banks().count();
    for(int i = 0; i <reg_bank_count ; i++) {
		inputs[i].reg_bank = (otawa::hard::RegBank *) pf->banks()[i];
		inputs[i]._is_input = 
			new AllocatedTable<bool>(inputs[i].reg_bank->count());
		inputs[i]._resource_index =
			new AllocatedTable<int>(inputs[i].reg_bank->count());
		for (int j=0 ; j<inputs[i].reg_bank->count() ; j++) {
			inputs[i]._is_input->set(j,true);
			inputs[i]._resource_index->set(j,-1);
		}
    }

    // build the resource for the used registers
    for (InstIterator inst(_sequence) ; inst ; inst++) {
		const elm::genstruct::Table<hard::Register *>& reads = inst->inst()->readRegs();
	
		for(int i = 0; i < reads.count(); i++) {
			for (int b=0 ; b<reg_bank_count ; b++) {
				if (inputs[b].reg_bank == reads[i]->bank()) {
					if (inputs[b]._is_input->get(reads[i]->number()) == true) {
						if (inputs[b]._resource_index->get(reads[i]->number()) == -1) {
							//new input coming from outside the sequence
							StringBuffer buffer;
							buffer << reads[i]->bank()->name() << reads[i]->number();
							RegResource * new_resource = new RegResource(buffer.toString(), reads[i]->bank(), reads[i]->number(), resource_index++);
							_resources.add(new_resource);
							new_resource->addUsingInst(inst);
							inputs[b]._resource_index->set(reads[i]->number(), _resources.length()-1);
						}
						else {
							((RegResource *)_resources[inputs[b]._resource_index->get(reads[i]->number())])->addUsingInst(inst);
						}
					}
				}
			}
		}
		const elm::genstruct::Table<hard::Register *>& writes = inst->inst()->writtenRegs();
		for(int i = 0; i < writes.count(); i++) {
			for (int b=0 ; b<reg_bank_count ; b++) {
				if (inputs[b].reg_bank == writes[i]->bank()) {
					inputs[b]._is_input->set(writes[i]->number(), false);
				}
			}
		}	
    }
  
    // build the resources for out-of-order execution
    if (is_ooo_proc) {
		int i = 0;
		for (InstIterator inst(_sequence) ; inst ; inst++) {
			StringBuffer buffer;
			buffer << "extconf[" << i << "]";
			ExternalConflictResource * new_resource = new ExternalConflictResource(buffer.toString(), inst, resource_index++);	
			_resources.add(new_resource);
			StringBuffer another_buffer;
			another_buffer << "intconf[" << i << "]";
			InternalConflictResource * another_new_resource = new InternalConflictResource(another_buffer.toString(), inst, resource_index++);	
			_resources.add(another_new_resource);
			i++;
		}
    }

    // clean up
    for(int i = 0; i <reg_bank_count ; i++) {
		delete inputs[i]._is_input;
		delete inputs[i]._resource_index;
    }
}


// ----------------------------------------------------------------

void ParExeGraph::build(bool compressed_cod) {

    createResources();

    createNodes();
    findDataDependencies();

    addEdgesForPipelineOrder();
    addEdgesForFetch();
    addEdgesForProgramOrder();

    addEdgesForMemoryOrder();
    addEdgesForDataDependencies();
  
    addEdgesForQueues();
    findContendingNodes();
  
}

// ----------------------------------------------------------------

ParExePipeline *ParExeGraph::pipeline(ParExeStage *stage, ParExeInst *inst) {
	return stage->findFU(inst->inst()->kind());;
}

void ParExeGraph::createNodes() {

    // consider every instruction
    for (InstIterator inst(_sequence) ; inst ; inst++)  {
		// consider every pipeline stage
		for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
	    
			// create node
			ParExeNode *node;
			if (stage->category() != ParExeStage::EXECUTE) {
				node = new ParExeNode(this, stage, inst);
				inst->addNode(node);
				stage->addNode(node);
				if (stage->category() == ParExeStage::FETCH) {
					inst->setFetchNode(node);
				}
				if (!_first_node)
					_first_node = node;
				if (inst->codePart() == PROLOGUE)
					_last_prologue_node = node;
				if (!_first_bb_node && (inst->codePart() == BODY) )
					_first_bb_node = node;
				_last_node = node;
			}
			else {
				// add FU nodes
				ParExePipeline *fu = pipeline(stage, inst);
				if(!fu)
					throw ParExeException(elm::_ << "cannot find FU for instruction " << inst->inst()->address() << " " << inst->inst());
				int index = 0;

				for(ParExePipeline::StageIterator fu_stage(fu); fu_stage; fu_stage++) {                         
					ParExeNode *fu_node = new ParExeNode(this, fu_stage, inst);
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

/**
 * Compute for each first FU node which is the FU node producing
 * the required data (and fill the producer list of a FU node).
 */
void ParExeGraph::findDataDependencies() {

	// allocate the rename table
    otawa::hard::Platform *pf = _ws->platform();
    AllocatedTable<rename_table_t> rename_tables(pf->banks().count());
    int reg_bank_count = pf->banks().count();
    for(int i = 0; i <reg_bank_count ; i++) {
		rename_tables[i].reg_bank = (otawa::hard::RegBank *) pf->banks()[i];
		rename_tables[i].table =
			new AllocatedTable<ParExeNode *>(rename_tables[i].reg_bank->count());
		for (int j=0 ; j<rename_tables[i].reg_bank->count() ; j++)
			rename_tables[i].table->set(j,NULL);
    }

    // consider every instruction
    for (InstIterator inst(_sequence) ; inst ; inst++)  {

    	// find first and last FU nodes
		ParExeNode *first_fu_node = NULL, *last_fu_node = NULL;
		for (InstNodeIterator node(inst); node ; node++){
			if (node->stage()->category() == ParExeStage::FU){
				if (!first_fu_node)
					first_fu_node = node;
				last_fu_node = node;
			}
		}

		// check for data dependencies
		const elm::genstruct::Table<hard::Register *>& reads = first_fu_node->inst()->inst()->readRegs();
		for(int i = 0; i < reads.count(); i++) {
			for (int b=0 ; b<reg_bank_count ; b++) {
				if (rename_tables[b].reg_bank == reads[i]->bank()) {
					ParExeNode *producer = rename_tables[b].table->get(reads[i]->number());
					if (producer != NULL) {
						first_fu_node->addProducer(producer);
					}
				}
			}
		}

		// fu_node is the last FU node
		const elm::genstruct::Table<hard::Register *>& writes = last_fu_node->inst()->inst()->writtenRegs();
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


/**
 * Called to add edges representing the order of stages traversed by an instruction.
 */
void ParExeGraph::addEdgesForPipelineOrder(void) {
    for (InstIterator inst(_sequence) ; inst ; inst++)
		for (int i=0 ; i<inst->numNodes()-1 ; i++)
			new ParExeEdge(inst->node(i), inst->node(i+1), ParExeEdge::SOLID);
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
void ParExeGraph::addEdgesForFetch(void) {
	static string cache_trans_msg = "cache", cache_inter_msg = "line", branch_msg = "branch";
    ParExeStage *fetch_stage = _microprocessor->fetchStage();

    // traverse all fetch nodes
    ParExeNode * first_cache_line_node = fetch_stage->firstNode();
    address_t current_cache_line = fetch_stage->firstNode()->inst()->inst()->address().offset() /  _cache_line_size;
    for(int i=0 ; i<fetch_stage->numNodes()-1 ; i++) {
		ParExeNode *node = fetch_stage->node(i);
		ParExeNode *next = fetch_stage->node(i+1);

		// taken banch ?
		if (node->inst()->inst()->topAddress() != next->inst()->inst()->address()){
			// fixed by casse: topAddress() is address() + size()
			ParExeEdge * edge = new ParExeEdge(node, next, ParExeEdge::SOLID, 0, branch_msg);
			edge->setLatency(_branch_penalty); // taken branch penalty when no branch prediction is enabled
			edge = new ParExeEdge(first_cache_line_node, next, ParExeEdge::SOLID, cache_inter_msg);
			edge->setLatency(_branch_penalty);
		}
		else
			new ParExeEdge(node, next, ParExeEdge::SLASHED);

		// new cache line?
		//if (cache)         FIXME !!!!!!!!!!!!!!!
		address_t cache_line = next->inst()->inst()->address().offset() /  _cache_line_size;
		if ( cache_line != current_cache_line){
			new ParExeEdge(first_cache_line_node, next, ParExeEdge::SOLID, 0, cache_trans_msg);
			if(first_cache_line_node != node)
				new ParExeEdge(node, next, ParExeEdge::SOLID, 0, cache_inter_msg);
			first_cache_line_node = next;
			current_cache_line = cache_line;
		}
		//    }	
    }
}


/**
 * This function add edges for the fetch stage. This function is only valid
 * if there is not instruction cache. Basically, it adds slashed edges between each
 * instruction except if an instruction in the sequence is the target of the branch
 * of the previous instruction. In this case, it adds a solid edge.
 */
void ParExeGraph::addEdgesForFetchWithDecomp(void) {


	// add edges for program order
	elm::genstruct::SLList<ParExeStage *> list;
	list.add(_microprocessor->fetchStage());
	addEdgesForProgramOrder(&list);

	// do not forget the branch solid edge
	ParExeNode *branch = 0;
	for(ParExeStage::NodeIterator node(_microprocessor->fetchStage()); node; node++) {

		// previously a branch
		if(branch && branch->inst()->inst()->topAddress() != node->inst()->inst()->address())
			new ParExeEdge(branch, node, ParExeEdge::SOLID, _branch_penalty);

		// is it a branch?
		if(node->inst()->inst()->isControl())
			branch = node;
		else
			branch = 0;
	}
}


/**
 * This function is called to add edges between the nodes to order the stage execution
 * of instruction according to the program order.
 * @param list_of_stages	List of stages to build the edges for
 * 							(if an empty pointer is passed, one is created containing the in-order stages).
 */
void ParExeGraph::addEdgesForProgramOrder(elm::genstruct::SLList<ParExeStage *> *list_of_stages){

    elm::genstruct::SLList<ParExeStage *> *list;
    if (list_of_stages != NULL)
		list = list_of_stages;
    else {
		// if no list of stages was provided, built the default list that includes all IN_ORDER stages
		list = new  elm::genstruct::SLList<ParExeStage *>;
		for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
			if (stage->orderPolicy() == ParExeStage::IN_ORDER){
				if (stage->category() != ParExeStage::FETCH){
					list->add(stage);
				}
				if (stage->category() == ParExeStage::EXECUTE){
					for (int i=0 ; i<stage->numFus() ; i++){
						ParExeStage *fu_stage = stage->fu(i)->firstStage();
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
			ParExeNode *node = stage->node(i);
			ParExeNode *next = stage->node(i+1);
			if (stage->width() == 1){
				new ParExeEdge(node, next, ParExeEdge::SOLID, 0, stage->name());
			}
			else {
				new ParExeEdge(node, next, ParExeEdge::SLASHED, 0, stage->name());
				if (count == stage->width()){
					ParExeNode *previous = stage->node(prev);
					new ParExeEdge(previous,next,ParExeEdge::SOLID, 0, stage->name());
					prev++;
				}
				else 
					count++;
			}
		}
    }
}


/**
 * Called to add edges to represent contention to access memory, basically, between FUs
 * of instructions performing memory access.
 */
void ParExeGraph::addEdgesForMemoryOrder(void) {
	static string msg = "memory order";

    ParExeStage *stage = _microprocessor->execStage();

    // looking in turn each FU
    for (int i=0 ; i<stage->numFus() ; i++) {
		ParExeStage *fu_stage = stage->fu(i)->firstStage();
		ParExeNode * previous_load = NULL;
		ParExeNode * previous_store = NULL;

		// look for each node of this FU
		for (int j=0 ; j<fu_stage->numNodes() ; j++){
			ParExeNode *node = fu_stage->node(j);

			// found a load instruction
			if (node->inst()->inst()->isLoad()) {

				// if any, dependency on previous store
				if(previous_store)
					new ParExeEdge(previous_store, node, ParExeEdge::SOLID, 0, msg);

				// current node becomes the new previous load
				for (InstNodeIterator last_node(node->inst()); last_node ; last_node++)
					if (last_node->stage()->category() == ParExeStage::FU)
						previous_load = last_node;
			}

			// found a store instruction
			if (node->inst()->inst()->isStore()) {

				// if any, dependency on previous store
				if (previous_store)
					new ParExeEdge(previous_store, node, ParExeEdge::SOLID, 0, msg);

				// if any, dependency on previous load
				if (previous_load)
					new ParExeEdge(previous_load, node, ParExeEdge::SOLID, 0, msg);

				// current node becomes the new previous store
				for (InstNodeIterator last_node(node->inst()); last_node ; last_node++)
					if (last_node->stage()->category() == ParExeStage::FU)
						previous_store = last_node;
			}
		}
    }
}

// ----------------------------------------------------------------

/**
 * Called to add edges for data dependencies, that is, if an instruction (a)
 * produces content of a register and instruction (b) uses this register value
 * create a solid edge between their execute stages.
 */
void ParExeGraph::addEdgesForDataDependencies(void){
    ParExeStage *exec_stage = _microprocessor->execStage();
    for (int j=0 ; j<exec_stage->numFus() ; j++) {
		ParExeStage *fu_stage = exec_stage->fu(j)->firstStage();
		for (int k=0 ; k<fu_stage->numNodes() ; k++) {
			ParExeNode *node = fu_stage->node(k);
			for (int p=0 ; p<node->numProducers(); p++) {
				ParExeNode *producer = node->producer(p);
				new ParExeEdge(producer, node, ParExeEdge::SOLID);
			}
		}
    }
}


/**
 * Called to add edges representing contention on the different
 * queues of the microprocessor.
 */
void ParExeGraph::addEdgesForQueues(void){

    // build edges for queues with limited capacity */
    for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
		ParExeStage * prod_stage;
		if (stage->sourceQueue() != NULL) {
			ParExeQueue *queue = stage->sourceQueue();
			int size = queue->size();
			prod_stage = queue->fillingStage();
			for (int i=0 ; i<stage->numNodes() - size ; i++) {
				ASSERT(i+size < prod_stage->numNodes());
				new ParExeEdge(stage->node(i), prod_stage->node(i + size), ParExeEdge::SLASHED, 0, queue->name());
			}
		}
    }
}

// ----------------------------------------------------------------

void ParExeGraph::findContendingNodes(){

    // search for contending nodes (i.e. pairs of nodes that use the same pipeline stage)
    for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
		if (stage->orderPolicy() == ParExeStage::OUT_OF_ORDER) {	
			if (stage->category() != ParExeStage::EXECUTE)  {
				for (int i=0 ; i<stage->numNodes() ; i++) 
					for (int j=0 ; j<stage->numNodes() ; j++) 
						if (i != j) 
							stage->node(i)->addContender(stage->node(j));
			}
			else {
				for (int k=0 ; k<stage->numFus() ; k++) {
					ParExeStage *fu_stage = stage->fu(k)->firstStage();
					for (int i=0 ; i<fu_stage->numNodes() ; i++) 
						for (int j=0 ; j<fu_stage->numNodes() ; j++) 
							if (i != j) 
								fu_stage->node(i)->addContender(fu_stage->node(j));
				}
			}
		}	
    }
}

// ----------------------------------------------------------------

ParExeGraph::~ParExeGraph() {
    for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
		stage->deleteNodes();
		if (stage->category() == ParExeStage::EXECUTE) {
			for (int i=0 ; i<stage->numFus() ; i++) {
				ParExePipeline *fu = stage->fu(i);
				for (ParExePipeline::StageIterator fu_stage(fu); fu_stage; fu_stage++)
					fu_stage->deleteNodes();
			}
		}
    }
    for (ParExeSequence::InstIterator inst(_sequence) ; inst ; inst++) {
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


static void escape(io::Output& out, const string& str) {
	static bool init = false;
	static cstring escaped = "{}|";
	static bool escape_tab[256];

	// initialization
	if(!init) {
		init = true;
		for(int i = 0; i < 256; i++)
			escape_tab[i] = false;
		for(int i = 0; i < escaped.length(); i++)
			escape_tab[escaped[i]] = true;
	}

	// escape the string
	for(int i = 0; i < str.length(); i++) {
		if(escape_tab[str[i]])
			out << '\\';
		out << str[i];
	}
}


/**
 * Escape special characters in the given string.
 * @param s		String to escape characters for.
 * @return		Escaped string.
 */
string escape(const string& s) {
	StringBuffer buf;
	for(int i = 0; i < s.length(); i++)
		switch(s[i]) {
		case '<':
		case '>':
		case '\\':
		case '{':
		case '}':	buf << '\\';
		/* no break */
		default:	buf << s[i]; break;
		}
	return buf.toString();
}


// ---------------------------------------
void ParExeGraph::dump(elm::io::Output& dotFile, const string& info) {
    int i=0;
    bool first_line = true;
    int width = 5;
    dotFile << "digraph G {\n";

    // dipsplay information if any
    if(info)
    	dotFile << "\"info\" [shape=record, label=\"{" << escape(info) << "}\"];\n";

    // display ressources
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
    dotFile << "\"] ; \n";
  
    // display instruction sequence
    dotFile << "\"code\" [shape=record, label= \"\\l";
    bool body = true;
    BasicBlock *bb = 0;
    for (InstIterator inst(_sequence) ; inst ; inst++) {
		if(inst->codePart() == BODY && body) {
			body = false;
			dotFile << "------\\l";
		}
    	BasicBlock *cbb = inst->basicBlock();
    	if(cbb != bb) {
    		bb = cbb;
    		dotFile << bb << "\\l";
    	}
    	dotFile << "I" << inst->index() << ": ";
		dotFile << "0x" << ot::address(inst->inst()->address()) << ":  ";
		escape(dotFile, elm::_ << inst->inst());
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
			if (node->inst()->codePart() == BODY)
				dotFile << "color=blue, ";
			dotFile << "label=\"" << node->stage()->name();
			dotFile << "(I" << node->inst()->index() << ") [" << node->latency() << "]\\l";
			escape(dotFile, elm::_ << inst->inst());
			dotFile << "| { ";
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
					 (node->stage()->category() != ParExeStage::EXECUTE) 
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
					if(next.edge()->latency() || next.edge()->name()) {
						dumpAttr(dotFile, first);
						dotFile << "label=\"";
						if(next.edge()->name())
							dotFile << escape(next.edge()->name());
						if(next.edge()->latency()) {
							if(next.edge()->name())
								dotFile << " (";
							dotFile << next.edge()->latency();
							if(next.edge()->name())
								dotFile << ')';
						}
						dotFile << "\"";
					}

					// edge style
					switch( next.edge()->type()) {
					case ParExeEdge::SOLID:
						if (node->inst()->index() == next->inst()->index()) {
							dumpAttr(dotFile, first);
							dotFile << "minlen=4";
						}
						break;
					case ParExeEdge::SLASHED:
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
ParExeGraph::ParExeGraph(
	WorkSpace *ws,
	ParExeProc *proc,
	ParExeSequence *seq,
	const PropList& props
)
:	_ws(ws),
 	_microprocessor(proc),
 	_sequence(seq),
 	_first_node(NULL),
 	_first_bb_node(NULL),
 	_last_prologue_node(NULL),
 	_last_node(NULL),
 	_branch_penalty(2),
 	_capacity(0)
{
	const hard::CacheConfiguration *cache = hard::CACHE_CONFIGURATION(ws);
	if (cache && cache->instCache()) {
		_cache_line_size = cache->instCache()->blockSize();
	}
	else {
		_cache_line_size = ws->process()->instSize();
		if(!_cache_line_size)
			_cache_line_size = 1;
	}
	_props = props;
}


/**
 * @fn void ParExeGraph::setFetchSize(int size);
 * Set the size in bytes used to fetch instructions.
 * @param size	Size in bytes of fetched blocks.
 */


// ----------------------------------------------------------------

void ParExeGraph::display(elm::io::Output&) {
}

/**
 * @class ParExeEdge
 * An edge in a @ref ParExeGraph.
 * @ingroup peg
 * @see peg
 */


/**
 * @class ParExeException
 * Exception thrown if there is an error during the build and the computation of @par ExeGraph.
 * @ingroup peg
 * @see peg
 */


/**
 * @class ParExeInst
 * Instruction as presented in the @ref ParExeGraph.
 * @ingroup peg
 * @see peg
 */

/**
 * @class ParExeNode
 * A node of the @ref ParExeGraph, that represents the crossing
 * of an instruction inside a microprocessor stage.
 * @ingroup peg
 * @see peg
 */

/**
 * @class ParExeSequence
 * A sequence of @ref ParExeInstruction for which a @ref ParExeGraph will be built.
 * @ingroup peg
 * @see peg
 */

}	// otawa


