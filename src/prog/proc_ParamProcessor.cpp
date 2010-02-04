/*
 *	$Id$
 *	(Abstract)ParamProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS <casse@irit.fr>
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

#include <otawa/proc/ParamProcessor.h>

namespace otawa {

/**
 * @class AbstractParamProcessor
 * Abstract form of a parametric processor.
 * @ingroup parametric
 */

/**
 * @fn Processor *AbstractParamProcessor::instantiate(const ActualFeature& feature) const;
 * Called to instantiate a processor. Must be overloaded by subclasses.
 * @param feature	Actual feature of the instantiation.
 * @return			Instantiated processor.
 */
AbstractParamProcessor::AbstractParamProcessor(cstring name, Version version)
	: AbstractIdentifier(name), vers(version) { }


/**
 */
AbstractParamProcessor::~AbstractParamProcessor(void)
	{ }


/**
 * @fn const Version& AbstractParamProcessor::version(void) const;
 * Get the version of the parametric processor.
 */


/**
 * @class ParamProcessor
 * A parametric processor allows to generate processor for parametric feature.
 * The actual processor is passed to the constructor using an instance of @ref AbstractMaker class.
 *
 * To use it, we have to define the parametric processor in the header file as:
 * @code
 * #include <otawa/proc/ParamProcessor.h>
 *
 * extern ParamProcessor MY_PARAMETRIC_PROCESSOR;
 * @endcode
 *
 * In the source file, we have to do:
 * @code
 * ParamProcessor MY_PARAMETRIC_PROCESSOR(
 * 	"MY_PARAMETRIC_PROCESSOR",
 *	Version(1, 0, 0),
 *	Version(1, new ParamProcessor::Maker<MyActualProcessor>())
 * @endcode
 *
 * @warning	The ParamProcessor is responsible to free the maker, do not pass
 * a global variable.
 * @ingroup parametric
 */


/**
 * @fn ParamProcessor::ParamProcessor(string name, Version version, AbstractMaker *maker)
 * @param name		Name of the parametric processor.
 * @param version	Version of the parametric processor.
 * @param maker		Processor maker.
 */

} // otawa
