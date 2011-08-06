/*
 *	$Id$
 *	EGBBTime class.
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


#include "EGBBTime.h"
#include <elm/io/OutFileStream.h>

using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;
using namespace otawa::graph;

#define TRACE(x) x

namespace otawa { namespace exegraph2 {

Identifier<String> GRAPHS_DIR("otawa::GRAPHS_DIR","");

// -- EGBBTime ------------------------------------------------------------------------------------------


EGBBTime::EGBBTime(EGBBTimeConfig *config, const PropList& props)
	: _config(config), BBProcessor() {
	require(otawa::hard::PROCESSOR_FEATURE);
}

EGBBTime::EGBBTime(const PropList& props)
: _config(NULL), BBProcessor() {
	require(otawa::hard::PROCESSOR_FEATURE);
	require(otawa::hard::CACHE_CONFIGURATION_FEATURE);
	require(otawa::ICACHE_CATEGORY2_FEATURE);
}


EGBBTime::EGBBTime(AbstractRegistration& reg)
: BBProcessor(reg) {
	require(otawa::hard::PROCESSOR_FEATURE);
	require(otawa::hard::CACHE_CONFIGURATION_FEATURE);
	require(otawa::ICACHE_CATEGORY2_FEATURE);
}

void EGBBTime::configure(const PropList& props) {
	BBProcessor::configure(props);

	_graphs_dir_name = GRAPHS_DIR(props);
	if(!_graphs_dir_name.isEmpty())
		_do_output_graphs = true;
	else
		_do_output_graphs = false;
	_props = props;
}

EGBBTime::~EGBBTime(){
	if (!_config || !_config->_block_seq_list_factory)
		delete _block_seq_list_factory;
	if (!_config || !_config->_solver)
		delete _solver;
	if (!_config || !_config->_builder)
		delete _builder ;
}


// -- processWorkSpace ------------------------------------------------------------------------------------------

void EGBBTime::processWorkSpace(WorkSpace *ws) {

	_ws = ws;
	if(otawa::hard::PROCESSOR(_ws) == &otawa::hard::Processor::null)
		throw ProcessorException(*this, "no processor to work with");

	if (!_config || !_config->_block_seq_list_factory)
		_block_seq_list_factory = new EGBlockSeqListFactory();
	else
		_block_seq_list_factory = _config->_block_seq_list_factory;
	if (!_config || !_config->_solver)
		_solver = new EGGenericSolver(ws);
	else
		_solver = _config->_solver;
	if (!_config || !_config->_builder)
		_builder = new EGGenericBuilder(_ws,_solver->nodeFactory(), _solver->edgeFactory());
	else
		_builder = _config->_builder;


	// Perform the actual process
	BBProcessor::processWorkSpace(ws);
}


// -- analyzePathContext ------------------------------------------------------------------------------------------

void EGBBTime::analyzeBlockSequence(EGBlockSeq *block_seq, int seq_index){

	int case_index = 0;
	BasicBlock * bb = block_seq->mainBlock();
	Edge *edge = block_seq->edge();

	TRACE(
		elm::cerr << "---------------\nAnalyzing block sequence ";
		block_seq->dump(elm::cerr);
		elm::cerr << "\n---------------\n";
	)
	ExecutionGraph * graph = new ExecutionGraph(_ws, block_seq);
	_builder->build(graph);
	_solver->solve(graph);
	if (_do_output_graphs){
		const elm::system::Path out_file_name(_ << _graphs_dir_name << "/" << "b" << bb->number() << "-ctxt" << seq_index << "-notime.dot");
		EGGenericDotDisplayFactory display_factory;
		display_factory.newEGDisplay(graph, out_file_name);
		//solver->display();
	}
	delete graph;
}


// -- processBB ------------------------------------------------------------------------------------------

void EGBBTime::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {

	// ignore empty basic blocks
	if (bb->countInstructions() == 0)
		return;

	if(isVerbose()) {
		log << "\n\t\t================================================================\n";
		log << "\t\tProcessing block b" << bb->number() << " (starts at " << bb->address() << " - " << bb->countInstructions() << " instructions)\n";
	}

	int seq_index = 0;

	EGBlockSeqList * seq_list = _block_seq_list_factory->newEGBlockSeqList(bb,_proc->lastStage()->width());

	for (EGBlockSeqList::SeqIterator seq(*seq_list) ; seq ; seq++){
		if(isVerbose()) {
			log << "\n\t\t----- Considering sequence: ";
			seq->dump(log);
			log << "\n";
		}
		analyzeBlockSequence(seq, seq_index);
		seq_index ++;
	}
}
} // namespace exegraph2
} // namespace otawa
