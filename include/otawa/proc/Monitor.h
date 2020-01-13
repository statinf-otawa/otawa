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
#ifndef OTAWA_PROC_MONITOR_H_
#define OTAWA_PROC_MONITOR_H_

#include <otawa/base.h>
#include "../prop.h"

namespace otawa {

using namespace elm;
class Processor;
class WorkSpace;


class Monitor {
public:
	static Monitor& null;

	Monitor();
	Monitor(Monitor& mon);
	~Monitor();

	typedef enum log_level_t {
		LOG_NONE = 0,
		LOG_PROC = 1,
		LOG_FILE = 2,
		LOG_DEPS = LOG_FILE,
		LOG_FUN = 3,
		LOG_CFG = LOG_FUN,
		LOG_BLOCK = 4,
		LOG_BB = LOG_BLOCK,
		LOG_INST = 5
	} log_level_t;

	// configuration
	inline WorkSpace *workspace(void) { return ws; }

	// output and logging
	elm::io::Output out;
	elm::io::Output log;
	inline bool isVerbose(void) const { return flags & IS_VERBOSE; }
	inline bool logFor(log_level_t tested) const { return tested <= log_level; }
	inline log_level_t logLevel(void) const { return log_level; }
	void configure(const PropList& props, string name = "");

protected:
	void setWorkspace(WorkSpace *workspace);
	static const t::uint32
		IS_VERBOSE		= 0x01,
		CUSTOM_SHIFT	= 16;
	t::uint32 flags;

private:
	log_level_t log_level;
	WorkSpace *ws;
	Monitor *parent;
	List<Monitor *> refs;
};

// configuration
extern p::id<elm::io::OutStream *> OUTPUT;
extern p::id<elm::io::OutStream *> LOG;
extern p::id<bool> VERBOSE;
extern p::id<Monitor::log_level_t> LOG_LEVEL;
extern p::id<string> LOG_FOR;

}	// otawa

#endif /* OTAWA_PROC_MONITOR_H_ */
