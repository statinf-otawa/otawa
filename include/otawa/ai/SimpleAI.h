/*
 *	WorkListDriver class interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef INCLUDE_OTAWA_AI_SIMPLEAI_H_
#define INCLUDE_OTAWA_AI_SIMPLEAI_H_

#include "WorkListDriver.h"

namespace otawa { namespace ai {

template <class A>
class SimpleAI {
public:
	typedef A adapter_t;

	SimpleAI(A& adapter)
		: _adapter(adapter), _driver(adapter.domain(), adapter.graph(), adapter.store()) { }

	void run(void) {
		typename A::domain_t::t d;
		while(_driver()) {
			_adapter.update(*_driver, d);
			_driver.check(d);
			_driver.next();
		}
	}

private:
	A& _adapter;
	WorkListDriver<typename A::domain_t, typename A::graph_t, typename A::store_t> _driver;
};

template <class A>
class EdgeSimpleAI {
public:
	typedef A adapter_t;

	EdgeSimpleAI(A& adapter)
		: _adapter(adapter), _driver(adapter.domain(), adapter.graph(), adapter.store()) { }

	void run(void) {
		typename A::domain_t::t d;
		while(_driver) {
			for(auto e = _adapter.graph().preds(_driver); e; e++) {
				_adapter.update(e, d);
				_driver.check(e, d);
			}
			_driver.next();
		}
	}

private:
	A& _adapter;
	WorkListDriver<typename A::domain_t, typename A::graph_t, typename A::store_t> _driver;
};

} }		// otawa::ai

#endif /* INCLUDE_OTAWA_AI_SIMPLEAI_H_ */
