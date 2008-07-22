/*
 *	$Id$
 *	CFGAdapter class implementation
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

#include <otawa/display/CFGAdapter.h>

namespace otawa { namespace display {

/**
 * @class CFGAdapter
 * This adapter implements the @ref otawa::concept::DiGraphWithNodeMap and
 * allows to use apply the @ref GenDrawer to a CFG.
 * 
 * To use this adapter, you have to pass it a CFG, to define a @ref Decorator
 * to give a representation of nodes and edges and to launch the drawing 
 * as below :
 * @code
 * 	CFG *my_cfg;
 * 	CFGAdapter adapter(my_cfg);
 * 	GenDrawer<CFGAdapter, MyDecorator> drawer(adapter);
 * 	drawer.draw();
 * @endcode
 * @ingroup display
 */


/**
 * @fn CFGAdapter::CFGAdapter(CFG *_cfg, WorkSpace *_ws);
 * Build the adapter.
 * @param _cfg	CFG to do adapt to.
 * @param _ws	Container workspace (optional).
 */

} } // otawa::display
