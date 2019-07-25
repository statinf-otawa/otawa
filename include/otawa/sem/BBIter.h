/*
 *	BBIter class interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_SEM_BBITER_H_
#define OTAWA_SEM_BBITER_H_

#include <otawa/cfg/BasicBlock.h>
#include <otawa/sem/PathIter.h>

namespace otawa { namespace sem {

class BBIter: public PreIterator<BBIter, sem::inst> {
public:
	void start(BasicBlock *bb);

	inline bool pathEnd(void) const { return si.pathEnd(); }
	inline bool isCond(void) const { return si.isFork(); }
	inline bool isFork(void) const { return si.isFork(); }
	inline bool instEnd(void) const { return si.ended(); }
	inline bool ended(void) const { return i.ended() && si.ended(); }

	inline sem::inst item(void) const { return si.item(); }
	inline Inst *instruction(void) const { return *i; }

	void next(void);
	void nextInst(void);
	void toEnd(void);

private:
	sem::Block b;
	sem::PathIter si;
	BasicBlock::InstIter i;
};

} }		// otawa::sem

#endif /* OTAWA_SEM_BBITER_H_BBITER_H_ */
