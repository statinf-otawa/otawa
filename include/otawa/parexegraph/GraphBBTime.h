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


namespace otawa {
    extern Identifier<String> GRAPHS_OUTPUT_DIRECTORY;
    extern Identifier<int> TIME;
  
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
    public:
	GraphBBTime(const PropList& props = PropList::EMPTY);
	void processWorkSpace(WorkSpace *ws);
	void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	elm::genstruct::SLList<PathContext *> * buildListOfPathContexts(BasicBlock *bb, int depth = 1);
	void FillSequence(PathContext *ctxt,
			  elm::genstruct::SLList<PathContext *> *context_list, 		       
			  int depth);
	ParExeSequence * buildSequence(PathContext *ctxt);
	void analyzePathContext(PathContext *ctxt, int context_index);
	void buildTimingContextListForICache(elm::genstruct::SLList<TimingContext *> *list, ParExeSequence *seq);
	void computeDefaultTimingContextForICache(TimingContext *dtctxt, ParExeSequence *seq);
	void outputGraph(G* graph, int bb_number, int context_number, int case_number);
    
    };
  // -- GraphBBTime ------------------------------------------------------------------------------------------

 template <class G>
GraphBBTime<G>::GraphBBTime(const PropList& props) 
    : BBProcessor() {
    _graphs_dir_name = GRAPHS_OUTPUT_DIRECTORY(props);
    if (!_graphs_dir_name.isEmpty())
	_do_output_graphs = true;
    else
	_do_output_graphs = false;
    if (CACHE_CONFIG_PATH(props))
	_do_consider_icache = true;
    else
	_do_consider_icache = false;
    _props = props;
    provide(ipet::BB_TIME_FEATURE);
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


// -- buildTimingContextListForICache ---------------------------------------------------------------------------

template <class G>
void GraphBBTime<G>::buildTimingContextListForICache(elm::genstruct::SLList<TimingContext *> * list, ParExeSequence *seq){
    int miss_latency = 10; // FIXME !!!


}

// -- computeDefaultTimingContextForICache ---------------------------------------------------------------------------

template <class G>
void GraphBBTime<G>::computeDefaultTimingContextForICache(TimingContext *dtctxt, ParExeSequence *seq){
    int miss_latency = 10; // FIXME !!!

    for (ParExeSequence::InstIterator inst(seq) ; inst ; inst++)  {
	LBlock *lb = LBLOCK(inst->inst());
	if (lb){
	    if (CATEGORY(lb) == ALWAYS_MISS) {
		NodeLatency * nl = new NodeLatency(inst->fetchNode(), miss_latency);
		dtctxt->addNodeLatency(nl);
	    }
	}
    }

}

// -- analyzePathContext ------------------------------------------------------------------------------------------

template <class G> 
void GraphBBTime<G>::analyzePathContext(PathContext*ctxt, int context_index){
    int exec_time;
    int max_bb_time = 0;
    int max_edge_time = 0;
    int case_index = 0;
    BasicBlock * bb = ctxt->mainBlock();
    Edge *edge = ctxt->edge();


    ParExeSequence *sequence = buildSequence(ctxt);
    G *execution_graph = new G(_ws,_microprocessor, sequence, _props);
    execution_graph->build();
    
    elm::genstruct::SLList<TimingContext *> timing_context_list;
    TimingContext default_timing_context;
    if (_do_consider_icache){

	computeDefaultTimingContextForICache(&default_timing_context, sequence);
	elm::cout << "\tDefault timing context: \n";
	for (TimingContext::NodeLatencyIterator nl(default_timing_context) ; nl ; nl++){
	    elm::cout << "\t\t" << nl->node()->name() << " : lat=" << nl->latency() << "\n";
	}
	execution_graph->setDefaultLatencies(&default_timing_context);
	
	buildTimingContextListForICache(&timing_context_list, sequence);
	//	elm::cout << "\tTiming contexts:\n";
/* 	for (elm::genstruct::SLList<TimingContext *>::Iterator tctxt(timing_context_list) ; tctxt ; tctxt++){ */
/* 	    elm::cout << "\t\tone context: \n"; */
/* 	    for (TimingContext::NodeLatencyIterator nl(*(tctxt.item())) ; nl ; nl++){ */
/* 		elm::cout << "\t\t\t" << nl->node()->name() << " : lat=" << nl->latency() << "\n"; */
/* 	    } */
/* 	} */
    }

    exec_time = execution_graph->analyze();
    //execution_graph->display(elm::cout);
    if (_do_output_graphs){
	outputGraph(execution_graph, bb->number(), context_index, case_index);
    }
    elm::cout << "\n\twcc = " << exec_time << "\n";
    if (exec_time > max_bb_time)
	max_bb_time = exec_time;
    if (exec_time > max_edge_time)
	max_edge_time = exec_time;

    
    if (otawa::ipet::TIME(bb) < max_bb_time)
	otawa::ipet::TIME(bb) = max_bb_time;

    if (edge){
	if (otawa::ipet::TIME(edge) < max_bb_time)
	    otawa::ipet::TIME(edge) = max_bb_time;
    }
    delete execution_graph;  
}


// -- processBB ------------------------------------------------------------------------------------------
  
template <class G>  
void GraphBBTime<G>::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {

  // ignore empty basic blocks
  if (bb->countInstructions() == 0)
    return;

  elm::cout << "================================================================\n";
  elm::cout << "Processing block b" << bb->number() << " (starts at " << bb->address() << " - " << bb->countInstructions() << " instructions)\n\n";

  int context_index = 0;

  elm::genstruct::SLList<PathContext *> *path_context_list = buildListOfPathContexts(bb);

  for (elm::genstruct::SLList<PathContext *>::Iterator ctxt(*path_context_list) ; ctxt ; ctxt++){
      elm::cout << "Considering context: ";
      ctxt->dump(elm::cout);
      elm::cout << "\n";
      analyzePathContext(ctxt, context_index);   
      context_index ++;
  }
}

} //otawa

#endif // GRAPH_BBTIME_H
