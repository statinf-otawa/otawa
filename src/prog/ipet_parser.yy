/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_parser.yy -- parser for IPET files.
 */
%{
#include "ExpNode.h"
#include <stdlib.h>
#include <otawa/ipet/ConstraintLoader.h>

using namespace otawa;

// Prototypes
int ipet_lex(void);
%}

%name-prefix="ipet_"
%locations
%defines
%error-verbose
%parse-param {otawa::ipet::ConstraintLoader *loader}

%union {
	char *id;
	long integer;
	double real;
	otawa::ExpNode *exp;
	otawa::ilp::Constraint::comparator_t comp;
}

%token <id> ID
%token <integer> INTEGER
%token <real> REAL
%token BB EDGE EOL
%token OP_LE OP_GE

%left	'+' '-'
%left	'*' '/'

%type <comp> comparator
%type <exp> exp

%%

file:
	/* empty */
|	lines
;

lines:
	line EOL
|	lines line EOL
;

line:
	/* empty */
|	BB ID INTEGER
		{ if(!loader->newBBVar($2, (address_t)$3)) YYABORT; free($2); }
|	EDGE ID INTEGER INTEGER
		{ if(!loader->newEdgeVar($2, (address_t)$3, (address_t)$4)) YYABORT; free($2); }
|	exp  comparator exp
		{ if(!loader->addConstraint($1, $2, $3)) YYABORT; }
;

comparator:
	'='
		{ $$ = ilp::Constraint::EQ; }
|	'<'
		{ $$ = ilp::Constraint::LT; }
|	OP_LE
		{ $$ = ilp::Constraint::LE; }
|	'>'
		{ $$ = ilp::Constraint::GT; }
|	OP_GE
		{ $$ = ilp::Constraint::GE; }
;

exp:
	INTEGER
		{ $$ = new ExpNode($1); }
|	REAL
		{ $$ = new ExpNode($1); }
|	ID
		{
			ilp::Var *var = loader->getVar($1);
			free($1);
			if(!var)
				YYABORT;
			else
				$$ = new ExpNode(var);
		}
|	'(' exp ')'
		{ $$ = $2; }
|	'+' exp
		{ $$ = new ExpNode(ExpNode::POS, $2);; }
|	'-' exp
		{ $$ = new ExpNode(ExpNode::NEG, $2);; }
|	exp '+' exp
		{ $$ = new ExpNode(ExpNode::ADD, $1, $3); }
|	exp '-' exp
		{ $$ = new ExpNode(ExpNode::SUB, $1, $3); }
|	exp '*' exp
		{ $$ = new ExpNode(ExpNode::MUL, $1, $3); }
|	exp '/' exp
		{ $$ = new ExpNode(ExpNode::DIV, $1, $3); }
;

%%

void ipet_error(otawa::ipet::ConstraintLoader *loader, const char *msg) {
	loader->out << "ERROR: " << msg << ".\n";
}
