
#include <otawa/parexegraph/ParExeGraph.h>

using namespace  otawa;

// --------------------------------------------------------------------------------------------------

int ParExeGraph::analyze() {

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
		return(0);

    ParExeNode *lp = _last_prologue_node;
  
    int default_lp = lp->d(numResources()-1);
    if (res->type() == Resource::STAGE)
		default_lp += _microprocessor->pipeline()->numStages() - ((StageResource *)(res))->stage()->index();
    if (res->type() == Resource::QUEUE) {
		StageResource * upper_bound = ((QueueResource *)(res))->upperBound();
		int u = upper_bound->index();
		if (lp->e(u)){
			int tmp = lp->d(u) + (_microprocessor->pipeline()->numStages() - upper_bound->stage()->index());
			if (tmp < default_lp)
				default_lp = tmp;
		}
    }

    int delta;
    if (lp->e(r))
		delta = a->d(r) - lp->d(r);
    else {
		delta = a->d(r) - default_lp; 
    }

  
    for (elm::genstruct::Vector<Resource *>::Iterator resource(_resources) ; resource ; resource++) {
		if (resource->type() == Resource::INTERNAL_CONFLICT) {
			int s = resource->index();
			ParExeNode * S = ((InternalConflictResource *) *resource)->node();
			if (a->e(s) && S->e(r)) {
				if (lp->e(s)){ 
					int tmp = a->d(s) - lp->d(s);
					if (tmp > delta) {
						delta = tmp;
					}
				} // end: is lp depends on S

				else { //lp does not depend on S
					for (elm::genstruct::SLList<elm::BitVector *>::Iterator mask(*(S->contendersMasksList())) ; mask ; mask++) {
						int tmp = a->d(s);
						tmp += (((mask->countBits()+S->lateContenders())/S->stage()->width())*S->latency());
						int tmp2 = 0;
						int tmp3;
	  
						if (mask->countBits() == 0) { // mask is null == no early contenders
							if (!lp->e(r))
								tmp3 = tmp + S->d(r) - /*lp->d(_latest_resource_index) - res->offset()*/default_lp;
							else
								tmp3 = tmp + S->d(r) - lp->d(r);
							if (tmp3 > delta) {
								delta = tmp3;
							}
						}
						else { // mask is not null
							StringBuffer buffer;
							for (elm::BitVector::OneIterator one(**mask) ; one ; one++) {
								ParExeNode *C = S->stage()->node(one.item());
								buffer << C->name() << "-";
								int c = -1;
								for (elm::genstruct::Vector<Resource *>::Iterator ic(_resources) ; ic ; ic++) {
									if (ic->type() == Resource::INTERNAL_CONFLICT) {
										if (((InternalConflictResource *) *ic)->node() == C) {
											c = ((InternalConflictResource *) *ic)->index();
										}
									}
								}
								assert(c != -1);
								assert(lp->e(c));
								if (lp->d(c) > tmp2)
									tmp2 = lp->d(c);
	      
							}// end: foreach one in mask
							if (lp->e(r)) {
								if (lp->d(r) - S->d(r) > tmp2) {
									tmp2 = lp->d(r) - S->d(r);
								}
							}
							else {
								int tmp4 = lp->d(numResources()-1) - S->d(r);
								if (res->type() == Resource::STAGE)
									tmp4 += _microprocessor->pipeline()->numStages() - ((StageResource *)(res))->stage()->index();
								if (tmp4 > tmp2) {		    
									tmp2 = /*lp->d(_latest_resource_index) + res->offset()*/default_lp - S->d(r);
								}
							}
							tmp2 = tmp - tmp2;
							if (tmp2 > delta) {
								delta = tmp2;
							}
						} // if mask not null
					}
				}
			}
		} // if resource is INTERNAL_CONFLICT
    } // end: foreach resource
    return(delta);
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
					node->initContenders(size);
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
			assert(mask->size() == _possible_contenders->size());
			_contenders_masks_list.addLast(mask);
			mask = new elm::BitVector(_possible_contenders->size());
			assert(mask->size() == _possible_contenders->size());
			mask->set(one.item());
			_contenders_masks_list.addLast(mask);
		}
		else {
			elm::genstruct::SLList<elm::BitVector *> new_masks;
			for (elm::genstruct::SLList<elm::BitVector *>::Iterator mask(_contenders_masks_list) ; 
				 mask ; mask++) {
				assert(mask->size() == _possible_contenders->size());
				elm::BitVector *new_mask = new elm::BitVector(**mask);
				assert(new_mask->size() == _possible_contenders->size());
				new_mask->set(one.item());
				new_masks.addLast(new_mask);
			}
			for (elm::genstruct::SLList<elm::BitVector *>::Iterator new_mask(new_masks) ; 
				 new_mask ; new_mask++)
				_contenders_masks_list.addLast(new_mask);
		}
    }
}



// ----------------------------------------------------------------

void ParExeGraph::createResources(){
  
    int resource_index = 0;
    bool is_ooo_proc = false;
  
    StartResource * new_resource = new StartResource((elm::String) "start", resource_index++);
    _resources.add(new_resource);
	
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

    for (ParExeProc::QueueIterator queue(_microprocessor) ; queue ; queue++) {
		int num = queue->size();
		if (num > _sequence->count())
			num = _sequence->count();
		for (int i=0 ; i<num ; i++) {
			StringBuffer buffer;
			buffer << queue->name() << "[" << i << "]";
			int _i = 0, _empty_i;
			for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
				if (stage == queue->emptyingStage())
					_empty_i = _i;
				_i++;
			}
			StageResource * upper_bound;
			int upper_bound_offset;
			for (elm::genstruct::Vector<Resource *>::Iterator resource(_resources) ; resource ; resource++) {
				if (resource->type() == Resource::STAGE) {
					if (((StageResource *)(*resource))->stage() == queue->emptyingStage()) {
						if (i < queue->size() - ((StageResource *)(*resource))->stage()->width() - 1) {
							if (((StageResource *)(*resource))->slot() == ((StageResource *)(*resource))->stage()->width()-1) {
								upper_bound = (StageResource *) (*resource);
								upper_bound_offset = (queue->size() - i) / ((StageResource *)(*resource))->stage()->width();
							}
						}
						else {
							if (((StageResource *)(*resource))->slot() == i - queue->size() + ((StageResource *)(*resource))->stage()->width()) {
								upper_bound = (StageResource *) (*resource);
								upper_bound_offset = 0;
							}
						}
					}
				}
			}
			assert(upper_bound);
			QueueResource * new_resource = new QueueResource(buffer.toString(), queue, i, resource_index++, upper_bound);
			_resources.add(new_resource);
		}
    }
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
				ParExePipeline *fu = stage->findFU(inst->inst()->kind()); 
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

void ParExeGraph::findDataDependencies() {

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

// ----------------------------------------------------------------
void ParExeGraph::addEdgesForPipelineOrder(){
    for (InstIterator inst(_sequence) ; inst ; inst++)  {
		for (int i=0 ; i<inst->numNodes()-1 ; i++){
			new ParExeEdge(inst->node(i), inst->node(i+1), ParExeEdge::SOLID);
		}
    }
}

// ----------------------------------------------------------------
void ParExeGraph::addEdgesForFetch(){
    ParExeStage *fetch_stage = _microprocessor->fetchStage();
    ParExeNode * first_cache_line_node = fetch_stage->firstNode();
    address_t current_cache_line = fetch_stage->firstNode()->inst()->inst()->address().address() /  _cache_line_size;
    for (int i=0 ; i<fetch_stage->numNodes()-1 ; i++) {
		ParExeNode *node = fetch_stage->node(i);
		ParExeNode *next = fetch_stage->node(i+1);
		// taken banch ?
		if (node->inst()->inst()->address().address() + 4/*instruction size: to be FIXED !!! */ != next->inst()->inst()->address().address()){
			ParExeEdge * edge = new ParExeEdge(node, next, ParExeEdge::SOLID);
			edge->setLatency(2); // taken branch penalty when no branch prediction is enabled
			edge = new ParExeEdge(first_cache_line_node, next, ParExeEdge::SOLID);
			edge->setLatency(2); 
		}
		else {
			new ParExeEdge(node, next, ParExeEdge::SLASHED);
		}
		// new cache line?
		//if (cache)         FIXME !!!!!!!!!!!!!!!
		address_t cache_line = next->inst()->inst()->address().address() /  _cache_line_size;
		if ( cache_line != current_cache_line){
			new ParExeEdge(first_cache_line_node, next, ParExeEdge::SOLID);
			new ParExeEdge(node, next, ParExeEdge::SOLID);
			first_cache_line_node = next;
			current_cache_line = cache_line;
		}
		//    }	
    }
}

// ----------------------------------------------------------------
void ParExeGraph::addEdgesForFetchWithDecomp(){
    ParExeStage *fetch_stage = _microprocessor->fetchStage();
    ParExeNode * first_cache_line_node = fetch_stage->firstNode();
    address_t current_cache_line = (address_t) fetch_stage->firstNode()->inst()->inst()->address() /  _cache_line_size;
    for (int i=0 ; i<fetch_stage->numNodes()-1 ; i++) {
		ParExeNode *node = fetch_stage->node(i);
		ParExeNode *next = fetch_stage->node(i+1);
		// taken banch ?
		address_t addr_node, addr_next;
		addr_node = node->inst()->inst()->address();
		addr_next = next->inst()->inst()->address();
		if ((addr_node != addr_next) && (addr_node + 4/*instruction size: to be FIXED !!! */ != addr_next)){
			ParExeEdge * edge = new ParExeEdge(node, next, ParExeEdge::SOLID);
			edge->setLatency(2); // taken branch penalty when no branch prediction is enabled
			edge = new ParExeEdge(first_cache_line_node, next, ParExeEdge::SOLID);
			edge->setLatency(2); 
		}
		else 
			new ParExeEdge(node, next, ParExeEdge::SLASHED);
		// new cache line?
		//if (cache)         FIXME !!!!!!!!!!!!!!!
		address_t cache_line = addr_next / _cache_line_size;
		if ( cache_line != current_cache_line){
			new ParExeEdge(first_cache_line_node, next, ParExeEdge::SOLID);
			new ParExeEdge(node, next, ParExeEdge::SOLID);
			first_cache_line_node = next;
			current_cache_line = cache_line;
		}
		//  }	
    }
}

// ----------------------------------------------------------------

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
				new ParExeEdge(node, next, ParExeEdge::SOLID);
			}
			else {
				new ParExeEdge(node, next, ParExeEdge::SLASHED);
				if (count == stage->width()){
					ParExeNode *previous = stage->node(prev);
					new ParExeEdge(previous,next,ParExeEdge::SOLID);
					prev++;
				}
				else 
					count++;
			}
		}
    }
}


// ----------------------------------------------------------------

void ParExeGraph::addEdgesForMemoryOrder(){

    ParExeStage *stage = _microprocessor->execStage();
    for (int i=0 ; i<stage->numFus() ; i++) {
		ParExeStage *fu_stage = stage->fu(i)->firstStage();
		ParExeNode * previous_load = NULL;
		ParExeNode * previous_store = NULL;
		for (int j=0 ; j<fu_stage->numNodes() ; j++){
			ParExeNode *node = fu_stage->node(j);
			if (node->inst()->inst()->isLoad()) {
				if (previous_store) {// memory access are executed in order  
					new ParExeEdge(previous_store, node, ParExeEdge::SOLID);
				}
				for (InstNodeIterator last_node(node->inst()); last_node ; last_node++){
					if (last_node->stage()->category() == ParExeStage::FU)
						previous_load = last_node;
				}
			}
			if (node->inst()->inst()->isStore()) {
				if (previous_store) {// memory access are executed in order
					new ParExeEdge(previous_store, node, ParExeEdge::SOLID);
				}
				if (previous_load) {// memory access are executed in order
					new ParExeEdge(previous_load, node, ParExeEdge::SOLID);
				}
				for (InstNodeIterator last_node(node->inst()); last_node ; last_node++){
					if (last_node->stage()->category() == ParExeStage::FU){
						previous_store = last_node;
					}
				}
			}
		}
    }
}

// ----------------------------------------------------------------

void ParExeGraph::addEdgesForDataDependencies(){
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

// ----------------------------------------------------------------

void ParExeGraph::addEdgesForQueues(){

    // build edges for queues with limited capacity */
    for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage ; stage++) {
		ParExeStage * prod_stage;
		if (stage->sourceQueue() != NULL) {
			ParExeQueue *queue = stage->sourceQueue();
			int size = queue->size();
			prod_stage = queue->fillingStage();
			for (int i=0 ; i<stage->numNodes() - size ; i++) {
				assert(i+size < prod_stage->numNodes());
				new ParExeEdge(stage->node(i), prod_stage->node(i + size), ParExeEdge::SLASHED);
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

// ---------------------------------------
void ParExeGraph::dump(elm::io::Output& dotFile) {
	
    dotFile << "digraph G {\n";
    dotFile << "\"legend\" [shape=record, label= \"{ ";
    int i=0;
    bool first_line = true;
    int width = 5;
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
  
    dotFile << "\"code\" [shape=record, label= \"";
    for (InstIterator inst(_sequence) ; inst ; inst++) {
		dotFile << "0x" << fmt::address(inst->inst()->address()) << ":  "; 
		inst->inst()->dump(dotFile);
		dotFile << "\\" << "n" ;
    }
    dotFile << "\"] ; \n";
  
  
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
			dotFile << "(I" << node->inst()->index() << ") [" << node->latency() << "]";
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
  
  
    int group_number = 0;
    for (InstIterator inst(_sequence) ; inst ; inst++) {	
		// dump edges
		for (InstNodeIterator node(inst) ; node ; node++) {
			for (Successor next(node) ; next ; next++) {
				if ( node != inst->firstNode()
					 ||
					 (node->stage()->category() != ParExeStage::EXECUTE) 
					 || (node->inst()->index() == next->inst()->index()) ) {				
					dotFile << "\"" << node->stage()->name();
					dotFile << "I" << node->inst()->index() << "\"";
					dotFile << " -> ";
					dotFile << "\"" << next->stage()->name();
					dotFile << "I" << next->inst()->index() << "\"";
					switch( next.edge()->type()) {
					case ParExeEdge::SOLID:
						if (node->inst()->index() == next->inst()->index())
							dotFile << "[minlen=4]";
						dotFile << " ;\n";
						break;
					case ParExeEdge::SLASHED:
						dotFile << " [style=dotted";
						if (node->inst()->index() == next->inst()->index())
							dotFile << ", minlen=4";
						dotFile << "] ;\n";
						break;	
					default:
						break;
					}	
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

// ----------------------------------------------------------------

void ParExeGraph::display(elm::io::Output&) {
}


