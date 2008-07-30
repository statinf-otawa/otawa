/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_parser.yy -- parser for IPET files.
 */
%{
#include "ExpNode.h"
#include <stdio.h>
#include <stdlib.h>
#include <otawa/ipet/ConstraintLoader.h>

//using namespace otawa;

// Prototypes
int ipet_lex(void);
int ipet_line = 1;
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
		{ if(!loader->newBBVar($2, otawa::Address($3))) YYABORT; free($2); }
|	EDGE ID INTEGER INTEGER
		{ if(!loader->newEdgeVar($2, otawa::Address($3), otawa::Address($4))) YYABORT; free($2); }
|	exp  comparator exp
		{ if(!loader->addConstraint($1, $2, $3)) YYABORT; }
;

comparator:
	'='
		{ $$ = otawa::ilp::Constraint::EQ; }
|	'<'
		{ $$ = otawa::ilp::Constraint::LT; }
|	OP_LE
		{ $$ = otawa::ilp::Constraint::LE; }
|	'>'
		{ $$ = otawa::ilp::Constraint::GT; }
|	OP_GE
		{ $$ = otawa::ilp::Constraint::GE; }
;

exp:
	INTEGER
		{ $$ = new otawa::ExpNode($1); }
|	REAL
		{ $$ = new otawa::ExpNode($1); }
|	ID
		{
			otawa::ilp::Var *var = loader->getVar($1);
			free($1);
			if(!var)
				YYABORT;
			else
				$$ = new otawa::ExpNode(var);
		}
|	'(' exp ')'
		{ $$ = $2; }
|	'+' exp
		{ $$ = new otawa::ExpNode(otawa::ExpNode::POS, $2);; }
|	'-' exp
		{ $$ = new otawa::ExpNode(otawa::ExpNode::NEG, $2);; }
|	exp '+' exp
		{ $$ = new otawa::ExpNode(otawa::ExpNode::ADD, $1, $3); }
|	exp '-' exp
		{ $$ = new otawa::ExpNode(otawa::ExpNode::SUB, $1, $3); }
|	exp '*' exp
		{ $$ = new otawa::ExpNode(otawa::ExpNode::MUL, $1, $3); }
|	exp '/' exp
		{ $$ = new otawa::ExpNode(otawa::ExpNode::DIV, $1, $3); }
;

%%

void ipet_error(otawa::ipet::ConstraintLoader *loader, const char *msg) {
	loader->log << "ERROR: " << loader->path << ":" << ipet_line << ": " << msg << ".\n";
}
