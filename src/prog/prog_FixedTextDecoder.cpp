/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	FixedTextDecoder class implementation
 */

#include <elm/assert.h>
#include <otawa/prog/FixedTextDecoder.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/proc/Registry.h>
#include <otawa/proc/Feature.h>

namespace otawa {

/**
 * @class FixedTextDecoder
 * Decode the text instruction for a simple RISC architecture, requiring
 * fixed size and fixed alignement instructions.
 * 
 * @par Provide
 * @ref DECODED_TEXT
 */


/**
 * Constructor.
 */
FixedTextDecoder::FixedTextDecoder(void)
: Processor("otawa::FixedTextDecoder", Version(1, 0, 0)) {
	provide(DECODED_TEXT);
}


/**
 */
void FixedTextDecoder::processWorkSpace(WorkSpace *fw) {
	ASSERT(fw);
	for(Process::FileIter file(fw->process()); file; file++) {
		if(isVerbose())
			out << "\tProcessing file " << file->name() << io::endl;
		for(File::SegIter seg(file); seg; seg++)
			if(seg->isExecutable()) {
				if(isVerbose())
					out << "\t\tProcessing segment " << seg->name() << io::endl;
				if(seg->size() % size != 0)
					warn("segment %s from file %s does not seems to be well aligned",
						&seg->name(), &file->name());
				for(address_t addr = seg->address();
				addr < seg->topAddress();
				addr += size)
					seg->findInstAt(addr);
			} 
	}
}


/**
 */
void FixedTextDecoder::setup(WorkSpace *fw) {
	size = fw->process()->instSize();
	if(!size)
		throw new ProcessorException(*this, "cannot process: not a fixed size ISA");
}


/**
 * static access to the processor.
 */
FixedTextDecoder FixedTextDecoder::_;


// Registration
static Registration reg(FixedTextDecoder::_,
	AUTODOC "/classotawa_1_1ets_1_1FixedTextDecoder.html");

} // otawa
