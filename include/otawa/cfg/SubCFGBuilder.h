/*
 *	$Id$
 *	SubCFGBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#ifndef OTAWA_CFG_SUBCFGBUILDER_H_
#define OTAWA_CFG_SUBCFGBUILDER_H_

#include <elm/genstruct/Vector.h>
#include <otawa/proc/Registration.h>
#include <otawa/proc/Processor.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

// predeclaration
class VirtualCFG;

// SubCFGBuilder class
class SubCFGBuilder: public Processor {
public:
	SubCFGBuilder(void);
	static Registration<SubCFGBuilder> reg;

	virtual void configure(const PropList &props);
protected:
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void cleanup (WorkSpace *fw);
private:
	Address start;
	elm::genstruct::Vector<Address> stops;
	VirtualCFG *vcfg;
};

extern Identifier<Address> CFG_START;
extern Identifier<Address> CFG_STOP;

}	// otawa

#endif /* OTAWA_CFG_SUBCFGBUILDER_H_ */


