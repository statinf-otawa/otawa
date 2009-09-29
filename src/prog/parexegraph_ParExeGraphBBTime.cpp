/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_TrivialBBTime.cpp --TrivialBBTime class implementation.
 */

#include <otawa/parexegraph/ParExeGraphBBTime.h>
#include <elm/io/OutFileStream.h>

using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;
using namespace otawa::graph;
using namespace otawa::ipet;

namespace otawa { 

Identifier<String> GRAPHS_DIRECTORY("dot graphs directory","");
Identifier<bool> COMPRESSED_CODE("compressed code",false);


ParExeGraphBBTime::ParExeGraphBBTime(const PropList& props) 
  : BBProcessor() {
   _graphs_dir_name = GRAPHS_DIRECTORY(props);
  _compressed_code = COMPRESSED_CODE(props);
  _props(props);
  if (_compressed_code)
    elm::cout << "compressed code !\n";

 }

void ParExeGraphBBTime::processWorkSpace(WorkSpace *ws) {

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
  
void ParExeGraphBBTime::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {

  // ignore empty basic blocks
  if (bb->countInstructions() == 0)
    return;

  int maxExecTime = 0;
  int bbExecTime;
  int context_index = 0;
  
  elm::cout << "================================================================\n";
  elm::cout << "Processing block b" << bb->number() << " (starts at " << bb->address() << " - " << bb->countInstructions() << " instructions)\n\n";

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
  
    ParExeGraph *execution_graph = new ParExeGraph(_ws, _microprocessor, &sequence, _props);
    execution_graph->build();
    bbExecTime = execution_graph->analyze();
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

  TIME(bb) = maxExecTime;

}


  
/**
 * @class ExecutionGraphBBTime
 * This basic block processor computes the basic block execution time using
 * the execution graph method described in the following papers:
 * @li X. Par, A. Roychoudhury, A., T. Mitra, "Modeling Out-of-Order Processors
 * for Software Timing Analysis", Proceedings of the 25th IEEE International
 * Real-Time Systems Symposium (RTSS'04), 2004.
 * @li
 */


/**
 * This property is used to pass the microprocessor description to the
 * code processor. As default, it is extracted from the system description.
 */


/**
 * This property gives an output stream used to output log messages about
 * the execution of the algorithm.
 */
//GenericIdentifier<elm::io::Output *>  ExecutionGraphBBTime::LOG_OUTPUT("otawa.ExecutionGraphBBTime.log", &cerr);


/**
 * Build the ExecutionGraphBBTime processor.
 * @param props	Configuration properties possibly including @ref PROC and
 * @ref LOG.
 */ 



/**
 * Initialize statistics collection for the given basic block.
 * @param bb	Basic block to use.
 */


/**
 * Record statistics for the given sequence.
 * @param sequence	Sequence to use.
 * @param cost		Cost of the sequence.
 */


/**
 * Record the given node in the prefix tree.
 * @param parent	Parent node.
 * @param bb		Current BB.
 * @param cost		Cost of the current prefix.
 * @return			Node matching the given BB.
 */


/**
 * Record statistics for the given node.
 * @param node	Node to record stats for.
 * @param cost	Cost of the current prefix.
 */




/**
 * Call to end the statistics of the current basic block.
 */


/**
 * Record the time delta on the input edges of the current block.
 * @param insts	Prologue.
 * @param cost	Cost of the block with the current prologue.
 * @param bb	Current basic block.
 */



/**
 * If the statistics are activated, this property is returned in statistics
 * property list to store @ref ExecutionGraphBBTime statistics.
 */
//GenericIdentifier<Vector <stat_t> *>
//	EXEGRAPH_PREFIX_STATS("exegraph_prefix_stats", 0, OTAWA_NS);
//
//
///**
// * If set to the true, ExeGraph will also put @ref TIME_DELTA properties on
// * edges. Using the @ref TimeDetaObjectFunctionModifier, it allow to improve
// * the accuracy of the computed WCET.
// */
//GenericIdentifier<bool> EXEGRAPH_DELTA("exegraph_delta", false, OTAWA_NS);
//
//
///**
// * Set to true in the configuration @ref ExecutionGraphBBTime configure, this
// * processor will generate extra contraints and objects functions factor
// * taking in account the difference of execution time according prologues
// * of evaluated blocks.
// */
//GenericIdentifier<bool> EXEGRAPH_CONTEXT("exegraph_context", false, OTAWA_NS);
//

/**
 * Compute extra context to handle context.
 * @param cost	Cost of the BB.
 * @param stat	Statistics node.
 * @param ctx	Context previous node.
 */


//template ExecutionGraphBBTime<ParExeGraph>;

}  // otawa
