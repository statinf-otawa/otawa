/*
 *	$Id$
 *	CFGCheckSummer processor implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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

#include <otawa/cfg/CFGCheckSummer.h>
#include <elm/checksum/Fletcher.h>
#include <otawa/cfg/CFG.h>
#include <otawa/prog/WorkSpace.h>

using namespace elm;

namespace otawa {

/**
 * @class CFGCheckSummer
 * Add to each involved CFG a checksum build on the instruction of the CFG.
 * This check may be useful to detect if the binary has changed between
 * two OTAWA invocation (for example, to check if information computed
 * on older version of a binary remains valid with the new version).
 * @author	H. Cassé <casse@irit.fr>
 *
 * @par Required Features
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @par Provided Features
 * @li @ref CFG_CHECKSUM_FEATURE
 *
 * @ingroup cfg
 */


/**
 * Build a new check summer.
 */
CFGCheckSummer::CFGCheckSummer(void): CFGProcessor("otawa::CFGCheckSummer", Version(1, 0, 0)) {
	provide(CFG_CHECKSUM_FEATURE);
}


/**
 */
void CFGCheckSummer::processCFG(WorkSpace *ws, CFG *cfg) {
	checksum::Fletcher sum;
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEnd()) {
			char buf[bb->size()];
			ws->process()->get(bb->address(), buf, bb->size());
			sum.put(buf, bb->size());
		}
	CHECKSUM(cfg) = sum.sum();
	if(logFor(LOG_PROC))
		log << "\t\tchecksum = " << (void *)(unsigned long)CHECKSUM(cfg) << io::endl;
}


static SilentFeature::Maker<CFGCheckSummer> maker;
/**
 * This feature ensures that each CFG has hooked a checksum allowing
 * to check binary modifications between launch of an OTAWA application.
 *
 * @par Properties
 * @li @ref CHECKSUM
 *
 * @ingroup cfg
 */
SilentFeature CFG_CHECKSUM_FEATURE("otawa::CFG_CHECKSUM_FEATURE", maker);


/**
 * This property hooked on a CFG provides a checksum build
 * on the instruction of the CFG.
 *
 * @ingroup cfg
 */
Identifier<unsigned long > CHECKSUM("otawa::CHECKSUM", 0);

} // otawa
