/*
 *	CacheHitComputation class interface
 *-
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
#ifndef OTAWA_ETS_CACHE_MISS_COMPUTATION_H
#define OTAWA_ETS_CACHE_MISS_COMPUTATION_H

#include <otawa/ast/ASTProcessor.h>

namespace otawa { namespace ets {

using namespace ast;

class CacheMissComputation: public ASTProcessor {
public:
	static p::declare reg;
	CacheMissComputation(p::declare& r = reg);

protected:
	void processAST(WorkSpace *fw, AST *ast);
	int computation(WorkSpace *fw, AST *ast);
};

} } // otawa::ets

#endif // OTAWA_ETS_CACHE_MISS_COMPUTATION_H



