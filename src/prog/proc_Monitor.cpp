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
 */


/**
 */
Monitor::Monitor(void): flags(0), log_level(LOG_NONE), ws(0) {
}

/**
 * This property identifier is used for setting the output stream used by
 * the processor to write results.
 */
Identifier<elm::io::OutStream *> OUTPUT("otawa::OUTPUT", &io::out);


/**
 * This property identifier is used for setting the log stream used by
 * the processor to write messages (information, warning, error).
 */
Identifier<elm::io::OutStream *> LOG("otawa::LOG", &io::err);


/**
 * This property activates the verbose mode of the processor: information about
 * the processor work will be displayed.
 */
Identifier<bool> VERBOSE("otawa::VERBOSE", false);


/**
 * Property passed in the configuration property list of a processor
 * to select the log level between LOG_PROC, LOG_CFG or LOG_BB.
 */
Identifier<Monitor::log_level_t> LOG_LEVEL("otawa::LOG_LEVEL", Monitor::LOG_NONE);



void Monitor::configure(const PropList& props) {

	// Process output
	out.setStream(*OUTPUT(props));
	log.setStream(*LOG(props));

	// process verbosity
	bool verbose;
	if(props.hasProp(VERBOSE))
		verbose = VERBOSE(props);
	else
		verbose = elm::sys::System::hasEnv(VERBOSE_ENV);
	if(verbose) {
		flags |= IS_VERBOSE;
		log_level = LOG_BB;
	}
	else
		flags &= ~IS_VERBOSE;

	// get the log level
	log_level_t level = LOG_LEVEL(props);
	if(level)
		log_level = level;
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
 * @enum log_level_t;
 * Defines the current level for logging. Possible values includes:
 * @li LOG_NONE -- no logging
 * @li LOG_PROC -- log only processor operations
 * @li LOG_FILE, LOG_DEPS -- log operations at file level,
 * @li LOG_FUN, LOG_CFG -- log operations at function level,
 * @li LOG_BLOCK, LOG_BB -- log operations at block level (equivalent to old verbose mode activation),
 * @li LOG_INST -- log operations at instruction level.
 *
 */

}	// otawa
