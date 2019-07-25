/*
 *	$Id$
 *	Symbolic Expression definition and processor
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#ifndef OTAWA_DATA_CLP_SYMBOLICEXPR_H_
#define OTAWA_DATA_CLP_SYMBOLICEXPR_H_

#include <elm/io.h>
#include <elm/io/Output.h>
#include <elm/data/Vector.h>
#include <elm/string/AutoString.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/data/clp/ClpValue.h>
#include <otawa/data/clp/ClpPack.h>

using namespace elm;

namespace otawa {

namespace hard {
	class Platform;
}	// hard

namespace se{

	typedef clp::Value V;

	/** Defined operators */
	typedef enum {
		NONE,
		CONST,		/** Constant */
		ADDR,		/** Memory reference */
		REG,		/** Register */
		NEG,		/** Opposite of */
		ADD,		/** Addition */
		CMP,		/** Undeterminate compare */
		CMPU,		/** Undeterminate unsigned compare */
					/** Determinate compare: */
		LE,			/**		less or equal */
		LT,			/**		less than */
		GE,			/**		greater or equal */
		GT,			/**		greater than */
		EQ,			/**		equal */
		NE,			/**		not equal */
					/**	Unsigned forms: */
		ULE,
		ULT,
		UGE,
		UGT,
		OR
	} op_t;

	/** Symbolic expression*/
	class SymbExpr{
	public:
		/** Constructor and destructor */
		/**
		 * Build a new symbolic expression
		 * You shoul instaciate childs instead of this expression
		*/
		SymbExpr(op_t op=NONE, SymbExpr *a=NULL, SymbExpr *b=NULL, V val=0, SymbExpr *parent=NULL):
			_op(op), _a(a), _b(b), _val(val), _parent(parent) {
			ASSERT(_op >= NONE && _op <= OR);
			if(_a != NULL)
				_a->set_parent(this);
			if(_b != NULL)
				_b->set_parent(this);
		}
		/**
		 * Copy constructor
		 * @param expr the expression to copy
		*/
		SymbExpr(const SymbExpr &expr):
			_op(expr._op), _val(expr._val), _parent(expr._parent) {
			ASSERT(_op >= NONE && _op <= OR);
			set_a(expr._a); // to copy expr._a
			set_b(expr._b); // to copy expr._b
		}
		/**
		 * Destructor of a symbolic expression
		 * This recursively delete a and b childs, and set to NULL the reference
		 * of this in the parent (if any).
		*/
		virtual ~SymbExpr(){
			delete _a;
			delete _b;
			if (_parent != NULL){
				if (_parent->_a == this)
					_parent->_a = NULL;
				else if (_parent->_b == this)
					_parent->_b = NULL;
			}
		}

		/** Operators */
		/**
		 * Copy operator
		 * @return a pointer to a copy of this
		*/
		virtual SymbExpr* copy(void);
		/**
		 * Affectation operator
		 * @param expr the expression to be copied in this
		 * @return this
		*/
		virtual SymbExpr& operator=(const SymbExpr& expr);
		/**
		 * Equality
		 * @param expr the expression to be compared with this
		 * @return if the expression reprensents the same as expr (recursively)
		*/
		virtual bool operator==(const SymbExpr& expr) const;
		/**
		 * Not equal operator
		 * @param expr the expression to be compared with this
		 * @return if the expression doesn't reprensents the same as expr
		*/
		bool operator!=(const SymbExpr& expr) const;

		/** Accessors **/
		/**
		 * @return the operator of the expression
		*/
		op_t op(void) const { return _op; }
		/**
		 * @return a pointer to the first child (a) of the expression (or NULL)
		*/
		SymbExpr* a(void) const { return _a; }
		/**
		 * @return a pointer to the second child (b) of the expression (or NULL)
		*/
		SymbExpr* b(void) const { return _b; }
		/**
		 * @return the value of this expression
		*/
		V val(void) const { return _val; }
		/**
		 * @return a pointer to the parent of this (or NULL)
		*/
		SymbExpr* parent(void) const { return _parent; }

		/** Operation **/
		/**
		 * Recursively replace in this the searched_se expression by new_se
		 * @param searched_se the researched expression
		 * @param new_se the expression that will replace searched_se
		 *
		 * new_se will be copied each time it replace a searched_se.
		*/
		void replace(SymbExpr *searched_se, SymbExpr *new_se);
		/**
		 * @return a string reprensentation of the expression
		*/
		virtual String asString(const hard::Platform *pf = 0);
		/**
		 * Output a string representation of the expression
		 * @param out the output channel
		 * @param pf  current plarform
		*/
		void print(io::Output& out, const hard::Platform *pf = 0);
		/**
		 * Try to recursively transform the expression into it its canonized
		 * form.
		*/
		virtual void canonize(void);
		/**
		 * Return the list of registers used
		 * (recursively) in this expression.
		 * @return a vector of int.
		*/
		virtual Vector<V> used_reg(void);
		/**
		 * Return the list of memory addresses used
		 * (recursively) in this expression.
		 * @return a vector of int.
		*/
		virtual Vector<V> used_addr(void);

		/** Setters */
		/**
		 * Set the first child expression (a)
		 * @param a a pointer to the new first child
		 *
		 * A *copy* of a will be set as first child.
		 * If there was a first child in this expression, it will be delete, but
		 * after the copy of a (so it's safe to set_a() a child expression of
		 * this->a()).
		*/
		void set_a(SymbExpr *a){
			SymbExpr *olda = _a;
			if (a) {
				_a = a->copy();
				_a->set_parent(this);
			} else
				_a = NULL;
			// in last, because a can be delete here, we must copy it before
			delete olda;
		}
		/**
		 * Set the second child expression (b)
		 * @param b a pointer to the new second child
		 *
		 * A *copy* of b will be set as second child.
		 * If there was a second child in this expression, it will be delete,
		 * but after the copy of b (so it's safe to set_b() a child expression
		 * of this->b()).
		*/
		void set_b(SymbExpr *b){
			SymbExpr *oldb = _b;
			if (b) {
				_b = b->copy();
				_b->set_parent(this);
			} else
				_b = NULL;
			// in last, because b can be delete here, we must copy it before
			delete oldb;
		}
		/**
		 * Set the reference to the parent of this
		 * @param parent a pointer to the new parent.
		 *
		 * Warning: this do not remove the reference to this in the old parent.
		*/
		void set_parent(SymbExpr *parent) { _parent = parent ; }

		virtual SymbExpr* solidifyAddress(clp::State& clpState, bool dig);

	protected:
		/** Attributes **/
		op_t _op;
		SymbExpr *_a;
		SymbExpr *_b;
		V _val;
		SymbExpr *_parent;
	};

	/** Constants */
	class SEConst: public SymbExpr{
	public:
		/** Constructors */
		/**
		 * Build a new Constant Symbolic Expression [K value]
		 * @param value the value of the constant
		 * @param parent a pointer to the parent
		*/
		SEConst(V value, SymbExpr *parent=NULL):
			SymbExpr(CONST, NULL, NULL, value, parent) {}
		/**
		 * Copy constructor
		 * @param expr the expression to copy
		*/
		SEConst(const SEConst &expr): SymbExpr(expr) {}

		/** Operators */
		/**
		 * Copy operator
			 * @return a pointer to a copy of this
		*/
		virtual SEConst* copy(void);
		/**
		 * Affectation operator
		 * @param expr the expression to be copied in this
		 * @return this
		*/
		virtual SymbExpr& operator=(const SEConst& expr);
		/**
		 * Equality
		 * @param expr the expression to be compared with this
		 * @return if the expression reprensents the same as expr (recursively)
		*/
		virtual bool operator==(const SymbExpr& expr) const;

		/** Operation **/
		/**
		 * @return a string reprensentation of the expression
		*/
		virtual String asString(const hard::Platform *pf = 0);
		/**
		 * Try to recursively transform the expression into it its canonized
		 * form.
		*/
		virtual void canonize(void);

		virtual SymbExpr* solidifyAddress(clp::State& clpState, bool dig);
	};

	/** Memory reference */
	// when _val is -1, this indicates that the SEAddr does not contain the constant address but a symbolic expression (_a != NULL).
	// otherwise _val is the constant address
	class SEAddr: public SymbExpr{
	public:
		/** Constructors */
		/**
		 * Build a new Memory Reference Symbolic Expression [@ value]
		 * @param value the memory address
		 * @param parent a pointer to the parent
		*/
		SEAddr(V value, SymbExpr* a = NULL, SymbExpr* parent = NULL) : SymbExpr(ADDR, a, NULL, value, parent) { }

		/**
		 * Copy constructor
		 * @param expr the expression to copy
		*/
		SEAddr(const SEAddr &expr): SymbExpr(expr) {}

		/** Operators */
		/**
		 * Copy operator
		 * @return a pointer to a copy of this
		*/
		virtual SEAddr* copy(void);
		/**
		 * Equality
		 * @param expr the expression to be compared with this
		 * @return if the expression reprensents the same as expr (recursively)
		*/
		virtual SymbExpr& operator=(const SEAddr& expr);
		/**
		 * Not equal operator
		 * @param expr the expression to be compared with this
		 * @return if the expression doesn't reprensents the same as expr
		*/
		virtual bool operator==(const SymbExpr& expr) const;

		/** Operation **/
		/**
		 * @return a string reprensentation of the expression
		*/
		virtual String asString(const hard::Platform *pf = 0);
		/**
		 * Try to recursively transform the expression into it its canonized
		 * form.
		*/
		virtual void canonize(void);
		/**
		 * Return the list of memory addresses used
		 * (recursively) in this expression.
		 * @return a vector of memory addresses.
		*/
		virtual Vector<V> used_addr(void);

		virtual SymbExpr* solidifyAddress(clp::State& clpState, bool dig);
	};

	/** Register */
	class SEReg: public SymbExpr{
	public:
		/** Constructors */
		/**
		 * Build a new Register Symbolic Expression [R value]
		 * @param value the identifier of the register (<0 for temp registers)
		 * @param parent a pointer to the parent
		*/
		SEReg(V value, SymbExpr *parent=NULL):
			SymbExpr(REG, NULL, NULL, value, parent) {}
		/**
		 * Copy constructor
		 * @param expr the expression to copy
		*/
		SEReg(const SEReg &expr): SymbExpr(expr) {}

		/** Operators */
		/**
		 * Copy operator
		 * @return a pointer to a copy of this
		*/
		virtual SEReg* copy(void);
		/**
		 * Affectation operator
		 * @param expr the expression to be copied in this
		 * @return this
		*/
		virtual SymbExpr& operator=(const SEReg& expr);
		/**
		 * Not equal operator
		 * @param expr the expression to be compared with this
		 * @return if the expression doesn't reprensents the same as expr
		*/
		virtual bool operator==(const SymbExpr& expr) const;

		/** Operation **/
		/**
		 * @return a string reprensentation of the expression
		*/
		virtual String asString(const hard::Platform *pf = 0);
		/**
		 * Try to recursively transform the expression into it its canonized
		 * form.
		*/
		virtual void canonize(void);
		/**
		 * Return the list of registers used
		 * (recursively) in this expression.
		 * @return a vector of int
		*/
		virtual Vector<V> used_reg(void);

		SymbExpr* solidifyAddress(clp::State& clpState, bool dig);
	};

	/** Negation */
	class SENeg: public SymbExpr{
	public:
		/** Constructors */
		/**
		 * Build a new Negation Symbolic Expression [- a]
		 * @param expr a pointer to the first child expression. This pointer
		 *			will be used as is (no copy), so make sure you 'new' or
		 *			'->copy()' the expression before.
		 * @param parent a pointer to the parent
		*/
		SENeg(SymbExpr *expr=NULL, SymbExpr *parent=NULL):
			SymbExpr(NEG, expr, NULL, 0, parent) {}
		/**
		 * Copy constructor
		 * @param expr the expression to copy
		*/
		SENeg(const SENeg &expr): SymbExpr(expr) {}

		/** Operators */
		/**
		 * Copy operator
		 * @return a pointer to a copy of this
		*/
		virtual SENeg* copy(void);
		/**
		 * Affectation operator
		 * @param expr the expression to be copied in this
		 * @return this
		*/
		virtual SymbExpr& operator=(const SENeg& expr);
		/**
		 * Not equal operator
		 * @param expr the expression to be compared with this
		 * @return if the expression doesn't reprensents the same as expr
		*/
		virtual bool operator==(const SymbExpr& expr) const;

		/** Operation **/
		/**
		 * @return a string reprensentation of the expression
		*/
		virtual String asString(const hard::Platform *pf = 0);
		/**
		 * Try to recursively transform the expression into it its canonized
		 * form.
		*/
		virtual void canonize(void);

		virtual SymbExpr* solidifyAddress(clp::State& clpState, bool dig);
	};

	/** Addition */
	class SEAdd: public SymbExpr{
	public:
		/** Constructors */
		/**
		 * Build a new Addition Symbolic Expression [+ a b]
		 * @param a is a pointer to the first child expression. This pointer
		 *			will be used as is (no copy), so make sure you 'new' or
		 *			'->copy()' the expression before.
		 * @param b is a pointer to the second child expression. This pointer
		 *			will be used as is (no copy), so make sure you 'new' or
		 *			'->copy()' the expression before.
		 * @param parent a pointer to the parent
		*/
		SEAdd(SymbExpr *a=NULL, SymbExpr *b=NULL, SymbExpr *parent=NULL):
			SymbExpr(ADD, a, b, 0, parent) {}
		/**
		 * Copy constructor
		 * @param expr the expression to copy
		*/
		SEAdd(const SEAdd &expr): SymbExpr(expr) {}

		/** Operators */
		/**
		 * Copy operator
		 * @return a pointer to a copy of this
		*/
		virtual SEAdd* copy(void);
		/**
		 * Affectation operator
		 * @param expr the expression to be copied in this
		 * @return this
		*/
		virtual SymbExpr& operator=(const SEAdd& expr);
		/**
		 * Equality
		 * @param expr the expression to be compared with this
		 * @return if the expression reprensents the same as expr (recursively)
		*/
		virtual bool operator==(const SymbExpr& expr) const;

		/** Operation **/
		/**
		 * @return a string reprensentation of the expression
		*/
		virtual String asString(const hard::Platform *pf = 0);
		/**
		 * Try to recursively transform the expression into it its canonized
		 * form.
		*/
		virtual void canonize(void);

		virtual SymbExpr* solidifyAddress(clp::State& clpState, bool dig);
	};

	/** Compare symbolic expression
		This class define three type of compare expression:
			* condition (if): SECmp(operator, expression)
			* undetermined compare: SECmp(CMP, exp1, exp2)
			* determined compare: SECmp(operator, exp1, exp2)
	*/
	class SECmp: public SymbExpr{
	public:
		/** Constructors */
		/**
		 * Build a new Addition Symbolic Expression]
		 * @param a is a pointer to the first child expression. This pointer
		 *			will be used as is (no copy), so make sure you 'new' or
		 *			'->copy()' the expression before.
		 * @param b is a pointer to the second child expression. This pointer
		 *			will be used as is (no copy), so make sure you 'new' or
		 *			'->copy()' the expression before.
		 * @param parent a pointer to the parent
		 *
		 * This class can reprensts three forms of expressions:
		 * (oplog is in [LE, LT, GE, GT, EQ, NE])
		 *	* [oplog a] for a condition (sem instr 'if')
		 *	* [CMP a b] for an undetermined compare (sem instr 'cmp')
		 *	* [oplog a b] for a determined compare (result of canonize)
		*/
		SECmp(op_t op, SymbExpr *a=NULL, SymbExpr *b=NULL, SymbExpr *parent=NULL):
			SymbExpr(op, a, b, 0, parent) {}
		/**
		 * Copy constructor
		 * @param expr the expression to copy
		*/
		SECmp(const SECmp &expr): SymbExpr(expr) {}

		/** Operators */
		/**
		 * Copy operator
		 * @return a pointer to a copy of this
		*/
		virtual SECmp* copy(void);
		/**
		 * Affectation operator
		 * @param expr the expression to be copied in this
		 * @return this
		*/
		virtual SymbExpr& operator=(const SECmp& expr);
		/**
		 * Not equal operator
		 * @param expr the expression to be compared with this
		 * @return if the expression doesn't reprensents the same as expr
		*/
		virtual bool operator==(const SymbExpr& expr) const;

		/** Operation **/
		/**
		 * @return a string reprensentation of the expression
		*/
		virtual String asString(const hard::Platform *pf = 0);
		/**
		 * Try to recursively transform the expression into it its canonized
		 * form.
		*/
		virtual void canonize(void);
		/**
		 * @return the logical not condition
		*/
		SECmp* logicalNot(void);

		/**
		 * Check if the expression is valid
		 */
		bool isValid(void);
	};


	extern Identifier<Vector<SECmp *> > REG_FILTERS;
	extern Identifier<Vector<SECmp *> > ADDR_FILTERS;

	class FilterBuilder {
	public:
		FilterBuilder(BasicBlock *_bb, clp::ClpProblem& problem);
	private:
		void getFilters(void);
		void iterateBranchPaths(const BasicBlock::Bundle& branchBundle, const Vector<BasicBlock::Bundle>& bundles);
		sem::cond_t reverseCond(sem::cond_t cond);
		SECmp *makeFilters(SECmp *se, const BasicBlock::Bundle& currentBundle, sem::Block& b, bool branch);
		void addFilters(SECmp *se, const Vector<BasicBlock::Bundle>& bundles);

		void prepareSemBlockPaths(Vector<sem::Block>& semBlocks, const sem::Block& b);

		BasicBlock *bb;
		// filters for the taken edge
		Vector<SECmp *> reg_filters;
		Vector<SECmp *> addr_filters;

		// filters for the non-taken edge
		Vector<SECmp *> reg_filters_not;
		Vector<SECmp *> addr_filters_not;

		// filters for the currently processing path
		Vector<SECmp *> curr_reg_filters;
		Vector<SECmp *> curr_addr_filters;
		Vector<V> curr_known_reg;
		Vector<V> curr_known_addr;

		clp::ClpStatePack pack;
	};


	/**
	 * Apply a filter on the value
	 * @param v the CLP to be filtred
	 * @param cmp_op compare operator
	 * @param f CLP to filter with
	*/
	void applyFilter(V &v, se::op_t cmp_op, V f);
} // se

} // otawa

inline io::Output& operator<<(io::Output& out, otawa::se::SymbExpr *sym)
	{ out << "=> "; sym->print(out); return out; }
// output
inline elm::io::Output& operator<<(elm::io::Output& out, otawa::se::SymbExpr &se) {
	cerr << "ici !\n";
	se.print(out);
	return out;
}

// used to build sem::Block for different paths when a Block contains if(s)
class SemInstNode {
	int semPC;
	bool parentCondition;
	SemInstNode* parent;
	SemInstNode* lChild;
	SemInstNode* rChild;
public:
	inline ~SemInstNode(void) { }
	inline SemInstNode(int pc, bool b, SemInstNode* p=NULL, SemInstNode* l=NULL, SemInstNode* r=NULL): semPC(pc), parentCondition(b), parent(p), lChild(l), rChild(r) { }
	inline int getPC(void) { return semPC; }
	inline SemInstNode* getParent(void) { return parent; }
	inline bool getCond(void) { return parentCondition; }
};


#endif /* OTAWA_DATA_CLP_SYMBOLICEXPR_H_ */
