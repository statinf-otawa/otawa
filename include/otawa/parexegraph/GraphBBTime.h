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

  
  using namespace elm::genstruct; 
    
template <class G>
  class GraphBBTime: public BBProcessor {
    private:
    WorkSpace *_ws;
    ParExeProc *_microprocessor;
    PropList _props;
    int _prologue_depth;
    OutStream *_output_stream;
    elm::io::Output *_output;
    String _graphs_dir_name;
    public:
    GraphBBTime(const PropList& props = PropList::EMPTY);
    void processWorkSpace(WorkSpace *ws);
    void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
    elm::genstruct::SLList<elm::genstruct::SLList<BasicBlock *> *> * buildListOfContexts(BasicBlock *bb, int depth = 1);
    void FillSequence(elm::genstruct::SLList<BasicBlock *> *seq,
		      elm::genstruct::SLList<elm::genstruct::SLList<BasicBlock *> *> *seq_list, 		       
		      int cap, int depth);
    bool isFull(elm::genstruct::SLList<BasicBlock *> *seq, int cap, int depth);
    //    int getLatency(Address address);
    //    void buildPrologueList(BasicBlock * bb,
/* 			   ParExeSequence * prologue,  */
/* 			   int capacity,  */
/* 			   elm::genstruct::DLList<ParExeSequence *> * prologue_list, */
/* 			   int depth); */
    
  };
  
// --------------------------------------------------------------------------------------------

 template <class G>
GraphBBTime<G>::GraphBBTime(const PropList& props) 
    : BBProcessor() {
    _graphs_dir_name = GRAPHS_OUTPUT_DIRECTORY(props);
    _props = props;
    provide(ipet::BB_TIME_FEATURE);
}
  
// --------------------------------------------------------------------------------------------

template <class G>  
void GraphBBTime<G>::processWorkSpace(WorkSpace *ws) {

  _ws = ws;
  const hard::Processor *proc = _ws->platform()->processor();

  if(!proc)
    throw ProcessorException(*this, "no processor to work with");
  else {
    _microprocessor = new ParExeProc(proc);
  }
   // Perform the actual process
  BBProcessor::processWorkSpace(ws);
}

// --------------------------------------------------------------------------------------------
 
 template <class G>
     bool GraphBBTime<G>::isFull(elm::genstruct::SLList<BasicBlock *> *seq, int cap, int depth){
     int num_insts = 0;
     int num_bbs = 0;
     for (elm::genstruct::SLList<BasicBlock *>::Iterator bb(*seq) ; bb; bb++){
	 num_bbs++;
	 num_insts += bb->countInstructions();
     }
     return ( (num_insts >= cap) || (num_bbs > depth));
 }

// --------------------------------------------------------------------------------------------
 
 template <class G>
 void GraphBBTime<G>::FillSequence(elm::genstruct::SLList<BasicBlock *> *seq,
				   elm::genstruct::SLList<elm::genstruct::SLList<BasicBlock *> *> *seq_list, 		       
				   int cap, int depth){

     elm::cout << "[FillSequence]";
     BasicBlock *bb = seq->last();
     elm::cout << " last bb is b" << bb->number() <<"\n";
     int num_preds = 0;
     for(BasicBlock::InIterator edge(bb); edge; edge++) {
	 BasicBlock *pred = edge->source();
	 if (!pred->isEntry() && !pred->isExit()) {
	     num_preds++;
	     elm::cout << "considering pred b" << pred->number() << "\n";
	     elm::genstruct::SLList<BasicBlock *> *new_seq = new elm::genstruct::SLList<BasicBlock *>();
	     for (elm::genstruct::SLList<BasicBlock *>::Iterator block(*seq) ; block ; block++)
		 new_seq->addLast(block);
	     new_seq->addFirst(pred);
	     elm::cout << "new_seq is: ";
	     for (elm::genstruct::SLList<BasicBlock *>::Iterator block(*new_seq) ; block ; block++)
		 elm::cout << "b" << block->number() << "-";
	     elm::cout << "\n";
	     if (isFull(new_seq, cap, depth)){
		 elm::cout << "seq is full!\n";
		 seq_list->addLast(new_seq);
	     }
	     else
		 FillSequence(new_seq, seq_list, cap, depth);
	 }
     }
     if (num_preds == 0){
	 seq_list->addLast(seq);
	 elm::cout << "no preds!\n";
     }
     else
	 delete seq;
 }
 

// --------------------------------------------------------------------------------------------
 
 template <class G>
     elm::genstruct::SLList<elm::genstruct::SLList<BasicBlock *> *> * GraphBBTime<G>::buildListOfContexts(BasicBlock *bb, int depth){
     assert(depth > 0);
     int cap = _microprocessor->lastStage()->width();
     elm::genstruct::SLList<elm::genstruct::SLList<BasicBlock *> *> * seq_list =
	 new elm::genstruct::SLList<elm::genstruct::SLList<BasicBlock *> *>();
     elm::genstruct::SLList<BasicBlock *> * bseq = new elm::genstruct::SLList<BasicBlock *>();
     bseq->addLast(bb);
 
     FillSequence(bseq, seq_list, cap, depth);

     return seq_list;
 }

// --------------------------------------------------------------------------------------------
  
template <class G>  
void GraphBBTime<G>::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {

  // ignore empty basic blocks
  if (bb->countInstructions() == 0)
    return;

  elm::cout << "================================================================\n";
  elm::cout << "Processing block b" << bb->number() << " (starts at " << bb->address() << " - " << bb->countInstructions() << " instructions)\n\n";

  int maxExecTime = 0;
  int bbExecTime;
  int context_index = 0;

  elm::genstruct::SLList<elm::genstruct::SLList<BasicBlock *> *> *seq_list = buildListOfContexts(bb);

  elm::cout << "\nList of contexts:\n";
  for (elm::genstruct::SLList<elm::genstruct::SLList<BasicBlock *> *>::Iterator seq(*seq_list) ; seq ; seq++){
      elm::cout << "\t";
      for (elm::genstruct::SLList<BasicBlock *>::Iterator block(*(seq.item())) ; block ; block++){
	  elm::cout << "b" << block->number() << "-";
      }
      elm::cout << "\n";
  }
  
  for(BasicBlock::InIterator edge(bb); edge; edge++) {
    int index = 0;
    BasicBlock *pred = edge->source();
    elm::cout << "sequence b" << pred->number() << "-b" << bb->number() << ":\n";
    ParExeSequence sequence;
    for(BasicBlock::InstIterator inst(pred); inst; inst++) {
      ParExeInst * eg_inst = 
	new ParExeInst(inst, pred, PROLOGUE, index++);
      sequence.addLast(eg_inst);
    }
    for(BasicBlock::InstIterator inst(bb); inst; inst++) {
      ParExeInst * eg_inst = 
	new ParExeInst(inst, pred, BODY, index++);
      sequence.addLast(eg_inst);
    }
  
    G *execution_graph = new G(_ws,_microprocessor, &sequence, _props);
    execution_graph->build();
    bbExecTime = execution_graph->analyze();
    execution_graph->display(elm::cout);
    if (!_graphs_dir_name.isEmpty()){
      elm::StringBuffer buffer;
      buffer << _graphs_dir_name << "/";
      buffer << bb->number() << "-c" << context_index << ".dot";
      elm::io::OutFileStream dotStream(buffer.toString());
      elm::io::Output dotFile(dotStream);
      execution_graph->dump(dotFile);
    }
    delete execution_graph;
    elm::cout << "\n\twcc = " << bbExecTime << "\n";
    if (bbExecTime > maxExecTime)
      maxExecTime = bbExecTime;
    context_index ++;
  }

  otawa::ipet::TIME(bb) = maxExecTime;

}

 
} //otawa

#endif // GRAPH_BBTIME_H
