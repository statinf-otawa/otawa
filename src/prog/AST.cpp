/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/AST.cpp -- implementation for AST class.
 */

#include <otawa/ast/AST.h>

/*
 * !!IMPROVEMENT!!
 * An AST type UNDEFINED may be added for qualifyingnot-available function body or AST part.
 */

namespace otawa {

/**
 * NOP AST class.
 */
class NOPAST: public AST {
public:
	inline NOPAST(void) { lock(); };
	virtual ast_kind_t kind(void) const { return AST_Nop; };
	virtual bool isNOP(void) { return true; };
	virtual Inst *first(void) { return 0; };
};
static NOPAST nop_inst;


/**
 * Undef AST class.
 */
class UndefAST: public AST {
public:
	inline UndefAST(void) { lock(); };
	virtual ast_kind_t kind(void) const { return AST_Undef; };
	virtual bool isUndef(void) { return true; };
	virtual Inst *first(void) { return 0; };
};
static UndefAST undef_inst;



/**
 * @class AST
 * This is the base class for the representation of programs as Abstract Syntax Trees.
 */

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

} // otawa
