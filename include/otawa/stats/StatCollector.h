/*
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

#include <elm/data/ListMap.h>
#include <elm/data/Vector.h>
#include <otawa/base.h>
#include <otawa/prop/ContextualProperty.h>

namespace otawa {

class BasicBlock;
	
class StatCollector {
public:
	virtual ~StatCollector();

	virtual cstring id() const;
	virtual void keywords(Vector<cstring>& kws);
	virtual cstring name() const = 0;
	virtual cstring unit() const;

	// value processing
	virtual bool isEnum() const;;
	virtual const cstring valueName(int value);
	virtual int total();

	// collection
	class Collector {
	public:
		virtual void collect(const Address& address, t::uint32 size, int value, const ContextualPath& path) = 0;
	};
	virtual void collect(Collector& collector) = 0;

	// statistics merge
	virtual int mergeContext(int v1, int v2);
	virtual int mergeAgreg(int v1, int v2);
	
	// V1.1 API
	virtual cstring label() const;
	virtual void definitions(ListMap<cstring, string>& defs) const;
	virtual cstring description() const;
	typedef enum {
		NO_OP = 0,
		SUM_OP = 1,
		MAX_OP = 2,
		MIN_OP = 3
	} operation_t;
	virtual operation_t lineOperation() const;
	virtual operation_t contextOperation() const;
	virtual operation_t concatOperation() const;
	int aggregate(int x, operation_t op, int y);

	class BBCollector {
	public:
		virtual void collect(BasicBlock *v, int value, const ContextualPath& path) = 0;
	};
	virtual bool supportsBB() const;
	virtual void collectBB(BBCollector& collector);
};

io::Output& operator<<(io::Output& out, StatCollector::operation_t op);

}	// otawa

#endif /* OTAWA_STATS_STATCOLLECTOR_H_ */
