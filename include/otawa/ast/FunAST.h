/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/FunAST.h -- interface for FunAST class.
 */
#ifndef OTAWA_AST_FUN_AST_H
#define OTAWA_AST_FUN_AST_H

#include <elm/datastruct/Map.h>
#include <elm/Collection.h>
#include <otawa/ast/AST.h>
#include <otawa/manager.h>

namespace otawa {

// FunAST class
class FunAST: public Lock {
	friend class ASTInfo;
	ASTInfo *info;
	Inst *ent;
	String _name;
	AST *_ast;
	~FunAST(void);
public:
	static Identifier<FunAST *> ID;
	FunAST(WorkSpace *fw, Inst *entry, String name = "");
	FunAST(ASTInfo *info, Inst *entry, String name = "");
	inline Inst *entry(void) const { return ent; };
	inline String& name(void) { return _name; };
	inline AST *ast(void) const { return _ast; };
	void setAst(AST *ast);
	inline void setName(const String& name) { _name = name; };
};

} // otawa

#endif	// OTAWA_AST_FUN_AST_H
