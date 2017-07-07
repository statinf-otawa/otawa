/*
 *	CCGBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
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
#ifndef OTAWA_CCG_BUILDER_H
#define OTAWA_CCG_BUILDER_H

#include <elm/assert.h>
#include <elm/data/Array.h>
#include <otawa/proc/Processor.h>
#include <otawa/ccg/features.h>

namespace otawa {

// Extern classes
class LBlockSet;
class CFG;
class LBlockSet;

namespace ccg {

// Collection class
class Collection {
	friend class Builder;
	AllocArray<Graph *> ccgs;
	inline Collection(int cnt): ccgs(cnt)
		{ for(int i = 0; i < cnt; i++) ccgs[i] = 0; }
public:
	inline ~Collection(void)
		{ for(int i = 0; i < ccgs.count(); i++) if(ccgs[i]) delete ccgs[i]; }
	inline int length(void) const { return ccgs.count(); }
	inline Graph *get(int index) const { return ccgs[index]; }
	inline Graph *operator[](int index) const { return get(index); }
};


// Builder class
class Builder: public Processor {
public:
	static Identifier<bool> NON_CONFLICT;

	static p::declare reg;
	Builder(p::declare& r = reg);

	virtual void processWorkSpace(WorkSpace *fw);

private:
	void processLBlockSet(WorkSpace *fw, LBlockSet *lbset);
};

} }	// otawa::ccg

#endif // OTAWA_CCG_CCGBUILDER_H
