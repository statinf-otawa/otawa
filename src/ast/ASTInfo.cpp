/*
 *	ASTInfo class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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
#include <otawa/ast/ASTInfo.h>
#include <otawa/prop/DeletableProperty.h>
#include <otawa/ast/features.h>

namespace otawa { namespace ast {

/**
 * @class ASTInfo
 * This class stores all the function known for the current framework. It provides list access
 * name mapped access.
 *
 * @ingroup ast
 */


/**
 * Identifier of the property storing the AST information on the framework object.
 *
 * @p Hook
 * @li @ref WorkSpace
 *
 * @p Feature
 * @li @ref otawa::ast::FEATURE
 *
 * @ingroup ast
 */
Identifier<ASTInfo *> INFO("otawa::ast::INFO", 0);


/**
 * Build an new AST info linked to the given framework.
 * @param ws	Workspace to link to.
 */
ASTInfo::ASTInfo(WorkSpace *ws) {
	ws->addProp(new DeletableProperty<ASTInfo *>(INFO, this));
}

/**
 * @fn Map<String, FunAST *>& ASTInfo::map(void);
 * Get the map of functions.
 * @return Function map.
 */


/**
 * @fn elm::Collection<FunAST *>& ASTInfo::functions(void);
 * Get the colleciton of functions.
 * @return Function collection.
 */


/**
 * Add a function to the AST information.
 * @param fun Function to add.
 */
void ASTInfo::add(FunAST *fun) {

	// Add it to the list
	funs.add(fun);

	// If there is a name, add it to the map
	String name = fun->name();
	if(name)
		_map.put(name, fun);
}


// Cleanup thanks toproperties
/*GenericProperty<ASTInfo *>::~GenericProperty(void) {
	delete value;
}*/


/**
 * Find the function at the given instruction.
 * If no functions exsists, return a new created function.
 * @param inst	First instruction of the function.
 * @return	Found or created AST function.
 */
FunAST *ASTInfo::getFunction(Inst *inst) {

	// Look in the instruction
	FunAST *fun = FUN(inst);

	// Create it else
	if(!fun)
		fun = new FunAST(this, inst);

	// Return the function
	//cout <<" name : "<< fun->name()<<'\n';
	return fun;
}


/**
 * Find or create the AST information attached to the given process.
 * @param ws	Workspace to look in.
 * @return		AST information of the process.
 */
ASTInfo *ASTInfo::getInfo(WorkSpace *ws) {

	// Look in the process
	elm::Option<ASTInfo *> result = INFO.get(ws);
	if(result)
		return *result;

	// Else build it
	return new ASTInfo(ws);
}


/**
 */
ASTInfo::~ASTInfo(void) {
	for(int i = 0; i < funs.length(); i++)
		delete funs[i];
}

} } // otawa::ast
