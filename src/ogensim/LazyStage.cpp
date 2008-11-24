/*
 *	$Id$
 *	exegraph module implementation
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

#include <otawa/gensim/LazyStage.h>

namespace otawa {
namespace gensim {

void LazyStageStats::dump(elm::io::Output& out_stream) {
	out_stream << "\n---- Stats from the " << _stage_name << " stage ----\n";
	out_stream << "number of processed instructions: "
			<< _number_processed_instructions << "\n";
}

LazyStageIQIQ::LazyStageIQIQ(sc_module_name name, int width,
		GenericState * gen_state, Trace *trace) :
  PipelineStage(name), sim_state(gen_state) {
	stage_width = width;
	_trace = trace;
	stats = new LazyStageStats((String) name);
	gen_state->addStats(stats);
	in_instruction = new sc_in<SimulatedInstruction *>[width];
	out_instruction = new sc_out<SimulatedInstruction *>[width];

	SC_METHOD(action);
	sensitive_pos << in_clock;
}

void LazyStageIQIQ::action() {
	*_trace << L5 << "[" << name() << " stage] (cycle " << sim_state->cycle()
			<<")\n";
	*_trace << L8 << "\t\tnumber of insts accepted by the destination queue = "
			<< in_number_of_accepted_outs.read() << "\n";
	for (int i =0; i<in_number_of_accepted_outs.read() ; i++) {
		leaving_instructions.removeFirst();
	}
	int accepted_ins = stage_width - leaving_instructions.count();
	out_number_of_accepted_ins.write(accepted_ins);
	*_trace << L8 << "\t\tnumber of insts accepted from the source queue = "
			<< accepted_ins << "\n";
	*_trace << L7 << "\t\tnumber of insts provided by the source queue = "
			<< in_number_of_ins.read() << "\n";
	for (int i=0; (i<in_number_of_ins.read()) && (i<accepted_ins); i++) {
		*_trace << L6 << "\tprocessing " << in_instruction[i].read()->dump() << "\n";
		leaving_instructions.addLast(in_instruction[i].read());
		stats->incrementNumberProcessedInstructions();
	}
	int outs = 0;
	for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator
			inst(leaving_instructions); inst; inst++) {
		if (outs <= stage_width)
			out_instruction[outs++].write(inst);
	}
	out_number_of_outs.write(outs);
	*_trace << L8
			<< "\t\tnumber of insts submitted to the destination queue = "
			<< outs << "\n";
}

}
} // otawa::gensim
