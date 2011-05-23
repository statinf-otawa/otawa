/*
 *	$Id$
 *	Dead Code processor interface
 *	
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#ifndef OTAWA_DATA_CLP_DEADCODE_H_
#define OTAWA_DATA_CLP_DEADCODE_H_

#include <otawa/proc/Processor.h>

namespace otawa {

// DeadCodeAnalysis class
class DeadCodeAnalysis: public Processor {
public:
	DeadCodeAnalysis(void);
protected:
	virtual void processWorkSpace(WorkSpace *ws);
};

extern Feature<DeadCodeAnalysis> DEAD_CODE_ANALYSIS_FEATURE;
extern Identifier<bool> NEVER_TAKEN;

}	// otawa

#endif /* OTAWA_DATA_CLP_DEADCODE_H_ */
