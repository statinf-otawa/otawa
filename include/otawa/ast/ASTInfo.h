/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/ASTInfo.h -- interface for ASTInfo class.
 */
#ifndef OTAWA_AST_AST_INFO_H
#define OTAWA_AST_AST_INFO_H

#include <elm/datastruct/HashTable.h>
#include <elm/datastruct/Vector.h>
#include <otawa/ast/FunAST.h>

namespace otawa {

// ASTInfo class
class ASTInfo: public PropList {
	friend class CallAST;
	friend class FunAST;
	friend class GenericProperty<ASTInfo *>;
	Vector<AutoPtr <FunAST> > funs;
	HashTable<String, AutoPtr<FunAST> > _map;
	void add(AutoPtr<FunAST> fun);
	ASTInfo(Process *proc);
public:
	static const id_t ID;
	static ASTInfo *getInfo(Process *proc);
	AutoPtr<FunAST> getFunction(Inst *inst);
	inline Map<String, AutoPtr<FunAST> >& map(void) { return _map; };
	inline elm::Collection< AutoPtr<FunAST> >& functions(void) { return funs; };
};
	
} // otawa

#endif // OTAWA_AST_AST_INFO_H
