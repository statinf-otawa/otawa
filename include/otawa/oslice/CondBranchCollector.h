/*
 *  CondBranchCollector class interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __OTAWA_OSLICE_COND_BRANCH_COLLECTOR_H__
#define __OTAWA_OSLICE_COND_BRANCH_COLLECTOR_H__

#include <elm/log/Log.h>
#include <otawa/oslice/InstCollector.h>
#include <otawa/oslice/oslice.h>



namespace otawa { namespace oslice {

extern p::feature COND_BRANCH_COLLECTOR_FEATURE;

typedef elm::avl::Set<BasicBlock*> BBSet; // used by buildReverseSynthLink, for CFG v2

class CondBranchCollector: public InstCollector {
public:
	static p::declare reg;
	CondBranchCollector(AbstractRegistration& _reg = reg);

protected:
	virtual void configure(const PropList &props);
	virtual bool interested(Inst* i);
};

} } // otawa::oslice

#endif
