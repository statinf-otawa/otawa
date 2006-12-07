/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	ilp/Var.h -- interface to the Var class.
 */
#ifndef OTAWA_ILP_VAR_H
#define OTAWA_ILP_VAR_H

#include <elm/string.h>
#include <elm/io.h>

namespace otawa { namespace ilp {

using namespace elm::io;

// Var class
class Var {
	elm::String _name;
public:
	inline Var(void);
	inline Var(const char *name);
	inline Var(elm::String& name);
	inline elm::String& name(void);
};

// Inlines
inline Var::Var(void) {
}

inline Var::Var(const char *name): _name(name) {
}

inline Var::Var(elm::String& name): _name(name) {
}

inline elm::String& Var::name(void) {
	return _name;
}

// Output
inline Output& operator<<(Output& out, Var *var) {
	out << var->name();
	return out;
}

} } // otawa::ilp

#endif	// OTAWA_ILP_VAR_H
