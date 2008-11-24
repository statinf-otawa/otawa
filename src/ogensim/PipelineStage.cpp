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

#include <otawa/gensim/PipelineStage.h>
#include <otawa/gensim/debug.h>

namespace otawa {
namespace gensim {

/**
 * Build a pipeline stage configuration for stages between two queues with
 * different input/output widths.
 * @param name	Stage name.
 * @param type	Stage type.
 * @param inqueue	Input queue.
 * @param outqueue	Output queue.
 * @param in_width	Input width.
 * @param out_width	Output width.
 */
PipelineStageConfiguration::PipelineStageConfiguration(CString name,
		pipeline_stage_t type, InstructionQueueConfiguration * inqueue,
		InstructionQueueConfiguration * outqueue, int in_width, int out_width) :
	stage_type(type), input_queue(inqueue == outqueue ? 0 : inqueue),
			output_queue(inqueue == outqueue ? 0 : outqueue),
			instruction_buffer(inqueue == outqueue ? inqueue : 0),
			stage_name(name), stage_width(out_width == 0 ? in_width : 0),
			in_stage_width(out_width == 0 ? 0 : in_width),
			out_stage_width(out_width == 0 ? 0 : in_width) {
	assert(inqueue || outqueue || instruction_buffer);
	assert(stage_width != 0 || in_stage_width || out_stage_width);
	assert(in_stage_width == 0 || inqueue);
	assert(out_stage_width == 0 || outqueue);
	if (input_queue) {
		input_queue->setNumberOfReadPorts(in_width);
	}
	if (output_queue) {
		output_queue->setNumberOfWritePorts(out_width ? out_width : stage_width);
	}
}

/**
 * Build a pipeline stage configuration for stages between two queues.
 * @param name		Stage name.
 * @param type		Stage type.
 * @param inqueue	Input queue (may be null).
 * @param outqueue	Outut queue (may be null).
 * @param width		Number of ports with queues.
 */
/*PipelineStageConfiguration::PipelineStageConfiguration(
 CString name,
 pipeline_stage_t type,
 InstructionQueueConfiguration *inqueue,
 InstructionQueueConfiguration * outqueue,
 int width
 )
 :	stage_type(type),
 input_queue(inqueue),
 output_queue(outqueue),
 instruction_buffer(0),
 stage_name(name),
 stage_width(width),
 in_stage_width(0),
 out_stage_width(0)
 {
 if (inqueue)
 inqueue->setNumberOfReadPorts(width);
 if (outqueue)
 outqueue->setNumberOfWritePorts(width);
 }*/

/**
 * Build a stage configuration for a stage working inside a queue.
 * @param name		Stage name.
 * @param type		stage type.
 * @param buffer	Used instruction queue.
 * @param width		Number of ports with the queue.
 */
/*PipelineStageConfiguration::PipelineStageConfiguration(
 CString name,
 pipeline_stage_t type,
 InstructionQueueConfiguration * buffer,
 int width
 )
 :	stage_type(type),
 input_queue(0),
 output_queue(0),
 instruction_buffer(buffer),
 stage_name(name),
 stage_width(width),
 in_stage_width(0),
 out_stage_width(0)
 {
 assert(buffer);
 }*/

pipeline_stage_t PipelineStageConfiguration::type() {
	return stage_type;
}

CString PipelineStageConfiguration::name() {
	return stage_name;
}

InstructionQueueConfiguration * PipelineStageConfiguration::inputQueue() {
	return input_queue;
}

InstructionQueueConfiguration * PipelineStageConfiguration::outputQueue() {
	return output_queue;
}

InstructionQueueConfiguration * PipelineStageConfiguration::instructionBuffer() {
	return instruction_buffer;
}

int PipelineStageConfiguration::width() {
	return stage_width;
}

void PipelineStageConfiguration::dump(elm::io::Output& out_stream) {
	out_stream << stage_name;
	switch (stage_type) {
	case FETCH:
		out_stream << " (FETCH) ";
		out_stream << "- width=" << stage_width << " ";
		out_stream << "- outqueue=" << output_queue->name() << "\n";
		break;
	case EXECUTE_IN_ORDER:
		out_stream << " (EXECUTE_IN_ORDER) ";
		out_stream << "- width=" << stage_width << " ";
		out_stream << "- inqueue=" << input_queue->name() << "\n";
		break;
	case EXECUTE_OUT_OF_ORDER:
		out_stream << " (EXECUTE_OUT_OF_ORDER) ";
		out_stream << "- width=" << stage_width << " ";
		out_stream << "- rob=" << instruction_buffer->name() << "\n";
		break;
	case COMMIT:
		out_stream << " (COMMIT) ";
		out_stream << "- width=" << stage_width << " ";
		out_stream << "- inqueue=" << input_queue->name() << "\n";
		break;
	case LAZYIQIQ:
		out_stream << " (LAZYIQIQ) ";
		out_stream << "- width=" << stage_width << " ";
		out_stream << "- inqueue=" << input_queue->name() << "\n";
		out_stream << "- outqueue=" << output_queue->name() << "\n";
		break;
	case DECOMP:
		out_stream << " (DECOMP) ";
		out_stream << "- width=" << stage_width << " ";
		out_stream << "- inqueue=" << input_queue->name() << "\n";
		out_stream << "- outqueue=" << output_queue->name() << "\n";
		break;}
}

}
} // otawa::gensim

