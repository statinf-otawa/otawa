/*
 *	StandardGenerator class implementation
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

#include <otawa/ipet.h>
#include <otawa/etime/AbstractTimeBuilder.h>

namespace otawa { namespace etime {

/**
 * @class Generator
 * TODO
 * @ingroup etime
 */

/**
 * TODO
 */
ILPGenerator::ILPGenerator(const Monitor& mon):
	Monitor(mon),
	_ws(nullptr),
	_sys(nullptr)
{ }


/**
 */
ILPGenerator::~ILPGenerator(void) {
}

/**
 * @fn void ILPGenerator::add(ParExeGraph *g, List<ConfigSet *> times);
 * TODO
 */

/**
 * @fn void ILPGenerator::complete(void);
 * TODO
 */

/**
 * @fn WorkSpace *ILPGenerator::workspace(void) const;
 * TODO
 */

/**
 * @fn ilp::System *ILPGenerator::system(void) const;
 * TODO
 */

/**
 * @fn void ILPGenerator::setWorkspace(WorkSpace *ws);
 * TODO
 */

/**
 * @fn void ILPGenerator::setSystem(ilp::System *sys);
 * TODO
 */


/**
 * TODO
 */
class StandardILPGenerator: public ILPGenerator {
public:

	StandardILPGenerator(const Monitor& mon): ILPGenerator(mon) {
	}

	void add(ParExeGraph *g, List<ConfigSet *> times) override {

	}

	virtual void complete(void) override {

	}

};

/**
 * TODO
 */
ILPGenerator *ILPGenerator::make(const Monitor& mon) {
	return new StandardILPGenerator(mon);
}

} }		// otawa::etime
