/*
 *	Domain and State classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2020, IRIT UPS.
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
#ifndef OTAWA_AI_DOMAIN_H_
#define OTAWA_AI_DOMAIN_H_

#include <elm/io/StructuredOutput.h>
#include <otawa/cfg/features.h>

namespace otawa { namespace ai {

class State {
	friend class AbstractInterpreter;
};

class Domain {
public:
	virtual ~Domain();

	virtual State *bot() = 0;
	virtual State *top() = 0;
	virtual State *entry() = 0;
	virtual bool equals(State *s1, State *s2) = 0;
	virtual State *join(State *s1, State *s2) = 0;

	virtual State *update(Edge *e, State *s) = 0;
	virtual State *update(Block *v, State *s);
	virtual State *join(State *s1, State *s2, Edge *e);

	virtual bool implementsPrinting();
	virtual void print(State *s, io::Output& out);

	virtual bool implementsIO();
	virtual void save(State *s, io::OutStream *out);
	virtual State *load(io::InStream *in);

	virtual bool implementsCodePrinting();
	virtual void printCode(Block *b, io::Output& out);
	virtual void printCode(Edge *e, io::Output& out);

	virtual bool implementsTracing();
	virtual void printTrace(State *s, io::StructuredOutput& out);
};

} }	// otawa::ai

#endif /* OTAWA_AI_DOMAIN_H_ */
