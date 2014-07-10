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
#include <otawa/hard/Platform.h>

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
	Vector<V> SymbExpr::used_reg(void){
		// recursively get reg for sub-expr
		Vector<V> a_reg, b_reg;
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
	Vector<V> SymbExpr::used_addr(void){
		// recursively get addr for sub-expr
		Vector<V> a_addr, b_addr;
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
		else
			return (_ << "(0x" << hex(_val.lower()) \
					  << ", 0x" << hex(_val.delta()) \
					  << ", 0x" << hex(_val.mtimes()) << ')');
	}
	void SEConst::canonize(void){}
	
	/************ SEAddr methods ************/
	SEAddr* SEAddr::copy(void){ return new SEAddr(_val); }
	SymbExpr& SEAddr::operator=(const SEAddr& expr){
		_op = expr._op;
		_val = expr._val;
		return *this;
	}
	bool SEAddr::operator==(const SymbExpr& expr) const {
		return (_op == expr.op() && _val == expr.val());
	}
	String SEAddr::asString(const hard::Platform *pf) {
		if(_val.isConst())
			return (_ << '@' << hex(_val.lower()));
		else
			return (_ << "@(0x" << hex(_val.lower()) \
					  << ", 0x" << hex(_val.delta()) \
					  << ", 0x" << hex(_val.mtimes()) << ')');
	}
	void SEAddr::canonize(void){}
	Vector<V> SEAddr::used_addr(void){
		Vector<V> vect;
		vect.add(_val);
		return vect;
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
	Vector<V> SEReg::used_reg(void){
		Vector<V> vect;
		vect.add(_val);
		return vect;
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
				_parent->set_a(new SEConst(V(0) - _a->val())); // WILL DELETE this !
				return;
			} else if (_parent->b() == this){
				_parent->set_b(new SEConst(V(0) - _a->val())); // WILL DELETE this !
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
				_parent->set_a(new SEConst(_a->val() + _b->val()));
				return;
			} else if (_parent->b() == this){
				// WILL DELETE this !
				_parent->set_b(new SEConst(_a->val() + _b->val()));
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
			s = s << "≤";
			break;
		case LT:
			s = s << '<';
			break;
		case GE:
			s = s << "≥";
			break;
		case GT:
			s = s << '>';
			break;
		case EQ:
			s = s << '=';
			break;
		case NE:
			s = s << "≠";
			break;
		case ULE:
			s = s << "u≤";
			break;
		case ULT:
			s = s << "u<";
			break;
		case UGE:
			s = s << "u≥";
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
	
	Identifier<Vector<SECmp *> > REG_FILTERS("otawa::se::REG_FILTERS");
	Identifier<Vector<SECmp *> > ADDR_FILTERS("otawa::se::ADDR_FILTERS");
	
	SECmp *getFilterForReg(SECmp *se, V reg, clp::ClpStatePack &pack, Inst *i, int sem, Vector<V> &used_reg, Vector<V> &used_addr){
		/* FIXME : This could be otptimized: we do a CLP analysis from the 
			begining of the BB each time we replace a register by its value */
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
		if (se->op() > CMPU && se->a() && se->a()->op() == REG && se->b() &&
			se->b()->op() == CONST /*&& (se->b()->val() != V::all)*/)
			return se;
		else{
			TRACEGF(cerr << "Bad filter: " << se->asString() << "\n";)
			return NULL;
		}
	}
	
	SECmp *getFilterForAddr(SECmp *se, V addr, clp::ClpStatePack &pack, Inst *i, int sem, Vector<V> &used_reg, Vector<V> &used_addr){
		/* FIXME: this could be otptimized: we do a CLP analysis from the 
			begining of the BB each time we replace an address by its value */
			clp::State state = pack.state_before(i->address(), sem);
		
		ASSERT(addr.isConst());
		
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
		
		// canonize
		se->canonize();
		// check if we have a filter
		if (se->op() > CMPU && se->a() && se->a()->op() == ADDR && se->b() &&
			se->b()->op() == CONST /*&& (se->b()->val() != V::all)*/)
			return se;
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

		// look all instructions
		Vector<Inst *> insts;
		Inst *branch = 0;
		for(BasicBlock::InstIter inst(bb); inst; inst++) {
			if(inst->isControl()) {
				if(inst->isConditional())
					branch = inst;
				break;
			}
			insts.add(inst);
		}

		// no conditional branch
		if(!branch)
			addFilters(0, insts);

		// build the conditional filters
		else
			iterateBranchPaths(branch, insts);

		// Set properties on the BB
		REG_FILTERS(bb) = reg_filters;
		ADDR_FILTERS(bb) = addr_filters;
	}

	/**
	 * Add the filters for the current instruction list (taken backward).
	 * @param se		Current conditional branch comparison.
	 * @param insts		Instructions of the block.
	 */
	void FilterBuilder::addFilters(SECmp *se, const Vector<Inst *>& insts) {
		sem::Block block;
		TRACEGF(String out);
		for(int i = insts.count() - 1; i >= 0; i--) {
			TRACEGF(out = _ << insts[i] << '\n' << out);
			block.clear();
			insts[i]->semInsts(block);
			se = makeFilters(se, insts[i], block);
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
	 * @param bb	Current basic block.
	 */
	void FilterBuilder::iterateBranchPaths(Inst *inst, const Vector<Inst *>& insts) {

		// initialize the iterations
		bool first = true;
		Vector<path_t> pstack;
		sem::Block istack, block;
		inst->semInsts(block);
		pstack.push(path_t(0, 0, false));

		// iterate on paths
		while(pstack) {

			// start the path
			path_t path = pstack.pop();
			istack.setLength(path.i);
			if(path.i != 0)
				istack[path.i - 1]._d = reverseCond(sem::cond_t(istack[path.i - 1].d()));

			// traverse instructions of the path
			while(path.n < block.count()) {
				istack.push(block[path.n]);
				if(block[path.n].op == sem::IF)
					pstack.push(path_t(istack.length(), path.n + block[path.n].b() + 1, path.b));
				else if(block[path.n].op == sem::CONT)
					break;
				else if(block[path.n].op == sem::BRANCH)
					path.b = true;
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
				SECmp *se = makeFilters(NULL, inst, istack);
				addFilters(se, insts);
			}
		}
	}

	/**
	 * Accumulate in the reg_filters and addr_filters filters for the
	 * given block.
	 * @param se		Current comparison.
	 * @param cur_inst	Current instruction.
	 * @param b			Block to work on.
	 */
	SECmp *FilterBuilder::makeFilters(SECmp *se, Inst *cur_inst, sem::Block& b) {

		for(int pc=b.length() - 1; pc >= 0; pc--){
			sem::inst& i = b[pc];

			// build the matching SE
			switch(i.op){

			case sem::IF:	// If inst is a if:
				// create a new symbexpr
				op_t log_op;
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
				default:			ASSERTP(false, "unsupported condition " << i.cond() << " at " << cur_inst->address()); break;
				}
				if(log_op)
					se = new SECmp(log_op, new SEReg(i.a()));
				break;

			// If inst is another instruction: replace
			case sem::LOAD:
				if (se){
					SEReg *rd = new SEReg(i.d());
					// get the adress of the register i.a()
					clp::State state = pack.state_after(cur_inst->address(), pc);
					clp::Value val = state.get(clp::Value(clp::REG, i.a()));
					if (val != clp::Value::all){
						if(!val.isConst()){
							// if val is a set, we cannot insert the memory
							// reference in the filter
							// TODO: maybe we should 'fork' the filter?
							// For the moment, if the load concern this expr
							// we set the se to NULL, to
							// invalidate the register i.a()
							Vector<V> used_reg = se->used_reg();
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
					Vector<V> used_reg = se->used_reg();
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
			}
			TRACEGF(String tmpout = _ << '\t' << i);

			// if SE, connect it
			if(se){
				TRACEGF(tmpout = tmpout << "\t\t=> " << se->asString());

				// canonize()
				se->canonize();
				TRACEGF(tmpout = tmpout << "\t\t=canonize=> " << se->asString());

				// find filters...
				if (se->op() > CMPU && se->a() && se->b()){
					Vector<V> used_reg = se->used_reg();
					Vector<V> used_addr = se->used_addr();
					bool has_tmp = false;
					for(int i = 0; i < used_reg.length(); i++)
						if (! (used_reg[i] >= 0))
							has_tmp = true;
					if (!has_tmp){
						// for each new register
						for(int i = 0; i < used_reg.length(); i++){
							if(! known_reg.contains(used_reg[i])){
								// get the filter
								SECmp *newfilter = getFilterForReg(se->copy(), used_reg[i], pack, cur_inst, pc, used_reg, used_addr);
								if (newfilter){
									TRACEGF(tmpout = _ << "\t\t\tNew filter: " << newfilter->asString() << '\n' << tmpout);
									reg_filters.add(newfilter);
									known_reg.add(used_reg[i]);
								}
							}
						}
						// for each new addr
						for(int i = 0; i < used_addr.length(); i++){
							if(! known_addr.contains(used_addr[i])){
								// get the filter
								SECmp *newfilter = getFilterForAddr(se->copy(), used_addr[i], pack, cur_inst, pc, used_reg, used_addr);
								if (newfilter){
									TRACEGF(tmpout = _ << "\t\t\tNew filter: " << newfilter->asString() << '\n' << tmpout);
									addr_filters.add(newfilter);
									known_addr.add(used_addr[i]);
								}
							}
						}
					}
				}
			} // end if(se)
			TRACEGF(tmpout = tmpout << '\n');
			TRACEGF(out = tmpout << out);
		} // end for (semantic instructions)

		// return current se
		return se;
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
		}
	}

	/**
	 * Apply a filter on the value
	 * @param v the CLP to be filtred
	 * @param cmp_op compare operator
	 * @param f CLP to filter with
	*/
	void applyFilter(V &v, se::op_t cmp_op, V f){
		bool reverse;
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
			   We cannnot test if the value is in the filter, because the filter
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
	}
	
	
} //se

} // otawa
