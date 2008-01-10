/*
 *	$Id$
 *	LoopInfoBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
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
#ifndef OTAWA_LOOP_INFO_BUILDER_H
#define OTAWA_LOOP_INFO_BUILDER_H

#include <elm/genstruct/Vector.h>
#include <elm/genstruct/SortedSLList.h>
#include <otawa/util/Dominance.h>
#include <otawa/prop/Identifier.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/proc/Feature.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/dfa/BitSet.h>

namespace otawa {



extern Identifier<BasicBlock*> ENCLOSING_LOOP_HEADER;
extern Identifier<BasicBlock*> LOOP_EXIT_EDGE;
extern Identifier<elm::genstruct::Vector<Edge*> *> EXIT_LIST;


// LoopInfoBuilder class
class LoopInfoBuilder: public CFGProcessor {
public:
        LoopInfoBuilder(void);
        virtual void processCFG(otawa::WorkSpace*, otawa::CFG*);
        
private:
		/**
		 * Builds the EXIT_LIST property for all the loop headers.
		 */
		void buildLoopExitList(otawa::CFG* cfg);
};

extern Feature<LoopInfoBuilder> LOOP_INFO_FEATURE;

}	// otawa

#endif	// OTAWA_LOOP_INFO_H
