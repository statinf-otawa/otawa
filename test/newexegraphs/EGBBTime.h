/*
 *	$Id$
 *	Interface to the EGBBTime classes.
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

#ifndef _EG_BBTIME_H_
#define _EG_BBTIME_H_

#include <otawa/prop/Identifier.h>
#include <otawa/cfg.h>
#include "ExecutionGraph.h"
#include <elm/io/OutFileStream.h>
#include <otawa/proc/BBProcessor.h>
#include "EGBlockSeqList.h"


namespace otawa { namespace newexegraph {
    extern Identifier<String> GRAPHS_DIR;
 //   extern Identifier<int> TIME;

    using namespace elm::genstruct;

    // -- class PathContext --------------------------------------------------------------------------------
    class EGPathContext{
    private:
		elm::genstruct::SLList<BasicBlock *> _bb_list;
		int _num_insts;
		int _num_bbs;
		BasicBlock * _bb;
		Edge * _edge;
    public:
		EGPathContext(BasicBlock *bb){
			_bb_list.addFirst(bb);
			_num_insts = bb->countInstructions();
			_num_bbs = 1;
			_bb = bb;
			_edge = NULL;
		}
		EGPathContext(const EGPathContext& ctxt){
			for (elm::genstruct::SLList<BasicBlock *>::Iterator block(ctxt._bb_list) ; block ; block++)
				_bb_list.addLast(block);
			_num_insts = ctxt._num_insts;
			_num_bbs = ctxt._num_bbs;
			_bb = ctxt._bb;
			_edge = ctxt._edge;
		}
		~EGPathContext(){
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
			inline BasicBlockIterator(const EGPathContext& ctxt)
				: elm::genstruct::SLList<BasicBlock *>::Iterator(ctxt._bb_list) {}
		};

    };


    // -- class EGBBTime ----------------------------------------------------------------------------------

    template <class G>
		class EGBBTime: public BBProcessor {
    private:
		WorkSpace *_ws;
		EGProc *_microprocessor;
		int _last_stage_cap;
		PropList _props;
		int _prologue_depth;
		OutStream *_output_stream;
		elm::io::Output *_output;
		String _graphs_dir_name;
		bool _do_output_graphs;
		EGBlockSeqListFactory * _block_seq_list_factory;

    public:
		EGBBTime(EGBlockSeqListFactory * block_seq_list_factory,
				const PropList& props = PropList::EMPTY);
		EGBBTime(const PropList& props = PropList::EMPTY);
		EGBBTime(AbstractRegistration& reg);
		virtual void configure(const PropList& props);

		void processWorkSpace(WorkSpace *ws);
		void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
		elm::genstruct::SLList<EGPathContext *> * buildListOfPathContexts(BasicBlock *bb, int depth = 1);
		void FillSequence(EGPathContext *ctxt,
						  elm::genstruct::SLList<EGPathContext *> *context_list,
						  int depth);
		EGSequence * buildSequence(EGPathContext *ctxt);
		void analyzePathContext(EGPathContext *ctxt, int context_index);
		void outputGraph(G* graph, int bb_number, int context_number, int case_number, const string& info = "");

    };
	// -- EGBBTime ------------------------------------------------------------------------------------------


	template <class G>
		EGBBTime<G>::EGBBTime(const PropList& props)
		: BBProcessor() {
		require(otawa::hard::PROCESSOR_FEATURE);
		_block_seq_list_factory = new EGBlockSeqListFactory();
	}

	template <class G>
		EGBBTime<G>::EGBBTime(EGBlockSeqListFactory * block_seq_list_factory,
								const PropList& props)
		: BBProcessor() {
		require(otawa::hard::PROCESSOR_FEATURE);
		_block_seq_list_factory = block_seq_list_factory;
	}

template <class G>
EGBBTime<G>::EGBBTime(AbstractRegistration& reg)
: BBProcessor(reg) {
	require(otawa::hard::PROCESSOR_FEATURE);
}

template <class G>
void EGBBTime<G>::configure(const PropList& props) {
	BBProcessor::configure(props);
	_graphs_dir_name = GRAPHS_DIR(props);
	if(!_graphs_dir_name.isEmpty())
		_do_output_graphs = true;
	else
		_do_output_graphs = false;
	_props = props;
}



	// -- processWorkSpace ------------------------------------------------------------------------------------------

	template <class G>
		void EGBBTime<G>::processWorkSpace(WorkSpace *ws) {

		_ws = ws;
		const otawa::hard::Processor *proc = otawa::hard::PROCESSOR(_ws);

		if(proc == &otawa::hard::Processor::null)
			throw ProcessorException(*this, "no processor to work with");
		else {
			_microprocessor = new EGProc(proc);
			_last_stage_cap = _microprocessor->lastStage()->width();

		}

		// Perform the actual process
		BBProcessor::processWorkSpace(ws);
	}


	// -- FillSequence ------------------------------------------------------------------------------------------

	template <class G>
		void EGBBTime<G>::FillSequence(EGPathContext *ctxt,
										  elm::genstruct::SLList<EGPathContext *> *context_list,
										  int depth){

		BasicBlock *bb = ctxt->lastBlock();
		int num_preds = 0;
		for(BasicBlock::InIterator edge(bb); edge; edge++) {
			BasicBlock *pred = edge->source();
			if (!pred->isEntry() && !pred->isExit()) {
				num_preds++;
				EGPathContext *new_ctxt = new EGPathContext(*ctxt);
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
		elm::genstruct::SLList<EGPathContext *> * EGBBTime<G>::buildListOfPathContexts(BasicBlock *bb, int depth){
		assert(depth > 0);
		elm::genstruct::SLList<EGPathContext *> * context_list = new elm::genstruct::SLList<EGPathContext *>();
		EGPathContext * ctxt = new EGPathContext(bb);

		FillSequence(ctxt, context_list, depth);

		return context_list;
	}

	// -- buildSequence ------------------------------------------------------------------------------------------

	template <class G>
		EGSequence * EGBBTime<G>::buildSequence(EGPathContext *ctxt){
		EGSequence * seq = new EGSequence();
		part_t part = PREDECESSOR;
		int index = 0;
		for (EGPathContext::BasicBlockIterator block(*ctxt) ; block ; block++){
			if (block == ctxt->mainBlock())
				part = BLOCK;
			for(BasicBlock::InstIterator inst(block); inst; inst++) {
				EGInst * par_exe_inst = new EGInst(inst, block, part, index++);
				seq->add(par_exe_inst);
			}
		}
		return seq;
	}

	// -- outputGraph ------------------------------------------------------------------------------------------

	template <class G>
		void EGBBTime<G>::outputGraph(G* graph, int bb_number, int context_index, int case_index, const string& info){
		elm::StringBuffer buffer;
		buffer << _graphs_dir_name << "/";
		buffer << "b" << bb_number << "-ctxt" << context_index << "-case" << case_index << ".dot";
		elm::io::OutFileStream dotStream(buffer.toString());
		elm::io::Output dotFile(dotStream);
		graph->dump(dotFile, info);
	}


	class test : public EGNodeFactory{

	};
	// -- analyzePathContext ------------------------------------------------------------------------------------------

	template <class G>
		void EGBBTime<G>::analyzePathContext(EGPathContext*ctxt, int context_index){

		int case_index = 0;
		BasicBlock * bb = ctxt->mainBlock();
		Edge *edge = ctxt->edge();


		EGSequence *sequence = buildSequence(ctxt);
		test * testObj = new test();
		G *execution_graph = new G(_ws,_microprocessor, sequence, testObj, _props);
		execution_graph->build();

		if (_do_output_graphs){
			outputGraph(execution_graph, bb->number(), context_index, case_index++,
				_ << "");
		}


		delete execution_graph;
	}


	// -- processBB ------------------------------------------------------------------------------------------

	template <class G>
		void EGBBTime<G>::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {

		// ignore empty basic blocks
		if (bb->countInstructions() == 0)
			return;

		if(isVerbose()) {
			log << "\n\t\t================================================================\n";
			log << "\t\tProcessing block b" << bb->number() << " (starts at " << bb->address() << " - " << bb->countInstructions() << " instructions)\n";
		}

		int context_index = 0;

		EGBlockSeqList * seq_list = _block_seq_list_factory->newEGBlockSeqList(bb,_microprocessor);

		for (EGBlockSeqList::SeqIterator seq(*seq_list) ; seq ; seq++){
			if(isVerbose()) {
				log << "\n\t\t----- Considering context: ";
				seq->dump(log);
				log << "\n";
			}
			//analyzePathContext(ctxt, context_index);
			context_index ++;
		}
	}

} // namespace newexegraph
} // namespace otawa

#endif // _EG_BBTIME_H_




