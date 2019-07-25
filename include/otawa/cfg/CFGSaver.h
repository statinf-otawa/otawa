/*
 *	CFGSaver class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
#ifndef OTAWA_CFG_CFGSAVER_H_
#define OTAWA_CFG_CFGSAVER_H_

#include <elm/sys/Path.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/BBProcessor.h>

namespace elm { namespace xom {
class Document;
class Element;
} }		// elm::xom

namespace otawa {

class CFGSaver: public BBProcessor {
public:
	static p::declare reg;
	CFGSaver(p::declare& r = reg);
	virtual void configure(const PropList& props);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	virtual void cleanup(WorkSpace *ws);

private:
	sys::Path path;
	io::OutStream *stream;
	xom::Document *doc;
	xom::Element *root;
	xom::Element *cfg_elt;
};

} // otawa

#endif /* OTAWA_CFG_CFGSAVER_H_ */
