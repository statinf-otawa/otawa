/*
 *	$Id$
 *	WCETComputation class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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
#ifndef OTAWA_IPET_WCET_COMPUTATION_H
#define OTAWA_IPET_WCET_COMPUTATION_H

#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {

// WCETComputation class
class WCETComputation: public Registered<WCETComputation, Processor> {
	ilp::System *system;
protected:
	virtual void processWorkSpace(WorkSpace *fw);
public:
	WCETComputation(void);
	static void init(void);
};

// Features
extern Feature<WCETComputation> WCET_FEATURE;

} } // otawa::ipet

#endif	// OTAWA_IPET_WCET_COMPUTATION_H
