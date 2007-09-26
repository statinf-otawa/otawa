/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	TextDecoder class implementation
 */

#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/FixedTextDecoder.h>
#include <otawa/prog/VarTextDecoder.h>
#include <otawa/proc/Registry.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

/**
 * Static access to the text decoder.
 */
TextDecoder TextDecoder::_;


// Registration
static Configuration follow_paths_config(TextDecoder::FOLLOW_PATHS,
	AUTODOC "/classotawa_1_1ets_1_1TextDecoder.html");
static Registration reg(TextDecoder::_,
	AUTODOC "/classotawa_1_1ets_1_1TextDecoder.html");


/**
 * This feature ensures that text segment of the process has been decoded
 * and, consequently, that instructions are available.
 * 
 * @par Provided properties
 * None.
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
 */


/**
 * Constructor.
 */
TextDecoder::TextDecoder(void)
:	Processor("otawa::TextDecoder", Version(1, 0, 0)),
	follow_paths(false)
{
	provide(DECODED_TEXT);
	config(follow_paths_config);
}


/**
 */
void TextDecoder::processWorkSpace(WorkSpace *fw) {
	
	// Decode the text
	Processor *decoder = fw->process()->decoder();
	if(!decoder) {
		if(isVerbose())
			out << "INFO: no default decoder\n";
		if(!fw->process()->instSize() || follow_paths) {
			if(isVerbose())
				out << "INFO: using VarTextDecoder\n";
			decoder = &VarTextDecoder::_;
		}
		else {
			if(isVerbose())
				out << "INFO: using FixedTextDecoder\n";
			decoder = &FixedTextDecoder::_;
		}
	}
	decoder->process(fw);
	
	// Put the labels
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
					}
			}
		}
}


/**
 */
void TextDecoder::configure(const PropList& props) {
	Processor::configure(props);
	follow_paths = FOLLOW_PATHS(props);
}


/**
 * This identifier is used to configure the @ref TextDecoder. It informs it
 * to decode text by following executions paths. It produces better results
 * only for fixed-size instruction architecture (like RISC) but causes an
 * increase in the computation time. This configuration property has no
 * effect on variable-size instruction set architectures (CISC).
 */
Identifier<bool> TextDecoder::FOLLOW_PATHS("otawa::TextDecoer::follow_paths", false);


} // otawa
