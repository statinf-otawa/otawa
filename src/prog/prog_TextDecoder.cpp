/*
 *	$Id$
 *	TextDecoder analyzer implementation
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

#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/FixedTextDecoder.h>
#include <otawa/prog/VarTextDecoder.h>
#include <otawa/proc/Registry.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/program.h>

namespace otawa {

// for internal use only
static Identifier<bool> LABELS_DONE("", false);

/**
 * Static access to the text decoder.
 */
TextDecoder TextDecoder::_;


/**
 * This feature ensures that text segment of the process has been decoded
 * and, consequently, that instructions are available.
 *
 * @par Provided properties
 * @li @ref FUNCTION_LABEL
 * @li @ref LABEL
 */
Feature<TextDecoder> DECODED_TEXT("otawa::DECODED_TEXT");


/**
 * @class TextDecoder
 * A default text decoder: try to find the most adapted text decoder. If the
 * instruction set has fixed size, use the @ref FixedTextDecoder(). Else call
 * the process provided text decoder.
 *
 * @par Configuration
 * @li @ref FOLLOW_PATHS
 *
 * @par Required Features
 * @li @ref otawa::FLOW_FACTS_FEATURE
 *
 * @par Provided Features
 * @li @ref DECODED_TEXT
 */


/**
 * Constructor.
 */
TextDecoder::TextDecoder(void)
:	Processor("otawa::TextDecoder", Version(1, 0, 0)),
	follow_paths(false)
{
	provide(DECODED_TEXT);
	//config(follow_paths_config);
	require(FLOW_FACTS_FEATURE);
}


/**
 */
void TextDecoder::processWorkSpace(WorkSpace *fw) {

	// Decode the text
	Processor *decoder = fw->process()->decoder();
	if(decoder) {
	    if(logFor(LOG_DEPS))
	        log << "INFO: using loader plugin decoder.\n";
	    decoder->process(fw, *conf_props);
        }
        else {
            if(logFor(LOG_DEPS))
                log << "INFO: no default decoder\n";
            if(!fw->process()->instSize() || follow_paths) {
                if(logFor(LOG_DEPS))
                	log << "INFO: using VarTextDecoder\n";
		    VarTextDecoder decoder;
		    decoder.process(fw, *conf_props);
            }
            else {
                if(logFor(LOG_DEPS))
                    log << "INFO: using FixedTextDecoder\n";
                FixedTextDecoder decoder;
		decoder.process(fw, *conf_props);
            }
	}

	// Put the labels
	if(!LABELS_DONE(fw->process())) {
		for(Process::FileIter file(fw->process()); file; file++)
			for(File::SymIter sym(file); sym; sym++) {
				ProgItem *item = file->findItemAt(sym->address());
				if(item) {
					Inst *inst = item->toInst();
					if(inst)
						switch(sym->kind()) {
						case Symbol::FUNCTION:
							FUNCTION_LABEL(inst).add(sym->name());
							break;
						case Symbol::LABEL:
							LABEL(inst).add(sym->name());
							break;
						default:
							break;
						}
				}
			}
		LABELS_DONE(fw->process()) = true;
	}
}


/**
 */
void TextDecoder::configure(const PropList& props) {
	Processor::configure(props);
	follow_paths = FOLLOW_PATHS(props);
	conf_props = &props;
}


/**
 * This identifier is used to configure the @ref TextDecoder. It informs it
 * to decode text by following executions paths. It produces better results
 * only for fixed-size instruction architecture (like RISC) but causes an
 * increase in the computation time. This configuration property has no
 * effect on variable-size instruction set architectures (CISC).
 */
Identifier<bool> TextDecoder::FOLLOW_PATHS("otawa::TextDecoer::FOLLOW_PATHS", false);


} // otawa
