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
		EGInstSeq * buildSequence(EGBlockSeq *bseq);
		void analyzePathContext(EGBlockSeq *seq, int context_index);
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



	// -- buildSequence ------------------------------------------------------------------------------------------

	template <class G>
		EGInstSeq * EGBBTime<G>::buildSequence(EGBlockSeq *bseq){
		EGInstSeq * seq = new EGInstSeq();
		part_t part = PREDECESSOR;
		int index = 0;
		for (EGBlockSeq::BasicBlockIterator block(*bseq) ; block ; block++){
			if (block == bseq->mainBlock())
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


	// -- analyzePathContext ------------------------------------------------------------------------------------------

	template <class G>
		void EGBBTime<G>::analyzePathContext(EGBlockSeq *bseq, int context_index){

		int case_index = 0;
		BasicBlock * bb = bseq->mainBlock();
		Edge *edge = bseq->edge();


		EGInstSeq *sequence = buildSequence(bseq);
		G *execution_graph = new G(_ws,_microprocessor, sequence, NULL, _props);
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
			analyzePathContext(seq, context_index);
			context_index ++;
		}
	}

} // namespace newexegraph
} // namespace otawa

#endif // _EG_BBTIME_H_




