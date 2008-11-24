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

#ifndef OTAWA_GENSIM_LAZY_H
#define OTAWA_GENSIM_LAZY_H

#include "PipelineStage.h"
#include "InstructionQueue.h"
#include "SimulatedInstruction.h"
#include <otawa/otawa.h>
#include "SimulationStats.h"
#include <otawa/util/Trace.h>
#include "GenericState.h"

namespace otawa {
namespace gensim {

class GenericState;

class LazyStageStats : public SimulationStats {
	int _number_processed_instructions;
	String _stage_name;
public:
	inline LazyStageStats(String stage_name) :
		_number_processed_instructions(0), _stage_name(stage_name) {
	}
	inline void incrementNumberProcessedInstructions() {
		_number_processed_instructions++;
	}
	virtual inline void reset(void) {
		_number_processed_instructions = 0;
	}
	void dump(elm::io::Output& out_stream);
};

class LazyStageIQIQ : public PipelineStage {
public:
	// signals
	sc_in<bool> in_clock;
	sc_in<SimulatedInstruction *> * in_instruction;
	sc_in<int> in_number_of_ins;
	sc_out<int> out_number_of_accepted_ins;
	sc_out<SimulatedInstruction *> * out_instruction;
	sc_out<int> out_number_of_outs;
	sc_in<int> in_number_of_accepted_outs;

private:
	// variables
	GenericState *sim_state;
	int stage_width;
	Trace *_trace;
	LazyStageStats *stats;
	elm::genstruct::SLList<SimulatedInstruction *> leaving_instructions;
public:
	LazyStageIQIQ(sc_module_name name, int width, GenericState * gen_state,
			Trace *trace);
	inline int stageWidth();

	SC_HAS_PROCESS(LazyStageIQIQ);
	void action();
};

inline int LazyStageIQIQ::stageWidth() {
	return stage_width;
}

}
} // otawa::gensim

#endif // OTAWA_GENSIM_LAZY_H
