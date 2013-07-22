/*
 *	ASTProcessor class interface
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
#ifndef OTAWA_PROC_ASTPROCESSOR_H_
#define OTAWA_PROC_ASTPROCESSOR_H_

#include <otawa/proc/Processor.h>
#include <otawa/ast/FunProcessor.h>
#include <otawa/ast/FunAST.h>
#include <otawa/ast/AST.h>

namespace otawa { namespace ast {
	
// Processor class
class ASTProcessor: public FunProcessor {
public:
	ASTProcessor(p::declare& r = FunProcessor::reg);
	~ASTProcessor(void);

protected:
	virtual void processAST(WorkSpace *ws, AST *ast)=0 ;
	virtual void processFun(WorkSpace *ws, FunAST *fa);
};

} } // otawa::ast

#endif //OTAWA_PROC_ASTPROCESSOR_H_
