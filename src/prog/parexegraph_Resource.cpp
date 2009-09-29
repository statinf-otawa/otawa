/*
 *	$Id$
 *	Documentation of Resource class.
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


/**
 * @class Resource
 * Generic representation of a resource in the processor (pipeline stage, functional unit, slot in an instruction queue, register value, etc...). It is used to implement the block cost computation algorithm described in: "A Context-Parameterized Model for Static Analysis of Execution Times", by Christine Rochange and Pascal Sainrat, in Transactions on High-Performance Embedded Architectures and Compilers, 2(3), Springer, October 2007.
 */

/**
 * @fn Resource::Resource(elm::String name, resource_type_t type, int index);
 * @param name Resource name (used for debugging purposes).
 * @param type Resource type
 * @param index Resource rank in the list of resources attached to a ParExeGraph. Resources are numbered in the order they are created.
 */

/**
 * @fn elm::String Resource::name();
 * @return Name of the resource.
 */

/**
 * @fn resource_type_t Resource::type();
 * @return Type of the resource.
 */

/**
 * @fn index Resource::index();
 * @return Index of the resource (i.e. its rank in the list of resources related to a ParExeGraph).
 */
