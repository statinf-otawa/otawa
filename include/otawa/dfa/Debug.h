/*
 *	dfa.Debug class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef OTAWA_DFA_DEBUG_H_
#define OTAWA_DFA_DEBUG_H_

#include <elm/json.h>
#include <elm/string.h>
#include <elm/sys/Path.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa { namespace dfa {

using namespace elm;

class Debug {
public:
	Debug(WorkSpace *ws, sys::Path path = "log.json");
	~Debug(void);
	json::Saver& addState(Block *bb);
	void addEvent(string event);

private:
	void complete(void);
	json::Saver saver;
	StringBuffer events;
	bool in_state;
	bool failed;
};

} } // otawa::dfa

#endif /* OTAWA_DFA_DEBUG_H_ */
