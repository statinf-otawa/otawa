/*
 *	$Id$
 *	Interface to the EGQueue, EGProc, EGStage, EGPipeline classes.
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

#ifndef _EG_SOLVER_H_
#define _EG_SOLVER_H_

#include "ExecutionGraph.h"
#include "EGScenario.h"

namespace otawa{
namespace exegraph2 {

class EGSolver{
public:
	virtual EGNodeFactory * nodeFactory() = 0;
	virtual EGEdgeFactory * edgeFactory() = 0;
	virtual void solve(ExecutionGraph *graph) = 0;

};

class EGGenericSolver : public EGSolver {
private:
	EGScenarioBuilder * _scenario_builder;
	WorkSpace *_ws;
public:
	EGGenericSolver(WorkSpace *ws, EGScenarioBuilder * scenario_builder=NULL)
	:_ws(ws)
	 {
		if (scenario_builder == NULL)
			_scenario_builder = new EGGenericScenarioBuilder();
		else
			_scenario_builder = scenario_builder;
	}
	EGNodeFactory * nodeFactory() {
		return new EGGenericNodeFactory();
	}
	EGEdgeFactory * edgeFactory() {
		return new EGGenericEdgeFactory();
	}
	void solve(ExecutionGraph *graph);
};


} // namespace exegraph2
} // namespace otawa

#endif // _EG_SOLVER_H_



