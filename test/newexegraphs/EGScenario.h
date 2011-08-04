/*
 *	$Id$
 *	Interface to the EGScenario, EGScenarioBuilder classes.
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

#ifndef _EG_SCENARIO_H_
#define _EG_SCENARIO_H_

#include "ExecutionGraph.h"

namespace otawa { namespace exegraph2 {

class EGScenario{

};

class EGScenariiList{

};

class EGScenarioBuilder{
protected:
	ExecutionGraph *_graph;
public:
	virtual elm::genstruct::Vector<EGScenario *> * build(ExecutionGraph * graph) = 0;
};

class EGGenericScenarioBuilder : public EGScenarioBuilder {
public:
	elm::genstruct::Vector<EGScenario *> * build(ExecutionGraph * graph);
};

} // namespace exegraph2
} // namespace otawa
#endif // _EG_SCENARIOBUILDER_H_

