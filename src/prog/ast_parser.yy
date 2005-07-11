/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/heptane/parser.yy -- parser for Heptane AST file.
 */
%{

#include <stdlib.h>
#include <memory.h>
#include <otawa/ast.h>
#include <elm/genstruct/Vector.h>
#include <otawa/ast/ASTLoader.h>

using namespace elm;
using namespace otawa;

// Proto
#define size_t unsigned long
int ast_lex(void);
void ast_error(otawa::ASTLoader *loader, const char *msg);

%}

%name-prefix="ast_"
%locations
%defines
%error-verbose
%parse-param {otawa::ASTLoader *loader}

%union {
	char *str;
	AST *ast;
}

%token ERROR
%token <str> LABEL
%token <str> NAME
%token SEQ IF WHILE DOWHILE FOR CODE APPEL VIDE
%token NUMBER UN_OP BIN_OP BI_OP

%left BIN_OP BI_OP '='

%nonassoc UN_OP

%type <ast> ast asts

%%

file:
	defs
;

defs:
	def
|	def defs
;

def:
	NAME '=' ast
		{ 
			ASTInfo *info = loader->fw->getASTInfo();
			address_t addr = loader->file->findLabel($1 + 1);
			if(!addr)
				throw LoadException("Cannot resolve label \"%s\".", $1);
			Inst *inst = loader->fw->findInstAt(addr);
			if(!inst)
				throw LoadException("Cannot find instruction at \"%s\".", $1);
			FunAST *fun = info->getFunction(inst);
			String name($1);
			fun->setName(name.substring(1, name.length() - 1));
			AST *ast($3);
			fun->setAst(ast);
			info->map().put(name.substring(1, name.length() - 1), fun);
			free($1);
		}
;

ast:
	VIDE
		{ $$ = &AST::NOP; }
|	CODE '(' LABEL ',' opt_calls LABEL ')'
		{
			$$ = loader->makeBlock($3, $6);
			free($3);
			free($6);
		}
|	SEQ '[' asts ']'
		{ $$ = $3; }
|	IF '(' ast ',' ast ',' ast ')'
		{ $$ = new IfAST($3, $5, $7); }
|	WHILE '(' range ',' ast ',' ast ')'
		{ $$ = new WhileAST($5, $7); }
|	DOWHILE '(' range ',' ast ',' ast ')'
		{ $$ = new DoWhileAST($7, $5); }
|	FOR '(' range ',' ast ',' ast ',' ast ',' ast ')'
		{ $$ = new ForAST($5, $7, $9, $11); }
;

asts:
	ast
		{ $$ = $1; }
|	asts ';' ast
		{ $$ = new SeqAST($1, $3); }
;

opt_calls:
	/* empty */
|	calls
;

calls:
	call
|	calls call
;

call:
	APPEL '{' NAME '}' ','
		{ loader->calls.add(String($3)); }
;

range:
	'[' max_exp ',' max_exp ']'
;

max_exp:
	NUMBER
|	NAME
|	NAME '(' opt_args ')'
|	'(' max_exp ')'
|	BI_OP max_exp
|	UN_OP max_exp
|	max_exp BI_OP max_exp
|	max_exp BIN_OP max_exp
|	max_exp '=' max_exp
;

opt_args:
	/* empty */
|	args
;

args:
	max_exp
|	args ',' max_exp
;


%%

// Handle error
void ast_error(otawa::ASTLoader *loader, const char *msg) {
	loader->onError("%s:%d: syntax error: %s", &loader->path, 0, msg);
}
