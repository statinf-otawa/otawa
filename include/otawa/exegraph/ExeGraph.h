/*
 *	$Id$
 *	exegraph module interface
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
#ifndef __EXEGRAPH_H__
#define __EXEGRAPH_H__

#include <elm/assert.h>
#include <elm/genstruct/Vector.h>
#include <otawa/graph/GenGraph.h>
#include "Microprocessor.h"
#include <otawa/instruction.h>
#include <elm/genstruct/DLList.h>
#include <otawa/otawa.h>
#include <otawa/hard/Platform.h>
#include <otawa/graph/PreorderIterator.h>
#include <otawa/cfg.h>

namespace otawa { 

extern Identifier<bool> START;

template <class N> class ExeGraph;


/* --------------------------
 * class ExeInst
 * --------------------------
 */

template <class N>
class ExeInst {				
 private:
  Inst * _inst;
  BasicBlock *_bb;
  int _index;
  typename ExeGraph<N>::code_part_t _part;
  elm::genstruct::DLList<N *> _nodes;

 public:
  inline ExeInst(Inst * inst, BasicBlock *bb, typename ExeGraph<N>::code_part_t part, int index) 
    : _inst(inst), _bb(bb), _index(index), _part(part) {}
  inline Inst * inst() 
  {return _inst;}
  inline typename ExeGraph<N>::code_part_t codePart() 
  {return _part;}
  inline int index() 
  {return _index;}
  inline void setIndex(int index) 
  {_index=index;}
  inline void addNode(N * node) 
  {_nodes.addLast(node);}
  inline void deleteNodes() 
  {_nodes.clear();}
  inline N * firstNode() 
  {return _nodes.first();}
  inline N * lastNode() 
  {return _nodes.last();}
  inline BasicBlock * basicBlock() 
  {return _bb;}
  class NodeIterator: public elm::genstruct::DLList<N *>::Iterator {
  public:
    inline NodeIterator(const ExeInst *inst)
      : elm::genstruct::DLList<N *>::Iterator(inst->_nodes) {}
  };
 };

/* --------------------------
 * class ExeSequence
 * --------------------------
 */

 template <class N>
   class ExeSequence : public elm::genstruct::DLList<ExeInst<N> *> {
 public:
  class InstIterator: public elm::genstruct::DLList<ExeInst<N> *>::Iterator {
  public:
    inline InstIterator(const ExeSequence *seq)
      : elm::genstruct::DLList<ExeInst<N> *>::Iterator(*seq) {}
  };
  inline int length()
  {return elm::genstruct::DLList<ExeInst<N> *>::count();}
 };


/* --------------------------
 * class ExeGraph
 * --------------------------
 */

 template <class N>
   class ExeGraph  {
 public:
   typedef enum code_part_t {
     BEFORE_PROLOGUE = 0,
     PROLOGUE = 1,
     BODY = 2,
     EPILOGUE = 3,
     CODE_PARTS_NUMBER  // should be the last value
   } code_part_t;
   typedef enum {MIN=0, MAX=1, BOUNDS=2} time_bound_t;
   typedef enum {READY=0, START=1, FINISH=2, STEPS=3} time_step_t;

   class ExeEdge: public GenGraph<N, ExeEdge>::GenEdge {
   public:
     typedef enum edge_type_t {	
       SOLID = 1,
       SLASHED = 2,
       NONE = 3
     } edge_type_t_t;	
   private:
     edge_type_t _type;
     elm::String _name;
   public: 
     inline ExeEdge(N *source, N *target, edge_type_t type)
       : GenGraph<N, ExeEdge>::GenEdge(source,target), _type(type) {
       StringBuffer _buffer;
       _buffer << GenGraph<N, ExeEdge>::GenEdge::source()->name() << "->" ;
       _buffer << GenGraph<N, ExeEdge>::GenEdge::target()->name();
       _name = _buffer.toString();
     }
     inline edge_type_t type(void) 
     {return _type;}
     inline elm::String name()
     {return _name;}
   };

   class ExeNode: public GenGraph<N, ExeEdge >::GenNode {
   private:
     ExeInst<N> *_inst;
     PipelineStage<N> * _pipeline_stage;
     int _latency[BOUNDS];
     bool _needs_operands;
     bool _produces_operands;
     elm::String _name;
   protected:
     elm::genstruct::Vector<N *> _contenders;
   public:
     inline ExeNode(ExeGraph<N> *graph, PipelineStage<N> *stage, ExeInst<N> *inst)
       :   GenGraph<N,ExeEdge>::GenNode((otawa::graph::Graph *)graph->graph()),
       _inst(inst), _pipeline_stage(stage), _needs_operands(false),
       _produces_operands(false)
	 {
	   _latency[MIN] = stage->minLatency();
	   _latency[MAX] = stage->maxLatency();	
	   StringBuffer _buffer;
	   _buffer << stage->shortName() << "(I" << inst->index() << ")";
	   _name = _buffer.toString();
	 }
     inline PipelineStage<N> * pipelineStage(void)
     {return _pipeline_stage;}
     inline ExeInst<N> * inst(void)
     {return _inst;}
     inline int latency()
     {return _latency[MIN];}
     inline int latency(time_bound_t bound) 
     {return _latency[bound];}
     inline void setLatency(time_bound_t bound, int latency)
     {_latency[bound] = latency;}
     inline bool needsOperands(void)
     {return _needs_operands;}
     inline void setNeedsOperands(bool val)
     {_needs_operands = val;}
     inline bool producesOperands(void) 
     {return _produces_operands;}
     inline void setProducesOperands(bool val)
     {_produces_operands = val;}
     inline void addContender(N *cont) 
     {_contenders.add(cont);}
     elm::genstruct::Vector<N *> * contenders()
       {return &_contenders;}
     inline elm::String name()
     {return _name;}
     class ContenderIterator: public elm::genstruct::Vector<N *>::Iterator {
     public:
       inline ContenderIterator(const ExeGraph<N>::ExeNode *node)
	 : elm::genstruct::Vector<N *>::Iterator(node->_contenders) {}
     };
   };

 protected:	
   N * _entry_node;
   N * _first_node[CODE_PARTS_NUMBER];
   N * _last_node[CODE_PARTS_NUMBER];		
   ExeSequence<N> * _sequence;
   Microprocessor<N> * _microprocessor;
   elm::String _part_name[CODE_PARTS_NUMBER];
   typedef struct rename_table_t {
     hard::RegBank * reg_bank;
     elm::genstruct::AllocatedTable<N *> *table;
   } rename_table_t;
   GenGraph<N, ExeEdge> _graph;
 public:
   inline ExeGraph()  {
     StringBuffer _buffer[CODE_PARTS_NUMBER];
     _buffer[BEFORE_PROLOGUE] << "BEFORE_PROLOGUE";
     _buffer[PROLOGUE] << "PROLOGUE";
     _buffer[BODY] << "BODY";
     _buffer[EPILOGUE] << "EPILOGUE";	
     for (int i=0 ; i<CODE_PARTS_NUMBER ; i++)
       _part_name[i] = _buffer[i].toString();
     _entry_node = NULL;
     for (int i=0 ; i<CODE_PARTS_NUMBER ; i++) {
       _first_node[i] = NULL;
       _last_node[i] = NULL;
     }
   }
   inline GenGraph<N, typename ExeGraph<N>::ExeEdge>  * graph()
   {return &_graph;}
   inline void setEntryNode(N *node)
   {_entry_node = node;}
   inline N * entryNode(void)
   {return _entry_node;}
   virtual void build(WorkSpace *fw,  Microprocessor<N> * microprocessor, ExeSequence<N> *sequence);
   inline elm::String partName(code_part_t part)
   {return _part_name[part].toString();}
   inline void setFirstNode(code_part_t part, N *node)
   {_first_node[part] = node;}
   inline void setLastNode(code_part_t part, N *node)
   {_last_node[part] = node;}
   inline N * firstNode(code_part_t part)
   {return _first_node[part];}
   inline N * lastNode(code_part_t part)
   {return _last_node[part];}  

   class PipelineIterator: public Microprocessor<N>::PipelineIterator {
   public:
     inline PipelineIterator(const Microprocessor<N> * processor)
       :  Microprocessor<N>::PipelineIterator(processor) {}
   };
   class FunctionalUnitIterator: 
   public elm::genstruct::Vector<typename PipelineStage<N>::FunctionalUnit *>::Iterator {
   public:
     inline FunctionalUnitIterator(PipelineStage<N> *stage)
       : elm::genstruct::Vector<typename PipelineStage<N>::FunctionalUnit *>::Iterator(stage->getFUs()) {}
   };
   class FunctionalUnitPipelineIterator : public PipelineStage<N>::FunctionalUnit::PipelineIterator {
   public:
     inline FunctionalUnitPipelineIterator(const typename PipelineStage<N>::FunctionalUnit *fu)
       : PipelineStage<N>::FunctionalUnit::PipelineIterator(fu) {}
   };
   class InstIterator : public ExeSequence<N>::InstIterator {
   public:
     inline InstIterator(const ExeSequence<N> *sequence)
       : ExeSequence<N>::InstIterator(sequence) {}
   };
   class InstNodeIterator : public ExeInst<N>::NodeIterator {
   public:
     inline InstNodeIterator(const ExeInst<N> *inst)
       : ExeInst<N>::NodeIterator(inst) {}
   };   
   class StageNodeIterator : public PipelineStage<N>::ExeNodeIterator {
   public:
     inline StageNodeIterator(const PipelineStage<N> *stage)
       : PipelineStage<N>::ExeNodeIterator(stage) {}
   };

   class Predecessor: public PreIterator<Predecessor, N *> {
   public:
     inline Predecessor(const N* node): iter(node) { }
     inline bool ended(void) const { return iter.ended(); }
     inline N *item(void) const { return iter->source(); }
     inline void next(void) { iter.next(); }
     inline ExeEdge *edge(void) const { return iter; }
   private:
     typename GenGraph<N,ExeEdge>::InIterator iter;
   };
		
   class Successor: public PreIterator<Successor, N *> {
   public:
     inline Successor(const N* node): iter(node) {}
     inline bool ended(void) const { return iter.ended(); }
     inline N *item(void) const { return iter->target(); }
     inline void next(void) { iter.next(); }
     inline ExeEdge *edge(void) const { return iter; }
   private:
     typename GenGraph<N,ExeEdge>::OutIterator iter;
   };

   typedef GenGraph<N,typename ExeGraph<N>::ExeEdge> Graph;
   class PreorderIterator: public graph::PreorderIterator<Graph> {
   public:
     inline PreorderIterator(ExeGraph<N> * graph, N *node)
       : graph::PreorderIterator<Graph>(*graph->graph(), node) {}
   }; 	 
 };



 template <class N>
   void ExeGraph<N>::build(WorkSpace *fw, Microprocessor<N> * microprocessor, ExeSequence<N> *sequence) {
   _sequence = sequence;
   _microprocessor = microprocessor;
   // Init rename tables
   otawa::hard::Platform *pf = fw->platform();
   AllocatedTable<rename_table_t> rename_tables(pf->banks().count());
   int reg_bank_count = pf->banks().count();
   for(int i = 0; i <reg_bank_count ; i++) {
     rename_tables[i].reg_bank = (otawa::hard::RegBank *) pf->banks()[i];
     rename_tables[i].table = 
       new AllocatedTable<N *>(rename_tables[i].reg_bank->count());
     for (int j=0 ; j<rename_tables[i].reg_bank->count() ; j++)
       rename_tables[i].table->set(j,NULL);
   }

   // clear node queues for instructions and for pipeline stages
   for (PipelineIterator stage(microprocessor) ; stage ; stage++) {
     stage->deleteNodes();
     if (stage->usesFunctionalUnits()) {
       for(FunctionalUnitIterator fu(stage); fu; fu++)
	 for (FunctionalUnitPipelineIterator fu_stage(fu); fu_stage; fu_stage++)
	   fu_stage->deleteNodes();
     }
   }
   int nb_inst = 0;
   for (InstIterator inst(sequence) ; inst ; inst++) {
     inst->deleteNodes();
     nb_inst++;
   }

   // build nodes
   // consider every pipeline stage
   for (PipelineIterator stage(microprocessor) ; stage ; stage++) {
     code_part_t current_code_part = BEFORE_PROLOGUE;

     // consider every instruction
     for (InstIterator inst(sequence) ; inst ; inst++)  {
       // the instruction before the prologue (if any) is unknown
       // => it deserves a specific treatment
       if (inst->codePart() == BEFORE_PROLOGUE) {
	 N * node = new N(this, stage, inst);
	 node->setLatency(MIN,0);
	 node->setLatency(MAX,0);
	 inst->addNode(node);
	 stage->addNode(node);
	 if (microprocessor->operandProducingStage() == stage) {
	   for (int b=0 ; b<reg_bank_count ; b++) {
	     for (int r=0 ; r < rename_tables[b].reg_bank ->count() ; r++) {
	       rename_tables[b].table->set(r,node);
	     }		
	   }
	 }
	 setFirstNode(BEFORE_PROLOGUE,inst->firstNode());
	 setLastNode(BEFORE_PROLOGUE,inst->lastNode());
       } 
       else {
	 if (!stage->usesFunctionalUnits()) {
	   N * node = new N(this, stage, inst);
	   inst->addNode(node);
	   stage->addNode(node);	
	   if (microprocessor->operandReadingStage() == stage) 
	     node->setNeedsOperands(true);			
	   if (microprocessor->operandProducingStage() == stage) 
	     node->setProducesOperands(true);
	 }
	 else {
	   typename PipelineStage<N>::FunctionalUnit *fu = stage->findFU(inst->inst()->kind()); 
	   bool first_fu_node = true;
	   N * node;
	   for(FunctionalUnitPipelineIterator fu_stage(fu); fu_stage; fu_stage++) {
	     node = new N(this, fu_stage, inst);
	     inst->addNode(node);
	     fu_stage->addNode(node);
	     if (first_fu_node) {
	       stage->addNode(node);
	       if (microprocessor->operandReadingStage() == stage)
		 node->setNeedsOperands(true);
	       first_fu_node = false;
	     }
	   }
	   if (microprocessor->operandProducingStage() == stage)
	     node->setProducesOperands(true);
	 }
	 if (inst->codePart() != current_code_part) {
	   current_code_part = inst->codePart();
	   setFirstNode(current_code_part,inst->firstNode());	 
	 }
	 setLastNode(current_code_part,inst->lastNode());
       }	
     }
   }

   setEntryNode(sequence->first()->firstNode());
   bool trace = false;

   // build edges for pipeline order and data dependencies
   for (InstIterator inst(sequence) ; inst ; inst++)  {
     N * previous = NULL;
     for (InstNodeIterator node (inst) ; node ; node++) {
       if (previous != NULL) {
	 // edge between consecutive pipeline stages
	 new ExeEdge(previous, node, ExeEdge::SOLID);
	 if (trace)
	   elm::cout << "SOLID edge between " << previous->name() << " and " << node->name() << " (pipeline order)\n";
       }
       previous = node;
       if (node->needsOperands()) {
	 // check for data dependencies
	 const elm::genstruct::Table<hard::Register *>& reads = node->inst()->inst()->readRegs();
	 for(int i = 0; i < reads.count(); i++) {
	   for (int b=0 ; b<reg_bank_count ; b++) {
	     if (rename_tables[b].reg_bank == reads[i]->bank()) {
	       N *producing_node = rename_tables[b].table->get(reads[i]->number());
	       if (producing_node != NULL) {
		 // check whether there is already an edge between the two nodes
		 bool exists = false;
		 for (Predecessor pred(node) ; pred ; pred++) {
		   if (pred == producing_node) {
		     exists = true;
		     break;
		   }
		 }
		 if (!exists) {
		   // add an edge for data dependency
		   if (trace)
		     elm::cout << "SOLID edge between " << producing_node->name() << " and " << node->name() << " (data dependency)\n";
		   new ExeEdge(producing_node, node, ExeEdge::SOLID);
		 }
	       }
	     }
	   }
	 }
       }   
       // note that this instruction produces some registers
       if  (node->producesOperands()) {
	 const elm::genstruct::Table<hard::Register *>& writes = 
	   node->inst()->inst()->writtenRegs();
	 for(int i = 0; i < writes.count(); i++) {
	   for (int b=0 ; b<reg_bank_count ; b++) {
	     if (rename_tables[b].reg_bank == writes[i]->bank()) {
	       rename_tables[b].table->set(writes[i]->number(),node);
	     }
	   }
	 }
       }
     }
   }

   // build edges for program order
   bool in_order = false;
   for (PipelineIterator stage(microprocessor) ; stage ; stage++) {
     if (stage->orderPolicy() == PipelineStage<N>::IN_ORDER) {
       if (microprocessor->operandReadingStage() == stage)
	 in_order = true; 
       if (stage->width() == 1) {
	 // scalar stage
	 N * previous = NULL;
	 for (StageNodeIterator node(stage) ; node ; node++) {
	   if (previous != NULL) {				  
	     // scalar stage => draw a solid edge
	     new ExeEdge(previous, node, ExeEdge::SOLID);
	     if (trace)
	       elm::cout << "SOLID edge between " << previous->name() << " and " << node->name() << " (program order)\n";
	   }
	   previous = node;
	 }
       } // end scalar
       else { //superscalar
	 if (stage->category() == PipelineStage<N>::FETCH) {
	   N * previous = NULL; 
	   for (StageNodeIterator node(stage) ; node ; node++) {			
	     // superscalar => draw a slashed edge between adjacent instructions
	     if(previous) {
	       if (previous->inst()->codePart() == BEFORE_PROLOGUE) {
		 new ExeEdge(previous, node, ExeEdge::SOLID);
	       }
	       else {
		 if (node->inst()->inst()->address().offset() != previous->inst()->inst()->address().offset() + 4)
		   new ExeEdge(previous, node, ExeEdge::SOLID);
		 else {
		   if (node->inst()->inst()->address().offset() % (stage->width()*4) == 0)
		     new ExeEdge(previous, node, ExeEdge::SOLID);
		   else
		     new ExeEdge(previous, node, ExeEdge::SLASHED);
		 }
	       }
	     }
	     previous = node;
	   }
	 } // end FETCH
	 else { // superscalar, other than FETCH
	   if (!stage->usesFunctionalUnits())  {
	     elm::genstruct::DLList<N *> previous_nodes;
	     N * previous = NULL;
	     for (StageNodeIterator node(stage) ; node ; node++) {			
	       // superscalar => draw a slashed edge between adjacent instructions	    
	       if(previous){
		 new ExeEdge(previous, node, ExeEdge::SLASHED);
		 if (trace)
		   elm::cout << "SLASHED edge between " << previous->name() << " and " << node->name() << " (stage width)\n";
	       }
	       // draw a solid edge to model the stage width 
	       if (previous_nodes.count() == stage->width()) {
		 new ExeEdge(previous_nodes.first(), node, ExeEdge::SOLID);
		 if (trace)
		   elm::cout << "SOLID edge between " << previous_nodes.first()->name() << " and " << node->name() << " (stage width)\n";
		 previous_nodes.removeFirst();	
	       }
	       previous_nodes.addLast(node);
	       previous = node;
	     }
	   } // no FUs
	   else { //uses FUs
	     for(FunctionalUnitIterator fu(stage); fu; fu++) {
	       PipelineStage<N> *fu_stage = fu->firstStage();
	       elm::genstruct::DLList<N *> previous_nodes;
	       N * previous = NULL;
	       for (StageNodeIterator node(fu_stage) ; node ; node++) {
		 if(previous) {
		   new ExeEdge(previous, node, ExeEdge::SLASHED);
		   if (trace)
		     elm::cout << "SLASHED edge between " << previous->name() << " and " << node->name() << " (stage width)\n";
		 }
		 // draw a solid edge to model the stage width 
		 if (previous_nodes.count() == fu_stage->width()) {
		   new ExeEdge(previous_nodes.first(), node, ExeEdge::SOLID);
		   if (trace)
		     elm::cout << "SOLID edge between " << previous_nodes.first()->name() << " and " << node->name() << " (stage width)\n";
		   previous_nodes.removeFirst();	
		 }
		 previous_nodes.addLast(node);
		 previous = node;
	       }
	     }
	   }
	 }//end superscalar, other than FETCH
       } // end superscalar
     } // end IN ORDER
     else { //stage OOO      
       if (stage->usesFunctionalUnits()) {
	 for(FunctionalUnitIterator fu(stage); fu; fu++) {
	   PipelineStage<N> *fu_stage = fu->firstStage();
	   N * previous_load = NULL;
	   N * previous_store = NULL;
	   for (StageNodeIterator node(fu_stage) ; node ; node++) {			
	     if (node->inst()->inst()->isMem()) { 
	       if (node->inst()->inst()->isLoad()) {
		 if (previous_store) {// memory access are executed in order  
		   new ExeEdge(previous_store, node, ExeEdge::SLASHED);
		   if (trace)
		     elm::cout << "SLASHED edge between " << previous_store->name() << " and " << node->name() << " (load after store)\n";
		 }
		 previous_load = node;
	       }
	       if (node->inst()->inst()->isStore()) {
		 if (previous_store) {// memory access are executed in order
		   new ExeEdge(previous_store, node, ExeEdge::SLASHED);
		   if (trace)
		     elm::cout << "SLASHED edge between " << previous_store->name() << " and " << node->name() << " (store after store)\n";
		 }
		 if (previous_load) {// memory access are executed in order
		   new ExeEdge(previous_load, node, ExeEdge::SLASHED);
		   if (trace)
		     elm::cout << "SLASHED edge between " << previous_load->name() << " and " << node->name() << " (store after load)\n";
		 }
		 previous_store = node;
	       }
	     }
	   }
	 }	
       }
     }
     if (in_order) {
       N * previous = NULL;
       for (InstIterator inst(sequence) ; inst ; inst++)  {
	 for (InstNodeIterator node (inst) ; node ; node++) {
	   if (node->needsOperands()) {
	     if (previous) {
	       new ExeEdge(previous, node, ExeEdge::SLASHED);
	       if (trace)
		 elm::cout << "SLASHED edge between " << previous->name() << " and " << node->name() << " (program order)\n";
	     }
	     previous = node;
	   }
	 }
       }
     }

     // build edges for queues with limited capacity
     if (stage->sourceQueue() != NULL) {	
       Queue<N> *queue = stage->sourceQueue();
       PipelineStage<N> * prod_stage;
       for (PipelineIterator st(microprocessor) ; st ; st++) {
	 if (st->destinationQueue() == queue) {
	   prod_stage = st;
	   break;
	 }
       }
       for (StageNodeIterator node(stage) ; node ; node++) {
	 // compute the index of the instruction that cannot be admitted
	 // into the queue until the current instruction leaves it
	 int index = node->inst()->index() + stage->sourceQueue()->size();
	 for (StageNodeIterator waiting_node(prod_stage) ; waiting_node ; waiting_node++) {
	   if (waiting_node->inst()->index() == index) {
	     new ExeEdge(node, waiting_node, ExeEdge::SLASHED);
	     break;
	   }
	 }
       }
     }
   }

   // search for contending nodes (i.e. pairs of nodes that use the same pipeline stage)
   for (PipelineIterator stage(microprocessor) ; stage ; stage++) {
     if (stage->orderPolicy() == PipelineStage<N>::OUT_OF_ORDER) {	
       if (!stage->usesFunctionalUnits())  {
	 for (StageNodeIterator node1(stage) ; node1 ; node1++) {
	   for (StageNodeIterator node2(stage) ; node2 ; node2++) {
	     if ((N*)node1 != (N*)node2) 
	       node1->addContender(node2);
	   }
	 }
       }
       else {
	 for(FunctionalUnitIterator fu(stage); fu; fu++) {
	   PipelineStage<N> *fu_stage = fu->firstStage();
	   for (StageNodeIterator node1(fu_stage) ; node1 ; node1++) {
	     for (StageNodeIterator node2(fu_stage) ; node2 ; node2++) {
	       if ((N*)node1 != (N*)node2) 
		 node1->addContender(node2);	
	     }
	   }				
	 }
       }
     }
   }	

   // Free rename tables
   for(int i = 0; i <reg_bank_count ; i++)
     delete rename_tables[i].table;
 }


}      

#endif // EXE_GRAPH
