/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	ilp/Var.h -- interface to the Var class.
 */
#ifndef OTAWA_ILP_VAR_H
#define OTAWA_ILP_VAR_H

#include <elm/io.h>

namespace otawa { namespace ilp {

using namespace elm;

// Var class
class Var {
	elm::String _name;
public:
	inline Var(void);
	inline Var(const char *name);
	inline Var(const elm::String& name);
	inline elm::String& name(void);
	inline String makeName(void) {
		if(_name) return _name;
		else return _ << "0x" << (void *)this;
	}
};

// Inlines
inline Var::Var(void) {
}

inline Var::Var(const char *name): _name(name) {
}

inline Var::Var(const elm::String& name): _name(name) {
}

inline elm::String& Var::name(void) {
	return _name;
}

// Output
inline io::Output& operator<<(io::Output& out, Var *var) {
	out << "ilp::Var(" << var->makeName() << ')';
	return out;
}

} } // otawa::ilp

#endif	// OTAWA_ILP_VAR_H
