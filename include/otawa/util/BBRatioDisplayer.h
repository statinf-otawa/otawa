/*
 *	$Id$
 *	BBRatioDisplayer processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_BBRATIODISPLAYER_H_
#define OTAWA_BBRATIODISPLAYER_H_

#include <elm/sys/Path.h>
#include <otawa/proc/BBProcessor.h>

namespace elm { namespace io { class OutStream; } }

namespace otawa {

namespace ilp { class System; }

// BBRatioDisplayer class
class BBRatioDisplayer: public BBProcessor {
public:
	static p::declare reg;
	BBRatioDisplayer(AbstractRegistration& r = reg);
	static Identifier<bool> TO_FILE;
	static Identifier<elm::sys::Path> PATH;
	
protected:
	virtual void configure(const PropList& props);
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	virtual void processCFG(WorkSpace *fw, CFG *cfg);
	virtual void processBB(WorkSpace *fw, CFG *cfg, Block *bb);
private:
	static otawa::Identifier<int> SUM;
	int wcet;
	ilp::System *system;
	elm::io::Output out;
	elm::sys::Path path;
	bool to_file;
	elm::io::OutStream *stream;
	bool line;
};

} // otawa

#endif /* OTAWA_BBRATIODISPLAYER_H_ */

