/*
 *	StandardEngine class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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

#include <otawa/etime/AbstractTimeBuilder.h>

namespace otawa { namespace etime {

/**
 * @class Engine
 * TODO
 * @ingroup etime
 */

/**
 */
Engine::~Engine(void) {
}

/**
 * Get the factory build an execution graph adapted to the engine.
 * Default implementation returns Factory::DEFAULT.
 * @return	Engine factory for execution graph.
 */
Factory *Engine::getFactory(void) {
	return Factory::make();
}

/**
 * @fn void Engine::compute(ParExeGraph *g, List<ConfigSet *> times, const Vector<EventCase>& events);
 * TODO
 */


/**
 * TODO
 */
class StandardEngine: public Engine {
public:

	void compute(ParExeGraph *g, List<ConfigSet *> times, const Vector<EventCase>& events) {

	}
};

// internal
static StandardEngine STANDARD_ENGINE;

/// TODO
Engine& Engine::DEFAULT = STANDARD_ENGINE;

} }	// otawa::etime

