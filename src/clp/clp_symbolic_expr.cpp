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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <cmath>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/prog/Inst.h>
#include <otawa/prog/sem.h>
#include <otawa/data/clp/ClpValue.h>
#include <otawa/data/clp/ClpState.h>
#include <otawa/data/clp/ClpPack.h>
#include <otawa/data/clp/SymbolicExpr.h>
#include <otawa/flowfact/features.h>
#include <otawa/hard/Platform.h>
#include <otawa/sem/PathIter.h>

// Filters:
// A filter is represent as a simple SEComp (Symbolic expression for comparison) pointer. The reg/addr filters
// associated with a BB are store in a Vector<SEComp*>. If there are multiple paths within a BB that involves
// the BRANCH, the SEComps for each path is then separated and associated with an OR-typed SEComp* as a "flag".
// i.e.
// [0] cmp t2, ?113, t2
// [1] if lt, t2, 1
// [2] branch t1
// [3] seti t3, 0x4800 (18432)
// [4] cmp t4, ?113, t3
// [5] if gt, t4, 2
// [6] seti t4, 0x1 (1)
// [7] seti t4, 0x0 (0)
// [8] set ?109, t4
//
// This will create two paths for the branch-taken scenario, one for [5] to be true, and one for [5] to be false.
// For the first path, reg filters of (r113 > t2), (r113 > 0x4800) are created.
// For the second path, reg filters of (r113 > t2), (r113 <= 0x4800) are created.
// Hence the register filters for the underlying code will be (r113 > t2), (r113 > 0x4800), ( OR ), (r113 > t2), (r113 <= 0x4800)
// The evaluation will put the domain values at beginning of the block, through two sets of the filters, and join them together.
// It is noted that (r113 > 0x4800) and (r113 <= 0x4800) complements with each other, the resulted domain will be the same as filtered with only (r113 > t2)

// Debug output for get filters
#define TRACEGF(t)	//t

namespace otawa{

namespace se{

	/************ SymbExpr methods ************/
	SymbExpr* SymbExpr::copy(void){
		SymbExpr *newa = NULL;
		SymbExpr *newb = NULL;
		if (_a != NULL)
			newa = _a->copy();
		if (_b != NULL)
			newb = _b->copy();
		return new SymbExpr(_op, newa, newb, _val);
	}
	SymbExpr& SymbExpr::operator=(const SymbExpr& expr){
		_op = expr._op;
		set_a(expr._a);
		set_b(expr._b);
		_val = expr._val;
		return *this;
	}
	bool SymbExpr::operator==(const SymbExpr& expr) const {
		return (_op == expr._op);
	}
	bool SymbExpr::operator!=(const SymbExpr& expr) const {
		return ! operator==(expr);
	}
	void SymbExpr::replace(SymbExpr *searched_se, SymbExpr *new_se){
		if (_a != NULL && *_a == *searched_se){
			set_a(new_se);
		} else if (_a != NULL /* && *_a != *searched_se */ )
			_a->replace(searched_se, new_se);
		if (_b != NULL && *_b == *searched_se){
			set_b(new_se);
		} else if (_b != NULL /* && *_b != *searched_se */ )
			_b->replace(searched_se, new_se);
	}
	String SymbExpr::asString(const hard::Platform *pf) { return "[generic]"; }
	void SymbExpr::print(io::Output& out, const hard::Platform *pf) { out << asString(pf); }
	void SymbExpr::canonize(void){
		if (_a)
			_a->canonize();
		if (_b)
			_b->canonize();
	}
	genstruct::Vector<V> SymbExpr::used_reg(void){
		// recursively get reg for sub-expr
		genstruct::Vector<V> a_reg, b_reg;
		if (_a)
			a_reg = _a->used_reg();
		if (_b)
			b_reg = _b->used_reg();
		// merge values of the b vector in the a vector
		for(int i=0; i < b_reg.length(); i++)
			if (! a_reg.contains(b_reg[i]))
				a_reg.add(b_reg[i]);
		return a_reg;
	}
	genstruct::Vector<V> SymbExpr::used_addr(void){
		// recursively get addr for sub-expr
		genstruct::Vector<V> a_addr, b_addr;

		if (_a)
			a_addr = _a->used_addr();
		if (_b)
			b_addr = _b->used_addr();
		// merge values of the b vector in the a vector
		for(int i=0; i < b_addr.length(); i++)
			if (! a_addr.contains(b_addr[i]))
				a_addr.add(b_addr[i]);

		return a_addr;
	}

	SymbExpr* SymbExpr::solidifyAddress(clp::State& clpState, bool dig) {
		if(_a) {
			_a = _a->solidifyAddress(clpState, dig);
			if(_a)
				_a->canonize();
		}
		if(_b) {
			_b = _b->solidifyAddress(clpState, dig);
			if(_b)
				_b->canonize();
		}
		if(!_a && !_b)
			return NULL;
		if(!_a)
			return _b;
		if(!_b)
			return _a;
		return this;

	}

	/************ SEConst methods ************/
	SEConst* SEConst::copy(void){ return new SEConst(_val); }
	SymbExpr& SEConst::operator=(const SEConst& expr){
		_op = expr._op;
		_val = expr._val;
		return *this;
	}
	bool SEConst::operator==(const SymbExpr& expr) const {
		return (_op == expr.op() && _val == expr.val());
	}
	String SEConst::asString(const hard::Platform *pf){
		if(_val.isConst())
			return (_ << "0x" << hex(_val.lower()));
		else if (_val.kind() == clp::ALL)
			return (_ << 'T');
		else {
			string temp;
			temp = temp << "(0x" << hex(_val.lower()) << ", 0x" << hex(_val.delta());
			if(_val.mtimes()!=-1)
				temp = temp << ", 0x" << hex(_val.mtimes()) << ")";
			else
				temp = temp << ", inf)";
			return temp;
		}
	}
	void SEConst::canonize(void){}

	SymbExpr* SEConst::solidifyAddress(clp::State& clpState, bool dig) {
		return new SEConst(_val, _parent);
	}

	/************ SEAddr methods ************/
	SEAddr* SEAddr::copy(void){ return new SEAddr(_val, _a?_a->copy():NULL); }
	SymbExpr& SEAddr::operator=(const SEAddr& expr){
		_op = expr._op;
		_val = expr._val;
		return *this;
	}
	bool SEAddr::operator==(const SymbExpr& expr) const {
		return (_op == expr.op() && _val == expr.val());
	}
	String SEAddr::asString(const hard::Platform *pf) {
		if (_a) { // the address is an expression
			return (_ << "@" << _a->asString(pf));
		}
		else if(_val.isConst()) {
			return (_ << "@" << hex(_val.lower()));
		}
		else
			return (_ << "@(0x" << hex(_val.lower()) \
					  << ", 0x" << hex(_val.delta()) \
					  << ", 0x" << hex(_val.mtimes()) << ')');
	}
	void SEAddr::canonize(void) {
		// recursive call
		if (_a)
			_a->canonize();

		// when _val == -1, this tells that before _a is canonized, this SEAddr does not contain a solid address value
		// hence _a will be checked to see if the solid address, i.e. _a is a CONST, is obtained, in this case
		// _val will be assigned to _a's value and _a will be deleted.
		// otherwise, if _a is not canonized as a constant, this SEAddr will remain as a symbolic address
		if(_a && _a->op() == CONST && _a->val() == V::top) {
			if (_parent->a() == this){
				// WILL DELETE this !
				SEConst* temp = new SEConst(V::top);
				_parent->set_a(temp);
				delete temp;
				return;
			} else if (_parent->b() == this){
				// WILL DELETE this !
				SEConst* temp = new SEConst(V::top);
				_parent->set_b(temp);
				delete temp;
				return;
			}
		}
		else if(_a && _a->op() == CONST) {
			_val = _a->val();
			delete _a;
			_a = NULL;
		}
	}

	genstruct::Vector<V> SEAddr::used_addr(void){
		genstruct::Vector<V> vect;
		if(!_a)
			vect.add(_val);
		return vect;
	}

	SymbExpr* SEAddr::solidifyAddress(clp::State& clpState, bool dig) {
		if(_a) {
			_a = _a->solidifyAddress(clpState, true);
			if(_a)
				_a->canonize();
			if(!_a)
				return NULL;

			return this;
		}
	}

	/************ SEReg methods ************/
	SEReg* SEReg::copy(void){
		return new SEReg(_val);
	}
	SymbExpr& SEReg::operator=(const SEReg& expr){
		_op = expr._op;
		_val = expr._val;
		return *this;
	}
	bool SEReg::operator==(const SymbExpr& expr) const {
		return (_op == expr.op() && _val == expr.val());
	}
	String SEReg::asString(const hard::Platform *pf){
		ASSERT(_val.isConst());
		if (_val >= 0) {
			if(pf)
				return pf->findReg(_val.lower())->name();
			else
				return (_ << 'r' << _val.lower());
		}
		else
			return (_ << 't' << - _val.lower());
	}
	void SEReg::canonize(void){}
	genstruct::Vector<V> SEReg::used_reg(void){
		genstruct::Vector<V> vect;
		vect.add(_val);
		return vect;
	}

	SymbExpr* SEReg::solidifyAddress(clp::State& clpState, bool dig) {
		ASSERT(_val.isConst());
		if(dig == false)
			return NULL;
		if(_val.lower() < 0)
			return NULL;

		clp::Value clpval = clpState.get(clp::Value(clp::REG, _val.lower()));
		if(clpval.isConst()) {
			return new SEConst(clpval, _parent);
		}
		else {
			return NULL;
		}
	}

	/************ SENeg methods ************/
	SENeg* SENeg::copy(void){
		SymbExpr *newa = NULL;
		newa = _a->copy();
		return new SENeg(newa);
	}
	SymbExpr& SENeg::operator=(const SENeg& expr){
		_op = expr._op;
		set_a(expr._a);
		return *this;
	}
	bool SENeg::operator==(const SymbExpr& expr) const {
		return (
			_op == expr.op() &&
			_a != NULL && expr.a() != NULL &&
			*_a == *(expr.a())
		);
	}
	String SENeg::asString(const hard::Platform *pf){
		return (_ << "[-| " << _a->asString(pf) << ']');
	}
	void SENeg::canonize(void){
		// recursive call
		if (_a)
			_a->canonize();

		/* In the next two case, we'll delete this, so we make sure we are
			referenced by a parent (and not created on the heap) */
		if(_parent == NULL)
			return;

		// [-, [K, <val>]] -> [K, eval(val * -1)]
		if (_a && _a->op() == CONST){
			if (_parent->a() == this){
				SEConst* temp = new SEConst(V(0) - _a->val());
				_parent->set_a(temp); // WILL DELETE this !
				delete temp;
				return;
			} else if (_parent->b() == this){
				SEConst* temp = new SEConst(V(0) - _a->val());
				_parent->set_b(temp); // WILL DELETE this !
				delete temp;
				return;
			}
		}
		// [-, [-, <expr>]] -> <expr>
		if(_a && _a->op() == NEG){
			if (_parent->a() == this){
				_parent->set_a(_a->a()); // WILL DELETE this !
				return;
			} else if (_parent->b() == this){
				_parent->set_b(_a->a()); // WILL DELETE this !
				return;
			}
		}
	}

	/************ SEAdd methods ************/
	SEAdd* SEAdd::copy(void){
		return new SEAdd(_a->copy(), _b->copy());
	}
	SymbExpr& SEAdd::operator=(const SEAdd& expr){
		_op = expr._op;
		set_a(expr._a);
		set_b(expr._b);
		return *this;
	}
	bool SEAdd::operator==(const SymbExpr& expr) const {
		return (
			_op == expr.op() &&
			_a != NULL && expr.a() != NULL &&
			*_a == *(expr.a()) &&
			_b != NULL && expr.b() != NULL &&
			*_b == *(expr.b())
		);
	}
	String SEAdd::asString(const hard::Platform *pf) {
		return (_ << "[+ " << _a->asString(pf) << ' ' << _b->asString(pf) << ']');
	}
	void SEAdd::canonize(void){
		// recursive call
		if (_a)
			_a->canonize();
		if (_b)
			_b->canonize();

		// [+, [K, <val1>], [K, <val2>]] -> [K, eval(<val1> + <val2>)]
		// This case will replace this in _parent !
		if (_parent && _a && _a->op() == CONST && _b && _b->op() == CONST){
			if (_parent->a() == this){
				// WILL DELETE this !
				SEConst* temp = new SEConst(_a->val() + _b->val());
				_parent->set_a(temp);
				delete temp;
				return;
			} else if (_parent->b() == this){
				// WILL DELETE this !
				SEConst* temp = new SEConst(_a->val() + _b->val());
				_parent->set_b(temp);
				delete temp;
				return;
			}
		}

		// [+, [K, <val1>], <expr1>] -> [+, <expr1>, [K, <val1>]]
		if (_a && _a->op() == CONST && _b){
			SymbExpr *expr = _b;
			_b = _a;
			_a = expr;
		}

		// [+, [+, V, <expr1>], <expr2>] -> [+, V, canonize([+, <expr1>, <expr2>])]
		// with V either a SEReg or a SEAddr
		if (_a && _a->op() == ADD && _a->a() &&
				(_a->a()->op() == REG || _a->a()->op() == ADDR)){
			SEAdd *newb = new SEAdd(_a->b()->copy(), _b);
			newb->canonize();
			_b = newb;
			set_a(_a->a());
		}
	}

	SymbExpr* SEAdd::solidifyAddress(clp::State& clpState, bool dig) {
		if(_a) {
			SymbExpr* newA = _a->solidifyAddress(clpState, dig);
			delete _a;
			_a = newA;
			if(_a)
				_a->canonize();
			else
				return NULL;
		}

		if(_b) {
			SymbExpr* newB = _b->solidifyAddress(clpState, dig);
			delete _b;
			_b = newB;
			if(_b)
				_b->canonize();
			else
				return NULL;
		}
		return this;
	}

	/************ SECmp utility ************/
	/* reverse a logical operator (to reverse operand in SECmp */
	op_t reverse(op_t logop){
		switch(logop){
		case LE:
			return GE;
		case LT:
			return GT;
		case GE:
			return LE;
		case GT:
			return LT;
		case ULE:
			return UGE;
		case ULT:
			return UGT;
		case UGE:
			return ULE;
		case UGT:
			return ULT;
		default:
			// EQ -> EQ; NE -> NE; others (invalid) are returned unchanged
			return logop;
		}
	}

	/************ SECmp methods ************/
	SECmp* SECmp::copy(void){
		if (_b == NULL)
			return new SECmp(_op, _a->copy());
		else
			return new SECmp(_op, _a->copy(), _b->copy());
	}
	SymbExpr& SECmp::operator=(const SECmp& expr){
		_op = expr._op;
		set_a(expr._a);
		set_b(expr._b);
		return *this;
	}
	bool SECmp::operator==(const SymbExpr& expr) const {
		if ((_op != expr.op()) || _a == NULL || expr.a() == NULL ||  (*_a != *(expr.a())))
			return false;
		if (_b == NULL && expr.b() == NULL)
			return true;
		if (_b == NULL || expr.b() == NULL)
			return false;
		else
			return (*_b == *(expr.b()));
	}
	String SECmp::asString(const hard::Platform *pf) {
		String s = "[";
		switch(_op){
		case OR:
			return "OR]";
		case CMP:
			s = s << "cmp";
			break;
		case CMPU:
			s = s << "cmpu";
			break;
		case LE:
			s = s << "<=";
			break;
		case LT:
			s = s << '<';
			break;
		case GE:
			s = s << ">=";
			break;
		case GT:
			s = s << '>';
			break;
		case EQ:
			s = s << '=';
			break;
		case NE:
			s = s << "/=";
			break;
		case ULE:
			s = s << "u<=";
			break;
		case ULT:
			s = s << "u<";
			break;
		case UGE:
			s = s << "u>=";
			break;
		case UGT:
			s = s << "u>";
			break;
		default:
			break;
		}
		s = s << ' ' << _a->asString(pf);
		if (_b != NULL)
			s = s << ' ' << _b->asString(pf);
		s = s << ']';
		return s;
	}

	void SECmp::canonize(void){
		// recursive call
		if (_a)
			_a->canonize();
		if (_b)
			_b->canonize();

		if (_op == CMP || _op == CMPU)
			return;		// we need a determinated CMP for further canonization

		bool cancont;

		do{
			cancont=false;

			// [<log_op>, [cmp, <expr1>, <expr2>]] -> [<log_op>, <expr1>, expr2>]
			if (_a && _a->op() == CMP && _b == NULL){
				set_b(_a->b()); // we must set b first, because we'll erase _a
				set_a(_a->a());
				cancont = true;
			}
			// The same for unsigned compare
			if (_a && _a->op() == CMPU && _b == NULL){
				/* change the operator into unsigned form */
				switch(_op){
				case LE:
					_op = ULE;
					break;
				case LT:
					_op = ULT;
					break;
				case GE:
					_op = UGE;
					break;
				case GT:
					_op = UGT;
					break;
				default:
					break;
				}
				set_b(_a->b()); // we must set b first, because we'll erase _a
				set_a(_a->a());
				cancont = true;
			}

			// [<log_op>, [K, <valeur>], <expr>] && <expr> != const
			// -> [reverse(<log_op>), <expr>, [K, <valeur>]]
			if (_a && _b && _a->op() == CONST && _b->op() != CONST){
				_op = reverse(_op);
				SymbExpr *expr = _b;
				_b = _a;
				_a = expr;
				cancont = true;
			}

			// [<log_op>, [+, <expr0>, <expr1>], <expr2>]
			// -> [<log_op>, <expr0>, canonize([+, <expr2>, [-, <expr1>]])]
			if (_a && _b && _a->op() == ADD && _a->a() && _a->b()){
				SEAdd newb = SEAdd(_b->copy(), new SENeg(_a->b()->copy()));
				set_b(&newb);
				set_a(_a->a());
				_b->canonize();
				cancont = true;
			}

			// [<log_op>, [-, <expr0>], expr1]
			// -> [reverse(<log_op>), <expr0>, canonize([-, <expr1>])]
			if (_a && _b && _a->op() == NEG && _a->a()){
				_op = reverse(_op);
				set_a(_a->a());
				SymbExpr *newb = new SENeg(_b->copy());
				set_b(newb);
				delete newb;
				_b->canonize();
				cancont = true;
			}
		}while(cancont);
	}

	SECmp* SECmp::logicalNot(void){
		/* not the logical operator */
		op_t newop;
		switch(_op){
		case LE:
			newop = GT;
			break;
		case LT:
			newop = GE;
			break;
		case GE:
			newop = LT;
			break;
		case GT:
			newop = LE;
			break;
		case EQ:
			newop = NE;
			break;
		case NE:
			newop = EQ;
			break;
		case ULE:
			newop = UGT;
			break;
		case ULT:
			newop = UGE;
			break;
		case UGE:
			newop = ULT;
			break;
		case UGT:
			newop = ULE;
			break;
		case OR:
			newop = NONE;
			break;
		default:
			// others (invalid) are returned unchanged
			newop = _op;
			break;
		}
		SECmp *notse = new SECmp(newop, _a->copy(), _b->copy());
		return notse;
	}

	bool SECmp::isValid(void) {
		if(_a->op() != _b->op())
			return true; // unknown, so guess it is true

		if((_a->op() == CONST) && (_b->op() == CONST)) { // then we can evaluate directly
			switch(_op){
			case LE:
				return (_a->val().lower() <= _b->val().lower());
			case LT:
				return (_a->val().lower() < _b->val().lower());
			case GE:
				return (_a->val().lower() >= _b->val().lower());
			case GT:
				return (_a->val().lower() > _b->val().lower());
			case EQ:
				return (*_a == *_b);
			case NE:
				return (*_a != *_b);
			case ULE: // FIXME: to implement
				return ((clp::uintn_t)(_a->val().lower()) <= (clp::uintn_t)(_b->val().lower()));
			case ULT:
				return ((clp::uintn_t)(_a->val().lower()) < (clp::uintn_t)(_b->val().lower()));
			case UGE:
				return ((clp::uintn_t)(_a->val().lower()) >= (clp::uintn_t)(_b->val().lower()));
			case UGT:
				return ((clp::uintn_t)(_a->val().lower()) > (clp::uintn_t)(_b->val().lower()));
			default:
				elm::cerr << "SECmp::isValid(): WARNING! unable to determine the validity for " << this->asString() << io::endl;
				return true;
			} // end of the switch
		} // end of CONST comparisons
		else if((_a->op() == REG) && (_b->op() == REG)) // always true for relations with registers
			return true;
		else if((_a->op() == ADDR) && (_b->op() == ADDR)) // always true for relations with registers
			return true;
		else {
			switch(_op){
			case EQ:
				return (*_a == *_b);
			case NE:
				return (*_a != *_b);
			case LT:
			case GE:
			case GT:
			case LE:
			case ULE:
			case ULT:
			case UGE:
			case UGT:
			default:
				elm::cerr << "SECmp::isValid(): WARNING! unable to determine the validity for " << this->asString() << io::endl;
				return true;
			}


		}

		return true;
	}

	/*
	 * To output a set of filters
	 */
	Output& operator<<(Output& o, genstruct::Vector<SECmp *> const& exprs) {
		bool fst = true;
		o << "{";
		for(genstruct::Vector<SECmp *>::Iterator vsei(exprs); vsei; vsei++) {
			if(!fst)
				o << ", ";
			else
				fst = false;

			o << vsei->asString();
		}
		o << "}";
		return o;
	}


	Identifier<genstruct::Vector<SECmp *> > REG_FILTERS("otawa::se::REG_FILTERS");
	Identifier<genstruct::Vector<SECmp *> > ADDR_FILTERS("otawa::se::ADDR_FILTERS");

	SECmp *getFilterForReg(SECmp *se, V reg, clp::ClpStatePack &pack, Inst *i, int sem, genstruct::Vector<V> &used_reg, genstruct::Vector<V> &used_addr){
		/* FIXME : This could be otptimized: we do a CLP analysis from the
			beginning of the BB each time we replace a register by its value */
		clp::State state = pack.state_before(i->address(), sem);

		ASSERT(reg.isConst());

		// replace other registers
		for (int i=0; i < used_reg.length(); i++){
			ASSERT(used_reg[i].isConst());
			if(used_reg[i].lower() != reg.lower()){
				// get the actual value of used_reg[i]
				clp::Value clpval = state.get(clp::Value(clp::REG, used_reg[i].lower()));
				SEConst *val = new SEConst(clpval);
				SEReg *r = new SEReg(used_reg[i]);
				se->replace(r, val);
				delete r;
				delete val;
			}
		}

		// replace other memory refs
		for (int i=0; i < used_addr.length(); i++){
			ASSERT(used_addr[i].isConst());
			// get the actual value of used_addr[i]
			clp::Value clpval = state.get(used_addr[i]);
			SEConst *val = new SEConst(clpval);
			SEAddr *a = new SEAddr(used_addr[i]);
			se->replace(a, val);
			delete a;
			delete val;
		}

		// canonize
		se->canonize();
		// check if we have a filter
		if (se->op() > CMPU && se->a() && se->a()->op() == REG && se->b() && se->b()->op() == CONST /*&& (se->b()->val() != V::all)*/) {
			                // a exists   // a is a REG         // b exits   // b is a CONST
			if (se->b()->val() == V::top) {
				return NULL;
			}
			else
				return se;
		}
		else{
			TRACEGF(cerr << "Bad filter: " << se->asString() << "\n";)
			return NULL;
		}
	}

	SECmp *getFilterForAddr(SECmp *se, V addr, clp::ClpStatePack &pack, const Bundle &i, int sem, genstruct::Vector<V> &used_reg, genstruct::Vector<V> &used_addr){
		/* FIXME: this could be otptimized: we do a CLP analysis from the
			beginning of the BB each time we replace an address by its value */
		clp::State state = pack.state_before(i.address(), sem);
		ASSERT(addr.isConst());
		while(1) {
			genstruct::Vector<V> used_reg = se->used_reg();
			genstruct::Vector<V> used_addr = se->used_addr();

			// replace other registers
			for (int i=0; i < used_reg.length(); i++){
				ASSERT(used_reg[i].isConst());
				// get the actual value of used_reg[i]
				clp::Value clpval = state.get(clp::Value(clp::REG, used_reg[i].lower()));
				SEConst *val = new SEConst(clpval);
				SEReg *r = new SEReg(used_reg[i]);
				se->replace(r, val);
				delete r;
				delete val;
			}

			// replace other memory refs
			for (int i=0; i < used_addr.length(); i++){
				ASSERT(used_addr[i].isConst());
				if (used_addr[i] != addr){
					// get the actual value of used_addr[i]
					clp::Value clpval = state.get(used_addr[i]);
					SEConst *val = new SEConst(clpval);
					SEAddr *a = new SEAddr(used_addr[i]);
					se->replace(a, val);
					delete a;
					delete val;
				}
			}

			SECmp* temp = se->copy();
			// canonize
			se->canonize();
			// check if we have a filter
			if(*temp == *se) {
				delete temp;
				break;
			}
			else {
				delete temp;
			}
		}

		if (se->op() > CMPU && se->a() && se->a()->op() == ADDR && se->b() && se->b()->op() == CONST /*&& (se->b()->val() != V::all)*/) {
			if (se->b()->val() == V::top)
				return NULL;
			if (!se->a()->val().isConst())
				return NULL;
			if (se->b()->val().isInf())
				return NULL;

			return se;
		}
		else{
			TRACEGF(cerr << "Bad filter: " << se->asString() << "\n";)
			return NULL;
		}
	}

	/**
	 * Build a filter and install them on the current BB.
	 * @param _bb	BB to work on.
	 */
	FilterBuilder::FilterBuilder(BasicBlock *_bb, clp::ClpProblem& problem)
	: bb(_bb), pack(bb, clp::ClpStatePack::Context(problem)) {
		getFilters();
	}

	/**
	 * Find filters that apply on the basic block
	 * Two properties are sets:
	 *		REG_FILTERS for filters on registers
	 *		ADDR_FILTERS for filters on memory addresses
	*/
	void FilterBuilder::getFilters(void){
		Vector<Bundle> bundles;
		Bundle branchBundle = 0;
		for(BasicBlock::BundleIter bbbi(bb); bbbi; bbbi++) {
			// find the branch bundle
			bool foundControl = false;
			if((*bbbi).kind() & Inst::IS_CONTROL) {
				for(Bundle::Iter bi(*bbbi); bi; bi++)
					if(bi->isControl() && !IGNORE_CONTROL(bi))
						if(bi->isConditional())
							branchBundle = *bbbi;
			}
			// add every bundle besides the branching bundle
			if(	(branchBundle == 0) ||
				((branchBundle != 0)  && (branchBundle.address() != (*bbbi).address())) ) {
				bundles.add(*bbbi);
			}
		}

		iterateBranchPaths(branchBundle, bundles); // create the filters, starting with the bundle that contains the branch

		REG_FILTERS(bb) = reg_filters;
		ADDR_FILTERS(bb) = addr_filters;

		for(BasicBlock::EdgeIter bbei=bb->outs(); bbei; bbei++) {
			if(bbei->isTaken()) {
				REG_FILTERS(bbei) = reg_filters;
				ADDR_FILTERS(bbei) = addr_filters;
			}
			else if(bbei->isNotTaken()) {
				REG_FILTERS(bbei) = reg_filters_not;
				ADDR_FILTERS(bbei) = addr_filters_not;
			}
		}
	}


	/**
	 * Add the filters for the current instruction list (taken backward).
	 * makeFilters is called for every instructions in the insts
	 * @param se		Current conditional branch comparison.
	 * @param insts		Instructions of the block.
	 */
	//void FilterBuilder::addFilters(SECmp *se, const Vector<Inst *>& insts) {
	void FilterBuilder::addFilters(SECmp *se, const Vector<Bundle>& bundles) {
		sem::Block block;
		TRACEGF(String out);
		for(int i = bundles.count() - 1; i >= 0; i--) {
			TRACEGF(out = _ << bundles[i].address() << '\n' << out);
			block.clear();
			bundles[i].semInsts(block);
			se = makeFilters(se, bundles[i], block, false);
		}
		TRACEGF(cerr << out);
		delete se;
	}

	// initialization
	typedef struct path_t {
		inline path_t(void): i(0), n(0), b(false) { }
		inline path_t(int _i, int _n, bool _b): i(_i), n(_n), b(_b) { }
		int i;	// i in istack
		int n;	// n in block
		bool b;	// branch found
	} path_t;

	/**
	 * Iterate on all semantics execution paths and call makeFilters().
	 * @param branchBundle	The bundle which contains the brach sem inst as the starting point of making the filters.
	 * @param bundles		The list of bundles to carry out filter making for each bundles in the processing BB after the branchBundle.
	 */
	void FilterBuilder::iterateBranchPaths(const Bundle& branchBundle, const Vector<Bundle>& bundles) {
		ASSERT(branchBundle);

		bool first = true;
		bool first_not = true;
		Vector<path_t> pstack;
		sem::Block semInstStack, block;
		branchBundle.semInsts(block);

		pstack.push(path_t(0, 0, false));
		while(pstack) {
			path_t path = pstack.pop();
			semInstStack.setLength(path.i);
			if(path.i != 0) { // besides the first pop, we reverse the condition as each popping is the "alternative path"
				semInstStack[path.i - 1]._d = reverseCond(sem::cond_t(semInstStack[path.i - 1].d()));
			}
			// Traverse instructions of the path. The path.n is the current index of the semantic instruction being inspected.
			while(path.n < block.count()) {
				if(block[path.n].op == sem::IF)
				{
					if(block[path.n].cond() == sem::NO_COND)
						path.n += block[path.n].b();
					else {
						semInstStack.push(block[path.n]);
						pstack.push(path_t(semInstStack.length(), path.n + block[path.n].b() + 1, path.b)); // add the alternative path to the work list
					}
				}
				else if(block[path.n].op == sem::CONT)
				{
					semInstStack.push(block[path.n]);
					break;
				}
				else if(block[path.n].op == sem::BRANCH)
				{
					semInstStack.push(block[path.n]);
					path.b = true;
				}
				else
					semInstStack.push(block[path.n]);

				path.n++;
			}

			// process the path
			if(path.b) {
				if(first)
					first = false;
				else {
					reg_filters.add(new SECmp(OR));
					addr_filters.add(new SECmp(OR));
				}
				// we have the control instruction inst, and its previous semantic instructions, we can
				// use the previous sem insts to build an expression the condition of the branch
				// the filter is making in the reverse order, so that the expression is reduced and enriched by the previous sem inst.
				// We first obtain the expression from the branch instruction. The semInstStack only contains the semantic instructions of the branchInst
				SECmp *se = makeFilters(NULL, branchBundle, semInstStack, true);
				// Now we use the just-created se to explore more conditions by following up the previous sem insts.
				addFilters(se, bundles);

				reg_filters.addAll(curr_reg_filters);
				addr_filters.addAll(curr_addr_filters);
				curr_reg_filters.clear();
				curr_addr_filters.clear();
				curr_known_reg.clear();
				curr_known_addr.clear();
			}
			else {
				if(first_not)
					first_not = false;
				else {
					reg_filters_not.add(new SECmp(OR));
					addr_filters_not.add(new SECmp(OR));
				}
				SECmp *se = makeFilters(NULL, branchBundle, semInstStack, true);
				addFilters(se, bundles);

				reg_filters_not.addAll(curr_reg_filters);
				addr_filters_not.addAll(curr_addr_filters);
				curr_reg_filters.clear();
				curr_addr_filters.clear();
				curr_known_reg.clear();
				curr_known_addr.clear();
			}
		} // end of pstack
	} // end of function

	/**
	 * Prepare semantic blocks according to the path within a given bundle.
	 * This function is used by makeFilters.
	 * @semBlocks 	the resulted blocks
	 * @b			the original semantic block
	 */
	void FilterBuilder::prepareSemBlockPaths(Vector<sem::Block>& semBlocks, const sem::Block& b) {
		// If there is a single IF, and a single BRANCH, then the treated block is of a conditional branch.
		// Since we are only interested in the taken path's filter, we just return one path in this case
		int numberOfIFs = 0;
		int numberOfBRANCHs = 0;
		Vector<SemInstNode*> leafNodes;
		Vector<SemInstNode*> toCleanUp;
		Vector<Pair<Pair<int, SemInstNode*>, bool> > workList; // double pair to create a triple of (next pc to follow, the node, and parent condition is true or false)
		SemInstNode* root = new SemInstNode(-1, NULL); // root
		toCleanUp.add(root);
		workList.push(pair(pair(0, root), true));
		while(workList.count()) {
			Pair<Pair<int, SemInstNode*>,bool> popped = workList.pop();
			SemInstNode* current = popped.fst.snd;
			bool cond = popped.snd;
			int i = 0;
			for(i = popped.fst.fst; i < b.length(); i++) {
				const sem::inst& si = b[i];
				if(si.op == sem::IF) {
					numberOfIFs++;
					SemInstNode* temp = new SemInstNode(i, cond, current);
					toCleanUp.add(temp);
					if(si.cond() != sem::NO_COND) // always jump
						workList.push(pair(pair(i+1, temp), true));
					workList.push(pair(pair(i+si.b()+1, temp), false));
					break;
				}
				else {
					SemInstNode* temp = new SemInstNode(i, cond, current);
					toCleanUp.add(temp);
					cond = true; // reset the value in case cond can be false
					current = temp;
				}

				if(si.op == sem::BRANCH)
					numberOfBRANCHs++;
			} // reaching the end of the sem block, or encountering IF

			// collecting the final block of the path when reaching the end of the block (i/pc >= block size)
			if(i >= b.length())
				leafNodes.add(current);
		}

		bool singlePath = false;
		if (numberOfIFs == 1 && numberOfBRANCHs == 1) { // normal case for conditional branch
			// then we just return one path
			singlePath = true;
			semBlocks.add(b);
		}
		else if (numberOfIFs > 1 && numberOfBRANCHs >= 1) { // need to see if this case ever happens
			// ASSERT(0); // just comment this out when this happens, want to observe this.
		}

		// constructing the sem blocks to process
		for(int i = 0; (i < leafNodes.count()) && (singlePath == false); i++) {
			semBlocks.add(sem::Block());
			SemInstNode* sin = leafNodes[i];
			bool parentCond = true;
			while(sin) {
				if(sin->getPC() != -1) {
					if(parentCond == false) { // need to reverse
						if(b[sin->getPC()].cond() != sem::NO_COND)  // no need to add no cond
							semBlocks[i].addFirst(sem::_if(invert(b[sin->getPC()].cond()), b[sin->getPC()].a(), b[sin->getPC()].b()));
					}
					else
						semBlocks[i].addFirst(b[sin->getPC()]);
					parentCond = sin->getCond();
				}
				sin = sin->getParent();
			}
		}
		for(Vector<SemInstNode*>::Iter toClean(toCleanUp); toClean; toClean++)
			delete *toClean;
	}


	/**
	 * Accumulate in the reg_filters and addr_filters for filter building.
	 * This function works in the reverse order of the semantic instruction so that the expression can be deduced by the previous (lower address) semantic instructions.
	 * given block.
	 * @param se_orig		Current comparison.
	 * @param currentBundle	Current bundle.
	 * @param b				Block to work on.
	 */
	SECmp *FilterBuilder::makeFilters(SECmp *se_orig, const Bundle& currentBundle, sem::Block& bb, bool branch) {
		typedef genstruct::Vector<SECmp *> filters_t;
		typedef genstruct::Vector<V> regs_t;
		typedef genstruct::Vector<V> addrs_t;

		genstruct::Vector<filters_t> temp_reg_filters;
		genstruct::Vector<filters_t> temp_addr_filters;
		genstruct::Vector<regs_t> temp_known_reg;
		genstruct::Vector<addrs_t> temp_known_addr;
		SECmp *seToReturn = 0;

		Vector<sem::Block> semBlocks; // each path is associated with a sem::Block
		if(branch)
			semBlocks.add(bb);
		else
			prepareSemBlockPaths(semBlocks, bb);

		for(int bi = 0; bi < semBlocks.count(); bi++) {
			temp_reg_filters.add(filters_t());
			temp_addr_filters.add(filters_t());
			temp_known_reg.add(regs_t());
			temp_known_addr.add(addrs_t());
		}

		SECmp *se = 0;
		bool pathFailed = false;

		for(int bi = 0; bi < semBlocks.count(); bi++) {
			// prepare the se for this path
			if(se_orig) // make a copy of se_orig
				se = se_orig->copy();
			else
				se = 0;

			sem::Block& b = semBlocks[bi];
			// traverse reversely in the given semantic instruction block
			for(int pc=b.length() - 1; pc >= 0; pc--){
				sem::inst& i = b[pc];
				// build the matching SE
				switch(i.op) {

				case sem::IF: { // If inst is a if:
						// create a new symbexpr
						op_t log_op = NONE;
						switch(i.cond()){
						case sem::LE: 		log_op = LE; break;
						case sem::LT: 		log_op = LT; break;
						case sem::GE: 		log_op = GE; break;
						case sem::GT: 		log_op = GT; break;
						case sem::EQ: 		log_op = EQ; break;
						case sem::NE:		log_op = NE; break;
						case sem::ULE: 		log_op = ULE; break;
						case sem::ULT: 		log_op = ULT; break;
						case sem::UGE: 		log_op = UGE; break;
						case sem::UGT: 		log_op = UGT; break;
						case sem::ANY_COND:	log_op = NONE; break;
						//default:			ASSERTP(false, "unsupported condition " << i.cond() << " at " << cur_inst->address()); break;
						default:			ASSERTP(false, "unsupported condition " << i.cond() << " at " << currentBundle.address()); break;
						}
						if(log_op) {
							temp_known_reg[bi].clear();
							temp_known_addr[bi].clear();
							// FIXME
							// this means if there are more than one IF semantic instructions to process, the one processed first will
							// be removed from the earth
							if(se)
								delete se;

							se = new SECmp(log_op, new SEReg(i.a()));
						}
					}
					break;

				case sem::SET:
					if(se) {
						SEReg *rd = new SEReg(i.d());
						SEReg *rs = new SEReg(i.a());
						se->replace(rd, rs);
						delete rd;
						delete rs;
					}
					break;

				// If inst is another instruction: replace
				case sem::LOAD:
					if (se){
						SEReg *rd = new SEReg(i.d());
						// get the address of the register i.a()
						//clp::State state = pack.state_after(cur_inst->address(), pc);
						clp::State state = pack.state_after(currentBundle.address(), pc);
						if(i.a() >= 0) {
							clp::Value val = state.get(clp::Value(clp::REG, i.a()));
							if (val != clp::Value::all){
								if(!val.isConst()){
									cerr << "WARNING: unconst address: " << val << endl;
									// if val is a set, we cannot insert the memory
									// reference in the filter
									// TODO: maybe we should 'fork' the filter?
									// For the moment, if the load concern this expr
									// we set the se to NULL, to
									// invalidate the register i.a()
									genstruct::Vector<V> used_reg = se->used_reg();
									for(int i = 0; i < used_reg.length(); i++){
										if(used_reg[i] == rd->val()){
											delete se;
											se = NULL;
											break;
										}
									}
								} else {
									SEAddr *a = new SEAddr(val.lower());
									se->replace(rd, a);
									delete a;
								}
							}

							else {
								SEAddr *a = new SEAddr(-1 ,new SEReg(i.a()));
								se->replace(rd, a);
								delete a;
							}
						} // if register to specify the address is not a temp register
						else {
							SEAddr *a = new SEAddr(-1 ,new SEReg(i.a()));
							se->replace(rd, a);
							delete a;
						}

						delete rd;
					}
					break;

				case sem::CMPU:
					if (se){
						SEReg *rd = new SEReg(i.d());
						SECmp *cmp = new SECmp(CMPU, new SEReg(i.a()), new SEReg(i.b()));
						se->replace(rd, cmp);
						delete rd;
						delete cmp;
					}
					break;

				case sem::CMP:
					if (se){
						SEReg *rd = new SEReg(i.d());
						SECmp *cmp = new SECmp(CMP, new SEReg(i.a()), new SEReg(i.b()));
						se->replace(rd, cmp);
						delete rd;
						delete cmp;
					}
					break;

				case sem::SETI:
					if (se){
						SEReg *rd = new SEReg(i.d());
						SEConst *c = new SEConst(i.a());
						se->replace(rd, c);
						delete rd;
						delete c;
					}
					break;

				case sem::ADD:
					if (se){
						SEReg *rd = new SEReg(i.d());
						SEAdd *add = new SEAdd(new SEReg(i.a()), new SEReg(i.b()));
						se->replace(rd, add);
						delete rd;
						delete add;
					}
					break;

				case sem::SUB:
					if (se){
						SEReg *rd = new SEReg(i.d());
						SEAdd *sub = new SEAdd(new SEReg(i.a()), new SENeg(new SEReg(i.b())));
						se->replace(rd, sub);
						delete rd;
						delete sub;
					}
					break;

				case sem::SCRATCH:
					if (se){
						SEReg *rd = new SEReg(i.d());
						// if the register rd is used in the expression, we set
						// se to NULL: we cannot find any further filter where
						// rd is implied.
						genstruct::Vector<V> used_reg = se->used_reg();
						for(int i = 0; i < used_reg.length(); i++){
							if(used_reg[i] == rd->val()){
								delete se;
								se = NULL;
								break;
							}
						}
						delete rd;
					}
					break;

				default:
					// wipe out the expression as the default behavior
					delete se;
					se = NULL;
					break;

				} // end of switch
				TRACEGF(String tmpout = _ << '\t' << i);

				if(se) {
					TRACEGF(tmpout = tmpout << "\t\t=> " << se->asString());
					se->canonize();
					TRACEGF(tmpout = tmpout << "\t\t=canonize=> " << se->asString());
					// find filters...
					// This is carried out by looking at the expression. If the expression does not contain any temporary registers
					// then that means the expression is fully resolved.
					if (se->op() > CMPU && se->a() && se->b()){

						SymbExpr* se2 = se->copy();
						clp::State state = pack.state_before(currentBundle.address(), pc);
						se2 = se2->solidifyAddress(state, false);

						genstruct::Vector<V> used_reg = se->used_reg();
						genstruct::Vector<V> used_addr;
						if(se2)
							used_addr = se2->used_addr();
						bool has_tmp = false;
						for(int i = 0; i < used_reg.length(); i++)
							if (! (used_reg[i] >= 0))
								has_tmp = true;
						if (!has_tmp){
							// check if there is a violation on the expression
							if (se->op() > CMPU && se->a() && se->b()) {
								if(!se->isValid()) {
									pathFailed = true;
									break;
								}
							}
							// Reach here when the symbolic expression can be used to create a filter.
							// for each new register
							for(int i = 0; i < used_reg.length(); i++) {
								if(!curr_known_reg.contains(used_reg[i]) && !temp_known_reg[bi].contains(used_reg[i])) { // one register can only be added once
									// get the filter
									SECmp *newfilter = getFilterForReg(se->copy(), used_reg[i], pack, currentBundle, pc, used_reg, used_addr);
									if (newfilter){
										TRACEGF(tmpout = _ << "\t\t\tNew filter: " << newfilter->asString() << '\n' << tmpout);
										temp_reg_filters[bi].add(newfilter);
										temp_known_reg[bi].add(used_reg[i]);
									}
								}
							} // end of processing current register filters

							// for each new addr
							for(int i = 0; i < used_addr.length(); i++) {
								if(!curr_known_addr.contains(used_addr[i]) && !temp_known_addr[bi].contains(used_addr[i])) { // one memory address can be only added once
									// get the filter
									SECmp *newfilter = getFilterForAddr(se->copy(), used_addr[i], pack, currentBundle, pc, used_reg, used_addr);
									if (newfilter){
										TRACEGF(tmpout = _ << "\t\t\tNew filter: " << newfilter->asString() << '\n' << tmpout);
										temp_addr_filters[bi].add(newfilter);
										temp_known_addr[bi].add(used_addr[i]);
									}
								}
							} // end of processing current address filters
						} // end of no temp register involved
					} // end of CMP
				} // end if(se)

				TRACEGF(tmpout = tmpout << '\n');
				TRACEGF(cerr << "filter: " << tmpout);
				TRACEGF(cerr << "{ "; if(se) se->print(cerr, 0); cerr << " }" << endl);
			} // end of for(int pc=b.length() - 1; pc >= 0; pc--)

			if(!pathFailed) { // if the path is valid
				if(seToReturn) { // if the other path is also valid, we clear the filter to prevent confusions
					ASSERT(se);
					delete se;
					seToReturn = 0;
					for(int bix = 0; bix < semBlocks.count(); bix++) {
						temp_known_reg[bix].clear();
						temp_reg_filters[bix].clear();
						temp_known_addr[bix].clear();
						temp_addr_filters[bix].clear();
					}
					break;
				}
				else
					seToReturn = se;
			}
			pathFailed = false; // reset the flag
			// now we take another path, if there is any
		} // end of for(int bi = 0; bi < semBlocks.count(); bi++) // for each path within sem block


		// now decide which filters to add
		regs_t temp_known_reg_all, temp_known_reg_repeat;
		// first find out what are the repeat registers
		for(int bi = 0; bi < semBlocks.count(); bi++)
			for (int bii = 0; bii < temp_known_reg[bi].count(); bii++)
				if(temp_known_reg_all.contains(temp_known_reg[bi][bii]))
					temp_known_reg_repeat.add(temp_known_reg[bi][bii]);
				else
					temp_known_reg_all.add(temp_known_reg[bi][bii]);

		// add the filters whose associated register is not in the repeat registers
		for(int bi = 0; bi < semBlocks.count(); bi++)
			for(int bii = 0; bii < temp_reg_filters[bi].count(); bii++)
				if(!temp_known_reg_repeat.contains(temp_reg_filters[bi][bii]->a()->val())) {
					curr_reg_filters.add(temp_reg_filters[bi][bii]);
					curr_known_reg.add(temp_reg_filters[bi][bii]->a()->val());
				}


		addrs_t temp_known_addr_all, temp_known_addr_repeat;
		// first find out what are the repeat addr
		for(int bi = 0; bi < semBlocks.count(); bi++)
			for (int bii = 0; bii < temp_known_addr[bi].count(); bii++)
				if(temp_known_addr_all.contains(temp_known_addr[bi][bii]))
					temp_known_addr_repeat.add(temp_known_addr[bi][bii]);
				else
					temp_known_addr_all.add(temp_known_addr[bi][bii]);

		// add the filters whose associated addr is not in the repeat addr
		for(int bi = 0; bi < semBlocks.count(); bi++)
			for(int bii = 0; bii < temp_addr_filters[bi].count(); bii++)
				if(!temp_known_addr_repeat.contains(temp_addr_filters[bi][bii]->a()->val())) {
					int index = curr_known_addr.indexOf(temp_addr_filters[bi][bii]->a()->val(), 0);
					if(!((index != -1) && (curr_addr_filters[index]->b()->val() == temp_addr_filters[bi][bii]->b()->val()))) {
						curr_addr_filters.add(temp_addr_filters[bi][bii]);
						curr_known_addr.add(temp_addr_filters[bi][bii]->a()->val());
					}
				}

		return seToReturn;
	}

	/**
	 * Build the reverse of a condition.
	 * @param cond	Condition to reverse.
	 * @return		Reversed condition.
	 */
	sem::cond_t FilterBuilder::reverseCond(sem::cond_t cond) {
		switch(cond) {
		case sem::NO_COND:	return sem::NO_COND;
		case sem::EQ: 		return sem::NE;
		case sem::LT:		return sem::GE;
		case sem::LE:		return sem::GT;
		case sem::GE:		return sem::LT;
		case sem::GT:		return sem::LE;
		case sem::ANY_COND:	return sem::ANY_COND;
		case sem::NE:		return sem::EQ;
		case sem::ULT:		return sem::UGE;
		case sem::ULE:		return sem::UGT;
		case sem::UGE:		return sem::ULT;
		case sem::UGT:		return sem::ULE;
		default:			ASSERT(false); return sem::NO_COND;
		}
	}

	/**
	 * Apply a filter on the value
	 * @param v the CLP to be filtred
	 * @param cmp_op compare operator
	 * @param f CLP to filter with
	*/
	void applyFilter(V &v, se::op_t cmp_op, V f){
		//bool reverse;
		V b;
		//clp::intn_t oldvdelta = v.delta();
		switch(cmp_op){
		case LT:	if(!f.swrap()) v.le(f.stop() - 1); break;
		case LE:	if(!f.swrap()) v.le(f.stop()); break;
		case GE:	if(!f.swrap()) v.ge(f.start()); break;
		case GT:	if(!f.swrap()) v.ge(f.start() + 1); break;
		case ULE:	if(!f.uwrap()) v.leu(f.stop()); break;
		case ULT:	if(!f.uwrap()) v.leu(f.stop() - 1); break;
		case UGE:	if(!f.uwrap()) v.geu(f.start()); break;
		case UGT:	if(!f.uwrap()) v.geu(f.start() + 1); break;
		case EQ:	v.inter(f); break;
		case NE:
			/* We can't do anything if the filter is not a constant.
			   We cannot test if the value is in the filter, because the filter
			   will never - at execution time - be the whole set, but just a
			   value in this set. */
			/*if (f.isConst()){
				if (v.isConst() && f.lower() == v.lower()){
					v.set(clp::NONE, 0, 0, 0);
				// if v is T, set to T/{f}
				} else if (v == V::all) {
					v.set(clp::VAL, f.lower() + 1, 1, clp::UMAXn - 1);
				// check in the value is one extremity of the clp
				} else if (f.lower() == v.lower()) {
					if (v.mtimes() > 1)
						v.set(v.kind(), v.lower() + v.delta(), v.delta(), v.mtimes() - 1);
					else
						v.set(v.kind(), v.lower() + v.delta(), 0, 0);
				} else if (v.lower() + (clp::intn_t)(v.delta() * v.mtimes()) == f.lower()) {
					if (v.mtimes() > 1)
						v.set(v.kind(), v.lower(), v.delta(), v.mtimes() - 1);
					else
						v.set(v.kind(), v.lower(), 0, 0);
				// if the clp as 3 values, check if its the one of the middle
				} else if (v.mtimes() == 2 && v.lower() + v.delta() == f.lower()) {
					v.set(v.kind(), v.lower(), v.delta() * 2, 1);
				}
			// check if v is in f
			}*/
			break;
		default:
			break;
		}

		/* keed the orientation of v*/
		/*if (oldvdelta < 0){
			v.reverse();
		}*/
		if(v.mtimes() == 0 || v.delta() == 0) { // regulating the result, if the mtimes or the delta is 0, then make the value to be constant
			v.set(v.kind(), v.lower(), 0, 0);
		}
	}
} //se

} // otawa
