/*
 *	CATConstraintBuilder class interface
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
#ifndef OTAWA_CAT_CATCONSTRAINTBUILDER_H
#define OTAWA_CAT_CATCONSTRAINTBUILDER_H


#include <elm/assert.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// Extern classes
class ContextTree;
namespace cache { class LBlockSet; }

namespace cat {
	
// CATConstraintBuilder class
class CATConstraintBuilder: public Processor {
public:
	static p::declare reg;
	CATConstraintBuilder(p::declare& r = reg);
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void configure(const PropList& props);

private:
	bool _explicit;
	void processLBlockSet(WorkSpace *fw, cache::LBlockSet *lbset);
	void buildLBLOCKSET(cache::LBlockSet *lcache, ContextTree *root);
};

} }	// otawa::cat


#endif //OTAWA_CAT_CATCONSTRAINTBUILDER_H_
