#include <otawa/gensim/PipelineStage.h>
#include <otawa/gensim/debug.h>

namespace otawa { namespace gensim {

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
PipelineStageConfiguration::PipelineStageConfiguration(
	CString name,
	pipeline_stage_t type,
	InstructionQueueConfiguration * inqueue,
	InstructionQueueConfiguration * outqueue,
	int in_width, 
	int out_width
)
:	stage_type(type),
	input_queue(inqueue == outqueue ? 0 : inqueue),
	output_queue(inqueue == outqueue ? 0 : outqueue),
	instruction_buffer(inqueue == outqueue ? inqueue : 0),
	stage_name(name),
	stage_width(out_width == 0 ? in_width : 0),
	in_stage_width(out_width == 0 ? 0 : in_width),
	out_stage_width(out_width == 0 ? 0 : in_width)
{
		assert(inqueue || outqueue || instruction_buffer);
		assert(stage_width != 0 || in_stage_width || out_stage_width);
		assert(in_stage_width == 0 || inqueue);
		assert(out_stage_width == 0 || outqueue);
		//elm::cout << "stage " << name;
		if(input_queue) {
			//elm::cout << " << " << input_queue->name();
			input_queue->setNumberOfReadPorts(in_width);
		}
		if(output_queue) {
			//elm::cout << " >> " << output_queue->name();
			output_queue->setNumberOfWritePorts(out_width ? out_width : stage_width);
		}
		//elm::cout << io::endl;
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
	switch(stage_type) {
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
	}
}

LazyStageIQIQ::LazyStageIQIQ(sc_module_name name, int width) 
	: PipelineStage(name) {
	stage_width = width;
	in_instruction = new sc_in<SimulatedInstruction *>[width];
	out_instruction = new sc_out<SimulatedInstruction *>[width];
	
	SC_METHOD(action);
	sensitive_pos << in_clock;
}

void LazyStageIQIQ::action() {
	TRACE(elm::cout << "LazyStage->action(): \n";)
	TRACE(elm::cout << "\tin_number_of_accepted_outs=" << in_number_of_accepted_outs.read() << "\n";)
	for (int i =0 ; i<in_number_of_accepted_outs.read() ; i++) {
		leaving_instructions.removeFirst();
	}
	int accepted_ins = stage_width - leaving_instructions.count();
	out_number_of_accepted_ins.write(accepted_ins);
	TRACE(elm::cout << "\tout_number_of_accepted_ins=" << accepted_ins << "\n";)	
	TRACE(elm::cout << "\tin_number_of_ins=" << in_number_of_ins.read() << "\n";)
	for (int i=0 ; (i<in_number_of_ins.read()) && (i<accepted_ins) ; i++) {
		TRACE(elm::cout << "\thandling at " << in_instruction[i].read()->inst()->address() << "\n";)
		leaving_instructions.addLast(in_instruction[i].read());
	}	
	int outs = 0;
	for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator inst(leaving_instructions) ; inst ; inst++) {
		if (outs <= stage_width) // FIXME
			out_instruction[outs++].write(inst);
	}
	out_number_of_outs.write(outs);
	TRACE(elm::cout << "\tout_number_of_outs=" << outs << "\n";)
}

} } // otawa::gensim

