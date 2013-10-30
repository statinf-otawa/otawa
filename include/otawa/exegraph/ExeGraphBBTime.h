/*
 *	$Id$
 *	exegraph module interface
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

#ifndef EXECUTIONGRAPH_BBTIME_H
#define EXECUTIONGRAPH_BBTIME_H

#include <elm/assert.h>
#include <otawa/proc/BBProcessor.h>
#include "ExeGraph.h"
#include <elm/genstruct/DLList.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ipet.h>



namespace otawa {

using namespace elm::genstruct; 

template <class G>
class ExeGraphBBTime: public BBProcessor {
protected:
	typedef typename G::Node ExeNode;
	WorkSpace *fw;
	PropList *properties;
	Microprocessor<ExeNode> *microprocessor;
public:
	ExeGraphBBTime(const PropList& props = PropList::EMPTY);
	ExeGraphBBTime(AbstractRegistration &reg)
		: BBProcessor(reg), fw(0), microprocessor(0) { provide(ipet::BB_TIME_FEATURE); }
	ExeGraphBBTime(cstring name, Version version)
		: BBProcessor(name, version), fw(0), microprocessor(0) { provide(ipet::BB_TIME_FEATURE); }

	// BBProcessor overload
	void processWorkSpace(WorkSpace *fw);
	void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb) {}
	// Configuration Properties
	static Identifier<Microprocessor<ExeNode> *> PROCESSOR;
	static Identifier<elm::io::Output *>  LOG_OUTPUT;
};

template <class G>
Identifier<Microprocessor<typename G::Node> *> 
ExeGraphBBTime<G>::PROCESSOR("", NULL);


template <class G>
ExeGraphBBTime<G>::ExeGraphBBTime(const PropList& props) 
:	BBProcessor(), fw(0),
	microprocessor(PROCESSOR(props))
{
	provide(ipet::BB_TIME_FEATURE);
}

template <class G>
void ExeGraphBBTime<G>::processWorkSpace(WorkSpace *fw) {
	bool built = false, reset = false;
	this->fw = fw;

	// If required, find the microprocessor
	if(!microprocessor) {
		microprocessor = PROCESSOR(fw);
		if(microprocessor) 
			reset = true;
		else {
			const hard::Processor *proc = hard::PROCESSOR(fw);
			if(!proc)
				throw ProcessorException(*this, "no processor to work with");
			else {
				reset = true;
				built = true;
				microprocessor = new Microprocessor<ExeNode>(proc);
			}
		}
	}
	//elm::cerr << "=> microprocessor = " << microprocessor << io::endl;

	// Perform the actual process
	BBProcessor::processWorkSpace(fw);

	// Cleanup if required
	if(built)
		delete microprocessor;
	if(reset)
		microprocessor = 0;
}


} //otawa

#endif // OTAWA_ExeGraphBBTime_H
