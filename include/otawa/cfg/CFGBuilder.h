/*
 *	$Id$
 *	CFGBuilder processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-09, IRIT UPS.
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

#ifndef OTAWA_CFG_CFG_BUILDER_H
#define OTAWA_CFG_CFG_BUILDER_H

#include <elm/genstruct/Vector.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/Processor.h>
#include <otawa/cfg/CFGInfo.h>

namespace otawa {

// CFGBuilder class
class CFGBuilder: public Processor {
public:
	CFGBuilder(void);

protected:
	virtual void processWorkSpace(WorkSpace *ws);

	virtual void makeCFG(WorkSpace *ws, CFG *cfg);
	virtual void makeBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);

private:
	CodeBasicBlock *nextBB(Inst *inst);
	CodeBasicBlock *thisBB(Inst *inst);
	void addSubProgram(Segment *seg, Inst *inst);
	void buildCFG(WorkSpace *ws, Segment *seg);
	void addFile(WorkSpace *ws, File *file);
	//void buildAll(WorkSpace *fw);
	void addCFG(Segment *seg, BasicBlock *bb);

	//genstruct::Vector<CFG *> _cfgs;
	CFGInfo *info;
};

} // otawa

#endif // OTAWA_CFG_CFG_BUILDER_H

