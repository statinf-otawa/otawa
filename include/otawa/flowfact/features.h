/*
 *	$Id$
 *	ContextualLoopBound class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_FLOWFACT_FEATURES_H_
#define OTAWA_FLOWFACT_FEATURES_H_

#include <elm/xom/Element.h>

#include <otawa/dfa/State.h>
#include <otawa/proc/AbstractFeature.h>
#include <otawa/prog/Inst.h>
#include "../prop.h"

namespace otawa {

// flow fact configuration
extern Identifier<Path> FLOW_FACTS_PATH;
extern Identifier<xom::Element *> FLOW_FACTS_NODES;
extern Identifier<bool> FLOW_FACTS_MANDATORY;

// features
extern p::feature FLOW_FACTS_FEATURE;
extern p::feature MKFF_PRESERVATION_FEATURE;

// control flow facts
extern Identifier<Address> BRANCH_TARGET;
extern Identifier<Address> CALL_TARGET;
extern Identifier<bool> IGNORE_CONTROL;
extern Identifier<bool> IGNORE_ENTRY;
extern Identifier<bool> IGNORE_SEQ;
extern Identifier<bool> IS_RETURN;
extern Identifier<bool> NO_CALL;
extern Identifier<bool> NO_RETURN;

extern Identifier<Inst::kind_t> ALT_KIND;

// loop bounds
extern Identifier<int> MAX_ITERATION;
extern Identifier<int> MIN_ITERATION;
extern Identifier<int> TOTAL_ITERATION;

// virtualization control
extern Identifier<bool> NO_INLINE;
extern Identifier<bool> INLINING_POLICY;

// memory facts
extern Identifier<Pair<Address, Address> > ACCESS_RANGE;

// state facts
extern Identifier<dfa::State*> PROVIDED_STATE;
extern Identifier<bool> EXIST_PROVIDED_STATE;

// flow fact administration
extern Identifier<bool> PRESERVED;

} // otawa

#endif /* OTAWA_FLOWFACT_FEATURES_H_ */
