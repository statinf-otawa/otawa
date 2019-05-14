/*
 *	$Id$
 *	ILPSystemGetter class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
#ifndef OTAWA_IPET_ILP_SYSTEM_GETTER
#define OTAWA_IPET_ILP_SYSTEM_GETTER

#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

// Extern declaration
namespace ilp {
	class System;
}

namespace ipet {

// SystemGetter class
class ILPSystemGetter: public Processor {
public:
	static p::declare reg;
	ILPSystemGetter(void);
protected:
	string plugin_name;
	void configure(const PropList &props) override;
	void processWorkSpace(WorkSpace *ws) override;
	void destroy(WorkSpace *ws) override;
private:
	bool max;
};

} } // otawa::ipet

#endif // OTAWA_IPET_ILP_SYSTEM_GETTER
