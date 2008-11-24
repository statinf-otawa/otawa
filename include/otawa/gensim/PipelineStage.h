/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
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
#ifndef OTAWA_GENSIM_PIPELINE_STAGE_H
#define OTAWA_GENSIM_PIPELINE_STAGE_H

#include <systemc.h>
#include <elm/genstruct/SLList.h>
#include "InstructionQueue.h"
#include "SimulatedInstruction.h"
#include <otawa/util/Trace.h>
#include "SimulationStats.h"
#include <elm/string/String.h>

namespace otawa {
namespace gensim {

  typedef enum {FETCH,
		EXECUTE_IN_ORDER,
		EXECUTE_OUT_OF_ORDER,
		COMMIT,
		LAZYIQIQ, DECOMP} pipeline_stage_t;

  class PipelineStageConfiguration {
    pipeline_stage_t stage_type;
    InstructionQueueConfiguration * input_queue;
    InstructionQueueConfiguration * output_queue;
    InstructionQueueConfiguration * instruction_buffer;
    CString stage_name;
    int stage_width, in_stage_width, out_stage_width;
  public:
    PipelineStageConfiguration(CString name, pipeline_stage_t type,
			       InstructionQueueConfiguration * inqueue,
			       InstructionQueueConfiguration * outqueue, int in_width,
			       int out_width);
    PipelineStageConfiguration(CString name, pipeline_stage_t type,
			       InstructionQueueConfiguration * inqueue,
			       InstructionQueueConfiguration * outqueue, int width);
    PipelineStageConfiguration(CString name, pipeline_stage_t type,
			       InstructionQueueConfiguration * buffer, int width);

    pipeline_stage_t type();
    CString name();
    InstructionQueueConfiguration * inputQueue();
    InstructionQueueConfiguration * outputQueue();
    InstructionQueueConfiguration * instructionBuffer();
    int width();
    int inWidth();
    int outWidth();
    void dump(elm::io::Output& out_stream);
  };

  class PipelineStage: public sc_module {
  public:
    PipelineStage(sc_module_name name) {
    }
    ;
    bool isEmpty() {
      return true; // by default, pipeline stages do not retain instructions
    }
  };

}
} // otawa::gensim

#endif // OTAWA_GENSIM_PIPELINE_STAGE_H

