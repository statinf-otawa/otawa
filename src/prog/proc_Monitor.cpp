/*
 *	Monitor class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS <casse@irit.fr>
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

#include <elm/sys/System.h>
#include <otawa/proc/Monitor.h>

namespace otawa {

extern cstring VERBOSE_ENV;

/**
 * @class Monitor
 * This class provide a unified interface to interact with external user.
 * It includes features for:
 * @li performing output,
 * @li log display and support,
 * @li current workspace.
 *
 * It aims to provide to code processor and external analysis
 * a way to perform IO in a clean and compatible way.
 *
 * The configuration of a monitor is obtained from a property list
 * and supports the following properties:
 * @li @ref OUTPUT
 * @li @ref LOG
 * @li @ref VERBOSE
 * @li @ref LOG_LEVEL
 *
 * In addition, verbosity can be activatedby declaring the environment variable 'OTAWA_VERBOSE'.
 */


/**
 */
Monitor::Monitor(void): flags(0), log_level(LOG_NONE), ws(nullptr), parent(nullptr) {
}


/**
 * Build a monitor by copy.
 * @param mon	Monitor to copy.
 */
Monitor::Monitor(Monitor& mon)
:	out(mon.out.stream()),
	log(mon.log.stream()),
	flags(mon.flags),
	log_level(mon.log_level),
	ws(mon.ws),
	parent(&mon)
{
	mon.refs.add(this);
}


///
Monitor::~Monitor() {
	if(parent != nullptr)
		parent->refs.remove(this);
	for(auto r: refs)
		r->parent = nullptr;
}


/**
 * Update the workspace in the monitor.
 * @param workspace	New workspace.
 */
void Monitor::setWorkspace(WorkSpace *workspace) {
	ws = workspace;
	for(auto r: refs)
		r->ws = ws;
}


/**
 * This property identifier is used for setting the output stream used by
 * the processor to write results.
 * @ingroup proc
 */
p::id<elm::io::OutStream *> OUTPUT("otawa::OUTPUT", &cout.stream());


/**
 * This property identifier is used for setting the log stream used by
 * the processor to write messages (information, warning, error).
 * @ingroup proc
 */
p::id<elm::io::OutStream *> LOG("otawa::LOG", &cerr.stream());


/**
 * This property activates the verbose mode of the processor: information about
 * the processor work will be displayed.
 * @ingroup proc
 */
p::id<bool> VERBOSE("otawa::VERBOSE", false);


/**
 * Property passed in the configuration property list of a processor
 * to select the log level between LOG_PROC, LOG_CFG or LOG_BB.
 * @ingroup proc
 */
p::id<Monitor::log_level_t> LOG_LEVEL("otawa::LOG_LEVEL", Monitor::LOG_NONE);


/**
 * Display logs only for the named processor. If no LOG_FOR is defined, all processors
 * are logged. Several LOG_FOR can be recorded in the configuration.
 * @ingroup proc
 */
p::id<string> LOG_FOR("otawa::LOG_FOR");


///
void Monitor::configure(const PropList& props, string name) {

	// Process output
	out.setStream(*OUTPUT(props));
	log.setStream(*LOG(props));

	// look to logging parameters
	bool verbose;
	if(props.hasProp(VERBOSE))
		verbose = VERBOSE(props);
	else
		verbose = elm::sys::System::hasEnv(VERBOSE_ENV);
	log_level_t level = LOG_LEVEL(props);

	// test named logging
	bool log_auth = true;
	if(name != "")
		for(auto n: LOG_FOR.all(props)) {
			if(n == name) {
				flags |= IS_QUIET;
				log_auth = true;
				if(!verbose || level == LOG_NONE)
					verbose = true;
				break;
			}
			else
				log_auth = false;
		}

	// if logging authorized
	if(log_auth) {

		// process verbosity
		if(verbose) {
			flags |= IS_VERBOSE;
			log_level = LOG_BB;
		}
		else
			flags &= ~IS_VERBOSE;

		// get the log level
		if(level) {
			log_level = level;
			if(level >= LOG_BLOCK)
				flags |= IS_VERBOSE;
		}
	}

	// record in the children
	for(auto r: refs) {
		r->out.setStream(out.stream());
		r->log.setStream(log.stream());
		r->log_level = log_level;
		if(flags & IS_VERBOSE)
			r->flags |= IS_VERBOSE;
		else
			r->flags &= ~IS_VERBOSE;
		if(flags & IS_QUIET)
			r->flags |= IS_QUIET;
		else
			r->flags &= ~IS_QUIET;
	}
}


/**
 * @fn	void Monitor::setWorkspace(WorkSpace *workspace);
 * Set the current workspace.
 * @param workspace		New current workspace.
 */


/**
 * @fn WorkSpace *Monitor::workspace(void);
 * Get the current workspace.
 * @return	Current workspace.
 */


/**
 * @fn t::uint32 Monitor::flags;
 * Contains various bit flags. Bits from 0 to CUSTOM_SHIFT - 1 are private
 * while bits from CUSTOM_SHIFT to 31 may be customized. To build
 * a bit mask for custom bit i, the CUSTOM_SHIFT may be used as below:
 * @code
 * static const t::uint32 MY_MASK = 1 << (i + CUSTOM_SHIFT);
 * @endcode
 */


/**
 * @var elm::io::Output Monitor::out;
 * Provide a stream to perform output for the user.
 */


/**
 * @fn elm::io::Output Monitor::log;
 * Provide a stream to perform log output. Should be used only when verbosity is activated.
 */


/**
 * @fn bool Monitor::isVerbose(void) const;
 * Test if the verbosity is activated.
 * @return	True if verbosity is activated, false else.
 */


/**
 * @fn bool Monitor::logFor(log_level_t tested) const;
 * Test if the given log level is activated or not.
 * @param tested	Tested log level.
 * @return			True if the log level is activated, false else.
 */


/**
 * @fn log_level_Monitor::t logLevel(void) const;
 * Get the current log level.
 * @return	Log level.
 */


/**
 * @enum Monitor::log_level_t;
 * Defines the current level for logging. Possible values includes:
 * @li LOG_NONE -- no logging
 * @li LOG_PROC -- log only processor operations
 * @li LOG_FILE, LOG_DEPS -- log operations at file level,
 * @li LOG_FUN, LOG_CFG -- log operations at function level,
 * @li LOG_BLOCK, LOG_BB -- log operations at block level (equivalent to old verbose mode activation),
 * @li LOG_INST -- log operations at instruction level.
 *
 */

static class NullMonitor: public Monitor {
public:
	NullMonitor(void) {
		out.setStream(io::OutStream::null);
		log.setStream(io::OutStream::null);
	}
} __null_mon;

/**
 * Monitor that output nothing.
 */
Monitor& Monitor::null = __null_mon;

}	// otawa
