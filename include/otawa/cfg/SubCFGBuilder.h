/*
 *	SubCFGBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009-18, IRIT UPS.
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

#include <elm/data/Vector.h>
#include <otawa/cfg/CFGTransformer.h>
#include <otawa/proc/Registration.h>
#include <otawa/proc/Processor.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

// SubCFGBuilder class
class SubCFGBuilder: public CFGTransformer {
public:
	SubCFGBuilder(void);
	static p::declare reg;
	virtual void configure(const PropList &props);
protected:
	virtual void transform(CFG *cfg, CFGMaker& maker);
private:
	Address _start_addr;
	elm::Vector<Address> _stop_addrs;
	location_t _start;
	elm::Vector<location_t> _stops;
	void floodForward(void);
	void floodBackward(void);
};

extern p::feature SPLIT_CFG;
extern p::id<Address> CFG_START;
extern p::id<Address> CFG_STOP;

}	// otawa

#endif /* OTAWA_CFG_SUBCFGBUILDER_H_ */


