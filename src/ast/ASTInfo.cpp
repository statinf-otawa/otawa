/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/prog/ASTInfo.cpp -- implementation for ASTInfo class.
 */

#include <otawa/ast/ASTInfo.h>

namespace otawa {

/**
 * @class ASTInfo
 * This class stores all the function known for the current framework. It provides list access
 * name mapped access.
 */

	
/**
 * Identifier of the property storing the AST information on the framework object.
 */
const id_t ASTInfo::ID = Property::getID("otawa.ast.info");

	
/**
 * Build an new AST info linked to the given framework.
 * @param fw	Framework to link to.
 */
ASTInfo::ASTInfo(FrameWork *fw) {
	fw->set<ASTInfo *>(ID, this);
}

/**
 * @fn Map<String, FunAST *>& ASTInfo::map(void);
 * Get the map of functions.
 * @return Function map.
 */


/**
 * @fn Collection<FunAST *>& ASTInfo::functions(void);
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


/**
 * Destructor.
 */
ASTInfo::~ASTInfo(void) {
	for(Iterator<FunAST *> fun(funs); fun; fun++)
		delete *fun;
}


// Cleanup thanks toproperties
GenericProperty<ASTInfo *>::~GenericProperty(void) {
	delete value;
}


/**
 * Find the function at the given instruction.
 * If no functions exsists, return a new created function.
 * @param inst	First instruction of the function.
 * @return	Found or created AST function.
 */
FunAST *ASTInfo::getFunction(Inst *inst) {
	
	// Look in the instruction
	FunAST *fun = inst->get<FunAST *>(FunAST::ID, 0);
	
	// Create it else
	if(!fun)
		fun = new FunAST(this, inst);
	
	// Return the function
	return fun;
}


} // otawa
