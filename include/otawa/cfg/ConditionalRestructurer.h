/*
 *	ConditionalRestructurer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef OTAWA_CFG_CONDITIONALRESTRUCTURER_H_
#define OTAWA_CFG_CONDITIONALRESTRUCTURER_H_

#include "CFGTransformer.h"
#include "features.h"

namespace otawa {

class ConditionalRestructurer: public CFGTransformer {
public:
	static p::declare reg;
	ConditionalRestructurer(p::declare& r = reg);

protected:
	virtual void transform(CFG *g, CFGMaker &m);

private:
	void split(Block *bb);
	Inst *nop(Inst *i, const Condition& cond = Condition());
	Inst *guard(Inst *i, const Condition& cond);
	Inst *cond(Inst *i);
	void make(Block *bb);

	Inst *_nop, *_anop;
};

} // otawa

#endif /* OTAWA_CFG_CONDITIONALRESTRUCTURER_H_ */
