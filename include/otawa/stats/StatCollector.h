/*
 *	$Id$
 *	StatCollector class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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
#ifndef OTAWA_STATS_STATCOLLECTOR_H_
#define OTAWA_STATS_STATCOLLECTOR_H_

#include <otawa/base.h>
#include <otawa/prop/ContextualProperty.h>

namespace otawa {

class StatCollector {
public:
	virtual ~StatCollector(void);

	virtual cstring name(void) const = 0;
	virtual cstring unit(void) const = 0;

	// value processing
	virtual bool isEnum(void) const = 0;
	virtual const cstring valueName(int value) = 0;
	virtual int total(void) = 0;

	// collection
	class Collector {
	public:
		virtual void enter(const ContextualStep& step) = 0;
		virtual void collect(const Address& address, t::uint32 size, int value) = 0;
		virtual void leave(void) = 0;
	};
	virtual void collect(Collector& collector) = 0;

	// statistics merge
	virtual int mergeContext(int v1, int v2) = 0;
	virtual int mergeAgreg(int v1, int v2) = 0;
};

}	// otawa

#endif /* OTAWA_STATS_STATCOLLECTOR_H_ */
