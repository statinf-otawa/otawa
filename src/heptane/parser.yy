/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/heptane/parser.yy -- parser for Heptane AST file.
 */
%{

#include <otawa/ast.h>
#include <elm/genstruct/Vector.h>
#include <stdlib.h>
using namespace otawa;

// Proto
int heptane_lex(void);
AST *make_block(CString entry, CString exit);

// Globals
otawa::Process *heptane_process;
otawa::File *heptane_file;
extern int heptane_line;
static genstruct::Vector<String> calls;

// Handle error
String heptane_message;
void yyerror(const char *msg) {
	StringBuffer buffer;
	buffer.print("%s.ast:%d: %s", &heptane_file->name(), heptane_line, msg);
	heptane_message = buffer.toString();
}

%}

%name-prefix="heptane_"
%locations
%defines
%error-verbose

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
			ASTInfo *info = ASTInfo::getInfo(heptane_process);
			FunAST *fun = info->getFunction($3->first());
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
			$$ = make_block($3, $6);
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
		{ calls.add(String($3)); }
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


/**
 * Find the address of the given raw label as found in an AST file.
 * @param raw_label			Raw label.
 * @return				Address maching the label.
 * @throw LoadException	If the label cannot be resolved.
 */
static address_t find_label(CString raw_label) {

	// Retrieve the file
	if(!heptane_file) {
		Iterator<File *> proc_files(*heptane_process->files());
		heptane_file = *proc_files;
	}
	assert(heptane_file);
	
	// Compute entry
	String label;
	if(raw_label.startsWith("__halt"))
		label = raw_label.substring(0, raw_label.length() - 1);
	else
		label = raw_label.substring(1, raw_label.length() - 2);

	// Retrieve the labels
	address_t addr = heptane_file->findLabel(label);
	if(!addr)
		throw LoadException("Cannot resolve label \"%s\".",
			&label.toCString());
	return addr;
}


/**
 * Build a block AST.
 * @param entry			Entry label.
 * @return				Block spanning from entry label to exit label.
 * @throw LoadException	Thrown if one label cannot be found.
 */
AST *make_block(CString entry, CString exit) {
	assert(heptane_process);
	
	// Retrieve entry instruction
	Inst *entry_inst = heptane_process->findInstAt(find_label(entry));
	if(!entry_inst)
		throw LoadException("Cannot find instruction at \"%s\".", 
			&entry);
	
	// Retrieve exit address
	address_t exit_addr = find_label(exit);
	
	// Any internal call
	if(!calls)
		return new BlockAST(entry_inst);
	
	// Find AST info
	ASTInfo *info = ASTInfo::getInfo(heptane_process);
	
	// Build the matching sequence
	AST *ast = 0;
	Inst *start = entry_inst, *inst;
	int cnt = 0;
	for(inst = entry_inst; !inst->atEnd(); inst = inst->next()) {
		if(inst->isCall()) {
			
			// Find the function
			Inst *target = inst->target();
			FunAST *fun = info->getFunction(target);
			
			// Build the AST
			AST *call = new CallAST(start, fun);
			if(!ast)
				ast = call;
			else
				ast = new SeqAST(ast, call);
			
			// Clean-up
			cnt++;
			start = inst->next();
			if(cnt >= calls.length())
				break;
		}
	}
	if(cnt < calls.length()) {
		throw LoadException("binary unconsistent with AST (%s).", &entry);
	}
	
	// Remaining block ?
	if(!start->atEnd() && start->address() != exit_addr)
		ast = new SeqAST(ast, new BlockAST(start));
	
	// Clean-up
	calls.clear();
	return ast;
}
