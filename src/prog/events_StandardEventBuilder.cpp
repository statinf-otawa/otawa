/*
 *	StandardEventBuilder class implementation
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
#include <otawa/events/StandardEventBuilder.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Machine.h>
#include <otawa/hard/Memory.h>
#include <otawa/icat3/features.h>
#include <otawa/ipet.h>

namespace otawa {

class CacheEvent: public otawa::Event {
public:
	CacheEvent(Inst *inst, ot::time cost):
		Event(inst),
		h(nullptr),
		_cost(cost),
		_occ(Event::NO_OCCURRENCE),
		nc(false)
		{ }
	
	ot::time cost(void) const override { return _cost; }
	type_t type(void) const override { return LOCAL; }
	occurrence_t occurrence(void) const override { return _occ; }
	int weight(void) const override { return 0; }
	
	virtual string detail() const override {
		StringBuffer buf;	
		buf << "fetch @" << this->inst()->topAddress() << ' ' << _occ;
		if(_occ == SOMETIMES) {
			buf << " (";
			
			if(!ah.isEmpty()) {
				bool fst = true;
				buf << "AH from ";
				for(auto e: ah) {
					if(fst)
						fst = false;
					else
						buf << ", ";
					buf << e->source();
				}
			}
			
			if(!am.isEmpty()) {
				if(!ah.isEmpty())
					buf << ", ";
				bool fst = true;
				buf << "AM from ";
				for(auto e: am) {
					if(fst)
						fst = false;
					else
						buf << ", ";
					buf << e->source();
				}
			}
			
			if(h != nullptr) {
				if(!am.isEmpty() || !ah.isEmpty())
					buf << ", ";
				buf << "PE at " << h;
			}

			buf << ")";
		}
		return buf.toString();
	}
	
	bool isEstimating(bool on) override { return (on && !nc) && (!on && !ah.isEmpty()); }
	
	void estimate(ilp::Constraint *cons, bool on) override {
		if(on) {
			// ... <= +oo
			if(nc)
				cons->addRight(type_info<double>::max, nullptr);
			
			// ... <= sum of AM count edge and of non-back entering h count
			else {
				for(auto e: am)
					cons->addRight(1, ipet::VAR(e));
				if(h != nullptr)
					for(auto e: h->inEdges())
						if(!otawa::BACK_EDGE(e))
							cons->addRight(1, ipet::VAR(e));				
			}
		}
		else
			// ... >= sum of AH edge count
			for(auto e: ah)
				cons->addRight(1, ipet::VAR(e));
	}


protected:
	List<Edge *> ah, am;
	Block *h;
	ot::time _cost;
	occurrence_t _occ;
	bool nc;
};


class FetchEvent: public CacheEvent {
public:
	FetchEvent(Inst *inst, ot::time cost): CacheEvent(inst, cost) { }
	kind_t kind(void) const override { return FETCH; }
	cstring name(void) const override { return "fetch stage"; }	

	void account(Edge *e, const icache::Access& acc) {
		switch(icat3::CATEGORY(acc)) {
		
		case icat3::AH:
			switch(_occ) {
			case Event::NO_OCCURRENCE:
				_occ = NEVER;
				break;
			case Event::ALWAYS:
			case Event::SOMETIMES:
				_occ = SOMETIMES;
				break;
			case Event::NEVER:
				break;
			};
			ah.add(e);
			break;
		
		case icat3::AM:
			switch(_occ) {
			case Event::NO_OCCURRENCE:
				_occ = ALWAYS;
				break;
			case Event::NEVER:
			case Event::SOMETIMES:
				_occ = SOMETIMES;
				break;
			case Event::ALWAYS:
				break;
			};
			am.add(e);
			break;			
		
		case icat3::PE:
			_occ = Event::SOMETIMES;
			if(h == nullptr)
				h = icat3::HEADER(e);
			else {
				auto nh = icat3::HEADER(e);
				if(Loop::of(h)->includes(Loop::of(nh)))
					h = nh;
			}
			break;
		
		case icat3::NC:
			_occ = Event::SOMETIMES;
			nc = true;
			break;
		
		case icat3::UC:
			ASSERTP(false, _ << "invalid icat3 category: %d" << icat3::CATEGORY(acc));
			break;
		}
	}

};


class MemEvent: public CacheEvent {
public:
	MemEvent(Inst *inst, ot::time cost): CacheEvent(inst, cost) { }
	kind_t kind(void) const override { return MEM; }
	cstring name(void) const override { return "memory stage"; }	

	void account(Edge *e, const dcache::BlockAccess& acc) {
		switch(dcache::CATEGORY(acc)) {
		
		case cache::ALWAYS_HIT:
			switch(_occ) {
			case Event::NO_OCCURRENCE:
				_occ = NEVER;
				break;
			case Event::ALWAYS:
			case Event::SOMETIMES:
				_occ = SOMETIMES;
				break;
			case Event::NEVER:
				break;
			};
			ah.add(e);
			break;
		
		case cache::ALWAYS_MISS:
			switch(_occ) {
			case Event::NO_OCCURRENCE:
				_occ = ALWAYS;
				break;
			case Event::NEVER:
			case Event::SOMETIMES:
				_occ = SOMETIMES;
				break;
			case Event::ALWAYS:
				break;
			};
			am.add(e);
			break;			
		
		case cache::FIRST_MISS:
			_occ = Event::SOMETIMES;
			if(h == nullptr)
				h = dcache::CATEGORY_HEADER(e);
			else {
				auto nh = dcache::CATEGORY_HEADER(e);
				if(Loop::of(h)->includes(Loop::of(nh)))
					h = nh;
			}
			break;
		
		case cache::NOT_CLASSIFIED:
		case cache::FIRST_HIT:
			_occ = Event::SOMETIMES;
			nc = true;
			break;
		
		case cache::INVALID_CATEGORY:
		case cache::TOP_CATEGORY:
			ASSERTP(false, _ << "invalid dcache category: %d" << dcache::CATEGORY(acc));
			break;
		}
	}
};


/**
 *  @class StandardEventBuilder
 * Default implementation for feature STANDARD_EVENT_FEATURE.
 * @ingroup events
 */

///
p::declare StandardEventBuilder::reg = p::init("otawa::StandardEventBuilder", Version(1, 0, 0))
	.make<StandardEventBuilder>()
	.extend<BBProcessor>()
	.provide(STANDARD_EVENT_FEATURE)
	.require(hard::MACHINE_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.use(icat3::CATEGORY_FEATURE);

///
StandardEventBuilder::StandardEventBuilder(p::declare& r):
	BBProcessor(r),
	has_icache(false),
	has_dcache(false)
	{ }

///
void StandardEventBuilder::setup(WorkSpace *ws) {
	mach = hard::MACHINE_FEATURE.get(ws);
	has_icache = mach->caches->hasInstCache() && ws->provides(icat3::CATEGORY_FEATURE);
	has_dcache = mach->caches->hasDataCache() && ws->provides(dcache::CATEGORY_FEATURE);
}

///
void StandardEventBuilder::processBB(WorkSpace *ws, CFG *g, Block *b) {
	if(!b->isBasic())
		return;
	auto bb = b->toBasic();

	// manage instruction cache events
	if(has_icache) {
		Vector<FetchEvent *> evts;
		
		// build events
		for(const auto& a: *icache::ACCESSES(*bb->inEdges().begin())) {
			auto evt = new FetchEvent(a.instruction(), mach->memory->readTime(a.address()));
			evts.add(evt);
			EVENT(bb).add(evt);
		}
		
		// record categories
		for(auto e: bb->inEdges()) {
			int i = 0;
			for(const auto& a: *icache::ACCESSES(e)) {
				evts[i]->account(e, a);
				i++;
			}
		}
	}
	
	// manage data cache events
	if(has_dcache) {
		Vector<MemEvent *> evts;
		
		// build events
		auto as = *dcache::DATA_BLOCKS(*bb->inEdges().begin());
		for(int i = 0; i < as.fst; i++) {
			MemEvent *evt = nullptr;
			const auto& a = as.snd[i];
			if(a.action() == dcache::BlockAccess::PURGE)
				continue;
			
			// build the event
			// TODO Taker into account case where address is different according to predecessors
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
			evt = new MemEvent(a.instruction(), t);
			evts.add(evt);
			EVENT(bb).add(evt);
		}
		
		// record categories
		for(auto e: bb->inEdges()) {
			int i = 0;
			as = *dcache::DATA_BLOCKS(*bb->inEdges().begin());
			for(int j = 0; j < as.fst; j++) {
				const auto& a = as.snd[j];
				if(a.action() == dcache::BlockAccess::PURGE)
					continue;
				evts[i]->account(e, a);
				i++;
			}
		}
	}
}

///
void StandardEventBuilder::dumpBB(Block *b, io::Output& out) {
	for(auto e: EVENT.all(b)) {
		auto evt = dynamic_cast<CacheEvent *>(e);
		if(evt != nullptr)
			out << "\t\t" << e->detail() << io::endl;
	}
}

///
void StandardEventBuilder::destroy(WorkSpace *ws) {	
}
	
}	// otawa

