/*
 *	GraphBBTime interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-12 IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_GRAPH_BBTIME_H
#define OTAWA_GRAPH_BBTIME_H

#include <otawa/ipet.h>
#include <otawa/prop/Identifier.h>
#include <otawa/cfg.h>
#include <otawa/cache/LBlockBuilder.h>
#include <otawa/hard/Memory.h>
#include <otawa/parexegraph/ParExeGraph.h>
#include <elm/io/OutFileStream.h>
#include <otawa/ipet.h>
#include <otawa/cache/cat2/CachePenalty.h>

using namespace otawa::cache;

namespace otawa {
    extern Identifier<String> GRAPHS_OUTPUT_DIRECTORY;
    extern Identifier<int> TIME;

    class LBlockManager {
    public:

    	LBlockManager(void): bb(0), lbs(0), i(0) { }

    	category_t next(BasicBlock *ibb, Inst *inst) {

    		// perform initialization for a new bb
    		if(ibb != bb) {
    			bb = ibb;
    			lbs = BB_LBLOCKS(bb);
    			i = 0;
    		}

    		// is it a l-block bound?
    		if(i < lbs->count() && lbs->get(i)->address() == inst->address()) {
    			category_t cat = CATEGORY(lbs->get(i));
    			if(cat == FIRST_MISS)
    				hd = cache::CATEGORY_HEADER(lbs->get(i));
    			i++;
    			return cat;
    		}
    		else
    			return INVALID_CATEGORY;
    	}

    	inline Block *header(void) const { return hd; }

    private:
    	BasicBlock *bb;
    	AllocArray<LBlock*> *lbs;
    	Block *hd;
    	int i;
    };

    
    // -- class PathContext --------------------------------------------------------------------------------
    class PathContext{
    private:
		List<BasicBlock *> _bb_list;
		int _num_insts;
		int _num_bbs;
		BasicBlock * _bb;
		Edge * _edge;
    public:
		PathContext(BasicBlock *bb){
			_bb_list.addFirst(bb);
			_num_insts = bb->count();
			_num_bbs = 1;
			_bb = bb;
			_edge = NULL;
		}
		PathContext(const PathContext& ctxt){
			for (List<BasicBlock *>::Iter block(ctxt._bb_list) ; block() ; block++)
				_bb_list.addLast(*block);
			_num_insts = ctxt._num_insts;
			_num_bbs = ctxt._num_bbs;
			_bb = ctxt._bb;
			_edge = ctxt._edge;
		}
		~PathContext(){
			_bb_list.clear();
		}
		void addBlock(BasicBlock * new_bb, Edge * edge){
			_bb_list.addFirst(new_bb);
			_num_insts += new_bb->count();
			_num_bbs += 1;
			if (_num_bbs == 1)
				_bb = new_bb;
			if (_num_bbs == 2)
				_edge = edge;
		}
		inline int numInsts()
		{ return _num_insts;}
		inline int numBlocks()
		{ return _num_bbs;}
		inline BasicBlock* lastBlock()
		{ return _bb_list.last();}
		inline BasicBlock* mainBlock()
		{ return _bb;}
		inline Edge * edge     ()
		{ return _edge;}
		void dump(io::Output& output) {
			for (List<BasicBlock *>::Iter bb(_bb_list) ; bb() ; bb++){
				output << "b" << bb->index() << "-";
			}
		}

		class BasicBlockIterator: public List<BasicBlock *>::Iter {
		public:
			inline BasicBlockIterator(const PathContext& ctxt)
				: List<BasicBlock *>::Iter(ctxt._bb_list) {}
		};

    };


    // -- class GraphBBTime ----------------------------------------------------------------------------------

    template <class G>
		class GraphBBTime: public BBProcessor {
    private:
		WorkSpace *_ws;
		PropList _props;
		int _prologue_depth;
		OutStream *_output_stream;
		elm::io::Output *_output;
		String _graphs_dir_name;

    protected:
		bool _do_output_graphs;
		bool _do_consider_icache;
		const hard::Memory *mem;
		const hard::Cache *icache;
		ParExeProc *_microprocessor;
		Vector<Resource *> _hw_resources;

		virtual int cacheMissPenalty(Address addr) const;
		virtual int memoryLatency(Address addr) const;
		virtual void buildNCTimingContextListForICache(List<TimingContext *> *list, ParExeSequence *seq);
		virtual void buildFMTimingContextListForICache(List<TimingContext *> *list, ParExeSequence *seq);
		virtual void computeDefaultTimingContextForICache(TimingContext *dtctxt, ParExeSequence *seq);
		virtual void BuildVectorOfHwResources();
		virtual void configureMem(WorkSpace *ws) {
			if(ws->isProvided(hard::CACHE_CONFIGURATION_FEATURE))
				icache = hard::CACHE_CONFIGURATION_FEATURE.get(ws)->instCache();
			else
				icache = nullptr;
			_do_consider_icache = icache;
			if(ws->isProvided(hard::MEMORY_FEATURE))
				mem = hard::MEMORY_FEATURE.get(ws);
			else
				mem = nullptr;
		}

    public:
		//GraphBBTime(const PropList& props = PropList::EMPTY);
		static p::declare reg;
		GraphBBTime(AbstractRegistration& _reg = reg);
		virtual void configure(const PropList& props);

		void processWorkSpace(WorkSpace *ws);
		void processBB(WorkSpace *ws, CFG *cfg, Block *bb);
		List<PathContext *> * buildListOfPathContexts(BasicBlock *bb, int depth = 1);
		void FillSequence(PathContext *ctxt,
						  List<PathContext *> *context_list,
						  int depth);
		ParExeSequence * buildSequence(PathContext *ctxt);
		void analyzePathContext(PathContext *ctxt, int context_index);
		int analyzeTimingContext(G* graph, TimingContext *NC_ctxt, TimingContext *FM_ctxt);
		void outputGraph(G* graph, int bb_number, int context_number, int case_number, const string& info = "");
    
    };
	// -- GraphBBTime ------------------------------------------------------------------------------------------

	template <class G>
	int GraphBBTime<G>::cacheMissPenalty(Address addr) const {
		const hard::Bank *bank = mem->get(addr);
		ASSERTP(bank, "no bank for memory access at " << addr);
		return bank->latency();
	}

	template <class G>
	int GraphBBTime<G>::memoryLatency(Address addr) const {
		const hard::Bank *bank = mem->get(addr);
		if(!bank)
			throw ProcessorException(*this, _ << "no bank for memory access at " << addr);
		return bank->latency();
	}

	/*template <class G>
		GraphBBTime<G>::GraphBBTime(const PropList& props) 
		: BBProcessor() {
		_graphs_dir_name = GRAPHS_OUTPUT_DIRECTORY(props);
		if (!_graphs_dir_name.isEmpty())
			_do_output_graphs = true;
		else
			_do_output_graphs = false;
		//_icache_miss_penalty = -1;
		if (CACHE_CONFIG_PATH(props)){
			_do_consider_icache = true;
		}
		else 
			_do_consider_icache = false;
   
		_props = props;
		provide(ipet::BB_TIME_FEATURE);
		//    provide(ICACHE_ACCURATE_PENALTIES_FEATURE);
	}*/

template <class G>
GraphBBTime<G>::GraphBBTime(AbstractRegistration& _reg): BBProcessor(_reg) { }

template <class G>
p::declare GraphBBTime<G>::reg = p::init("", Version(1, 0, 0))
	.make<GraphBBTime<G> >()
	.base(BBProcessor::reg)
	.require(hard::PROCESSOR_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.provide(ipet::BB_TIME_FEATURE);


template <class G>
void GraphBBTime<G>::configure(const PropList& props) {
	BBProcessor::configure(props);
	_graphs_dir_name = GRAPHS_OUTPUT_DIRECTORY(props);
	if(!_graphs_dir_name.isEmpty())
		_do_output_graphs = true;
	else
		_do_output_graphs = false;
	//_icache_miss_penalty = -1;
	if(CACHE_CONFIG_PATH(props)){
		_do_consider_icache = true;
	}
	else
		_do_consider_icache = false;
	_props = props;
}


  
	// -- processWorkSpace ------------------------------------------------------------------------------------------

	template <class G>  
		void GraphBBTime<G>::processWorkSpace(WorkSpace *ws) {

		_ws = ws;
		const hard::Processor *proc = hard::PROCESSOR_FEATURE.get(_ws);
		if(proc == &hard::Processor::null)
			throw ProcessorException(*this, "no processor to work with");
		_microprocessor = new ParExeProc(proc);
		BuildVectorOfHwResources();															// ===== TO BE ENABLED
		configureMem(_ws);

		// Perform the actual process
		BBProcessor::processWorkSpace(ws);
	}

	// -- Build vector of pipeline-related resources -----------------------------------------------------------

	template <class G>
		void GraphBBTime<G>::BuildVectorOfHwResources(){

    int resource_index = 0;
    //bool is_ooo_proc = false;

    // build the start resource
    StartResource * new_resource = new StartResource((elm::String) "start", resource_index++);
    _hw_resources.add(new_resource);

    // build resource for stages and FUs
    for (ParExePipeline::StageIterator stage(_microprocessor->pipeline()) ; stage() ; stage++) {
		if (stage->category() != ParExeStage::EXECUTE) {
			for (int i=0 ; i<stage->width() ; i++) {
				StringBuffer buffer;
				buffer << stage->name() << "[" << i << "]";
				StageResource * new_resource = new StageResource(buffer.toString(), *stage, i, resource_index++);
				_hw_resources.add(new_resource);
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
						_hw_resources.add(new_resource);
					}
				}
			}
			/*else
				is_ooo_proc = true;*/
		}
    }

    // build resources for queues
    for (ParExeProc::QueueIterator queue(_microprocessor) ; queue() ; queue++) {
		for (int i=0 ; i< queue->size() ; i++) {
			StringBuffer buffer;
			buffer << queue->name() << "[" << i << "]";

			// find upper extraction bound
			StageResource * upper_bound = nullptr;
			for (Vector<Resource *>::Iter resource(_hw_resources) ; resource() ; resource++) {
				if (resource->type() == Resource::STAGE) {
					StageResource *s = ((StageResource *)(*resource));

					if(s->stage() == queue->emptyingStage()) {
						if(i < queue->size() - s->stage()->width() /*- 1*/) {
							if(s->slot() == s->stage()->width()-1)
								upper_bound = s;
						}
						else {
							if(s->slot() == i - queue->size() + s->stage()->width())
								upper_bound = s;
						}
					}

				}
			}
			ASSERT(upper_bound);

			// build the queue resource
			QueueResource * new_resource = new QueueResource(buffer.toString(), *queue, i, resource_index++, upper_bound, _microprocessor->pipeline()->numStages());
			_hw_resources.add(new_resource);
		}

    }

		// build resources for registers
		//    const otawa::hard::Platform::banks_t & reg_banks  =  ws->platform()->banks() ;
		//	elm::cout << reg_banks.count() << " banks to consider \n";
		//	for (int b=0 ; b<reg_banks.count() ; b++) {
		//		otawa::hard::RegBank * bank = (otawa::hard::RegBank *) reg_banks.get(b);
		//		elm::cout << reg_banks.count() << " bank" << b << " has " << bank->count() << " registers\n";
		//		for (int r=0 ; r<bank->count() ; r++)
		//		{
		//			StringBuffer buffer;
		//			buffer << bank->name() << r;
		//			RegResource * new_resource = new RegResource(buffer.toString(), bank, r, resource_index++);
		//			_resources.add(new_resource);
		//		}
		//	}
}

	// -- FillSequence ------------------------------------------------------------------------------------------
 
	template <class G>
		void GraphBBTime<G>::FillSequence(PathContext *ctxt,
										  List<PathContext *> *context_list,
										  int depth){

		BasicBlock *bb = ctxt->lastBlock();
		int last_stage_cap = _microprocessor->lastStage()->width();
		int num_preds = 0;
		for(BasicBlock::EdgeIter edge = bb->ins(); edge(); edge++) {
			Block *pred = edge->source();
			if (pred->isBasic()) {
				BasicBlock *pred = edge->source()->toBasic();
				num_preds++;
				PathContext *new_ctxt = new PathContext(*ctxt);
				new_ctxt->addBlock(pred, *edge);
				if ( (new_ctxt->numInsts() >= last_stage_cap)
					 &&
					 (new_ctxt->numBlocks() > depth) )
					context_list->addLast(new_ctxt);
				else
					FillSequence(new_ctxt, context_list, depth);
			}
		}
		if (num_preds == 0){
			context_list->addLast(ctxt);
		}
		else
			delete ctxt;
	}
 

	// -- buildListOfPathContexts ---------------------------------------------------------------------------------------
 
	template <class G>
		List<PathContext *> * GraphBBTime<G>::buildListOfPathContexts(BasicBlock *bb, int depth){
		ASSERT(depth > 0);
		List<PathContext *> * context_list = new List<PathContext *>();
		PathContext * ctxt = new PathContext(bb);
  
		FillSequence(ctxt, context_list, depth);

		return context_list;
	}

	// -- buildSequence ------------------------------------------------------------------------------------------

	template <class G> 
		ParExeSequence * GraphBBTime<G>::buildSequence(PathContext *ctxt){
		ParExeSequence * seq = new ParExeSequence();
		code_part_t part = PROLOGUE;
		int index = 0;
		for (PathContext::BasicBlockIterator block(*ctxt) ; block() ; block++){
			if (*block == ctxt->mainBlock())
				part = BLOCK;
			for(BasicBlock::InstIter inst = block->insts(); inst(); inst++) {
				ParExeInst * par_exe_inst = new ParExeInst(*inst, *block, part, index++);
				seq->addLast(par_exe_inst);
			}
		}
		return seq;
	}

	// -- outputGraph ------------------------------------------------------------------------------------------

	template <class G> 
		void GraphBBTime<G>::outputGraph(G* graph, int bb_number, int context_index, int case_index, const string& info){
		elm::StringBuffer buffer;
		buffer << _graphs_dir_name << "/";
		buffer << graph->lastNode()->inst()->basicBlock()->cfg()->name() << "-bb" << bb_number << "-ctxt" << context_index << "-case" << case_index << ".dot";
		elm::io::OutFileStream dotStream(buffer.toString());
		elm::io::Output dotFile(dotStream);
		graph->dump(dotFile, info);
	}


	// -- buildNCTimingContextListForICache ---------------------------------------------------------------------------

	template <class G>
		void GraphBBTime<G>::buildNCTimingContextListForICache(List<TimingContext *> * list, ParExeSequence *seq){
 
		List<TimingContext *> * to_add = new List<TimingContext *>();

		// process NOT_CLASSIFIED lblocks
		LBlockManager lbm;
		for (ParExeSequence::InstIterator inst(seq) ; inst() ; inst++)  {
			category_t cat = lbm.next(inst->basicBlock(), inst->inst());
			if (cat != INVALID_CATEGORY){
				if (cat == cache::NOT_CLASSIFIED){
					if (list->isEmpty()){
						TimingContext *tctxt = new TimingContext();
						NodeLatency * nl = new NodeLatency(inst->fetchNode(), cacheMissPenalty(inst->inst()->address()));
						tctxt->addNodeLatency(nl);
						list->addLast(tctxt);
					}
					else {
						for (List<TimingContext *>::Iter tctxt(*list) ; tctxt() ; tctxt++){
							TimingContext *new_tctxt = new TimingContext(tctxt.item());
							NodeLatency * nl = new NodeLatency(inst->fetchNode(), cacheMissPenalty(inst->inst()->address()));
							new_tctxt->addNodeLatency(nl);
							to_add->addLast(new_tctxt);
						}
						for (List<TimingContext *>::Iter tctxt(*to_add) ; tctxt() ; tctxt++){
							list->addLast(tctxt.item());
						}
						to_add->clear();
						TimingContext *new_tctxt = new TimingContext();
						NodeLatency * nl = new NodeLatency(inst->fetchNode(), cacheMissPenalty(inst->inst()->address()));
						new_tctxt->addNodeLatency(nl);
						list->addLast(new_tctxt);
					}
				}
			}
		}
		delete to_add;
	}

	// -- buildFMTimingContextListForICache ---------------------------------------------------------------------------

	template <class G>
		void GraphBBTime<G>::buildFMTimingContextListForICache(List<TimingContext *> * list, ParExeSequence *seq){
 
		Block *header0 = NULL;
		Block *header1 = NULL;
		int num_headers = 0;

		// process FIRST_MISS lblocks

		// find FIRST_MISS headers
		LBlockManager lbm;
		for (ParExeSequence::InstIterator inst(seq) ; inst() ; inst++)  {
			category_t cat = lbm.next(inst->basicBlock(), inst->inst());
			if (cat != INVALID_CATEGORY){
				if (cat == cache::FIRST_MISS){
					Block *header = lbm.header();
					//		elm::cout << "found header b" << header->number() << "\n";
					if (header0 == NULL){
						header0 = header;
						//	    elm::cout << "\tsaved in header0\n";
						num_headers++;
					}
					else {
						if (header0 != header){
							if (header1 == NULL){
								if (Dominance::dominates(header, header0)){
									header1 = header0;
									header0 = header;
									//		elm::cout << "\tsaved in header0 (header1 takes header0)\n";
								}
								else {
									header1 = header;
									//		elm::cout << "\tsaved in header1\n";
								}
								num_headers++;
							}
							else { 
								if (header1 != header) {
									// third header: is not expected to be found - could be implemented by ignoring the first headers in the sequence
									ASSERTP(0, "this sequence has more than 2 headers for cache categories: this is not supported so far\n");
								}
								// else
								//	elm::cout << "\talready found in header1\n";
							}	  
						} // header0 != header
						// else {
						//	elm::cout << "\talready found in header0\n";
						// }
					}
				}
			}
		}
		// create timing contexts
		if (num_headers){
			if (num_headers == 1){
				TimingContext *tctxt_first = new TimingContext(header0);
				tctxt_first->setType(CachePenalty::MISS); 
				list->addLast(tctxt_first);

				LBlockManager lbm;
				for (ParExeSequence::InstIterator inst(seq) ; inst() ; inst++)  {
					cache::category_t cat = lbm.next(inst->basicBlock(), inst->inst());
					if (cat != cache::INVALID_CATEGORY){
						if (cat == cache::FIRST_MISS){ // must be with header0
							NodeLatency * nl = new NodeLatency(inst->fetchNode(), cacheMissPenalty(inst->inst()->address()));
							tctxt_first->addNodeLatency(nl);
						}
					}
				}
				/* 	    elm::cout << "One header: context is: \n"; */
				/* 	    for (TimingContext::NodeLatencyIterator nl(*tctxt_first) ; nl ; nl++){ */
				/* 		elm::cout << "\t\t\t" << (nl.item())->node()->name() << " : lat=" << (nl.item())->latency() << "\n"; */
				/* 	    }	 */
			}
			else { // num_headers == 2
				TimingContext *tctxt_first_first = new TimingContext(header0, header1);
				tctxt_first_first->setType(CachePenalty::MISS_MISS);
				list->addLast(tctxt_first_first);
				TimingContext *tctxt_others_first = new TimingContext(header0, header1);
				tctxt_others_first->setType(CachePenalty::HIT_MISS);
				list->addLast(tctxt_others_first);
				TimingContext *tctxt_first_others = new TimingContext(header0, header1);
				tctxt_first_others->setType(CachePenalty::x_HIT);
				list->addLast(tctxt_first_others);
				LBlockManager lbm;
				for (ParExeSequence::InstIterator inst(seq) ; inst() ; inst++)  {
					cache::category_t cat = lbm.next(inst->basicBlock(), inst->inst());
					if (cat != cache::INVALID_CATEGORY){
						if (cat == cache::FIRST_MISS){
							Block *header = lbm.header();
							NodeLatency * nl = new NodeLatency(inst->fetchNode(), cacheMissPenalty(inst->inst()->address()));
							tctxt_first_first->addNodeLatency(nl);
							nl = new NodeLatency(inst->fetchNode(), cacheMissPenalty(inst->inst()->address()));
							if (header == header0){
								tctxt_first_others->addNodeLatency(nl);
							}
							else {// must be header==header1
								tctxt_others_first->addNodeLatency(nl);
							}
						}
					}
				}
			}
		}
	}

	// -- computeDefaultTimingContextForICache ---------------------------------------------------------------------------

	template <class G>
		void GraphBBTime<G>::computeDefaultTimingContextForICache(TimingContext *dtctxt, ParExeSequence *seq){
		LBlockManager lbm;
		for (ParExeSequence::InstIterator inst(seq) ; inst() ; inst++)  {
			cache::category_t cat = lbm.next(inst->basicBlock(), inst->inst());
			if (cat != cache::INVALID_CATEGORY){
				if (cat == cache::ALWAYS_MISS) {
					NodeLatency * nl = new NodeLatency(inst->fetchNode(), cacheMissPenalty(inst->inst()->address()));
					dtctxt->addNodeLatency(nl);
				}
			}
		}

	}

	// -- analyzeTimingContext ------------------------------------------------------------------------------------------

	template <class G> 
		int GraphBBTime<G>::analyzeTimingContext(G* graph, TimingContext *NC_ctxt, TimingContext *FM_ctxt){
		graph->restoreDefaultLatencies();
		if (NC_ctxt)
			graph->setLatencies(NC_ctxt);
		if (FM_ctxt)
			graph->setLatencies(FM_ctxt);
		int cost = graph->analyze();
		return cost;
	}

	// -- analyzePathContext ------------------------------------------------------------------------------------------

	template <class G> 
		void GraphBBTime<G>::analyzePathContext(PathContext*ctxt, int context_index){

		int case_index = 0;
		BasicBlock * bb = ctxt->mainBlock();
		Edge *edge = ctxt->edge();


		ParExeSequence *sequence = buildSequence(ctxt);
		G *execution_graph = new G(_ws,_microprocessor, &_hw_resources, sequence, _props);
		execution_graph->build();
    
		// no cache
		if(!_do_consider_icache)
			for(typename G::InstIterator inst(execution_graph); inst(); inst++)
				inst->fetchNode()->setLatency(memoryLatency(inst->inst()->address()));

		// compute reference cost
		int reference_cost = execution_graph->analyze();
		if(isVerbose())
			log << "\t\treference cost = " << reference_cost << "\n\n";
		if (_do_output_graphs){
			outputGraph(execution_graph, bb->index(), context_index, case_index++,
				_ << "reference cost = " << reference_cost);
		}

		// case of a cache
		if (_do_consider_icache){

			// verbosity
			if(isVerbose()) {
				LBlockManager lbm;
				for (ParExeSequence::InstIterator inst(sequence) ; inst() ; inst++)  {
					cache::category_t cat = lbm.next(inst->basicBlock(), inst->inst());
					if (cat != cache::INVALID_CATEGORY){
						log << "\t\t\tcategory of I" << inst->index() << " is ";
						switch(cat){
						case cache::ALWAYS_HIT:
							log << "ALWAYS_HIT\n";
							break;
						case cache::ALWAYS_MISS:
							log << "ALWAYS_MISS\n";
							break;
						case cache::FIRST_MISS:
							log << "FIRST_MISS (with header b" << lbm.header()->index() << ")\n";
							break;
						case cache::NOT_CLASSIFIED:
							log << "NOT_CLASSIFIED\n";
							break;
						default:
							log << "unknown !!!\n";
							break;
						}
					}
				}
			}

			// set constant latencies (ALWAYS_MISS in the cache)
			TimingContext default_timing_context;
			computeDefaultTimingContextForICache(&default_timing_context, sequence);
			int default_icache_cost = reference_cost;
			if (!default_timing_context.isEmpty()){
				if(isVerbose()) {
					log << "\t\t\t\tdefault timing context: misses for";
					for (TimingContext::NodeLatencyIterator nl(default_timing_context) ; nl() ; nl++){
						log << "I" << nl->node()->inst()->index() << ", ";
					}
					log << " - ";
				}
				execution_graph->setDefaultLatencies(&default_timing_context);
				default_icache_cost = execution_graph->analyze();
				if (_do_output_graphs) {
					StringBuffer buf;
					buf << "cost with AM = " << default_icache_cost << "\\lAM (";
					for (TimingContext::NodeLatencyIterator nl(default_timing_context) ; nl() ; nl++)
						buf << "I" << nl->node()->inst()->index() << ", ";
					buf << ")";
					outputGraph(execution_graph, bb->index(), context_index, case_index++, buf.toString());
				}
				if(isVerbose())
					log << "cost = " << default_icache_cost << " (only accounting for fixed latencies)\n\n";
				if (default_icache_cost > reference_cost)
					reference_cost = default_icache_cost;
			}
	    
			// consider variable latencies (FIRST_MISS, NOT_CLASSIFIED)
			List<TimingContext *> NC_timing_context_list;
			List<TimingContext *> FM_timing_context_list;
			buildNCTimingContextListForICache(&NC_timing_context_list, sequence);	
			buildFMTimingContextListForICache(&FM_timing_context_list, sequence);

			int index = 0;
			CachePenalty *cache_penalty = new CachePenalty();

			bool first = true;
			if (!FM_timing_context_list.isEmpty()){
				for (List<TimingContext *>::Iter FM_tctxt(FM_timing_context_list) ; FM_tctxt() ; FM_tctxt++){
					if (first) {
						cache_penalty->setHeader(0, FM_tctxt->header(0));
						cache_penalty->setHeader(1, FM_tctxt->header(1));
						first = false;
					}
					if (!NC_timing_context_list.isEmpty()){
						for (List<TimingContext *>::Iter NC_tctxt(NC_timing_context_list) ; NC_tctxt() ; NC_tctxt++){
							int NC_cost = analyzeTimingContext(execution_graph, NC_tctxt.item(), NULL);
							if (NC_cost > reference_cost)
								reference_cost = NC_cost;
							int cost = analyzeTimingContext(execution_graph, NC_tctxt.item(), FM_tctxt.item());
							if(isVerbose()) {
								log << "\n\t\tcontext " << index << ": ";
								for (TimingContext::NodeLatencyIterator nl(*(NC_tctxt.item())) ; nl() ; nl++){
									log << "I" << (nl.item())->node()->inst()->index() << ",";
								}
								for (TimingContext::NodeLatencyIterator nl(*(FM_tctxt.item())) ; nl() ; nl++){
									log << "I" << (nl.item())->node()->inst()->index() << ",";
								}
								log << "  - ";
								log << "cost=" << cost;
								log << "  - NC_cost=" << NC_cost << "\n";
							}
			
							int penalty = cost - reference_cost; 
							// default_icache_cost is when all NCs hit
							if ((FM_tctxt->type() == CachePenalty::x_HIT) && (penalty < 0))
								penalty = 0;  // penalty = max [ hit-hit, miss-hit ]
							if (penalty > cache_penalty->penalty(FM_tctxt->type()))
								cache_penalty->setPenalty(FM_tctxt->type(), penalty);
							//cache_penalty->dump(elm::cout);
							//elm::cout << "\n";
							//elm::cout << " (penalty = " << penalty << " - p[" << FM_tctxt->type() << "] = " << cache_penalty->penalty(FM_tctxt->type()) << ")\n";
							if (_do_output_graphs) {
								StringBuffer buf;
								buf << "FM-NC cost = " << cost << "\\lfirst(";
									for (TimingContext::NodeLatencyIterator nl(*(FM_tctxt.item())) ; nl() ; nl++)
										buf << "I" << (nl.item())->node()->inst()->index() << ",";
								buf << "),\\lNC(";
									for (TimingContext::NodeLatencyIterator nl(*(NC_tctxt.item())) ; nl() ; nl++)
										buf << "I" << (nl.item())->node()->inst()->index() << ",";
								buf << ")";
								outputGraph(execution_graph, bb->index(), context_index, case_index++, buf.toString());
							}
						} 
					}
					else { // no NC context
						int cost = analyzeTimingContext(execution_graph, NULL, FM_tctxt.item());
						if(isVerbose()) {
							log << "\t\tcontext " << index << ": ";
							for (TimingContext::NodeLatencyIterator nl(*(FM_tctxt.item())) ; nl() ; nl++){
								log << "I" << (nl.item())->node()->inst()->index() << ",";
							}
							log << "  - ";
							log << "cost=" << cost << "\n";
						}
						int penalty = cost - reference_cost;
						if ((FM_tctxt->type() == CachePenalty::x_HIT) && (penalty < 0))
							penalty = 0;  // penalty = max [ hit-hit, miss-hit ]
						if (penalty > cache_penalty->penalty(FM_tctxt->type()))
							cache_penalty->setPenalty(FM_tctxt->type(), penalty);
						/* cache_penalty->dump(elm::cout); */
/* 							elm::cout << "\n"; */
/* 						elm::cout << " (penalty = " << penalty << " - p[" << FM_tctxt->type() << "] = " << cache_penalty->penalty(FM_tctxt->type()) << ")\n"; */
						if (_do_output_graphs){
							StringBuffer buf;
							buf << "NC cost = " << cost << "\\lNC (";
								for (TimingContext::NodeLatencyIterator nl(*(FM_tctxt.item())) ; nl() ; nl++)
									buf << "I" << (nl.item())->node()->inst()->index() << ",";
							buf << ")";
							outputGraph(execution_graph, bb->index(), context_index, case_index++, buf.toString());
						}
					}
				}
	    
			}
			else { // no FM context
				for (List<TimingContext *>::Iter NC_tctxt(NC_timing_context_list) ; NC_tctxt() ; NC_tctxt++){
					int NC_cost = analyzeTimingContext(execution_graph, NC_tctxt.item(), NULL);
					if(isVerbose()) {
						log << "\t\tcontext " << index << ": ";
						for (TimingContext::NodeLatencyIterator nl(*(NC_tctxt.item())) ; nl() ; nl++){
							log << "I" << (nl.item())->node()->inst()->index() << ",";
						}
						log << " - ";
						log << "cost=" << NC_cost << "\n";
					}
					if (NC_cost > reference_cost)
						reference_cost = NC_cost;
					if (_do_output_graphs){
						StringBuffer buf;
						buf << "NC cost = " << NC_cost << "\\l NC (";
						for (TimingContext::NodeLatencyIterator nl(*(NC_tctxt.item())) ; nl() ; nl++)
							buf << "I" << (nl.item())->node()->inst()->index() << ",";
						buf << ")";
						outputGraph(execution_graph, bb->index(), context_index, case_index++, buf.toString());
					}
				}
			}

			// set the cache penalty
			if (cache_penalty->header(0)){
				ICACHE_PENALTY(bb) = cache_penalty;
				if (edge)
					ICACHE_PENALTY(edge) = cache_penalty;
				if(isVerbose()) {
					log << "\t\tcache penalty: ";
					cache_penalty->dump(log);
					log << "\n";
				}
			}
			
		}

		// record the times
		if(isVerbose())
			log << "\t\tReference cost: " << reference_cost << "\n";
		if (otawa::ipet::TIME(bb) < reference_cost)
			otawa::ipet::TIME(bb) = reference_cost;
		if (edge){
			if (otawa::ipet::TIME(edge) < reference_cost){
				otawa::ipet::TIME(edge) = reference_cost;
			}
		}
    
		// cleanup
		delete execution_graph;  
	}


	// -- processBB ------------------------------------------------------------------------------------------
  
	template <class G>  
		void GraphBBTime<G>::processBB(WorkSpace *ws, CFG *cfg, Block *block) {

		// ignore empty basic blocks
		if(!block->isBasic())
			return;
		BasicBlock *bb = block->toBasic();
		if(!bb->count())
			return;

		if(isVerbose()) {
			log << "\n\t\t================================================================\n";
			log << "\t\tProcessing block b" << bb->index() << " (starts at " << bb->address() << " - " << bb->count() << " instructions)\n";
		}

		int context_index = 0;

		List<PathContext *> *path_context_list = buildListOfPathContexts(bb);

		for (List<PathContext *>::Iter ctxt(*path_context_list) ; ctxt() ; ctxt++){
			if(isVerbose()) {
				log << "\n\t\t----- Considering context: ";
				ctxt->dump(log);
				log << "\n";
			}
			analyzePathContext(*ctxt, context_index);
			context_index ++;
		}
	}


} //otawa

#endif // OTAWA_GRAPH_BBTIME_H
