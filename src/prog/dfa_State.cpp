/*
 *	dfa::State class -- state description for data flow analysis.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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

#include <elm/stree/SegmentBuilder.h>
#include <elm/sys/System.h>
#include <otawa/program.h>
#include <otawa/proc/Processor.h>
#include <otawa/hard/Platform.h>
#include <otawa/dfa/State.h>

namespace otawa { namespace dfa {

/**
 * @class State
 * Represents a value of the data state.
 */

/**
 * Build a none value.
 */
Value::Value(void): _kind(NONE), _base(0), _delta(0), _count(0) {
}


/**
 * Build a constant value.
 * @param base	Constant value
 */
Value::Value(t::uint32 base): _kind(CONST), _base(base), _delta(0), _count(0) {
}


/**
 * Build a value usually as an interval but if lo == up, it is considered as a constant.
 * @param lo	Lower value.
 * @param up	Upper value.
 */
Value::Value(t::uint32 lo, t::uint32 up) {
	if(lo == up) {
		_kind = CONST;
		_base = 0;
		_delta = 0;
		_count = 0;
	}
	else {
		_kind = INTERVAL;
		_base = lo;
		_delta = 1;
		_count = up - lo;
	}
}


/**
 * According to the triple (base, delta, count), build a value as:
 * @li a constant (if count = 0),
 * @li an interval (if delta = 1),
 * @li a CLP, i.e., a value in { base + delta * k / 0 <= k <= count }
 * @param base	Base value.
 * @param delta	Delta value.
 * @param count	Count value.
 */
Value::Value(t::uint32 base, t::uint32 delta, t::uint32 count) {
	if(count == 0) {
		_kind = CONST;
		_base = base;
		_delta = 0;
		_count = 0;
	}
	else {
		if(delta == 1)
			_kind = INTERVAL;
		else
			_kind = CLP;
		_base = base;
		_delta = 1;
		_count = count;
	}
}


/**
 * @fn Value::kind_t Value::kind(void) const;
 * Get the kind of the value.
 * @return	Value kind.
 */


/**
 * @fn Value::operator bool(void) const;
 * Test if the value is none or not.
 * @return	True if it is not none, false else.
 */


/**
 * @fn t::uint32 Value::value(void) const;
 * Get the value for a constant.
 * @return	Constant value.
 */


/**
 * @fn t::uint32 Value::low(void) const;
 * Get the lower value of the interval.
 * @return	Interval lower value.
 */


/**
 * @fn t::uint32 Value::up(void) const;
 * Get the upper value of the interval.
 * @return	Interval upper value.
 */


/**
 * @fn t::uint32 Value::base(void) const;
 * Get base value of a CLP.
 * @return	Base CLP value.
 */


/**
 * @fn t::uint32 Value::delta(void) const;
 * Get the delta value of a CLP.
 * @return	CLP delta value.
 */


/**
 * @fn t::uint32 Value::count(void) const;
 * Get the count value of a CLP.
 * @return	CLP count value.
 */


/**
 * @class State
 * Data flow state. Often used to describe the initial state as the join
 * of the user external information on data state and from the initialized data state
 * from the executable content.
 */


/**
 * Build the initial state.
 * @param process	Current process.
 */
State::State(Process& process): proc(process) {
	stree::SegmentBuilder<address_t, bool> builder(false);
	for(Process::FileIter file(&proc); file; file++)
		for(File::SegIter seg(file); seg; seg++)
			if(seg->isExecutable() || (seg->isInitialized() && !seg->isWritable()))
				builder.add(seg->address().offset(), seg->topAddress() - 1, true);
	builder.make(mem);
}


/**
 * Set the value of a register.
 * @param reg	Set register.
 * @param val	Set value.
 */
void State::set(const hard::Register *reg, Value val) {
	if(regs.hasKey(reg))
		regs.remove(reg);
	regs.put(reg, val);
}


/**
 * Get a register value.
 * @param reg	Register to look for.
 * @return		Found value or None value.
 */
Value State::get(const hard::Register *reg) const {
	return regs.get(reg, Value());
}


/**
 * @fn bool State::isInitialized(Address addr);
 * Test if the current state contains initialized data.
 * @param addr	Address to test.
 * @return		True if the data is initialized, false else.
 */


/**
 * @fn void State::get(const Address& a, t::uint8 &v) const;
 * Get a value from the initialized memory of the process.
 * @param a		Address to get value from.
 * @param v		Returned value.
 * @warning		This function must only be called on an initialized address. Else unexpected result may arise.
 */


/**
 * @fn void State::get(const Address& a, t::uint16 &v) const;
 * Get a value from the initialized memory of the process.
 * @param a		Address to get value from.
 * @param v		Returned value.
 * @warning		This function must only be called on an initialized address. Else unexpected result may arise.
 */


/**
 * @fn void State::get(const Address& a, t::uint32 &v) const;
 * Get a value from the initialized memory of the process.
 * @param a		Address to get value from.
 * @param v		Returned value.
 * @warning		This function must only be called on an initialized address. Else unexpected result may arise.
 */


/**
 * @fn void State::get(const Address& a, t::uint64 &v) const;
 * Get a value from the initialized memory of the process.
 * @param a		Address to get value from.
 * @param v		Returned value.
 * @warning		This function must only be called on an initialized address. Else unexpected result may arise.
 */


/**
 * @fn void State::get(const Address& a, float &v) const;
 * Get a value from the initialized memory of the process.
 * @param a		Address to get value from.
 * @param v		Returned value.
 * @warning		This function must only be called on an initialized address. Else unexpected result may arise.
 */


/**
 * @fn void State::get(const Address& a, double &v) const;
 * Get a value from the initialized memory of the process.
 * @param a		Address to get value from.
 * @param v		Returned value.
 * @warning		This function must only be called on an initialized address. Else unexpected result may arise.
 */


/**
 * @fn void State::get(const Address& a, Address &v) const;
 * Get a value from the initialized memory of the process.
 * @param a		Address to get value from.
 * @param v		Returned value.
 * @warning		This function must only be called on an initialized address. Else unexpected result may arise.
 */


/**
 * Record a configured memory item.
 * @param	cell	Memory cell description to record.
 */
void State::record(const MemCell& cell) {
	cmem.put(cell.address(), cell);
}


class InitialStateBuilder: public Processor {
public:
	static p::declare reg;
	InitialStateBuilder(p::declare& r = reg): Processor(r) { }

protected:

	void configure(const PropList& props) {
		Processor::configure(props);
		reg_inits.clear();
		for(Identifier<Pair<const hard::Register *, Value> >::Getter init(props, REG_INIT); init; init++)
			reg_inits.add(*init);
		for(Identifier<MemCell>::Getter init(props, MEM_INIT); init; init++)
			mem_inits.add(*init);
	}

	void processWorkSpace(WorkSpace *ws) {
		State *state = new State(*ws->process());
		for(int i = 0; i < reg_inits.count(); i++)
			state->set(reg_inits[i].fst, reg_inits[i].snd);
		for(int i = 0; i < mem_inits.count(); i++)
			state->record(mem_inits[i]);
		track(INITIAL_STATE_FEATURE, INITIAL_STATE(ws) = state);
	}

private:

	// TODO		Add it to string class.
	string strip(string s) {
		static string blanks = " \t\n";
		while(s && blanks.indexOf(s[0]) >= 0)
			s = s.substring(1);
		while(s && blanks.indexOf(s[s.length() - 1]) >= 0)
			s = s.substring(0, s.length() - 1);
		return s;
	}

	void split(string s, genstruct::Vector<string>& parts) {
		int p = s.indexOf(' ');
		while(p >= 0) {
			if(p > 0)
				parts.add(s.substring(0, p));
			s = s.substring(p + 1);
			p = s.indexOf(' ');
		}
		if(s)
			parts.add(s);
	}

	genstruct::Vector<Pair<const hard::Register *, Value> > reg_inits;
	genstruct::Vector<MemCell> mem_inits;
};

p::declare InitialStateBuilder::reg = p::init("otawa::dfa::InitialStateBuilder", Version(1, 0, 0))
	.maker<InitialStateBuilder>()
	.provide(INITIAL_STATE_FEATURE);


/**
 * This feature ensures that the executable and user information have been parsed to make the initial
 * state of the task.
 *
 * @p Configuration
 * @li @ref REG_INIT
 * @li @ref MEM_INIT
 *
 * @p Attributes
 * @li @ref INITIAL_STATE
 */
p::feature INITIAL_STATE_FEATURE("otawa::dfa::INITIAL_STATE_FEATURE", new Maker<InitialStateBuilder>());


/**
 * This configuration attribute allows to specify an initial value for a register.
 *
 * @p Feature
 * @li @ref INITIAL_STATE_FEATURE
 */
Identifier<Pair<const hard::Register *, Value> > REG_INIT("otawa::dfa::REG_INIT");


/**
 * This configuration attribute allows to specify an initial value for a memory cell.
 * Implictely, it allows also to assign a type to the memory cell.
 *
 * @p Feature
 * @li @ref INITIAL_STATE_FEATURE
 */
Identifier<MemCell> MEM_INIT("otawa::dfa::MEM_INIT");


/**
 * This attribute provides the initial state of the task.
 *
 * @p Hook
 * @li @ref WorkSpace
 *
 * @p Feature
 * @li @ref INITIAL_STATE_FEATURE
 */
Identifier<State *> INITIAL_STATE("otawa::dfa::INITIAL_STATE", 0);

} }	// otawa::dfa
