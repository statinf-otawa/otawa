/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	ExeGraphBBTime.h -- ExeGraphBBTime class interface.
 */
#ifndef GRAPH_BBTIME_H
#define GRAPH_BBTIME_H

#include <otawa/ipet.h>
#include <otawa/prop/Identifier.h>
#include <otawa/cfg.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/hard/Memory.h>
#include <otawa/parexegraph/ParExeGraph.h>
#include <elm/io/OutFileStream.h>
#include <otawa/ipet.h>
#include <otawa/cache/cat2/CachePenalty.h>


namespace otawa {
    extern Identifier<String> GRAPHS_OUTPUT_DIRECTORY;
    extern Identifier<int> TIME;

 
	/*      extern Feature<GraphBBTime> ICACHE_ACCURATE_PENALTIES_FEATURE; */

    using namespace elm::genstruct; 
    
    // -- class PathContext --------------------------------------------------------------------------------
    class PathContext{
    private:
		elm::genstruct::SLList<BasicBlock *> _bb_list;
		int _num_insts;
		int _num_bbs;
		BasicBlock * _bb;
		Edge * _edge;
    public:
		PathContext(BasicBlock *bb){
			_bb_list.addFirst(bb);
			_num_insts = bb->countInstructions();
			_num_bbs = 1;
			_bb = bb;
			_edge = NULL;
		}
		PathContext(const PathContext& ctxt){
			for (elm::genstruct::SLList<BasicBlock *>::Iterator block(ctxt._bb_list) ; block ; block++)
				_bb_list.addLast(block);
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
			_num_insts += new_bb->countInstructions();
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
			for (elm::genstruct::SLList<BasicBlock *>::Iterator bb(_bb_list) ; bb ; bb++){
				output << "b" << bb->number() << "-";
			}
		}

		class BasicBlockIterator: public elm::genstruct::SLList<BasicBlock *>::Iterator {
		public:
			inline BasicBlockIterator(const PathContext& ctxt)
				: elm::genstruct::SLList<BasicBlock *>::Iterator(ctxt._bb_list) {}
		};

    };


    // -- class GraphBBTime ----------------------------------------------------------------------------------

    template <class G>
		class GraphBBTime: public BBProcessor {
    private:
		WorkSpace *_ws;
		ParExeProc *_microprocessor;
		int _last_stage_cap;
		PropList _props;
		int _prologue_depth;
		OutStream *_output_stream;
		elm::io::Output *_output;
		String _graphs_dir_name;
		bool _do_output_graphs;
		bool _do_consider_icache;
		const hard::Memory *mem;
		int cacheMissPenalty(Address addr) const;

    public:
		GraphBBTime(const PropList& props = PropList::EMPTY);
		GraphBBTime(AbstractRegistration& reg);
		virtual void configure(const PropList& props);

		void processWorkSpace(WorkSpace *ws);
		void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
		elm::genstruct::SLList<PathContext *> * buildListOfPathContexts(BasicBlock *bb, int depth = 1);
		void FillSequence(PathContext *ctxt,
						  elm::genstruct::SLList<PathContext *> *context_list, 		       
						  int depth);
		ParExeSequence * buildSequence(PathContext *ctxt);
		void analyzePathContext(PathContext *ctxt, int context_index);
		int analyzeTimingContext(G* graph, TimingContext *NC_ctxt, TimingContext *FM_ctxt);
		void buildNCTimingContextListForICache(elm::genstruct::SLList<TimingContext *> *list, ParExeSequence *seq);
		void buildFMTimingContextListForICache(elm::genstruct::SLList<TimingContext *> *list, ParExeSequence *seq);
		void computeDefaultTimingContextForICache(TimingContext *dtctxt, ParExeSequence *seq);
		void outputGraph(G* graph, int bb_number, int context_number, int case_number);
    
    };
	// -- GraphBBTime ------------------------------------------------------------------------------------------

	template <class G>
	int GraphBBTime<G>::cacheMissPenalty(Address addr) const {
		const hard::Bank *bank = mem->get(addr);
		ASSERTP(bank, "no bank for memory access at " << addr);
		return bank->latency();
	}

	template <class G>
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
	}

template <class G>
GraphBBTime<G>::GraphBBTime(AbstractRegistration& reg)
: BBProcessor(reg) {
	provide(ipet::BB_TIME_FEATURE);
}

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
		const hard::Processor *proc = _ws->platform()->processor();

		if(!proc)
			throw ProcessorException(*this, "no processor to work with");
		else {
			_microprocessor = new ParExeProc(proc);
			_last_stage_cap = _microprocessor->lastStage()->width();

		}

		// look for memory hierarchy
		const hard::Cache *cache = _ws->platform()->cache().instCache();
		_do_consider_icache = cache;
		mem = &_ws->platform()->memory();

		// Perform the actual process
		BBProcessor::processWorkSpace(ws);
	}


	// -- FillSequence ------------------------------------------------------------------------------------------
 
	template <class G>
		void GraphBBTime<G>::FillSequence(PathContext *ctxt,
										  elm::genstruct::SLList<PathContext *> *context_list, 		       
										  int depth){

		BasicBlock *bb = ctxt->lastBlock();
		int num_preds = 0;
		for(BasicBlock::InIterator edge(bb); edge; edge++) {
			BasicBlock *pred = edge->source();
			if (!pred->isEntry() && !pred->isExit()) {
				num_preds++;
				PathContext *new_ctxt = new PathContext(*ctxt);
				new_ctxt->addBlock(pred, edge);
				if ( (new_ctxt->numInsts() >= _last_stage_cap)
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
		elm::genstruct::SLList<PathContext *> * GraphBBTime<G>::buildListOfPathContexts(BasicBlock *bb, int depth){
		assert(depth > 0);
		elm::genstruct::SLList<PathContext *> * context_list = new elm::genstruct::SLList<PathContext *>();
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
		for (PathContext::BasicBlockIterator block(*ctxt) ; block ; block++){
			if (block == ctxt->mainBlock())
				part = BODY;
			for(BasicBlock::InstIterator inst(block); inst; inst++) {
				ParExeInst * par_exe_inst = new ParExeInst(inst, block, part, index++);
				seq->addLast(par_exe_inst);
			}
		}
		return seq;
	}

	// -- outputGraph ------------------------------------------------------------------------------------------

	template <class G> 
		void GraphBBTime<G>::outputGraph(G* graph, int bb_number, int context_index, int case_index){
		elm::StringBuffer buffer;
		buffer << _graphs_dir_name << "/";
		buffer << "b" << bb_number << "-ctxt" << context_index << "-case" << case_index << ".dot";
		elm::io::OutFileStream dotStream(buffer.toString());
		elm::io::Output dotFile(dotStream);
		graph->dump(dotFile);
	}


	// -- buildNCTimingContextListForICache ---------------------------------------------------------------------------

	template <class G>
		void GraphBBTime<G>::buildNCTimingContextListForICache(elm::genstruct::SLList<TimingContext *> * list, ParExeSequence *seq){
 
		elm::genstruct::SLList<TimingContext *> * to_add = new elm::genstruct::SLList<TimingContext *>();

		// process NOT_CLASSIFIED lblocks
		for (ParExeSequence::InstIterator inst(seq) ; inst ; inst++)  {
			LBlock *lb = LBLOCK(inst->inst());
			if (lb){
				if (CATEGORY(lb) == NOT_CLASSIFIED){
					if (list->isEmpty()){
						TimingContext *tctxt = new TimingContext();
						NodeLatency * nl = new NodeLatency(inst->fetchNode(), cacheMissPenalty(inst->inst()->address()));
						tctxt->addNodeLatency(nl);
						list->addLast(tctxt);
					}
					else {
						for (elm::genstruct::SLList<TimingContext *>::Iterator tctxt(*list) ; tctxt ; tctxt++){
							TimingContext *new_tctxt = new TimingContext(tctxt.item());
							NodeLatency * nl = new NodeLatency(inst->fetchNode(), cacheMissPenalty(inst->inst()->address()));
							new_tctxt->addNodeLatency(nl);
							to_add->addLast(new_tctxt);
						}
						for (elm::genstruct::SLList<TimingContext *>::Iterator tctxt(*to_add) ; tctxt ; tctxt++){
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
		void GraphBBTime<G>::buildFMTimingContextListForICache(elm::genstruct::SLList<TimingContext *> * list, ParExeSequence *seq){
 
		BasicBlock *header0 = NULL;
		BasicBlock *header1 = NULL;
		int num_headers = 0;

		// process FIRST_MISS lblocks

		// find FIRST_MISS headers
		for (ParExeSequence::InstIterator inst(seq) ; inst ; inst++)  {
			LBlock *lb = LBLOCK(inst->inst());
			if (lb){
				if (CATEGORY(lb) == FIRST_MISS){
					BasicBlock *header = CATEGORY_HEADER(lb);
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

				for (ParExeSequence::InstIterator inst(seq) ; inst ; inst++)  {
					LBlock *lb = LBLOCK(inst->inst());
					if (lb){
						if (CATEGORY(lb) == FIRST_MISS){ // must be with header0
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
				for (ParExeSequence::InstIterator inst(seq) ; inst ; inst++)  {
					LBlock *lb = LBLOCK(inst->inst());
					if (lb){
						if (CATEGORY(lb) == FIRST_MISS){ 
							BasicBlock *header = CATEGORY_HEADER(lb);
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
  
		for (ParExeSequence::InstIterator inst(seq) ; inst ; inst++)  {
			LBlock *lb = LBLOCK(inst->inst());
			if (lb){
				if (CATEGORY(lb) == ALWAYS_MISS) {
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
		G *execution_graph = new G(_ws,_microprocessor, sequence, _props);
		execution_graph->build();
    
		// no cache
		if(!_do_consider_icache)
			for(typename G::InstIterator inst(execution_graph); inst; inst++)
				inst->fetchNode()->setLatency(cacheMissPenalty(inst->inst()->address()));

		// compute reference cost
		int reference_cost = execution_graph->analyze();
		//execution_graph->display(elm::cout);
		if (_do_output_graphs){
			outputGraph(execution_graph, bb->number(), context_index, case_index++);
		}
		if(isVerbose())
			log << "\t\treference cost = " << reference_cost << "\n\n";
    
		TimingContext default_timing_context;

		if (_do_consider_icache){

			if(isVerbose())
				for (ParExeSequence::InstIterator inst(sequence) ; inst ; inst++)  {
					LBlock *lb = LBLOCK(inst->inst());
					if (lb){
						log << "\t\t\tcategory of I" << inst->index() << " is ";
						switch( CATEGORY(lb)){
						case ALWAYS_HIT:
							log << "ALWAYS_HIT\n";
							break;
						case ALWAYS_MISS:
							log << "ALWAYS_MISS\n";
							break;
						case FIRST_MISS:
							log << "FIRST_MISS (with header b" << CATEGORY_HEADER(lb)->number() << ")\n";
							break;
						case NOT_CLASSIFIED:
							log << "NOT_CLASSIFIED\n";
							break;
						default:
							log << "unknown !!!\n";
							break;
						}
					}
				}

			// set constant latencies (ALWAYS_MISS in the cache)
			TimingContext default_timing_context;
			//assert(_icache_miss_penalty >= 0);
			computeDefaultTimingContextForICache(&default_timing_context, sequence);
			int default_icache_cost = reference_cost;
			if (!default_timing_context.isEmpty()){
				if(isVerbose()) {
					log << "\t\t\t\tdefault timing context: misses for";
					for (TimingContext::NodeLatencyIterator nl(default_timing_context) ; nl ; nl++){
						log << "I" << nl->node()->inst()->index() << ", ";
					}
					log << " - ";
				}
				execution_graph->setDefaultLatencies(&default_timing_context);
				default_icache_cost = execution_graph->analyze();
				if (_do_output_graphs){
					outputGraph(execution_graph, bb->number(), context_index, case_index++);
				}
				if(isVerbose())
					log << "cost = " << default_icache_cost << " (only accounting for fixed latencies)\n\n";
				if (default_icache_cost > reference_cost)
					reference_cost = default_icache_cost;
			}
	    
 
			// consider variable latencies (FIRST_MISS, NOT_CLASSIFIED)
			elm::genstruct::SLList<TimingContext *> NC_timing_context_list;
			elm::genstruct::SLList<TimingContext *> FM_timing_context_list;
			buildNCTimingContextListForICache(&NC_timing_context_list, sequence);	
			buildFMTimingContextListForICache(&FM_timing_context_list, sequence);

			int index = 0;
			CachePenalty *cache_penalty = new CachePenalty();

			bool first = true;
			if (!FM_timing_context_list.isEmpty()){
				for (elm::genstruct::SLList<TimingContext *>::Iterator FM_tctxt(FM_timing_context_list) ; FM_tctxt ; FM_tctxt++){
					if (first) {
						cache_penalty->setHeader(0, FM_tctxt->header(0));
						cache_penalty->setHeader(1, FM_tctxt->header(1));
						first = false;
					}
					if (!NC_timing_context_list.isEmpty()){
						for (elm::genstruct::SLList<TimingContext *>::Iterator NC_tctxt(NC_timing_context_list) ; NC_tctxt ; NC_tctxt++){
							int NC_cost = analyzeTimingContext(execution_graph, NC_tctxt.item(), NULL);
							if (NC_cost > reference_cost)
								reference_cost = NC_cost;
							int cost = analyzeTimingContext(execution_graph, NC_tctxt.item(), FM_tctxt.item());
							if(isVerbose()) {
								log << "\n\t\tcontext " << index << ": ";
								for (TimingContext::NodeLatencyIterator nl(*(NC_tctxt.item())) ; nl ; nl++){
									log << "I" << (nl.item())->node()->inst()->index() << ",";
								}
								for (TimingContext::NodeLatencyIterator nl(*(FM_tctxt.item())) ; nl ; nl++){
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
							if (_do_output_graphs){
								outputGraph(execution_graph, bb->number(), context_index, case_index++);
							}
						} 
					}
					else { // no NC context
						int cost = analyzeTimingContext(execution_graph, NULL, FM_tctxt.item());
						if(isVerbose()) {
							log << "\t\tcontext " << index << ": ";
							for (TimingContext::NodeLatencyIterator nl(*(FM_tctxt.item())) ; nl ; nl++){
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
							outputGraph(execution_graph, bb->number(), context_index, case_index++);
						}
					}
				}
	    
			}
			else { // no FM context
				for (elm::genstruct::SLList<TimingContext *>::Iterator NC_tctxt(NC_timing_context_list) ; NC_tctxt ; NC_tctxt++){
					int NC_cost = analyzeTimingContext(execution_graph, NC_tctxt.item(), NULL);
					if(isVerbose()) {
						log << "\t\tcontext " << index << ": ";
						for (TimingContext::NodeLatencyIterator nl(*(NC_tctxt.item())) ; nl ; nl++){
							log << "I" << (nl.item())->node()->inst()->index() << ",";
						}
						log << " - ";
						log << "cost=" << NC_cost << "\n";
					}
					if (NC_cost > reference_cost)
						reference_cost = NC_cost;
/* 					int penalty = cost - default_icache_cost; */
/* 					if (penalty < 0) */
/* 						penalty = 0; // penalty when all NCs hit is default_icache_cost; */
/* 					if (penalty > cache_penalty->penalty(CachePenalty::MISS)) */
/* 						cache_penalty->setPenalty(CachePenalty::MISS, penalty); */
/* 					cache_penalty->dump(elm::cout); */
/* 					elm::cout << "\n"; */
				   //elm::cout << " (penalty = " << penalty << " - p[0] = " << cache_penalty->penalty(CachePenalty::MISS) << ")\n";
					if (_do_output_graphs){
						outputGraph(execution_graph, bb->number(), context_index, case_index++);
					}
				}
			}
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
		if(isVerbose())
			log << "\t\tReference cost: " << reference_cost << "\n";
		if (otawa::ipet::TIME(bb) < reference_cost)
			otawa::ipet::TIME(bb) = reference_cost;
		if (edge){
			if (otawa::ipet::TIME(edge) < reference_cost){
				otawa::ipet::TIME(edge) = reference_cost;
			}
		}
    
		delete execution_graph;  
	}


	// -- processBB ------------------------------------------------------------------------------------------
  
	template <class G>  
		void GraphBBTime<G>::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {

		// ignore empty basic blocks
		if (bb->countInstructions() == 0)
			return;

		if(isVerbose()) {
			log << "\n\t\t================================================================\n";
			log << "\t\tProcessing block b" << bb->number() << " (starts at " << bb->address() << " - " << bb->countInstructions() << " instructions)\n";
		}

		int context_index = 0;

		elm::genstruct::SLList<PathContext *> *path_context_list = buildListOfPathContexts(bb);

		for (elm::genstruct::SLList<PathContext *>::Iterator ctxt(*path_context_list) ; ctxt ; ctxt++){
			if(isVerbose()) {
				log << "\n\t\t----- Considering context: ";
				ctxt->dump(log);
				log << "\n";
			}
			analyzePathContext(ctxt, context_index);   
			context_index ++;
		}
	}


} //otawa

#endif // GRAPH_BBTIME_H
