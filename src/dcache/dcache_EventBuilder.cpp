/*
 *	dcache::EventBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2021, IRIT UPS.
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

#include <otawa/cfg/features.h>
#include <otawa/cfg/Loop.h>
#include <otawa/dcache/features.h>
#include <otawa/events/features.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Machine.h>
#include <otawa/hard/Memory.h>
#include <otawa/ipet.h>

namespace otawa { namespace dcache {

class MemEvent: public Event {
public:
	MemEvent(const BlockAccess& acc, ot::time cost):
		otawa::Event(acc.instruction()),
		_acc(acc),
		_cost(cost)
		{ }
	
	ot::time cost(void) const override { return _cost; }
	type_t type(void) const override { return LOCAL; }
	int weight(void) const override { return 0; }
	kind_t kind(void) const override { return MEM; }
	cstring name(void) const override { return "ME"; }

	occurrence_t occurrence(void) const override {
		switch(dcache::CATEGORY(_acc)) {
		case AH:
			return NEVER;
		case AM:
			return ALWAYS;
		default:
			return SOMETIMES;
		}
	}

	string detail() const override {
		StringBuffer buf;
		buf << "ME(" << _cost << ") @" << inst()->address() << ": " << dcache::CATEGORY(_acc);
		if(dcache::CATEGORY(_acc) == PE)
			buf << " (" << dcache::CATEGORY_FEATURE(_acc) << ")";
		return buf.toString();
	}
	
	bool isEstimating(bool on) override { return on == true; }
	
	void estimate(ilp::Constraint *cons, bool on) override {
		if(!on)
			return;
		switch(dcache::CATEGORY(_acc)) {
		case AH:
			break;
		case PE:
			for(auto e: dcache::CATEGORY_HEADER(_acc)->inEdges())
				if(!BACK_EDGE(e))
					cons->addRight(1, ipet::VAR(e));
		default:
			cons->addRight(type_info<ot::time>::max, nullptr);
			break;
		}
	}

private:
	const BlockAccess& _acc;
	ot::time _cost;
};


/**
 * Build events of data cache accesses for a block.
 * 
 * **Provided features:**
 * * otawa::dcache::EVENTS_FEATURE
 * 
 * **Required features:**
 * * otawa::dcache::CATEGORY_FEATURE
 * * otawa::hard::MACHINE_FEATURE
 * 
 * @ingroup events
 */
class EventBuilder: public BBProcessor {
public:
	static p::declare reg;
	EventBuilder(p::declare& r = reg): BBProcessor(r), mach(nullptr) { }

protected:

	///
	void setup(WorkSpace *ws) override {
		mach = hard::MACHINE_FEATURE.get(ws);
	}

	///
	void processBB(WorkSpace *ws, CFG *g, otawa::Block *b) override {
		if(!b->isBasic())
			return;
		auto bb = b->toBasic();
		
		for(const auto& a: *DATA_BLOCKS(bb)) {
			if(a.action() == dcache::BlockAccess::PURGE)
				continue;
			
			// compute time
			ot::time t;
			Address addr = a.address();
			if(addr.isNull()) {
				// TODO should be a range
				if(a.action() == dcache::BlockAccess::LOAD)
					t = mach->memory->worstReadAccess();
				else
					t = mach->memory->worstWriteAccess();				
			}
			else {
				if(a.action() == dcache::BlockAccess::LOAD)
					t = mach->memory->readTime(addr);
				else
					t = mach->memory->writeTime(addr);
			}
			EVENT(bb).add(new MemEvent(a, t));
		}
	}

	///
	void dumpBB(otawa::Block *b, io::Output& out) override {
		for(auto e: EVENT.all(b)) {
			auto evt = dynamic_cast<MemEvent *>(e);
			if(evt != nullptr)
				out << "\t\t" << e->detail() << io::endl;
		}
	}

private:
	const hard::Machine *mach;
};


///
p::declare EventBuilder::reg = p::init("otawa::dcache::EventBuilder", Version(1, 0, 0))
	.make<EventBuilder>()
	.extend<BBProcessor>()
	.provide(EVENTS_FEATURE)
	.require(hard::MACHINE_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(CATEGORY_FEATURE);

///

p::feature EVENTS_FEATURE("otawa::dcache::EVENTS_FEATURE", p::make<EventBuilder>());

} }	// otawa::dcache

