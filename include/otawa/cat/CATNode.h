/*
 *	CATNode class interface
 *	Copyright (c) 2006, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_CAT_CATNODE_H
#define OTAWA_CAT_CATNODE_H

#include <otawa/instruction.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {

namespace cache {
	class LBlockSet;
	class LBlock;
}

namespace cat {

// CatNode class
class CATNode: public elm::inhstruct::DLNode, public PropList {
	
	BasicBlock *headerlblock;
	//BasicBlock *headerevolution;
	bool inloop;
	//bool hasheaderevolution;
	cache::LBlock *lbl;
	
	
public:

	//constructor
	CATNode(cache::LBlock *lblock);
	
	// methodes
	
	void setHEADERLBLOCK(BasicBlock *hlb,bool loop);
	//void setHEADEREVOLUTION(BasicBlock *hev, bool evolution);
	BasicBlock *HEADERLBLOCK(void);
	//BasicBlock *HEADEREVOLUTION(void);
	bool INLOOP (void);
	//bool HASHEADEREVOLUTION(void);
	cache::LBlock *lblock(){return lbl;};
};

} }	// otawa::cat

#endif // OTAWA_CAT_CATNODE_H
