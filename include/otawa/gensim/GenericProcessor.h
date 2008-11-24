/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
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

#ifndef OTAWA_GENSIM_GENERIC_PROCESSOR_H
#define OTAWA_GENSIM_GENERIC_PROCESSOR_H

#include <elm/genstruct/SLList.h>
#include <systemc.h>
#include "PipelineStage.h"
#include "Fetch.h"
#include "Execute.h"
#include "LazyStage.h"
#include "Memory.h"
#include "SimulatedInstruction.h"
#include <otawa/gensim/Cache.h>
#include <otawa/prop/Identifier.h>
#include "SimulationStats.h"
#include <otawa/util/Trace.h>
#include <elm/io/Output.h>
#include <elm/system/Path.h>
#include <otawa/hard/CacheConfiguration.h>
#include <elm/io/OutFileStream.h>

namespace otawa {
namespace gensim {

 extern Identifier<String> TRACE_FILE_PATH; 
 extern Identifier<int> TRACE_LEVEL; 

// External class
class GenericState;

class ProcessorStats : public SimulationStats {
	int _time;
public:
	inline ProcessorStats() :
		_time(0) {
	}
	inline void incrementTime() {
		_time++;
	}
	virtual inline void reset(void) {
		_time = 0;
	}
	void dump(elm::io::Output& out_stream);
};

// ProcessorConfiguration class
class ProcessorConfiguration {
	elm::genstruct::SLList<PipelineStageConfiguration *> pipeline_stages;
	elm::genstruct::SLList<InstructionQueueConfiguration *> instruction_queues;
	elm::genstruct::SLList<FunctionalUnitConfiguration *> functional_units;

public:
	elm::genstruct::Vector<elm::Pair<Inst::kind_t, FunctionalUnitConfiguration *> >
			fu_bindings;
	void addBinding(Inst::kind_t kind, FunctionalUnitConfiguration *fu_conf) {
		fu_bindings.add(Pair<Inst::kind_t, FunctionalUnitConfiguration *>(kind,
				fu_conf));
	}
	void addPipelineStage(PipelineStageConfiguration * new_stage) {
		pipeline_stages.addLast(new_stage);
	}
	void addInstructionQueue(InstructionQueueConfiguration * new_queue) {
		instruction_queues.addLast(new_queue);
	}
	void addFunctionalUnit(FunctionalUnitConfiguration * new_fu) {
		functional_units.addLast(new_fu);
	}
	elm::genstruct::SLList<InstructionQueueConfiguration *> * instructionQueuesList() {
		return &instruction_queues;
	}
	elm::genstruct::SLList<PipelineStageConfiguration *> * pipelineStagesList() {
		return &pipeline_stages;
	}
	elm::genstruct::SLList<FunctionalUnitConfiguration *> * functionalUnitsList() {
		return &functional_units;
	}

	void dump(elm::io::Output& out_stream);
};

SC_MODULE(GenericProcessor) {
	// signals
	sc_signal<bool> clock;

	// variables
	elm::genstruct::SLList<InstructionQueue *> instruction_queues;
	elm::genstruct::SLList<PipelineStage *> pipeline_stages;
	elm::genstruct::AllocatedTable<rename_table_t> * rename_tables;
	elm::genstruct::SLList<SimulatedInstruction *> active_instructions;
	ProcessorStats stats;
	io::OutStream *_trace_stream;
	io::Output* _trace_file;
	Trace *_trace;
	int _trace_level;
	bool _ended;

public:
	bool isEmpty();
	void step();
	void Flush();

	GenericProcessor(sc_module_name name, ProcessorConfiguration * conf,
			GenericState * sim_state, otawa::hard::Platform *pf);

	GenericProcessor(sc_module_name name, ProcessorConfiguration * conf,
			GenericState * sim_state, otawa::hard::Platform *pf,
			String trace_file_path, int trace_level);
	~GenericProcessor() {
	}

	void build(ProcessorConfiguration * conf, GenericState * sim_state,
			otawa::hard::Platform *pf, int trace_level);

}
;

}
} // otawa::gensim

#endif // OTAWA_GENSIM_GENERIC_PROCESSOR_H
