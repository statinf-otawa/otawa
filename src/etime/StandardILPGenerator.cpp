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

#include <elm/data/quicksort.h>
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
	_sys(nullptr),
	_explicit(false),
	_recording(false)
{ }


/**
 */
ILPGenerator::~ILPGenerator(void) {
}

/**
 * @fn void ILPGenerator::add(Edge *e, List<ConfigSet *> times, const Vector<EventCase>& events);
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
 * @fn bool ILPGenerator::isExplicit(void) const;
 * TODO
 */

/**
 * @fn bool ILPGenerator::isRecording(void) const;
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
 * @fn void ILPGenerator::setExplicit(bool exp);
 * TODO
 */

/**
 * @fn void ILPGenerator::setRecording(bool recording);
 * TODO
 */


/**
 * TODO
 */
class StandardILPGenerator: public ILPGenerator {

	typedef t::uint32 mask_t;

	class Split {
	public:
		inline Split(int c):
			cnt(c),
			pos(0),
			neg(0),
			com(0),
			unu(0),
			lts_time(0),
			hts_time(0)
		{ }

		int cnt;
		mask_t pos;
		mask_t neg;
		mask_t com;
		mask_t unu;
		ot::time lts_time;
		ot::time hts_time;
	};

	/**
	 * Collects variables linking events with blocks in ILP.
	 */
	class EventCollector {
	public:

		EventCollector(Event *event): imprec(0), evt(event) { }
		typedef enum {
			PREFIX_OFF = 0,
			PREFIX_ON = 1,
			BLOCK_OFF = 2,
			BLOCK_ON = 3,
			SIZE = 4
		} case_t;

		inline Event *event(void) const { return evt; }

		/**
		 * Add a variable contribution.
		 * @param c		Case of contribution.
		 * @param var	Variable of contribution (null for blurred contribution driving to estimation).
		 */
		void contribute(case_t c, ilp::Var *var) {
			if(!var)
				imprec |= 1 << c;
			else
				vars[c].add(var);
		}

		/**
		 * Make the variable and constraint for the current event.
		 * According to the variable list and the provided overestimation,
		 * may generate zero, one or several constraints.
		 * @param sys	System to create constraints in.
		 */
		void make(ilp::System *sys) {
			int s = PREFIX_OFF ;
			if(evt->place() == BLOCK)
				s = BLOCK_OFF;
			for(int c = s; c < SIZE; ++c) {
				if(vars[c] && evt->isEstimating(isOn(case_t(c)))) {
					ilp::Constraint *cons = sys->newConstraint(
						/*"event constraint"*/evt->name() ,
						/*(imprec & (1 << c)) ?*/ ilp::Constraint::GE /*: ilp::Constraint::EQ*/);
					evt->estimate(cons, isOn(case_t(c)));
					for(List<ilp::Var *>::Iter v(vars[c]); v; v++)
						cons->addRight(1, *v);
				}
			}
		}

	private:

		/**
		 * Test if the event is on or off.
		 * @param c		Case to test.
		 * @return		True if on, false if off.
		 */
		inline bool isOn(case_t c) {
			static bool ons[SIZE] = { false, true, false, true };
			return ons[c];
		}

		t::uint32 imprec;
		Event *evt;
		List<ilp::Var *> vars[SIZE];
	};

public:

	/**
	 */
	StandardILPGenerator(const Monitor& mon): ILPGenerator(mon) {
	}

	/*
	 */
	void add(Edge *e, List<ConfigSet *> times, const Vector<EventCase>& all_events) override {
		ASSERT(0 < times.count());

		// mono-time case
		if(times.count() == 1) {
			genForOneCost(times[0]->time(), e, all_events);
			return;
		}

		// count dynamic events
		int dyn_cnt = 0;
		for(auto e = *all_events; e; e++)
			if((*e).event()->occurrence() == SOMETIMES)
				dyn_cnt++;
		ASSERTP(times.count() <= (1 << dyn_cnt), times.count() << " events");

		// put all configurations in a vector
		Vector<ConfigSet *> confs;
		for(auto conf = *times; conf; conf++)
			confs.add(conf);
		class ConfigSetCompare {
		public:
			static int compare(const ConfigSet *cs1, const ConfigSet *cs2) {
				if(cs1->time() == cs2->time())
					return 0;
				else if(cs1->time() < cs2->time())
					return -1;
				else
					return +1;
			}
		};
		quicksort(confs, ConfigSetCompare());

		// initialization
		int best_p = 0;
		ot::time best_cost = type_info<ot::time>::max;

		// set of configurations
		ConfigSet set;
		for(int i = confs.length() - 1; i >= 0; i--)
			set.push(*confs[i]);

		// computation
		for(int p = 1; p < confs.length(); p++) {

			// update set and values
			set.pop(*confs[p - 1]);

			// scan the set of values
			Split split(dyn_cnt);
			set.scan(split.pos, split.neg, split.unu, split.com, split.cnt);

			// x^c_hts = sum{e in E_i /\ (\E c in HTS /\ e in c) /\ (\E c in HTS /\ e not in c)} w_e
			ot::time x_hts = 0;
			for(auto ev = *all_events; ev; ev++)
			//for(int i= 0; i < split.cnt; i++)
				if((*ev).index() >= 0 && (split.com & (1 << (*ev).index())) != 0)
					x_hts += (*ev).event()->weight();

			// x^p_hts = max{e in E_i /\ (\A c in HTS -> e in c)} w_e
			for(auto ev = *all_events; ev; ev++)
				if((*ev).index() >= 0 && (split.pos & (1 << (*ev).index())) != 0)
					x_hts = max(x_hts, ot::time((*ev).event()->weight()));
			// x_hts = max(x^c_hts, x^p_hts)

			// cost = x_hts t_hts + (x_i - x_hts) t_lts
			int weight = WEIGHT(e->sink());
			if(x_hts > weight)
				x_hts = weight;
			ot::time cost = x_hts * confs.top()->time() + (weight - x_hts) * confs[p - 1]->time();
			if (isVerbose())
				log << "\t\t\t\tHTS [" << p << " - " << (confs.length() - 1) << "], cost = " << cost << " (" << x_hts << "/" << weight << ")\n";

			// look for best cost
			if(cost < best_cost) {
				best_p = p;
				best_cost = cost;
			}
		}
		if (logFor(LOG_BB))
			log << "\t\t\t\tbest HTS [" << best_p << " - " << (confs.length() - 1) << "]\n";

		// look in the split
		ConfigSet hts;
		Split split(dyn_cnt);
		makeSplit(confs, best_p, hts, split);
		hts.scan(split.pos, split.neg, split.unu, split.com, split.cnt);
		if(isVerbose())
			log << "\t\t\t\t"
				<<   "pos = " 		<< Config(split.pos).toString(split.cnt)
				<< ", neg = " 		<< Config(split.neg).toString(split.cnt)
				<< ", unused = " 	<< Config(split.unu).toString(split.cnt)
				<< ", complex = "	<< Config(split.com).toString(split.cnt)
				<< io::endl;

		// contribute
		contributeSplit(e, all_events, split);

	}

	virtual void complete(void) override {

	}

	/**
	 * Generate the constraints when only one cost is considered for the edge.
	 * @param cost		Edge cost.
	 * @param edge		Current edge.
	 * @param events	List of edge events.
	 */
	void genForOneCost(ot::time cost, Edge *edge, const Vector<EventCase>&  events) {
		ilp::System *sys = system();
		ilp::Var *var = ipet::VAR(edge);
		ASSERT(var);

		// logging
		if(logFor(LOG_BB))
			log << "\t\t\t\tcost = " << cost << io::endl;

		// add to the objective function
		sys->addObjectFunction(cost, var);
		if(isRecording())
			LTS_TIME(edge) = cost;

		// generate constant contribution
		contributeConst(edge, events);

		// generate variable contribution
		for(auto ev: events)
			if(ev->occurrence() == SOMETIMES) {
				get(ev.event())->contribute(make(ev.event(), ev.part(), true), 0);
				get(ev.event())->contribute(make(ev.event(), ev.part(), false), 0);
			}
	}

	/**
	 * Build the set after split.
	 * @param confs		Current configuration.
	 * @param p			Split position.
	 * @param hts		HTS result set.
	 * @param lts_time	LTS time.
	 * @param hts_time	HTS time.
	 * @param cnt		Count of dynamic events.
	 */
	void makeSplit(const Vector<ConfigSet *>& confs, int p, ConfigSet& hts, Split& split) {
		split.lts_time = confs[p - 1]->time();
		split.hts_time = confs.top()->time();
		hts = ConfigSet(split.hts_time);
		for(int i = p; i < confs.length(); i++)
			hts.add(*confs[i]);
		if(logFor(LOG_BLOCK)) {
			log << "\t\t\t\t" << "LTS time = " << split.lts_time << ", HTS time = " << split.hts_time << " for { ";
			bool fst = true;
			for(ConfigSet::Iter conf(hts); conf; conf++) {
				if(fst)
					fst = false;
				else
					log << ", ";
				log << (*conf).toString(split.cnt);
			}
			log << " }\n";
		}
	}

	/**
	 * Contribute to WCET estimation in split way, x_HTS and x_LTS,
	 * with two sets of times.
	 * @param e			Evaluated edge.
	 * @param events	Dynamic event list.
	 * @param split		Split result.
	 */
	void contributeSplit(Edge *e, const Vector<EventCase>& events, const Split& split) {
		ilp::System *sys = system();

		// new HTS variable
		string hts_name;
		if(isExplicit()) {
			StringBuffer buf;
			buf << "e_";
			if(e->source()->isBasic())
				buf << e->source()->index() << "_";
			buf << e->sink()->index() << "_"
				<< e->sink()->cfg()->label() << "_hts";
			hts_name = buf.toString();
		}
		ilp::Var *x_hts = sys->newVar(hts_name);

		// wcet += time_lts x_edge + (time_hts - time_lts) x_hts
		ilp::Var *x_edge = ipet::VAR(e);
		sys->addObjectFunction(split.lts_time, x_edge);
		sys->addObjectFunction(split.hts_time - split.lts_time, x_hts);
		if(isRecording()) {
			LTS_TIME(e) = split.lts_time;
			HTS_OFFSET(e) = split.hts_time - split.lts_time;
		}

		// 0 <= x_hts <= x_edge
		ilp::Constraint *cons = sys->newConstraint("0 <= x_hts", ilp::Constraint::LE);
		cons->addRight(1, x_hts);
		cons = sys->newConstraint("x_hts <= x_edge", ilp::Constraint::LE);
		cons->addLeft(1, x_hts);
		cons->addRight(1, x_edge);

		// NOTATION
		// C^e_p -- constraint of event e for position p (p in {prefix, block})
		// unprecise(C^e_p) in B (T if C^e_p unprecise e.g. use of <=, _ else and as a default)

		// foreach e in events do
		for(auto ev = *events; ev; ev++)
			if((*ev).event()->occurrence() == SOMETIMES) {

			// if e in pos_events then C^e_p += x_hts / p = prefix if e in prefix, block
			if((split.pos & (1 << (*ev).index())) != 0)
				get((*ev).event())->contribute(make((*ev).event(), (*ev).part(), true), x_hts);

			// else if e in neg_events then C^e_p += x_edge - x_hts / p = prefix if e in prefix, block
			else if((split.neg & (1 << (*ev).index())) != 0)
				get((*ev).event())->contribute(make((*ev).event(), (*ev).part(), false), x_hts);

			// else unprecise(C^e_p) = T / p = prefix if e in prefix, block
			else {
				get((*ev).event())->contribute(make((*ev).event(), (*ev).part(), true), 0);
				get((*ev).event())->contribute(make((*ev).event(), (*ev).part(), false), 0);
			}
		}

		// generate constant contribution
		// TODO
		//contributeConst();

		// special contribution with com
		// TODO
		/*if(split.com) {
			// sum{e in com} x_e >= x_hts
			static string msg = "complex constraint for times";
			ilp::Constraint *c = sys->newConstraint(msg, ilp::Constraint::GE, 0);
			for(int i = 0; i < events.length(); i++)
				if((split.com & (1 << i)) && events[i].fst->isEstimating(true))
					events[i].fst->estimate(c, true);
			c->addRight(1, x_hts);
		}*/
	}

	/**
	 * Get the collector for the given event.
	 * If it doesn't exist, create it.
	 * @param event		Concerned event.
	 * @return			Matching collector (never null).
	 */
	EventCollector *get(Event *event) {
		EventCollector *coll = colls.get(event, 0);
		if(!coll) {
			coll = new EventCollector(event);
			colls.put(event, coll);
		}
		return coll;
	}

	/**
	 * Build an event selector case.
	 */
	EventCollector::case_t make(const Event *e, part_t part, bool on) {
		switch(part) {
		case IN_PREFIX:	return on ? EventCollector::PREFIX_ON : EventCollector::PREFIX_OFF;
		case IN_BLOCK:	return on ? EventCollector::BLOCK_ON : EventCollector::BLOCK_OFF;
		default:		ASSERT(false); return EventCollector::PREFIX_ON;
		}
	}

	/**
	 * Generate contribution for constant events.
	 */
	void contributeConst(Edge *e, const Vector<EventCase>& all_events) {

		// foreach e in always(e) do C^e_p += x_edge
		for(auto ev = *all_events; ev; ev++)
			switch((*ev).event()->occurrence()) {
			case ALWAYS:	get((*ev).event())->contribute(make((*ev).event(), (*ev).part(), true), ipet::VAR(e)); break;
			case NEVER:		get((*ev).event())->contribute(make((*ev).event(), (*ev).part(), false), ipet::VAR(e)); break;
			default:		break;
			}
	}

	HashMap<Event *, EventCollector *> colls;
};

/**
 * TODO
 */
ILPGenerator *ILPGenerator::make(const Monitor& mon) {
	return new StandardILPGenerator(mon);
}

} }		// otawa::etime

