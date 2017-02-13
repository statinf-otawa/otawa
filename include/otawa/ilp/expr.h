/*
 *	ilp::Expr and ilp::Cons classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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
#ifndef OTAWA_ILP_EXPR_H_
#define OTAWA_ILP_EXPR_H_

#include <otawa/ilp/System.h>

namespace otawa { namespace ilp {

// var class
class var {
public:
	inline explicit var(System& sys, Var::type_t t = Var::INT, string name = ""): _v(sys.newVar(name)) { }
	inline explicit var(System *sys, Var::type_t t = Var::INT, string name = ""): _v(sys->newVar(name)) { }
	inline var(Var *v): _v(v) { }
	inline operator Var *(void) const { return _v; }
	inline operator Term(void) const { return Term(1, _v); }
private:
	Var *_v;
};

inline var x(Var *v) { return var(v); }
inline Term operator*(coef_t c, var v) { return Term(v, c); }
inline Term operator*(var v, coef_t c) { return Term(v, c); }
inline Term operator*(int c, var v) { return Term(v, c); }
inline Term operator*(var v, int c) { return Term(v, c); }
inline Term operator*(int64_t c, var v) { return Term(v, c); }
inline Term operator*(var v, int64_t c) { return Term(v, c); }


// cons class
class cons {
public:
	inline cons(void): l(true), c(0) { }
	inline cons(Constraint *cc): l(cc->comparator() == Constraint::UNDEF), c(cc)  { }

	inline cons operator+(int t) 				{ if(l) c->add(t); else c->sub(t); return *this; }
	inline cons operator-(int t) 				{ if(l) c->sub(t); else c->add(t); return *this; }
	inline cons operator+(const Term& t) 		{ if(l) c->add(t); else c->sub(t); return *this; }
	inline cons operator-(const Term& t) 		{ if(l) c->sub(t); else c->add(t); return *this; }
	inline cons operator+(const Expression& e)	{ if(l) c->add(e); else c->sub(e); return *this; }
	inline cons operator-(const Expression& e) 	{ if(l) c->sub(e); else c->add(e);; return *this; }

	inline cons operator+=(int t) 				{ if(l) c->add(t); else c->sub(t); return *this; }
	inline cons operator-=(int t) 				{ if(l) c->sub(t); else c->add(t); return *this; }
	inline cons operator+=(const Term& t) 		{ if(l) c->add(t); else c->sub(t); return *this; }
	inline cons operator-=(const Term& t) 		{ if(l) c->sub(t); else c->add(t); return *this; }
	inline cons operator+=(const Expression& e)	{ if(l) c->add(e); else c->sub(e); return *this; }
	inline cons operator-=(const Expression& e) { if(l) c->sub(e); else c->add(e);; return *this; }

	inline cons operator==(int t) 				{ set(Constraint::EQ); return *this + t; }
	inline cons operator==(const Term& t) 		{ set(Constraint::EQ); return *this + t; }
	inline cons operator==(const Expression& f)	{ set(Constraint::EQ); return *this + f; }
	inline cons operator<=(int t) 				{ set(Constraint::LE); return *this + t; }
	inline cons operator<=(const Term& t) 		{ set(Constraint::LE); return *this + t; }
	inline cons operator<=(const Expression& f) { set(Constraint::LE); return *this + f; }
	inline cons operator<(int t) 				{ set(Constraint::LT); return *this + t; }
	inline cons operator<(const Term& t) 		{ set(Constraint::LT); return *this + t; }
	inline cons operator<(const Expression& f) 	{ set(Constraint::LT); return *this + f; }
	inline cons operator>=(int t) 				{ set(Constraint::GE); return *this + t; }
	inline cons operator>=(const Term& t) 		{ set(Constraint::GE); return *this + t; }
	inline cons operator>=(const Expression& f) { set(Constraint::GE); return *this + f; }
	inline cons operator>(int t) 				{ set(Constraint::GT); return *this + t; }
	inline cons operator>(const Term& t) 		{ set(Constraint::GT); return *this + t; }
	inline cons operator>(const Expression& f) 	{ set(Constraint::GT); return *this + f; }

	inline Constraint *operator&(void) const { return c; }
	inline Constraint *operator->(void) const { return c; }
	inline void free(void) { delete c; c = 0; }
private:
	void set(Constraint::comparator_t comp) { l = false; c->setComparator(comp); }
	bool l;
	Constraint *c;
};

class model {
public:

	class model_cons: public cons {
	public:
		inline model_cons(const cons& c): cons(c) { }
		inline cons operator%(int t) { return cons::operator+(t); }
		inline cons operator%(const Term& t) { return cons::operator+(t); }
		inline cons operator%(const Expression& t) { return cons::operator+(t); }
		inline operator cons(void) const { return *this; }
	};

	inline model(System *s): _s(s) { }
	inline model_cons operator()(void) const { return cons(_s->newConstraint(Constraint::UNDEF)); }
	inline model_cons operator()(const string& label) const { cons c(_s->newConstraint(Constraint::UNDEF)); c->setLabel(label); return c; }
	inline System *operator&(void) const { return _s; }
	inline System *operator->(void) const { return _s; }
	inline operator System *(void) const { return _s; }
	inline ilp::var var(const string& name) const { return _s->newVar(name); }

	inline model& operator+=(const Term& t) { _s->addObject(t); return *this; }
	inline model& operator-=(const Term& t) { _s->addObject(t); return *this; }

private:
	System *_s;
};


// Expression generation
inline Expression operator+(const Term& t1, const Term& t2) { Expression e; e.add(t1); e.add(t2); return e; }
inline Expression operator-(const Term& t1, const Term& t2) { Expression e; e.add(t1); e.sub(t2); return e; }
inline Expression operator+(const Expression& e, const Term& t) { Expression r(e); r.add(t); return r; }
inline Expression operator-(const Expression& e, const Term& t) { Expression r(e); r.sub(t); return r; }

} }		// otawa::ilp

#endif /* OTAWA_ILP_EXPR_H_ */
