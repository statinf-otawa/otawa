/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	TextDecoder class implementation
 */

#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/FixedTextDecoder.h>
#include <otawa/proc/Registry.h>

namespace otawa {

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
 */


/**
 * Constructor.
 */
TextDecoder::TextDecoder(void)
: Processor("otawa::TextDecoder", Version(1, 0, 0)) {
	provide(DECODED_TEXT); 
}


/**
 */
void TextDecoder::processWorkSpace(WorkSpace *fw) {
	
	// Decode the text
	int size = fw->process()->instSize();
	if(size)
		FixedTextDecoder::_.process(fw);
	else {
		Processor& proc = fw->process()->decoder();
		proc.process(fw);
	}
	
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
 * Static access to the text decoder.
 */
TextDecoder TextDecoder::_;


// Registration
static Registration reg(TextDecoder::_,
	AUTODOC "/classotawa_1_1ets_1_1TextDecoder.html");

} // otawa
