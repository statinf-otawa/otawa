/*
 *	StandardBuilder class implementation
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
 * @class Builder
 * TODO
 * @ingroup etime
 */


Builder::Builder(void)
:	_ws(nullptr),
	_processor(nullptr),
	_resources(nullptr),
	_factory(nullptr)
{
}

/**
 */
Builder::~Builder(void) {
}

/**
 * @fn ParExeGraph *Builder::build(ParExeSequence *seq);
 * TODO
 */

/**
 * @fn ParExeProc *Builder::processor(void) const;
 * TODO
 */

/**
 * @fn resources_t *Builder::resources(void) const;
 * TODO
 */

/**
 * @fn Factory *Builder::factory(void) const;
 * TODO
 */

/**
 * @fn void Builder::setProcessor(ParExeProc *processor);
 * TODO
 */

/**
 * @fn void Builder::setResources(resources_t *resources);
 * TODO
 */

/**
 * @fn void Builder::setFactory(Factory *factory);
 * TODO
 */


/**
 * @class StandardBuilder
 * TODO
 */
class StandardBuilder: public Builder {
public:

	ParExeGraph *build(ParExeSequence *seq) override {
		ASSERT(workspace() != nullptr);
		ASSERT(processor() != nullptr);
		ASSERT(resources() != nullptr);
		ASSERT(factory() != nullptr);

		PropList props;
		ParExeGraph *g = new ParExeGraph(workspace(), processor(), resources(), seq, props);
		g->build();
		return g;
	}

};

// internal
static StandardBuilder STANDARD_BUILDER;

/**
 * TODO
 */
Builder& Builder::DEFAULT = STANDARD_BUILDER;

} }	// otawa::etime
