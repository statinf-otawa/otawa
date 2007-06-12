/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/FunAST.cpp -- interface for FunAST class.
 */

#include <elm/datastruct/Vector.h>
#include <elm/datastruct/HashTable.h>
#include <otawa/ast/FunAST.h>
#include <otawa/ast/ASTInfo.h>

namespace otawa {

	
/**
 * @class FunAST
 * This class represents functions in the AST representation. It provides the root of the AST.
 */
	
/**
 * A property with this identifier is put on each instruction, start of an AST function.
 * Its value is of type "AST *".
 */
Identifier<FunAST *> FunAST::ID("otawa::FunAST::id", 0);

	
/**
 * Build a new function AST.
 * @param fw		workspace containing the function.
 * @param entry	First instruction of the function.
 * @param name	Function name.
 */
FunAST::FunAST(WorkSpace *fw, Inst *entry, String name)
: ent(entry), _name(name), _ast(&AST::UNDEF) {
	
	// Mark the entry
	ent->set<FunAST *>(ID, this);
	
	// Obtain an AST information
	info = fw->get<ASTInfo *>(ASTInfo::ID, 0);
	if(!info)
		info = new ASTInfo(fw);
	
	// If no name, look for a name
	String label = LABEL(ent);
	if(label)
		_name = label;
		
	// Record the function
	info->add(this);	
}


/**
 * Build a new function AST.
 * @param info		AST information.
 * @param entry	First instruction of the function.
 * @param name	Function name.
 */
FunAST::FunAST(ASTInfo *info, Inst *entry, String name)
: ent(entry), _name(name), _ast(&AST::UNDEF) {
	
	// Mark the entry
	ent->set<FunAST *>(ID, this);
	
	// If no name, look for a name
	String label = LABEL(ent);
	if(label)
		_name = label;
		
	// Record the function
	info->add(this);	
}


/**
 * Remove any link with the instruction representation.
 */
FunAST::~FunAST(void) {
	//ent->removeProp(ID);
	_ast->release();
}


/**
 * @fn Inst *FunAST::entry(void) const
 * Get the instruction entry of this function.
 * @return	Entry instruction.
 */


/**
 * @fn const String& FunAST::name(void) const;
 * Get the name of the function.
 * @return Function name.
 */


/**
 * @fn AST *FunAST::ast(void) const;
 * Get the AST representing the body of this function.
 * @return Body AST.
 */


/**
 * Modify the AST representing the body of this function.
 * @param ast New body AST.
 */
void FunAST::setAst(AST *ast) {
	_ast = ast;
}


} // otawa
