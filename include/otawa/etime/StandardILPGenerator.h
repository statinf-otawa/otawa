/*
 *	StandardILPGenerator class interface
 *
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
#ifndef OTAWA_ETIME_STANDARDILPGENERATOR_H_
#define OTAWA_ETIME_STANDARDILPGENERATOR_H_

#include "AbstractTimeBuilder.h"

namespace otawa { namespace etime {

class StandardILPGenerator: public ILPGenerator {

	typedef t::uint32 mask_t;

	class EventCollector {
		typedef enum {
			PREFIX_OFF = 0,
			PREFIX_ON = 1,
			BLOCK_OFF = 2,
			BLOCK_ON = 3,
			SIZE = 4
		} case_t;

	public:
		EventCollector(Event *event);
		inline Event *event(void) const { return evt; }
		void boundImprecise(EventCase c);
		void boundPositive(EventCase c, ilp::Var *x, bool prec);
		void boundNegative(EventCase c, ilp::Var *x, bool prec);
		void make(ilp::System *sys);

	private:
		inline bool isOn(case_t c)
			{ static bool ons[SIZE] = { false, true, false, true }; return ons[c]; }
		t::uint32 imprec;
		Event *evt;
		List<ilp::Var *> vars[SIZE];
	};

public:
	StandardILPGenerator(Monitor& mon);
	void process(WorkSpace *ws) override;

protected:
	void configure(const PropList& props) override;
	void contributeBase(ot::time time) override;
	void contributeTime(ot::time t_hts) override;
	void contributePositive(EventCase event, bool prec) override;
	void contributeNegative(EventCase event, bool prec) override;
	void prepare(Edge *e, const Vector<EventCase>& events, int dyn_cnt);
	void finish(const Vector<EventCase>& events);
private:
	virtual void process(Edge *e);
	void process(Edge *e, ParExeSequence *s, Vector<EventCase>& events, int dyn_cnt);
	EventCollector *get(Event *event);

	HashMap<Event *, EventCollector *> colls;
	Edge *_edge;
	ot::time _t_lts;
	ilp::Var *_x_e, *_x_hts;
	bool _t_lts_set;
	BitVector _done;
	int _eth;
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_STANDARDILPGENERATOR_H_ */
