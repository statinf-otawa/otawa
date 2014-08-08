/*
 *	AST class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003, IRIT UPS.
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
#include <otawa/ast/AST.h>
#include <otawa/proc/ProcessorPlugin.h>

/*
 * !!IMPROVEMENT!!
 * An AST type UNDEFINED may be added for qualifying not-available function body or AST part.
 */

/**
 * @defgroup ast AST Plugin
 *
 * This module provides facilities to handle an executable
 * as an AST (Abstract Syntactic Tree). This allows to implements
 * ETS (Extending Timing Schema) approach to compute the WCET as in:
 *
 * S.S. LIM, Y.H. BAE, G.T. JANG, S.L. MIN, C.Y. PARK, H. SHIN, H., C.S. KIM, C. S.
 * An accurate worst case timing analysis for RISC processors.
 * IEEE transactions on software engineering, 21(7), 593-604, 1995.
 *
 * The AST is loaded from an external file (usually generated from source files)
 * in the same format as Heptane tool (http://www.irisa.fr/alf/index.php?option=com_content&view=article&id=29&Itemid=0).
 *
 * @par Plugin Information
 * @li name: otawa/ast
 * @li header: <otawa/ast/features.h>
 */

namespace otawa { namespace ast {
	
/**
 * NOP AST class.
 */
class NOPAST: public AST {
public:
	inline NOPAST(void) { lock(); };
	virtual void release(void) { };
	virtual ast_kind_t kind(void) const { return AST_Nop; };
	virtual bool isNOP(void) { return true; };
	virtual Inst *first(void) { return 0; };
	virtual int countInstructions(void) const { return 0; };
};
static NOPAST nop_inst;


/**
 * Undef AST class.
 */
class UndefAST: public AST {
public:
	inline UndefAST(void) { lock(); };
	virtual void release(void) { };
	virtual ast_kind_t kind(void) const { return AST_Undef; };
	virtual bool isUndef(void) { return true; };
	virtual Inst *first(void) { return 0; };
	virtual int countInstructions(void) const { return 0; };
};
static UndefAST undef_inst;



/**
 * @class AST
 * This is the base class for the representation of programs as Abstract Syntax Trees.
 *
 * @ingroup ast
 */


/**
 * Deletion of AST can only be performed using this method.
 */
void AST::release(void) {
	delete this;
}

/**
 * @fn otawa::ast_kind_t AST::kind(void) const
 * Get the kind of the AST.
 */

/**
 * @fn BlockAST *AST::toBlock(void);
 * Get the block AST if this AST is a block, null else.
 * @return Block AST or null.
  */

/**
 * @fn BlockAST *AST::toSeq(void);
 * Get the sequence AST if this AST is a sequence, null else.
 * @return Sequence AST or null.
  */

/**
 * @fn BlockAST *AST::toIf(void);
 * Get the selection AST if this AST is a selection, null else.
 * @return Selection AST or null.
  */

/**
 * @fn BlockAST *AST::toWhile(void);
 * Get the repetition AST if this AST is a repeatition, null else.
 * @return Repetition AST or null.
  */

/**
 * @fn BlockAST *AST::toDoWhile(void);
 * Get the repetition AST if this AST is a repetition, null else.
 * @return Repetition AST or null.
  */

/**
 * @fn BlockAST *AST::toFor(void);
 * Get the repetition AST if this AST is a repetition, null else.
 * @return Repetition AST or null.
  */


/**
 * @fn CallAST *AST::toCall(void);
 * Get the call AST if this AST is a call, null else.
 * @return Call AST or null.
  */


/**
 * Unique instance of the NOP AST.
 */
AST& AST::NOP = nop_inst;


/**
 * Unique instance of the Undef AST.
 */
AST& AST::UNDEF = undef_inst;


/**
 * @fn bool AST::isNOP(void);
 * Test if the AST is the NOP AST.
 * @return True if it is the NOP AST, false else.
 */


/**
 * @fn bool AST::isUndef(void);
 * Test if the AST is the undefined AST.
 * @return True if it is the undefined AST, false else.
 */
 
 /**
 * Count the number of instructions in the ast block.
 * @return	Number of instruction in the ast block.
 */
int AST::countInstructions(void) const {
	return 1;
}

class Plugin: public ProcessorPlugin {
public:
	typedef genstruct::Table<AbstractRegistration * > procs_t;

	Plugin(void): ProcessorPlugin("otawa::ast", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
	virtual procs_t& processors(void) const { return procs_t::EMPTY; };
};

} }		// otawa::ast

otawa::ast::Plugin OTAWA_PROC_HOOK;
otawa::ast::Plugin& otawa_ast = OTAWA_PROC_HOOK;
