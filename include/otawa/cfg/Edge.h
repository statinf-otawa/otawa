/*
 *	Edge class interface
 *	Copyright (c) 2003-12, IRIT UPS.
 *
 *	This file is part of OTAWA
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
#ifndef OTAWA_CFG_EDGE_H
#define OTAWA_CFG_EDGE_H

#include <elm/assert.h>
#include <otawa/cfg/CFG.h>

namespace otawa {

// Classes
class BasicBlock;

// Edge class
class Edge: public PropList {
	friend class CFG;
public:
	typedef enum kind_t {
		NONE = 0,
		TAKEN = 1,
		NOT_TAKEN = 2,
		CALL = 3,
		VIRTUAL = 4,
		VIRTUAL_CALL = 5,
		VIRTUAL_RETURN = 6,
		EXN_CALL = 7,
		EXN_RETURN = 8
	} kind_t;
	static cstring kindName(kind_t kind);

	Edge(BasicBlock *source, BasicBlock *target, kind_t kind = TAKEN);
	~Edge(void);

	inline BasicBlock *source(void) const { return src; };
	inline BasicBlock *target(void) const { return tgt; };
	inline kind_t kind(void) const { return knd; };

	inline CFG *calledCFG(void) const {
		ASSERT(knd == CALL);
		if(!tgt)
			return 0;
		else
			return ENTRY(tgt);
	}

private:
	kind_t knd;
	BasicBlock *src, *tgt;
	void toCall(void);
};

Output& operator<<(Output& out, const Edge *edge);
Output& operator<<(Output& out, Edge *edge);

// Deprecated
typedef Edge::kind_t edge_kind_t;
#define EDGE_Null		Edge::NONE
#define EDGE_Taken		Edge::TAKEN
#define	EDGE_NotTaken	Edge::NOT_TAKEN
#define	EDGE_Call		Edge::CALL
#define EDGE_Virtual	Edge::VIRTUAL

} // otawa

#endif	// OTAWA_CFG_EDGE_H

