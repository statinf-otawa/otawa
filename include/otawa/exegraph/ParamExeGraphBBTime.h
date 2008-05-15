/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	ExeGraphBBTime.h -- ExeGraphBBTime class interface.
 */
#ifndef PARAM_EXECUTIONGRAPH_BBTIME_H
#define PARAM_EXECUTIONGRAPH_BBTIME_H

#include "ExeGraphBBTime.h"
#include "ParamExeGraph.h"
#include <otawa/ipet.h>
#include <otawa/prop/Identifier.h>



namespace otawa {

extern Identifier<String> OUTPUT_FILE;
extern Identifier<int> OUTPUT_LEVEL;
extern Identifier<String> GRAPHS_DIR;

	
using namespace elm::genstruct; 
	
class ParamExeGraphBBTime: public ExeGraphBBTime<ParamExeGraph> {
 private:
  int _prologue_depth;
  OutStream *_output_stream;
  elm::io::Output *_output;
  String _graphs_dir_name;
  Trace *_trace;
 public:
  ParamExeGraphBBTime(const PropList& props = PropList::EMPTY, int prologue_depth=0);
  void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
  void buildPrologueList(BasicBlock * bb,
			 ExeSequence<ExeNode> * prologue, 
			 int capacity, 
			 elm::genstruct::DLList<ExeSequence<ExeNode> *> * prologue_list,
			 int depth);
  class InstIterator : public ParamExeGraph::InstIterator {
  public:
    inline InstIterator(const ExeSequence<ExeNode> * sequence)
      : ParamExeGraph::InstIterator(sequence) {}
  };
};
 
 
 
} //otawa

#endif // ParamExeGraphBBTime_H
