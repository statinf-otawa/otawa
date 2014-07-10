/*
 *	$Id$
 *	hard::Processor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-10, IRIT UPS.
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

#include <otawa/hard/Processor.h>
#include <otawa/hard/ProcessorBuilder.h>
#include <otawa/manager.h>
#include <elm/serial2/XOMUnserializer.h>

namespace otawa { namespace hard {

/**
 * @class Processor
 * Description of a processor pipeline. A pipeline is viewed as a collection
 * of stages (@ref otawa::hard::Stage) separated by instructions queues
 * (@ref otawa::hard::Queue).
 * @ingroup hard
 */

/**
 * @class Stage
 * @ingroup hard
 * 
 * This class represents a stage in a pipeline. The stages have a type:
 * @li @ref otawa::hard::Stage::FETCH -- a stage loading instruction from memory
 * (usually the initial stage of a pipeline).
 * @li @ref otawa::hard::Stage::LAZY -- a simple stage that is traversed by instructions,
 * @li @ref otawa::hard::Stage::EXEC -- a stage that performs execution of instructions
 * (it contains functionnal units that performs the work of the instructions),
 * @li @ref otawa::hard::Stage::COMMIT -- a stage that writes result of an instruction
 * in the processor state (usually the end of the traversal of the instructions).
 * 
 * The stages are described by the following attributes:
 * @li the name,
 * @li the width (number of instructions traversing in parallel the stage),
 * @li the latency (number of cycles taken to traverse the stage),
 * @li the order (is the instructions traversal perform in program order or not),
 * @li for the EXEC stage only, a functionnal unit list (@ref otawa::hard::FunctionalUnit)
 * and dispatch list (@ref otawa::hard::Dispatch).
 */

/**
 * @class Queue
 * @ingroup hard
 * 
 * The instructions queues stores instruction that come from one stage to another one.
 * Possibly, an instruction queue may have an internal execution stage that uses
 * the stored instruction to compute their result.
 */

/**
 * @class FunctionalUnit
 * @ingroup hard
 * 
 * A functional unit is specialized in the computation of some kinds of instructions.
 * They are contained in a stage (@ref otawa::hard::Stage) of type EXEC. 
 * The selection of a functional unit is performed thanks to the @ref otawa::hard::Dispatch
 * objects provided in the execution stage.
 * 
 * A functional unit is defined by:
 * @li its latency (computation time in cycles),
 * @li its width (number of instructions traversing the functional unit in parallel),
 * @li pipeline property indicating that the function unit is pipelined or not
 * (that is it may chain instruction execution cycles after cycles).
 */

/**
 * @class Dispatch
 * @ingroup hard
 * 
 * A dispatch object allows to map some kinds of instructions to a functional unit.
 * To find if the dispatch match an instruction, an AND is performed between the
 * instruction kind and the dispatch type and if the result is equal to the dispatch
 * type, this dispatch functional unit is selected. This means that the dispatched
 * functional unit only process instructions that meets the all properties found
 * in the dispatch kind.
 */


const Processor Processor::null;


/**
 * Load a processor descriptor from the given file.
 * @param path	Path to the file.
 * @throw LoadException 	Thrown if an error is found.
 */
hard::Processor *Processor::load(const elm::system::Path& path) throw(LoadException) {
	Processor *_processor = new Processor();
	try {
		elm::serial2::XOMUnserializer unser(&path);
		unser >> *_processor;
		unser.flush();
		return _processor;
	}
	catch(elm::io::IOException& e) {
		delete _processor;
		throw LoadException(&e.message());
	}
}


/**
 * Load a processor description from an XML element.
 * @param element			XML element to load from.
 * @throw LoadException 	Thrown if an error is found.
 */
hard::Processor *Processor::load(xom::Element *element) throw(LoadException) {
	Processor *_processor = new Processor();
	try {
		elm::serial2::XOMUnserializer unser(element);
		unser >> *_processor;
		unser.flush();
		return _processor;
	}
	catch(elm::Exception& e) {
		delete _processor;
		throw LoadException(&e.message());
	}
}


/**
 * Attempt to obtain a processor pipeline description.
 * It looks the following elements from its configuration property list (in the given order):
 * @li @ref PROCESSOR
 * @li @ref PROCESSOR_ELEMENT (unserialize the processor description from the given  XML element)
 * @li @ref PROCESSOR_PATH (get the processor description from the given file)
 *
 * @par Provided Features
 * @li @ref PROCESSOR_FEATURE
 */
class ProcessorProcessor: public otawa::Processor {
public:
	static p::declare reg;
	ProcessorProcessor(p::declare& r = reg): Processor(r), xml(0), config(0) { }

	virtual void configure(const PropList& props) {
		Processor::configure(props);
		config = otawa::PROCESSOR(props);
		if(!config) {
			xml = PROCESSOR_ELEMENT(props);
			if(!xml)
				path = PROCESSOR_PATH(props);
		}
	}

protected:
	virtual void processWorkSpace(WorkSpace *ws) {
		if(config) {
			hard::PROCESSOR(ws) = config;
			if(logFor(LOG_DEPS))
				log << "\tcustom processor configuration\n";
		}
		else if(xml) {
			config = hard::Processor::load(xml);
			track(PROCESSOR_FEATURE, hard::PROCESSOR(ws) = config);
			if(logFor(LOG_DEPS))
				log << "\tprocessor configuration from XML element\n";
		}
		else if(path) {
			if(logFor(LOG_DEPS))
				log << "\tprocessor configuration from \"" << path << "\"\n";
			config = hard::Processor::load(path);
			track(PROCESSOR_FEATURE, hard::PROCESSOR(ws) = config);
		}
		else if(logFor(LOG_DEPS))
			log << "\tno processor configuration\n";
	}

private:
	const hard::Processor *config;
	xom::Element *xml;
	Path path;
};

p::declare ProcessorProcessor::reg = p::init("otawa::ProcessorProcessor", Version(1, 0, 0))
	.provide(PROCESSOR_FEATURE)
	.maker<ProcessorProcessor>();


static SilentFeature::Maker<ProcessorProcessor> maker;
/**
 * This feature ensures we have obtained the memory configuration
 * of the system.
 *
 * @par Properties
 * @li @ref otawa::hard::PROCESSOR
 */
SilentFeature PROCESSOR_FEATURE("otawa::hard::PROCESSOR_FEATURE", maker);


/**
 * Current memory.
 *
 * @par Hooks
 * @li @ref otawa::WorkSpace
 *
 * @par Features
 * @li @ref otawa::CACHE_CONFIGURATION_FEATURE
 *
 * @par Default Value
 * Cache configuration without any cache (never null).
 */
Identifier<const hard::Processor *> PROCESSOR("otawa::hard::PROCESSOR", &Processor::null);


/**
 */
void QueueBuilder::complete(void) {
	if(completed)
		return;
	completed = true;
	if(!interns.isEmpty()) {
		queue->intern.allocate(interns.length());
		for(int i = 0; i < interns.length(); i++)
			queue->intern[i] = interns[i];
	}
}


/**
 */
void StageBuilder::complete(void) {
	if(completed)
		return;
	completed = true;
	if(!fus.isEmpty()) {
		stage->fus.allocate(fus.length());
		for(int i = 0; i < fus.length(); i++)
			stage->fus[i] = fus[i];
	}
	if(!disps.isEmpty()) {
		stage->dispatch.allocate(disps.length());
		for(int i = 0; i < disps.length(); i++)
			stage->dispatch[i] = disps[i];
	}
}


/**
 */
void ProcessorBuilder::complete(void) {
	if(completed)
		return;
	completed = true;
	if(!stages.isEmpty()) {
		proc->stages.allocate(stages.length());
		for(int i = 0; i < stages.length(); i++)
			proc->stages[i] = stages[i];
	}
	if(!queues.isEmpty()) {
		proc->queues.allocate(queues.length());
		for(int i = 0; i < queues.length(); i++)
			proc->queues[i] = queues[i];
	}
}

} } // otawa::hard

ENUM_BEGIN(otawa::hard::Stage::type_t)
VALUE(otawa::hard::Stage::FETCH),
VALUE(otawa::hard::Stage::LAZY),
VALUE(otawa::hard::Stage::EXEC),
VALUE(otawa::hard::Stage::COMMIT),
VALUE(otawa::hard::Stage::DECOMP)
ENUM_END

SERIALIZE(otawa::hard::PipelineUnit);
SERIALIZE(otawa::hard::FunctionalUnit);
SERIALIZE(otawa::hard::Dispatch);
SERIALIZE(otawa::hard::Stage);
SERIALIZE(otawa::hard::Queue);
SERIALIZE(otawa::hard::Processor);

namespace elm { namespace serial2 {

void __unserialize(Unserializer& s, otawa::hard::Dispatch::type_t& v) {

	// List  of identifiers
	static elm::value_t values[] = {
			VALUE(otawa::Inst::IS_COND),
			VALUE(otawa::Inst::IS_CONTROL),
			VALUE(otawa::Inst::IS_CALL),
			VALUE(otawa::Inst::IS_RETURN),
			VALUE(otawa::Inst::IS_MEM),
			VALUE(otawa::Inst::IS_LOAD),
			VALUE(otawa::Inst::IS_STORE),
			VALUE(otawa::Inst::IS_INT),
			VALUE(otawa::Inst::IS_FLOAT),
			VALUE(otawa::Inst::IS_ALU),
			VALUE(otawa::Inst::IS_MUL),
			VALUE(otawa::Inst::IS_DIV),
			VALUE(otawa::Inst::IS_SHIFT),
			VALUE(otawa::Inst::IS_TRAP),
			VALUE(otawa::Inst::IS_INTERN),
			value("", 0)
	};

	// Build the type
	v = 0;
	String text;
	__unserialize(s, text);
	while(text) {

		// Get the component
		int pos = text.indexOf('|');
		String item;
		if(pos < 0) {
			item = text;
			text = "";
		}
		else {
			item = text.substring(0, pos);
			text = text.substring(pos + 1);
		}

		// Find the constant
		bool done = false;
		for(int i = 0; values[i].name(); i++) {
			CString cst = values[i].name();
			if(item == cst ||
					(cst.endsWith(item) && cst[cst.length() - item.length() - 1] == ':')) {
				done = true;
				v |= values[i].value();
				break;
			}
		}
		if(!done) {
			t::uint32 m;
			try {
				item >> m;
				v |= m;
			}
			catch(io::IOException& e) {
				throw io::IOException(_ << "unknown symbol \"" << item << "\".");
			}
		}
	}
}

void __serialize(Serializer& s, otawa::hard::Dispatch::type_t v) {
	ASSERT(0);
}

} } // elm::serial2
