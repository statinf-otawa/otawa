/*
 *	$Id$
 *	CFGChecker processor implementation
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

#include <otawa/cfg/CFGChecker.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/CFG.h>

namespace otawa {


/**
 * In configuration of @ref otawa::CFGChecker . Inform the CFG checker
 * that it must not issue an exception when an error is found: only a warning.
 */
Identifier<bool> CFGChecker::NO_EXCEPTION("otawa::CFGChecker::NO_EXCEPTION", false);


/**
 * @class CFGChecker
 * The analysis does not produce anything but simply check some properties
 * on the involved CFG:
 * @li	there exists at least one path from the entry to the exit,
 * @li	no branch is unresolved.
 *
 * @par Provided Features
 * @li @ref CHECKED_CFG_FEATURE
 *
 * @par Configuration
 * @li @ref otawa::CFGChecker::NO_EXCEPTION
 */

Registration<CFGChecker> CFGChecker::reg(
	"otawa::CFGChecker",
	Version(1, 0, 0),
	p::base, &CFGProcessor::reg,
	p::provide, &CHECKED_CFG_FEATURE,
	p::end
);


/**
 */
CFGChecker::CFGChecker(void): BBProcessor(reg) {
}


/**
 */
void CFGChecker::configure(const PropList& props) {
	BBProcessor::configure(props);
	no_exn = NO_EXCEPTION(props);
}


/**
 */
void CFGChecker::setup(WorkSpace *ws) {
	failed = false;
}


/**
 */
void CFGChecker::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	if(bb->isExit())
		return;
	BasicBlock::OutIterator out(bb);
	if(!out) {
		failed = true;
		Inst *control = bb->controlInst();
		if(control->isControl() && !control->target())
			warn(_ << "instruction at " << cfg->format(control->address()) << " (" << control->address() << ") contains unresolved branches.");
		else
			warn(_ << "disconnected CFG at " << bb << " (" << cfg->format(bb->address()) << ")\n");
	}
}


/**
 */
void CFGChecker::processCFG(WorkSpace *ws, CFG *cfg) {
	BBProcessor::processCFG(ws, cfg);
	BasicBlock::InIterator in(cfg->exit());
	if(!in) {
		failed = true;
		warn(_ << "CFG " << cfg->label() << " is not connected (this may be due to infinite or unresolved branches).");
	}
}


/**
 */
void CFGChecker::cleanup(WorkSpace *ws) {
	if(!no_exn && failed)
		throw otawa::Exception("CFG checking has show anomalies (see above for details).");
}


/**
 * This feature ensures that
 * @li	there exists at least one path from the entry to the exit,
 * @li	no branch is unresolved.
 *
 * @p Default Processor
 * @li @ref CFGChecker
 */
static SilentFeature::Maker<CFGChecker> maker;
SilentFeature CHECKED_CFG_FEATURE("otawa::CHECKED_CFG_FEATURE", maker);

}	// otawa
