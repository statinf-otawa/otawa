/*
 *	$Id$
 *	DynProcessor class interface
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
#ifndef OTAWA_PROC_DYNPROCESSOR_H_
#define OTAWA_PROC_DYNPROCESSOR_H_

#include <otawa/proc/Processor.h>

namespace otawa {

// ProcessorNotFound exception
class ProcessorNotFound: public otawa::Exception {
public:
	ProcessorNotFound(string name);
	inline const string& name(void) const { return _name; }
private:
	string _name;
};

// DynProcessor class
class DynProcessor {
public:
	DynProcessor(cstring name);
	~DynProcessor(void);
	inline string name(void) const { return proc->name(); }
	inline Version version(void) const { return proc->version(); }
	inline void configure(const PropList &props) { proc->configure(props); }
	void process(WorkSpace *ws, const PropList &props=PropList::EMPTY);

private:
	Processor *proc;
};

}	// otawa

#endif /* OTAWA_PROC_DYNPROCESSOR_H_ */
