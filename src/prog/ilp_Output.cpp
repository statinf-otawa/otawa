/*
 *	ilp::Output class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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

#include <otawa/ilp/Output.h>
#include <otawa/ipet/features.h>
#include <elm/sys/System.h>
#include <elm/serial2/serial.h>
#include <otawa/ilp/System.h>

namespace otawa { namespace ilp {

static SilentFeature::Maker<Output> maker;
/**
 * This feature ensures that an output of the ILP system has been produced.
 *
 * @p Default processor
 * @li @ref otawa::ilp::Output
 *
 * @p Configuration
 * @li @ref OUTPUT_FORMAT
 * @li @ref OUTPUT_PATH
 * @li @ref OUTPUT
 */
SilentFeature OUTPUT_FEATURE("otawa::ilp::OUTPUT_FEATURE", maker);

/**
 * Select the output format to build the @ref OUTPUT_FEATURE.
 * Accepted values includes @ref ilp::LP_SOLVE (1), @ref ilp::CPLEX (2) or @ref::ilp::MOSEK (3).
 */
Identifier<format_t> OUTPUT_FORMAT("otawa::ilp::OUTPUT_FORMAT", LP_SOLVE);

/**
 * Select the file to perform the output during build of @ref OUTPUT_FEATURE.
 */
Identifier<sys::Path> OUTPUT_PATH("otawa::ilp::OUTPUT_PATH", "");

/**
 * Select the output stream to perform the output to to build a @ref OUTPUT_FEATURE.
 */
Identifier<elm::io::OutStream *> OUTPUT("otawa::ilp::OUTPUT", 0);


/**
 * @class Output
 * This processor provides an output of the built ILP system.
 * Possible output includes standard output, any stream output or file.
 * The supported output format are LP_SOLVE, CPLEX or MOSEK.
 */

p::declare Output::reg = p::init("otawa::ilp::Output", Version(1, 0, 0))
	.maker<Output>()
	.provide(OUTPUT_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE);

/**
 */
Output::Output(p::declare &r): Processor(r), format(CPLEX), stream(0), do_free(false) {
}

/**
 */
Output::~Output(void) {
}

/**
 */
void Output::configure(const PropList &props) {
	Processor::configure(props);
	format = OUTPUT_FORMAT(props);
	stream = ilp::OUTPUT(props);
	path = OUTPUT_PATH(props);
}

/**
 */
void Output::setup(WorkSpace *ws) {
	if(!stream) {
		if(path)
			try {
				stream = sys::System::createFile(path);
				do_free = true;
				if(logFor(LOG_FILE))
					log << "INFO: outputting to " << path << io::endl;
			} catch(elm::sys::SystemException& e) {
				throw ProcessorException(*this, _ << "cannot open " << path << ": " << e.message());
			}
		else {
			stream = &io::out;
			if(logFor(LOG_FILE))
				log << "INFO: outputting to custom stream\n";
		}
	}
	else if(logFor(LOG_FILE))
		log << "INFO: outputting to stdout\n";
}

/**
 */
void Output::processWorkSpace(WorkSpace *ws) {
	ilp::System *system = ipet::SYSTEM(ws);
	ASSERT(system);
	system->dump(format, *stream);
}

/**
 */
void Output::cleanup(WorkSpace *ws) {
	if(stream && do_free)
		delete stream;
}

}	// ilp

}	// otawa

namespace elm {
	template <> cstring enum_info<otawa::ilp::format_t>::name(void) { return "otawa::ilp::format_t"; }
	template <> enum_info<otawa::ilp::format_t>::value_t enum_info<otawa::ilp::format_t>::values[] = {
		value("LP_SOLVE", otawa::ilp::LP_SOLVE),
		value("CPLEX", otawa::ilp::CPLEX),
		value("MOSEK", otawa::ilp::MOSEK),
		last()
	};
}
