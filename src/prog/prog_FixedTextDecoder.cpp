/*
 *	$Id$
 *	FixedTextDecoder analyzer implementation
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

#include <elm/assert.h>
#include <otawa/prog/FixedTextDecoder.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/proc/Registry.h>
#include <otawa/proc/Feature.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prog/File.h>

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
FixedTextDecoder::FixedTextDecoder(void): Processor(reg) {
}


/**
 */
void FixedTextDecoder::processWorkSpace(WorkSpace *fw) {
	ASSERT(fw);
	for(Process::FileIter file(fw->process()); file; file++) {
		if(logFor(LOG_DEPS))
			log << "\tProcessing file " << file->name() << io::endl;
		for(File::SegIter seg(file); seg; seg++)
			if(seg->isExecutable()) {
				if(logFor(LOG_CFG))
					log << "\t\tProcessing segment " << seg->name() << io::endl;
				if(seg->size() % size != 0)
					warn(elm::_ << "segment " << seg->name() << " from file "
						<< file->name() << " does not seems to be well aligned");
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
		throw ProcessorException(*this, "cannot process: not a fixed size ISA");
}


// Registration
Registration<FixedTextDecoder> FixedTextDecoder::reg(
	"otawa::FixedTextDecoder",
	Version(1, 0, 0),
	p::provide, &DECODED_TEXT,
	p::end
);

} // otawa
