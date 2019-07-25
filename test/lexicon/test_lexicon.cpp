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

#include <elm/data/ListQueue.h>
#define OTAWA_LEXICON_STAT
#define OTAWA_LEXICON_DUMP
#include <otawa/dfa/Lexicon.h>
#include <otawa/cfg/features.h>
#include <otawa/app/Test.h>

using namespace elm;
using namespace otawa;

const ot::size ROW_SIZE = 32;
typedef Address state_t;
const state_t BOT = 0;
const state_t TOP = 0xffffffff;
typedef Address action_t;

class DRAMRow: public dfa::Lexicon<state_t, action_t> {
public:
	DRAMRow(): _bot(add(BOT)), _top(add(TOP)) {
	}
	inline Handle *bot() const { return _bot; }
	inline Handle *top() const { return _top; }

protected:

	void doUpdate(const state_t& s, const action_t& a, state_t& r) override {
		r = a;
	}

	void doJoin(const state_t& s1, const state_t& s2, state_t& r) {
		if(s1 == s2)
			r = s1;
		else if(s1 == BOT)
			r = s2;
		else if(s2 == BOT)
			r = s1;
		else
			r = TOP;
	}

private:
	Handle *_bot, *_top;
};

p::id<DRAMRow::Handle *> STATE("", nullptr);

class TestLexicon: public Test {
public:
	TestLexicon(void): Test("test_lexicon") { }

protected:

	void generate(io::Output& out) override {
		this->require(otawa::COLLECTED_CFG_FEATURE);

		// prepare data
		DRAMRow dom;
		const CFGCollection *coll = CFGCollection::get(workspace());
		STATE(coll->entry()->entry()) = dom.top();
		ListQueue<Block *> wl;
		wl.put(coll->entry()->entry());

		// fix point
		while(wl) {
			Block *b = wl.get();
			DRAMRow::Handle *s = STATE(b);

			// exit case
			if(b->isExit()) {
				for(auto c: b->cfg()->callers())
					for(auto e: c->outEdges())
						propagate(dom, s, e->sink(), wl);
				continue;
			}

			// call case
			else if(b->isSynth()) {
				if(b->toSynth()->callee() != nullptr) {
					propagate(dom, s, b->toSynth()->callee()->entry(), wl);
					continue;
				}
				else
					s = dom.top();
			}

			// BB case
			else if(b->isBasic()) {
				BasicBlock *bb = b->toBasic();
				for(Address a = bb->address().roundDown(ROW_SIZE); a < bb->topAddress(); a+= ROW_SIZE)
					s = dom.update(s, a);
			}

			// propagates
			for(auto e: b->outEdges())
				propagate(dom, s, e->sink(), wl);
		}

		// dump results
		for(auto b: coll->blocks()) {
			DRAMRow::Handle *s = STATE(b);
			cout << b << " (in " << b->cfg() << "): " << **s << io::endl;
			s->setAlive();
			if(b->isBasic()) {
				BasicBlock *bb = b->toBasic();
				for(Address a = bb->address().roundDown(ROW_SIZE); a < bb->topAddress(); a += ROW_SIZE) {
					s = dom.update(s, a);
					s->setAlive();
				}
			}
		}
		cout << dom.stateCount() << " state(s)\n";
		cout << dom.minUpdateTrans() << " min update transition(s)\n";
		cout << dom.maxUpdateTrans() << " max update transition(s)\n";
		cout << dom.countAlive() << " alive state(s)\n";

		dom.dump("dump.dot");
	}

	inline void propagate(DRAMRow& dom, DRAMRow::Handle *s, Block *b, ListQueue<Block *>& wl) {
		DRAMRow::Handle *ss = STATE(b);
		if(s != ss) {
			if(ss == nullptr)
				STATE(b) = s;
			else
				STATE(b) = dom.join(s, ss);
			wl.put(b);
		}
	}

};

OTAWA_RUN(TestLexicon);
