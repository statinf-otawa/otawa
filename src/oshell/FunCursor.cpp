/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/FunCursor.cpp -- implementation FunCursor class.
 */

#include <otawa/ast.h>
#include "FunCursor.h"

namespace otawa {

/**
 * @class FunCursor
 * Cursor for navigating inside the functions of the AST.
 */

	
/**
 * Build a new cursor for the given AST function.
 * @param back	Back cursor.
 * @param function	Function to handle.
 */
FunCursor::FunCursor(Cursor *back, FunAST *function)
: Cursor(back), fun(function) {
};


// Cursor overload
void FunCursor::path(Output& out) {
	bck->path(out);
	out << '/' << fun->name();
}


// Cursor overload
void FunCursor::info(Output& out) {
	out << "FUNCTION " << fun->name() << '\n';
}


// Cursor overload
void FunCursor::list(Output& out) {
	info(out);
	outputAST(out, fun->ast(), 0);
}


// Cursor overload
void FunCursor::display(Output& out) {
	out << fun->name() << "()";
}


/**
 * Output the given AST to the given output stream.
 * @param out	Output channel.
 * @param ast	AST to output.
 * @param ind	Current indentation.
 */
void FunCursor::outputAST(Output& out, AST *ast, int ind) {
	
	// Display indentation
	for(int i = 0; i < ind; i++)
		out << "  ";
	ind++;
	
	// Display the AST
	switch(ast->kind()) {
	case AST_Undef:
		out << "UNDEFINED\n";
		break;
	case AST_Nop:
		out << "NOP\n";
		break;
	case AST_Block:
		out << "BLOCK at " << ast->toBlock()->first()->address()
			<<  " (" << (int)ast->toBlock()->size() << ").\n";
		break;
	case AST_Call:
		out << "CALL at " << ast->toCall()->first()->address();
		if(ast->toCall()->function()->name())
			out << " (" << ast->toCall()->function()->name() << ')';
		out << '\n';
		break;
	case AST_Seq:
		out << "SEQUENCE " << '\n';
		outputSeq(out, ast, ind + 1);
		break;
	case AST_If:
		out << "IF\n";
		outputAST(out, ast->toIf()->condition(), ind);
		outputAST(out, ast->toIf()->thenPart(), ind);
		outputAST(out, ast->toIf()->elsePart(), ind);
		break;
	case AST_While:
		out << "WHILE\n";
		outputAST(out, ast->toWhile()->condition(), ind);
		outputAST(out, ast->toWhile()->body(), ind);
		break;
	case AST_DoWhile:
		out << "DOWHILE\n";
		outputAST(out, ast->toDoWhile()->body(), ind);
		outputAST(out, ast->toDoWhile()->condition(), ind);
		break;
	case AST_For:
		out << "FOR\n";
		outputAST(out, ast->toFor()->initialization(), ind);
		outputAST(out, ast->toFor()->condition(), ind);
		outputAST(out, ast->toFor()->incrementation(), ind);
		outputAST(out, ast->toFor()->body(), ind);
		break;
	default:
		ASSERT(0);
	}
}


/**
 * Count the instructions in the AST.
 * @param out	Output channel.
 * @param ast	AST to output.
 * @param ind	Current indentation.
 */ 
void FunCursor::countAST(Output& out, AST *ast, int ind) {
	
	// Display indentation
	for(int i = 0; i < ind; i++)
		out << "  ";
	ind++;
	
	// Count instructions
	int count = ast->countInstructions();
	
	// Display the AST
	switch(ast->kind()) {
	case AST_Undef:
		out << "UNDEFINED\n";
		break;
	case AST_Nop:
		out << "NOP\n";
		break;
	case AST_Block:
		out << "BLOCK at " << ast->toBlock()->first()->address()
			<<  " -> " << count << "\n";
		break;
	case AST_Call:
		out << "CALL at " << ast->toCall()->first()->address();
		if(ast->toCall()->function()->name())
			out << " (" << ast->toCall()->function()->name() << ')';
		out << " -> " << count << '\n';
		break;
	case AST_Seq:
		out << "SEQUENCE -> " << count << '\n';
		countAST(out, ast->toSeq()->child1(), ind + 1);
		countAST(out, ast->toSeq()->child2(), ind + 1);
		break;
	case AST_If:
		out << "IF -> " << count << "\n";
		countAST(out, ast->toIf()->condition(), ind);
		countAST(out, ast->toIf()->thenPart(), ind);
		countAST(out, ast->toIf()->elsePart(), ind);
		break;
	case AST_While:
		out << "WHILE -> " << count << "\n";
		countAST(out, ast->toWhile()->condition(), ind);
		countAST(out, ast->toWhile()->body(), ind);
		break;
	case AST_DoWhile:
		out << "DOWHILE -> " << count << "\n";
		countAST(out, ast->toDoWhile()->body(), ind);
		countAST(out, ast->toDoWhile()->condition(), ind);
		break;
	case AST_For:
		out << "FOR -> " << count << "\n";
		countAST(out, ast->toFor()->initialization(), ind);
		countAST(out, ast->toFor()->condition(), ind);
		countAST(out, ast->toFor()->incrementation(), ind);
		countAST(out, ast->toFor()->body(), ind);
		break;
	default:
		ASSERT(0);
	}
}


/**
 * Output the given sequence AST.
 * @param out	Output channel.
 * @param ast	AST to output.
 * @param ind	Current indentation.
 */
void FunCursor::outputSeq(Output& out, AST *ast, int ind) {
	
	// Sequence AST ?
	if(ast->kind() != AST_Seq)
		outputAST(out, ast, ind);
	
	// Process children else
	else {
		outputSeq(out, ast->toSeq()->child1(), ind);
		outputSeq(out, ast->toSeq()->child2(), ind);
	}
}


/**
 */
void FunCursor::perform(Output& out, int argc, CString argv[]) {
	if(argv[0] == "count") {
		if(argc > 1)
			throw PerformException("count requires no argument.");
		countAST(out, fun->ast(), 0);			
	}
	
	else
		bck->perform(out, argc, argv);
}

} // otawa
