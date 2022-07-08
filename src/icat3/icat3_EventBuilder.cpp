/*
 *	icat3::EventBuilder class implementation
 *	Copyright (c) 2021, IRIT UPS.
 *
 *	This file is part of OTAWA
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

#include <otawa/cfg/Loop.h>
#include <otawa/events/features.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Machine.h>
#include <otawa/hard/Memory.h>
#include <otawa/ipet.h>
#include <otawa/icat3/features.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa { namespace icat3 {

	
///
#if 0
class FetchEvent: public otawa::Event {
public:
	FetchEvent(Inst *inst, ot::time cost):
		Event(inst),
		h(nullptr),
		_cost(cost),
		_occ(Event::NO_OCCURRENCE),
		nc(false)
		{ }
	
	kind_t kind(void) const override { return FETCH; }
	cstring name(void) const override { return "fetch stage"; }	
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
		
		case icat3::FIRST_HIT:
		case icat3::NC:
			_occ = Event::SOMETIMES;
			nc = true;
			break;
		
		case icat3::TOP_CATEGORY:
		case icat3::UC:
			ASSERTP(false, _ << "invalid icat3 category: %d" << icat3::CATEGORY(acc));
			break;
		}
	}

private:
	List<Edge *> ah, am;
	Block *h;
	ot::time _cost;
	occurrence_t _occ;
	bool nc;
};
#endif


///
class FetchEvent: public otawa::Event {
public:
	FetchEvent(const icache::Access& acc, ot::time cost):
		Event(acc.instruction()),
		_cost(cost),
		_acc(acc)
		{ }
	
	kind_t kind(void) const override { return FETCH; }
	cstring name(void) const override { return "fetch stage"; }	
	ot::time cost(void) const override { return _cost; }
	type_t type(void) const override { return LOCAL; }
	int weight(void) const override { return 0; }
	
	occurrence_t occurrence(void) const override {
		switch(icat3::CATEGORY(_acc)) {
		case AH:
			return NEVER;
		case AM:
			return ALWAYS;
		case PE:
		case NC:
			return SOMETIMES;
		default:
			ASSERT(false);
			return NO_OCCURRENCE;
		}
	}
	
	virtual string detail() const override {
		StringBuffer buf;	
		buf << "fetch @" << _acc << ": " << icat3::CATEGORY(_acc);
		if(icat3::CATEGORY(_acc) == PE)
			buf << " (" << icat3::CATEGORY_HEADER(_acc) << ")";
		return buf.toString();
	}
	
	bool isEstimating(bool on) const override { return on; }
	
	void estimate(ilp::Constraint *cons, bool on) const override {
		if(on)
			switch(icat3::CATEGORY(_acc)) {
			case AH:
				break;
			case PE:
				for(auto e: icat3::CATEGORY_HEADER(_acc)->inEdges())
					if(!otawa::BACK_EDGE(e))
						cons->addRight(1, ipet::VAR(e));
				break;
			case AM:
			case NC:
				cons->addRight(type_info<double>::max, nullptr);
				break;
			default:
				ASSERT(false);
			}
	}

private:
	ot::time _cost;
	const icache::Access& _acc;
};


/**
 * Event Builder for icat3 i.e. for instruction cache.
 * 
 * **Implemented features**
 * * otawa::icat3::EVENTS_FEATURE
 * @ingroup icat3
 */
class EventBuilder: public BBProcessor {
public:
	static p::declare reg;
	
	EventBuilder(p::declare& r = reg): BBProcessor(reg) { }

protected:

	void setup(WorkSpace *ws) override {
		mach = hard::MACHINE_FEATURE.get(ws);
	}
	
	void processBB(WorkSpace *ws, CFG *g, Block *b) override {
		if(!b->isBasic())
			return;
		auto bb = b->toBasic();			
		for(const auto& a: *icache::ACCESSES(bb))
			EVENT(bb).add(new FetchEvent(a, mach->memory->readTime(a.address())));
	}
	
	void destroy(WorkSpace *ws) override {
		
	}
	
	void dumpBB(Block *b, io::Output& out) override {
		for(auto e: EVENT.all(b)) {
			auto evt = dynamic_cast<FetchEvent *>(e);
			if(evt != nullptr)
				out << "\t\t" << e->detail() << io::endl;
		}
	}

private:
	const hard::Machine *mach;
};

///
p::declare EventBuilder::reg = p::init("otawa::icat3::EventBuilder", Version(1, 0, 0))
	.make<EventBuilder>()
	.extend<BBProcessor>()
	.provide(EVENTS_FEATURE)
	.require(hard::MACHINE_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(icat3::CATEGORY_FEATURE);


/**
 * Feature ensuring that the generation of events (class otawa::Event) on the blocks.
 * These events reflect the behaviour of the instruction cache.
 * 
 * **Properties**
 * * otawa::EVENT -- hooked to the block the event applies to.
 * 
 * **Default implementation:** otawa::icat3::EventBuilder
 * @ingroup icat3
 */
p::feature EVENTS_FEATURE("otawa::icat3::EVENTS_FEATURE", p::make<EventBuilder>());

}}	// otawa::icat
