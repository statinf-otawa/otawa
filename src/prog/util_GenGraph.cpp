/*
 *	$Id$
 *	GenGraph class implementation
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

#include <otawa/util/GenGraph.h>

namespace otawa {

/**
 * @class GenGraph<N, E> <otawa/util/GenGraph.h>
 * GenGraph is inherited from @ref Graph class but enforce the type checking of
 * the objects of graph: only nodes of type N and edges of type E are
 * accepted.
 * @param N	Type of nodes (must inherit from @ref GenGraph<N, E>::Node).
 * @param E Type of edges (must inherit from @ref GenGraph<N, E>::Edge).
 * @ingroup graph
 */

} // otawa

