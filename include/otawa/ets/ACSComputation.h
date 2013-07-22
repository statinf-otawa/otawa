/*
 *	ACSComputation class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
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
#ifndef OTAWA_ETS_ACS_COMPUTATION_H
#define OTAWA_ETS_ACS_COMPUTATION_H

#include <elm/genstruct/HashTable.h>
#include <otawa/ast/ASTProcessor.h>
#include <otawa/ets/AbstractCacheState.h>
#include <otawa/hard/CacheConfiguration.h>

namespace otawa { namespace ets {

using namespace ast;

// ACSComputation class	
class ACSComputation: public ASTProcessor {
public :
	static p::declare reg;
	ACSComputation(p::declare& r = reg);
	~ACSComputation(void);

protected :
	void processAST(WorkSpace *fw, AST *ast);
	virtual void processWorkSpace(WorkSpace *ws);

private:
	AbstractCacheState *applyProcess(WorkSpace *fw, AST *ast, AbstractCacheState *acs);
	void initialization(WorkSpace *fw, AST *ast, AbstractCacheState *acs);
	int cache_line_length;
	int cache_size;
};

} } // otawa::ets

#endif // OTAWA_ETS_ACS_COMPUTATION_H


