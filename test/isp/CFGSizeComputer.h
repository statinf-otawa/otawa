/*
 *	$Id$
 *	CFGSizeComputer processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-09, IRIT UPS.
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
#ifndef OTAWA_CFG_SIZE_COMPUTER_H
#define OTAWA_CFG_SIZE_COMPUTER_H

#include <elm/genstruct/Vector.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/ContextualProcessor.h>
#include <otawa/cfg/Edge.h>
#include <otawa/cfg/VirtualCFG.h>

namespace otawa {

// CFGCollector Class
class CFGSizeComputer: public ContextualProcessor {
	
  private:
    elm::genstruct::Vector<address_t> _min_addr;
    elm::genstruct::Vector<address_t> _max_addr;
    elm::genstruct::Vector<CFG *> _inlined_cfg;
    int _current_level;
public:
	CFGSizeComputer(void);

protected:
	virtual void enteringCall(WorkSpace *ws, CFG *cfg, BasicBlock *caller, BasicBlock *callee);
	virtual void leavingCall(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	virtual void avoidingRecursive(WorkSpace *ws, CFG *cfg, BasicBlock *caller, BasicBlock *callee);
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	
};

// CFG_SIZE_FEATURE => a deplacer vers cfg/features.h
extern SilentFeature CFG_SIZE_FEATURE;
extern Identifier<size_t> CFG_SIZE;
extern Identifier<size_t> CFG_CUMULATIVE_SIZE;
extern Identifier<address_t> CFG_LOWER_ADDR;
extern Identifier<address_t> CFG_HIGHER_ADDR;
extern Identifier<address_t> CFG_CUMULATIVE_LOWER_ADDR;
extern Identifier<address_t> CFG_CUMULATIVE_HIGHER_ADDR;


} // otawa

#endif // OTAWA_CFG_SIZE_COMPUTER_H
