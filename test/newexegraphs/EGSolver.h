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

namespace otawa{
namespace exegraph2 {

class EGSolver{
private:
	EGNodeFactory * _node_factory;
public:
	inline EGSolver(){
		_node_factory = new EGNodeFactory();
	}
	inline EGNodeFactory * nodeFactory()
		{return _node_factory;}
	void solve(ExecutionGraph *graph);

};

class EGSolverFactory {
public:
	EGSolver * newEGSolver()
		{ return new EGSolver();}
};

} // namespace exegraph2
} // namespace otawa

#endif // _EG_SOLVER_H_



