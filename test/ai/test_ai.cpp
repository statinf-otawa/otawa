/*
 *	This file is part of OTAWA
 *	Copyright (c) 2019, IRIT UPS.
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

//#define OTAWA_AI_DEBUG

#include <elm/int.h>
#include <elm/io/Output.h>
#include <elm/util/Version.h>
#include <elm/util/LockPtr.h>

#include <otawa/app/Test.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/Inst.h>
#include "../../include/otawa/ai/BlockAnalysis.h"

using namespace elm;
using namespace otawa;

typedef enum tristate_t {
	no = 0,
	yes = 1,
	maybe = -1,
	undef = -2
} tristate_t;

Output& operator<<(Output& out, tristate_t x) {
	switch(x) {
	case no:	out << "no"; break;
	case yes:	out << "yes"; break;
	case maybe:	out << "maybe"; break;
	case undef:	out << "undef"; break;
	default:	ASSERT(false);
	}
	return out;
}

class RAMRowBufferAccessor: public Lock {
public:
	virtual ~RAMRowBufferAccessor() { }
	virtual void move(Block *v) = 0;
	virtual void next(Inst *i) = 0;
	virtual Address row() = 0;
	virtual tristate_t loaded() = 0;
};

class RAMRowBufferInterface {
public:
	virtual ~RAMRowBufferInterface() { }
	virtual LockPtr<RAMRowBufferAccessor> access() = 0;
	virtual t::uint32 bufferSize() = 0;
};

extern p::interfaced_feature<RAMRowBufferInterface> RAM_ROW_BUFFER_ANALYSIS;

typedef t::int32 tag_t;


class RAMRowBufferDomain {
public:
	typedef tag_t t;
	static const int ROW_BUFFER_BITS = 8;
	static const tag_t BOT = -1, TOP = -2;
	inline tag_t tag(Address a) const { return a.offset() >> ROW_BUFFER_BITS; }

	void setup(WorkSpace *ws) { }
	void cleanup(WorkSpace *ws) { }

	t update(t d, Inst *i) const { return tag(i->address()); }

	tag_t init(WorkSpace *ws) { return TOP; }

	tag_t bot() const { return BOT; }

	tag_t join(tag_t x1, tag_t x2) const {
		if(x1 == TOP || x2 == TOP) return TOP;
		else if(x1 == BOT) return x2;
		else if(x2 == BOT) return x1;
		else return TOP;
	}

	tag_t update(Block *v, tag_t x) const {
		if(!v->isBasic())
			return x;
		else {
			for(auto i: *v->toBasic())
				x = update(x, i);
			return x;
		}
	}

	bool equals(tag_t x1, tag_t x2) const { return x1 == x2; }

	void set(tag_t& x, tag_t y) const { x = y; }

	void print(tag_t x, Output& out) const {
		if(x == TOP)
			out << "T";
		else if(x == BOT)
			out << "_";
		else
			out << io::hex(x << ROW_BUFFER_BITS);
	}
};


class RAMRowBufferAnalysis: public ai::BlockAnalysis<RAMRowBufferDomain>, public RAMRowBufferInterface {
public:

	static p::declare reg;
	RAMRowBufferAnalysis(): ai::BlockAnalysis<RAMRowBufferDomain>(reg) { }

	void *interfaceFor(const AbstractFeature& f) override { return static_cast<RAMRowBufferInterface *>(this); }

	class Accessor: public RAMRowBufferAccessor {
	public:
		inline Accessor(RAMRowBufferAnalysis &_a): a(_a), x(RAMRowBufferDomain::BOT), l(maybe) { }
		void move(Block *v) override { a.domain().set(x, a.in(v)); }
		void next(Inst *i) override {
			RAMRowBufferDomain::t y = x;
			x = a.domain().update(x, i);
			if(y == RAMRowBufferDomain::TOP)
				l = maybe;
			else if(y == RAMRowBufferDomain::BOT)
				l = undef;
			else
				l = x == y ? yes : no;
		}
		Address row() override {
			if(x == RAMRowBufferDomain::TOP || x == RAMRowBufferDomain::BOT)
				return Address::null;
			else
				return x << RAMRowBufferDomain::ROW_BUFFER_BITS;
		}
		tristate_t loaded() override { return l; }
	private:
		RAMRowBufferAnalysis& a;
		RAMRowBufferDomain::t x;
		tristate_t l;
	};

	LockPtr<RAMRowBufferAccessor> access() override { return new Accessor(*this); }
	t::uint32 bufferSize() override{ return 1 << RAMRowBufferDomain::ROW_BUFFER_BITS; }

};

p::declare RAMRowBufferAnalysis::reg = p::init("RAMRowBufferAnalysis", Version(1, 0, 0))
	.extend<BlockAnalysis<RAMRowBufferDomain>>()
	.make<RAMRowBufferAnalysis>()
	.provide(RAM_ROW_BUFFER_ANALYSIS);

p::interfaced_feature<RAMRowBufferInterface> RAM_ROW_BUFFER_ANALYSIS("RAM_ROW_BUFFER_ANALYSIS", p::make<RAMRowBufferAnalysis>());


class TestAI: public Test {
public:
	TestAI(void): Test("test_ai") { }

protected:

	void generate(io::Output& out) override {
		require(RAM_ROW_BUFFER_ANALYSIS);
		auto a = RAM_ROW_BUFFER_ANALYSIS.get(workspace())->access();
		for(auto v: COLLECTED_CFG_FEATURE.get(workspace())->blocks())
			if(v->isBasic()) {
				a->move(v);
				cout << v << io::endl;
				for(auto i: *v->toBasic()) {
					a->next(i);
					cout << i->address() << ": row " << a->row() << " (" << a->loaded() << ")\n";
				}
			}
	}

};

OTAWA_RUN(TestAI);
