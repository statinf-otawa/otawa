/*
 *	This file is part of OTAWA
 *	Copyright (c) 2006-11, IRIT UPS.
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
//#define CATCH_STT
//#define HAI_DEBUG
//#define HAI_JSON
#include <math.h>
#include <elm/data/HashMap.h>
#include <otawa/prog/File.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/Segment.h>
#include <otawa/prog/sem.h>
#include <otawa/hard/Register.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/dfa/hai/HalfAbsInt.h>
#include <otawa/dfa/hai/WideningListener.h>
#include <otawa/dfa/hai/WideningFixPoint.h>
#include <otawa/data/clp/ClpValue.h>
#include <otawa/data/clp/ClpState.h>
#include <otawa/data/clp/ClpAnalysis.h>
#include <otawa/data/clp/ClpPack.h>
#include <otawa/data/clp/DeadCodeAnalysis.h>
#include <otawa/data/clp/SymbolicExpr.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/hard/Platform.h>
#include <elm/data/quicksort.h>
#include <otawa/ipet/features.h>
#include <otawa/data/clp/features.h>
#include <otawa/hard/Memory.h>
#include <otawa/dfa/State.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <elm/log/Log.h>

using namespace elm;
using namespace otawa;

// Debug output for the state
#define TRACES(t)	//t
// Debug output for the domain
#define TRACED(t)	//t
// Debug output for the problem
#define TRACEP(t)	//t
// Debug output for Update function
#define TRACEU(t)	//t
// Debug output for instructions in the update function
#define TRACEI(t)	//t
// Debug output with only the values handled by an instruction
#define TRACESI(t)	//t
// Debug output with alarm of creation of T
#define TRACEA(t)	//t
// Debug only the join function
#define TRACEJ(t)	//t
// Debug only, alarm on store to T
#define ALARM_STORE_TOP(t)	t
//#define STATE_MULTILINE
#define TRACE_INTERSECT(t) // t


// enable to load data from segments when load results with T
#define DATA_LOADER

// reading 1K mem at a time is strange enough
#define MEMORY_ACCESS_THRESHOLD 1024

namespace otawa { namespace clp {


Identifier<bool> VERBOSE("otawa::clp::VERBOSE", false);


/**
 * @defgroup clp	CLP Analysis
 *
 * @code
 * #include <otawa/data/clp/features.h>
 * @endcode
 *
 * Circular-Linear Progression analysis is a data flow analysis that attempts to assign to each register
 * or to each memory cell a triplet (b, d, n) with b, d, n integers. This triplet represent the set if values
 * {b + k d / 0 <= k <= n }. Such a representation fits well the cases of array addresses where b is the base
 * address of the array, d the size of the array elements and n the number of elements. Therefore, it provides
 * interesting result when processing machine language.
 *
 * Ensuring it has been performed needs only a requirement on @ref otawa::clp::CLP_ANALYSIS_FEATURE.
 *
 * This analysis is performed on the semantic (@ref sem) representation of language machine.
 * To use  the result of the analysis requires to handle @ref otawa::clp::State at the entry
 * (@ref otawa::clp::STATE_IN) or at the exit (@ref otawa::clp::STATE_OUT) of a basic block.
 * To get a more precise few of the program state inside the basic block, you have to declare
 * @ref otawa::clp::Manager and to use it to traverse semantic instructions paths as in the example
 * below:
 *
 * @code
 * #include <otawa/clp/features.h>
 * ...
 * otawa::clp::Manager man;
 * man.start(bb);
 * do {
 * 		// do something
 * } while(man.next());
 * @endcode
 *
 */

/**
 * This identifier is a configuration for the @ref Analysis processor.
 * It allows to provide initial values for the registers involved in the analysis.
 * The argument is a pair of register and its initial value as an address.
 * A null address express the fact that the register is initialized with the default
 * stack pointer address.
 * @ingroup clp
 */
Identifier<Analysis::init_t> Analysis::INITIAL(
		"otawa::clp::CLP_ANALYSIS_INITIAL",
		pair((const hard::Register *)0, Address::null));


/**
 * Return positive GCD of two unsigned integers.
 * @param a	First integer.
 * @param b	Second integer.
 * @return	GCD(a, b).
 */
static uintn_t ugcd(uintn_t a, uintn_t b) {
	if (a == 0)
		return b;
	else if (b == 0)
		return a;
	else
		return ugcd(b, a % b);
}


/**
 * Return the GCD (Greatest Common Divisor) of two long integers.
 * Result is negative if both a and b are negative, positive else.
 * @param a		First value.
 * @param b		Second value.
 * @return		Negative GCD(a, b) if a < 0 and b < 0, positive GCD else.
 */
static intn_t gcd(intn_t a, intn_t b){
	if(a > 0) {
		if(b > 0)
			return ugcd(a, b);
		else
			return ugcd(a, -b);
	}
	else {
		if(b > 0)
			return ugcd(-a, b);
		else
			return -ugcd(-a, -b);
	}
}


/**
 * Return the lcm of two long integers
*/
inline long lcm(long a, long b){
	return elm::abs(a * b) / gcd(a, b);
}

/**
 * Return the min with a signed comparison
*/
inline intn_t min(intn_t a, intn_t b){
	if ( a < b)
		return a;
	else
		return b;
}
/**
 * Return the max with a signed comparison
*/
inline intn_t max(intn_t a, intn_t b){
	if ( a > b)
		return a;
	else
		return b;
}
/**
 * Return the min with an unsigned comparison
*/
inline uintn_t umin(uintn_t a, uintn_t b){
	if (a < b)
		return a;
	else
		return b;
}
/**
 * Return the max with an usigned comparison
*/
inline uintn_t umax(uintn_t a, uintn_t b){
	if ( a > b)
		return a;
	else
		return b;
}


/**
 * @class Value
 * Represents a CLP value.
 * @ingroup clp
 */


/**
 * Forms two CLP values which are the components of [base value, the value just before overflow]
 * and [the value just after overflow, upper bound of this part]. Refer to the beginning of the
 * section 6 of [Sen et Srikant, 2007].
 * @param p the CLP value contains [base value, the value just before overflow]
 * @param q the CLP value contains [the value just after overflow, upper bound of this part]
 */
void Value::PQValue(Value &p, Value &q) {
	ASSERT(_delta >= 0);
	intn_t l, u;
	l = _base;
	u = _base + _delta * _mtimes;
	if(l <= u) {
		p = *this;
		q = Value::none;
	}
	else {

		// calculate the P value
		// just to be careful with sign and unsigned ops, so use int64 and break the operation apart......
		t::int64 maxPm = MAXn;
		maxPm = maxPm - l;
		maxPm = maxPm / _delta;
		intn_t maxPd = (maxPm == 0)? 0 : _delta;
		p.set(VAL, l, maxPd, maxPm);

		// calculate the Q value
		t::int64 minQm = u;
		minQm = minQm - MINn;
		minQm = minQm / _delta;
		intn_t minQ = u - _delta * minQm;
		intn_t minQd = (minQm == 0)? 0 : _delta;
		q.set(VAL, minQ, minQd, minQm);
	}
}

/**
 */
Value Value::operator+(const Value& val) const{
	Value v = *this;
	v.add(val);
	return v;
}

Value Value::operator-(const Value& val) const{
	Value v = *this;
	v.sub(val);
	return v;
}


/**
 * Multiply another set to the current one with the higher 32 bits
 * @param val the value to multiply
 */
Value& Value::mulh(const Value& val){
	if (_kind == NONE && val._kind == NONE) 	/* NONE + NONE = NONE */
		*this = none;
	else if (_kind == ALL || val._kind == ALL) 	/* ALL + anything = ALL */
		*this = all;
	else if (_delta == 0 && val._delta == 0) {	/* two constants */
		t::uint64 result = _base;
		result = result * val._base;
		result = result >> 32;
		_base = result;
	}
	else
		*this = all;

	return *this;
}


/**
 * Multiply another set to the current one with the lower 32 bits
 * @param val the value to multiply
 */
Value& Value::mul(const Value& val){
	if (_kind == NONE && val._kind == NONE) 	/* NONE + NONE = NONE */
		*this = none;
	else if (_kind == ALL || val._kind == ALL) 	/* ALL + anything = ALL */
		*this = all;
	else if (_delta == 0 && val._delta == 0) {	/* two constants */
		t::uint64 result = _base;
		result = result * val._base;
		result = result & 0xFFFF;
		_base = (intn_t)result;
	}
	else
		*this = all;

	return *this;
}


/**
 * Add another set to the current one
 * @param val the value to add
 */
Value& Value::add(const Value& val){
	if (_kind == NONE && val._kind == NONE) 	/* NONE + NONE = NONE */
		*this = none;
	else if (_kind == ALL || val._kind == ALL) 	/* ALL + anything = ALL */
		*this = all;
	else if (_delta == 0 && val._delta == 0) 	/* two constants */
		set(_kind, _base + val._base, 0, 0);
	else if(direction() == val.direction()) {
		if(isInf() || val.isInf()) { // same direction, either one of the component is inf
			intn_t g = gcd(_delta, val._delta);
			intn_t l = _base + val._base;
			set(VAL, l, g, UMAXn);
		}
		else { // same direction, other cases
			intn_t g = gcd(_delta, val._delta);
			intn_t l = _base + val._base;
			uintn_t m1 = _mtimes * (elm::abs(_delta) / elm::abs(g));
			uintn_t m2 = val._mtimes * (elm::abs(val._delta) / elm::abs(g));
			uintn_t mtimes =  m1 + m2;
			if(isInf() || val.isInf()) // if one of the value is with inf mtimes, then the result should be infinite too.
				mtimes = UMAXn;
			else if(UMAXn - m1 < m2) // check if m1 + m2 > UMAXn, however UMAXn is the largest value, so we use UMAXn - m1 to see if there is an overflow
				mtimes = UMAXn;
			set(VAL, l, g, mtimes);

		}
	}
	else {
		if(isInf() && val.isInf()) // diff direction, both of the component are inf
			*this = Value::all;
		else { // different direction
				Value temp(val);
				if(isInf()) 			// inf will take more effect
					temp.reverse(); 	// temp2 needs to follow temp1's direction
				else if (val.isInf())
					reverse();
				else if(_delta < 0) 	// if none of them is infinite, then make the negative delta to the positive one by reversing it
					reverse();
				else 					// same for the 2nd operand
					temp.reverse();
				intn_t g = gcd(_delta, temp._delta);
				intn_t l = _base + temp._base;
				uintn_t m1 = _mtimes * (elm::abs(_delta) / elm::abs(g));
				uintn_t m2 = temp._mtimes * (elm::abs(temp._delta) / elm::abs(g));
				uintn_t mtimes =  m1 + m2;
				if(isInf() || val.isInf()) // if one of the value is with inf mtimes, then the result should be infinite too.
					mtimes = UMAXn;
				else if((UMAXn - m1) < m2)
					mtimes = UMAXn;
				set(VAL, l, g, mtimes);
		}
	}
	return *this;
}

/**
 * Subtract another set to the current one
 * @param val the value to subtract
 */
void Value::sub(const Value& val) {
	if (_kind == NONE && val._kind == NONE)		/* NONE - NONE = NONE */
		*this = none;
	else if (_kind == ALL || val._kind == ALL)	/* ALL - anything = ALL */
		*this = all;
	else if (_delta == 0 && val._delta == 0)	/* two constants */
		set(_kind, _base - val._base, 0, 0);
	else {
		Value temp(val);
		temp._delta = -temp._delta;
		temp._base = -temp._base;
		add(temp);
	}
}


/**
 * Print a human representation of the CLP
 * @param out the stream to output the representation
 */
void Value::print(io::Output& out) const {
	if (_kind == ALL)
		out << 'T';
	else if (_kind == NONE)
		out << '_';
	else if ((_delta == 0) && (_mtimes ==  0))
		out << "k(0x" << io::hex(_base) << ')';
		//out << "k(" << _lower << ')';
	else {
		if(_base >= 0)
			out << "(0x" << io::hex(_base);
		else {
			intn_t _baseToPrint = 0 - _base;
			out << "(-0x" << io::hex(_baseToPrint);
		}
		if(_delta >= 0)
			out << ", 0x" << io::hex(_delta);
		else {
			intn_t _deltaToPrint =  0-_delta;
			out << ", -0x" << io::hex(_deltaToPrint);
		}
		if(_mtimes == uintn_t(-1))
			out << ", inf)";
		else
			out << ", 0x" << io::hex(_mtimes) << ')';
	}
}

/**
 * Left shift the current value.
 * @param val the value to shift the current one with. Must be a positive
 *				constant.
*/
void Value::shl(const Value& val) {
	if(!val.isConst() || val._base < 0){
		set(ALL, 0, 1, UMAXn);
	} else if (_kind != NONE && _kind != ALL) {
		if (_delta == 0 && _mtimes == 0) {
			// set(VAL, _base << val._base, 0, 0);
			long a = _base;
			long b = val._base;
			set(VAL, a << b, 0, 0);
		}
		else
			set(VAL, _base << val._base, _delta << val._base, _mtimes);
	}
}

/**
 * Right shift the current value.
 * @param val the value to shift the current one with. Must be a positive
 *				constant.
*/
Value& Value::shr(const Value& val) {
	if(!val.isConst() || val._base < 0){
		set(ALL, 0, 1, UMAXn);
	} else
	if (_kind != NONE && _kind != ALL) {
		if (_delta == 0 && _mtimes == 0) {
			//set(VAL, _base >> val._base, 0, 0);
			long a = _base;
			long b = val._base;
			set(VAL, a >> b, 0, 0);

		}
#		ifdef USE_ORIGINAL_SHR
		else if (_delta % 2 == 0)
			set(VAL, _base >> val._base, _delta >> val._base, _mtimes);
		else
			set(VAL, _base >> val._base, 1, (_delta * _mtimes) >> val._base);
#		else
		else if(_delta % (1 << val._base) == 0) {
			set(VAL, _base >> val._base, _delta >> val._base, _mtimes);
		}
		else {
			t::uint64 mtimes_new = _mtimes;
			mtimes_new = mtimes_new * elm::abs(_delta);
			mtimes_new = (mtimes_new >> val._base) + 1;
			intn_t delta_new;
			if(_delta >= 0)
				delta_new = 1;
			else
				delta_new = -1;
			
			// over-approximation
			if(_mtimes == UMAXn)
				mtimes_new = UMAXn;
			
			set(VAL, _base >> val._base, delta_new, mtimes_new);
		}
#		endif		
	}
	return *this;
}

/**
 * Perform OR operator on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::_or(const Value& val) {
	if(_kind == ALL  || val.kind() == NONE)
		return;
	if(val.kind() == ALL || _kind == NONE) {
		*this = val;
		return;
	}
	if(val.isConst()) {
		if(isConst())
			_base |= val._base;
		/*else if(val._lower < _delta)
			_lower |= val._lower;*/		// TO CHECK
		else
			*this = all;
	}
	else {
		*this = all;
		/*if(OCLP_IS_CST(*this))
			set(V)
			&& _lower < val.delta())
			set(VAL, _lower | val._lower, val.delta(), val.mtimes());
		else
			*this = all;*/
	}
}


/**
 * @fn bool isTop(void) const;
 * Test if the value is top, that is, any value.
 * @return	True if it is top, false else.
 */


/**
 * Join another set to the current one
 * @param val the value to be joined with
 */
Value& Value::join(const Value& val) {
	if ((*this) == val)							/* A U A = A (nothing to do) */
		return *this;
	else if (_kind == ALL || val._kind == ALL)  /* ALL U anything = ALL */
		set(ALL, 0, 1, UMAXn);
	else if (_kind == NONE)						/* NONE U A = A */
		set(VAL, val._base, val._delta, val._mtimes);
	else if (val._kind == NONE)					/* A U NONE = A (nothing to do) */
		return *this;
	else if(isConst() && val.isConst()) {/* k1 U k2 */
		if(val._base > _base)
			set(VAL, _base, val._base - _base, 1);
		else
			set(VAL, val._base, _base - val._base, 1);
	}
	else {										/* other cases */
		if(isConst() || val.isConst()) {
			// k is the constant value
			// v is the none constant value
			Value v, k;
			if(isConst()) {
				k = *this;
				v = val;
			}
			else {
				k = val;
				v = *this;
			}

			// now check if k is within v
			if(k.inter(v).kind() != NONE) {
				*this = v;
				return *this;
			}
		}

		if((isInf() && val.isInf())) {
			//  <--------------------------| *this join
			//                  |----------------------------------> val
			//  <--------------------------------------------------> = T (result)
			if((delta() >= 0 && val.delta() < 0) || (delta() < 0 && val.delta() >= 0)) {
				*this = ALL;
			}
			//             |-------------------------------> *this join
			//   |-----------------------------------------> val
			//   |-----------------------------------------> (result)
			// OR
			//   <------------------------| *this join
			//   <------------------------------| val
			//   <------------------------------| (result)
			else {
				intn_t new_delta = gcd(_delta, val._delta);
				intn_t new_base;
				if((new_delta > 0 && _base < val._base) || (new_delta < 0 && _base > val._base))
					new_base = _base;
				else
					new_base = val._base;
				set(VAL, new_base, new_delta, UMAXn);
			}
			return *this;
		}

		if(isInf() || val.isInf()) {
			Value a, b; // a is with inifite mtimes, b is not
			if(isInf()) {
				a = *this;
				b = val;
			}
			else {
				a = val;
				b = *this;
			}

			// make sure the delta will go with the infinite
			if((a.delta() >= 0 && b.delta() < 0) || (a.delta() < 0 && b.delta() >= 0)) {
				b.reverse();
			}

			intn_t new_delta = gcd(a.delta(), b.delta());
			//  |-------------------------------> a
			//       |---------------| b
			//  |-------------------------------> result
			// OR
			//          |-----------------------> a
			//  |----------| b
			//  |-------------------------------> result
			// OR
			//                   |--------------> a
			//  |------| b
			//  |-------------------------------> result
			if(a.delta() >= 0) {
				intn_t new_base;
				if(a.lower() < b.lower())
					new_base = a.lower();
				else
					new_base = b.lower();
				set(VAL, new_base, new_delta, UMAXn);
			}
			//  <-------------------------------| a
			//       |---------------| b
			//  <-------------------------------| result
			// OR
			//  <------------| a
			//            |----------| b
			//  <--------------------| result
			// OR
			//  <--------------| a
			//                           |------| b
			//  <-------------------------------- result
			else {
				intn_t new_base;
				if(a.lower() > b.lower())
					new_base = a.lower();
				else
					new_base = b.lower();
				set(VAL, new_base, new_delta, UMAXn);
			}
			return *this;
		}

		uintn_t g = gcd(gcd(elm::abs(start() - val.start()), _delta), val._delta);
		intn_t ls = min(start(), val.start());
		t::int64 u1 = t::int64(start()) + t::int64(elm::abs(_delta)) * t::int64(_mtimes);
		t::int64 u2 = t::int64(val.start()) +t::int64(elm::abs(val._delta)) * t::int64(val._mtimes);
		t::int64 umax;
		if (u1 > u2)
			umax = u1;
		else
			umax = u2;
		set(VAL, ls, g, (umax - ls) / g);
	}
	return *this;
}

/**
 * Perform a widening to the infinite (to be filtred later)
 * @param val the value of the next iteration state
*/
Value& Value::widening(const Value& val) {
	/* widen(NONE, NONE) = NONE */
	if (_kind == NONE && val._kind == NONE)
		return *this;

	/* widen(ALL, *) = ALL */
	else if (_kind == ALL || val._kind == ALL)
		*this = all;
		//set(ALL, 0, 1, UMAXn);

	/* this == val = val */
	else if (*this == val)
		return *this;

	// widen((k, 0, 0), (k', 0, 0)) = (k, k' - k, 1)
	else if (isConst() && val.isConst()) {
		_delta = val._base - _base;
		_mtimes = clp::UMAXn;
	}

	// widen((k, 0, 0), (k', d', n')) =
	if(isConst()) {
		// if d' > 0 /\ k < k' then (k, d', n' + (k' - k) / d')
		// if d' < 0 /\ k' < k then (k, d', n' + (k' - k) / d')
		if((val._delta > 0 and _base <= val._base)
		or (val._delta < 0 and _base >= val._base))
			return *this = Value(VAL, _base, val._delta, val.mtimes() +  (_base - val._base) / val._delta);
		// else T
		else
			return *this = all;
	}

	// widen((k, d, n), (k', 0, 0)) = T
	if(val.isConst())
		return *this = all;

	// if n = n' = ∞
	if(isInf() and val.isInf()) {
		// if d = d' /\ |k - k'| % d = 0 then (k, d, n)
		if((_delta == val._delta) && (elm::abs(val._base - _base) % _delta == 0))
			return *this;
		// else T
		else {
			*this = all;
			return *this;
		}
	}

	// when d != d' /\ d != -d', widen((k, d, -), (k', d', -)) = T
	//else if (_delta != val._delta && _delta != - val._delta)
	//	*this = all;

	// when start(k', d', n') <= start(k, d, n)  /\ stop(k', d', n') <= stop(k, d, n),
	// widen((k', d', n'), (k, d, n)) = (stop(k, d, n), -D, -inf / D) with D = |d| if stop(k', d', n') = stop(k, d, n), 1 else
	else if (val.start() <= start() && val.stop() <= stop()){
		// go to negatives
		uintn_t absd = elm::abs(_delta);
		uintn_t startd = start() - val.start(), stopd = stop() - val.stop();
		if( absd != elm::abs(val.delta())	// if the absolute value of the deltas are different
		|| (stopd != 0 && stopd != absd)	// if the difference between stop values do not fall on delta... should be (stopd % absd) != 0
		|| startd != absd)	// if the difference between start values do not fall on dela ... should be (startd % absd) != 0
			absd = 1;
		set(_kind, stop(), -absd, UMAXn / absd);
	}

	// when start(k', d', n') >= start(k, d, n)  /\ stop(k', d', n') >= stop(k, d, n),
	// widen((k', d', n'), (k, d, n)) = (start(k', d', n'), D, -inf / D) with D = |d| if start(k', d', n') = start(k, d, n), 1 else
	else if (val.start() >= start() && val.stop() >= stop()) {
		// go the positive
		uintn_t absd = elm::abs(_delta);
		uintn_t startd = val.start() - start(), stopd = val.stop() - stop();
		if(absd != elm::abs(val.delta()) || (startd != 0 && startd != absd) || stopd != absd)
			absd = 1;
		set(_kind, start(), absd, UMAXn / absd);
	}

	// else widen((k, d, n), (k', d', n')) = T
	else
		*this = all;

	// regulate the results, if delta or mtimes is 0, then treat the result as a constant
	if(_kind == VAL && (_delta == 0 || _mtimes == 0))
		set(_kind, _base, 0, 0);

	return *this;
	check();
}

/**
 * Perform a widening, knowing flow facts for the loop
 * @param val the value of the next iteration state
 * @param loopBound the maximum number of iteration of the loop
*/
void Value::ffwidening(const Value& val, int loopBound){
	if (_kind == NONE && val._kind == NONE) /* widen(NONE, NONE) = NONE */
		return;
	else if (_kind == ALL || val._kind == ALL) /* widen(ALL, *) = ALL */
		set(ALL, 0, 1, UMAXn);
	else if (*this == val)					/* this == val -> do nothing */
		return;
	else if (isConst() && val.isConst()) {
		if (_base < val._base)
			/* widen((k1, 0, 0), (k2, 0, 0)) = (k1, k2 - k1, N) */
			set(VAL, _base, val._base - _base, loopBound);
		else {
			/* widen((k1, 0, 0), (k2, 0, 0)) = (k1-N(k1-k2),k1-k2,N) */
			int step = _base - val._base;
			set(VAL, _base - loopBound * step, step, loopBound);
		}
	}
	else if ((_delta == val._delta) &&		/* avoid division by 0 */
			 ((val._base - _base) % _delta == 0) &&
			 (_mtimes >= val._mtimes))
		return;				/* val == this + k => this */
	else
		/* other cases: T */
		set(ALL, 0, 1, UMAXn);
}

/**
 * Intersection with the current value.
 * @param val the value to do the intersection with
 */
Value& Value::inter(const Value& val) {

	// In this function, numbers are refs to doc/inter/clpv2-inter.pdf

	// 2. Special cases ========================

	// 2.0 bottom
	if(((*this) == Value::none) || (val == Value::none)) {
		*this = Value::none;
		return *this;
	}

	// 2.1. A n A (T n T)
	if ((*this) == val)
		return *this;
	else if(isTop()) {
		*this = val;
		return *this;
	}
	else if(val.isTop())
		return *this;

	// 2.2. cst n cst
	if(isConst() && val.isConst()){
		if(_base != val._base)
			set(NONE, 0, 0, 0);
		return *this;
	}

	// 2.3. cst n clp || clp n cst
	// TODO		direction problem?
	if (isConst()) {
		Value temp(val);
		if(!temp.direction())
			temp.reverse();

		long diff;
		if((temp.upper() > temp.lower()) || (_base > temp._base)) // normal case
			diff = (long)_base - (long)temp._base;
		else
			diff = (uintn_t)(_base) - (long)temp._base;

		// if the difference of the values is not a multiple of the delta, then the value does not fall on the interval
		if((diff % temp._delta) != 0) {
			*this = Value::none;
			return *this;
		}
		if((uintn_t)(diff / temp._delta) > temp._mtimes)
			*this = Value::none;
		else
			set(VAL, _base, 0, 0);
		return *this;
	}

	if(val.isConst()) {
		Value temp(*this);
		if(!temp.direction())
			temp.reverse();

		long diff;
		if((temp.upper() > temp.lower()) || (_base > temp._base)) // normal case
			diff = (long)val._base - (long)temp._base;
		else
			diff = (uintn_t)(val._base) - (long)temp._base;

		// if the difference of the values is not a multiple of the delta, then the value does not fall on the interval
		if((diff % temp._delta) != 0) {
			*this = Value::none;
			return *this;
		}

		if((uintn_t)(diff / temp._delta) > temp._mtimes) {
			*this = Value::none;
			return *this;
		}
		else
			set(VAL, val._base, 0, 0);
		return *this;
	 }


	// need to make sure they go to the same, and positive direction
	TRACE_INTERSECT(elm::cout << "*this = " << *this << io::endl;)
	TRACE_INTERSECT(elm::cout << "  val = " << val << io::endl;)
	Value val1(*this);
	Value val2(val);
	if(!val1.direction())
		val1.reverse();
	if(!val2.direction())
		val2.reverse();

	TRACE_INTERSECT(elm::cout << "val1  = " << val1 << io::endl;)
	TRACE_INTERSECT(elm::cout << "val2  = " << val2 << io::endl;)

	TRACE_INTERSECT(elm::cout << "_base = " << val1._base << io::endl;)
	TRACE_INTERSECT(elm::cout << "_delta = " << val1._delta << io::endl;)
	TRACE_INTERSECT(elm::cout << "_mtimes = " << (val1._mtimes) << " HEX: 0x" << hex(val1._mtimes) << io::endl;)
	TRACE_INTERSECT(elm::cout << "val2._base = " << val2._base << io::endl;)
	TRACE_INTERSECT(elm::cout << "val2._delta = " << val2._delta << io::endl;)
	TRACE_INTERSECT(elm::cout << "val2._mtimes = " << (val2._mtimes) << " HEX: 0x" << hex(val2._mtimes) << io::endl;)

	// 2.5 now carry out intersection, first we take care of the circularity and overflow as section 6 in [Sen et Srikant, 2007]
	// if upperbound < lowerbound, means circularity and overflow
	intn_t val1upper = val1._base + val1._delta * val1._mtimes;
	intn_t val2upper = val2._base + val2._delta * val2._mtimes;
	TRACE_INTERSECT(elm::cout << "val1upper = " << val1upper << io::endl;)
	TRACE_INTERSECT(elm::cout << "val2upper = " << val2upper << io::endl;)
	if((val1upper < val1._base) || (val2upper < val2._base)) {
		static int iii = 0;
		iii++;
		ASSERT(iii == 1);
		// make P and Q for A and B
		Value PA, PB, QA, QB, PA2, QA2;
		val1.PQValue(PA, QA);
		val2.PQValue(PB, QB);
		PA2 = PA;
		QA2 = QA;
		TRACE_INTERSECT(elm::cout << "PA = " << PA << io::endl;)
		TRACE_INTERSECT(elm::cout << "QA = " << QA << io::endl;)
		TRACE_INTERSECT(elm::cout << "PB = " << PB << io::endl;)
		TRACE_INTERSECT(elm::cout << "QB = " << QB << io::endl;)

		// intersection for each component
		PA.inter(PB);
		PA2.inter(QB);
		QA.inter(PB);
		QA2.inter(QB);
		TRACE_INTERSECT(elm::cout << "PAx = " << PA << io::endl;)
		TRACE_INTERSECT(elm::cout << "PA2x = " << PA2 << io::endl;)
		TRACE_INTERSECT(elm::cout << "QAx = " << QA << io::endl;)
		TRACE_INTERSECT(elm::cout << "QA2x = " << QA2 << io::endl;)

		// join the results
		PA.join(PA2).join(QA).join(QA2);
		*this = PA;
		iii--;
		return *this;
	}

	// 2.5.0 not overlapping intervals
	// the one with lower start, should have enough mtimes to catch up the start of the other
	if(	((val1._base > val2._base) && ((elm::abs(val1._base - val2._base) / val2._delta) > val2._mtimes)) ||
		((val2._base > val1._base) && ((elm::abs(val2._base - val1._base) / val1._delta) > val1._mtimes)) ) {
		set(NONE, 0, 0, 0);
		return *this;
	}

	// 2.5.1 then for non overflowing CLP values, we take similar steps in 5.1 of [Sen et Srikant, 2007]
	intn_t u1x, u2x; // upper bound of the CLP values
	intn_t resultBase; // the lower bound of the resulting CLP
	uintn_t resultMtimes, resultDelta; // the mtimes and delta for the result
	// for following the assumption, largerBaseV._base > smallerBaseV.base
	Value largerBaseV, smallerBaseV;
	if(val1._base >= val2._base) {
		largerBaseV = val1;
		smallerBaseV = val2;
	}
	else {
		largerBaseV = val2;
		smallerBaseV = val1;
	}
	// find the new delta
	resultDelta = lcm(largerBaseV._delta, smallerBaseV._delta);
	// find the minimal j'
	uintn_t j = 0;
	bool foundj = false;
	for(j = 0; j < (uintn_t)smallerBaseV._delta; j++) {
		if((largerBaseV._base - smallerBaseV._base + j * largerBaseV._delta) % smallerBaseV._delta == 0) {
			foundj = true;
			break;
		}
	}

	if(foundj == false) {
		*this = none;
		return *this;
	}

	// find the new base
	resultBase = largerBaseV._base + j * largerBaseV._delta;
	// check if the new base is lower than the upper bounds of the both CLP
	u1x = smallerBaseV._base + smallerBaseV._delta * smallerBaseV._mtimes;
	u2x = largerBaseV._base + largerBaseV._delta * largerBaseV._mtimes;
	if(resultBase > min(u1x,u2x)) {
		set(NONE, 0, 0, 0);
		return *this;
	}
	// find the new mtimes
	resultMtimes = (min(u1x, u2x) - resultBase) / resultDelta;
	if(resultMtimes == 0) // in case mtimes is 0, which means a constant value
		resultDelta = 0;

	val1.set(VAL, resultBase, resultDelta, resultMtimes);

	// if both directions are the same, we keep the direction
	if(direction() == val.direction())
		*this = val1;
	// if different directions, we need to have positive delta
	else if (!val1.direction()) {
		val1.reverse();
		*this = val1;
	}
	else
		*this = val1;
	return *this;
}

/**
 * Reverse the CLP direction (swap upper and lower bounds, and use
 * the opposite of delta as new delta).
*/
void Value::reverse(void){
	/*if(!isConst()) {
		uintn_t dist = (uintn_t)abs(start() - stop());
		set(clp::VAL, stop(), -delta(), dist / delta());
	}*/
	set(clp::VAL, _base + _delta * _mtimes, -_delta, _mtimes);
}


/**
 * Filter the current value with signed values greater than k.
 * @param k		Threshold.
 */
Value& Value::ge(intn_t k) {

	// all cases
	if(*this == all) {
		*this = Value(VAL, k, 1, MAXn-k);
		return *this;
	}

	// none cases
	if(*this == none)
		return *this;

	// case of constant
	if(isConst()) {
		if(k > _base)
			*this = none;
		return *this;
	}

	// d >= 0 => inter((b, d, n), (k, 1, inf+ - k)
	if(_delta > 0) {
		inter(Value(VAL, k, 1, MAXn - k));
		return *this;
	}

	// d < 0 !!!
	// if wrapping, change the current value for no wrapping
	if(swrap())
		_mtimes = (MAXn - k) / (-_delta);

	// b <= k -> _
	if(_base <= k) {
		*this = none;
		return *this;
	}

	// b + dn >= k -> (b, d, n)
	if(_base + _delta * intn_t(_mtimes) >= k)
		return *this;

	// _ -> (b, d, (k - b) / d
	else
		_mtimes = (k - _base) / _delta;

	check();
	return *this;
}


/*
 * LE logic
 * ========
 *
 * ASSUMPTION: used algorithms does not wrap!
 * Without such an assumption, filter becomes inefficient.
 * LEMMA: restrict (l, d, n) to non-wrapping part.
 * 		if d >= 0,	(l, d, (+inf - l) / d)
 * 		else		(l, d, (-inf - l) / d)
 *
 * 3 cases
 * 		)####----(		{ x <= k }
 * 		)-----##-(		case a
 * 		)--####--(		case b
 * 		)-##-----(		case c
 * 	case a: _ 								(no intersection)
 * 	case c: (b, l, n)						(identity)
 * 	case b: (l, d, (k - l)/d) 	if d >= 0	(forward intersection)
 *			(l + ds, d, d - ks	if d < 0	(backward intersection)
 *			with s = (l - k + d + 1) / d
 */

/**
 * Filter the current value with signed values lesser than k.
 * @param k		Threshold.
 */
Value& Value::le(intn_t k) {

	// make the value from ALL to [k, -inf]
	if(*this == all) {
		// first obtain the difference between k and -inf (MINn), use a 64 bit value to prevent overflow
		t::int64 m = k;
		m = m - MINn;
		Value temp(VAL, k, -1, m);
		// since we have ALL, there is no way to know the direction, so we make it positive d = 1
		temp.reverse();
		*this = temp;
		return *this;
	}

	// simple cases
	if(*this == none)
	//if(*this == all || *this == none)
		return *this;
	if(isConst()) {
		if(k < _base)
			*this = none;
		return *this;
	}

	// simple cases
	if(*this == all || *this == none)
		return *this;
	if(isConst()) {
		if(k < _base)
			*this = none;
		return *this;
	}

	// not so simple
	else {

		// wrap fix
		if(swrap()) {
			if(_delta >= 0)
				_mtimes = (uintn_t(MAXn) - _base) / _delta;
			else
				_mtimes = (uintn_t(MINn) + _base) / (-_delta);
		}

		// apply le
		if(start() > k)		// case a
			*this = none;
		else if(stop() < k)	// case c
			return *this;
		else {				// case b
			if(_delta >= 0)
				_mtimes = (k - _base) / _delta;
			else {
				intn_t s = (_base - k - _delta - 1) / (-_delta);
				ASSERT(s >= 0);
				_base = _base + _delta * s;
				_mtimes -= s;
			}

		}
	}

	check();
	return *this;
}


/**
 * Filter the current value with unsigned values greater than k.
 * @param k		Threshold.
 */
Value& Value::geu(uintn_t k) {

	// all case
	if(*this == all) {
		*this = Value(VAL, k, 1, UMAXn-k);
		return *this;
	}

	// none cases
	if(*this == none)
		return *this;

	// case of constant
	if(isConst()) {
		if(k > uintn_t(_base))
			*this = none;
		return *this;
	}

#ifndef CLP_ORIGINAL_GEU
	if(_delta > 0) {
		// see if we need to replace the _base
		intn_t new_base = _base;
		if(((uintn_t)_base) < k)
			new_base = k;
		// calculate the upper bound
		t::uint64 v = _delta;
		v = _delta * _mtimes;
		v = v + _base;
		if(v < k) { // means the whole CLP is less than k
			*this = none;
			return *this;
		}
		// find the new mtimes
		uintn_t new_mtimes = (v - new_base) / _delta;
		*this = Value(VAL, new_base, _delta, new_mtimes);
		return *this;
	}
	else {
		reverse();
		// see if we need to replace the _base
		intn_t new_base = _base;
		if(((uintn_t)_base) < k)
			new_base = k;
		// calculate the upper bound
		t::uint64 v = _delta;
		v = _delta * _mtimes;
		v = v + _base;
		if(v < k) { // means the whole CLP is less than k
			*this = none;
			return *this;
		}
		// find the new mtimes
		uintn_t new_mtimes = (v - new_base) / _delta;
		*this = Value(VAL, new_base, _delta, new_mtimes);
		reverse();
		return *this;
	}
#else
	// d >= 0 => inter((b, d, n), (k, 1, inf+ - k)
	if(_delta > 0) {
		inter(Value(VAL, k, 1, UMAXn - k));
		return *this;
	}

	// d < 0 !!!
	// if wrapping, change the current value for no wrapping
	if(uwrap())
		_mtimes = (UMAXn - k) / (-_delta);

	// b <= k -> _
	if(uintn_t(_base) <= k) {
		*this = none;
		return *this;
	}

	// b + dn >= k -> (b, d, n)
	if(uintn_t(_base + _delta * _mtimes) >= k)
		return *this;

	// _ -> (b, d, (k - b) / d
	else
		_mtimes = intn_t(k - _base) / _delta;
#endif
	check();
	return *this;
}


/**
 * Filter the current value with unsigned values lesser than k.
 * @param k		Threshold.
 */
Value& Value::leu(uintn_t k) {

	// for un-signed case, ALL will be filtered to [0, k]
	if(*this == all) {
		*this = Value(VAL, 0, 1, k);
		return *this;
	}

	// nothing to filter will be nothing still
	if(*this == none)
		return *this;

	// case of constant
	if(isConst()) {
		if(k < uintn_t(_base))
			*this = none;
		return *this;
	}

	// d < 0 => inter((b, d, n), (k, 1, inf+ - k)
	if(_delta < 0) {
		inter(Value(VAL, 0, 1, k));
		return *this;
	}

	// d > 0 !!!
	// if wrapping, change the current value for no wrapping
	if(uwrap())
		_mtimes = k / _delta;


	// check if 0 falls in the middle of the range (zero-crossing)
	// then move the base to be >= 0
	if(_base < 0 && (_base + _delta * _mtimes) > 0) {
		intn_t newbase = (((0 - _base) / _delta)) * _delta + _base;
		if(newbase < 0)
			newbase = newbase + _delta;
		_base = newbase;
		// calculate the new mtimes
		_mtimes = (k - uintn_t(_base)) / _delta;
		return *this;
	}

	// b >= k -> _
	if(uintn_t(_base) >= k) {
		*this = none;
		return *this;
	}

	// b + dn >= k -> (b, d, n)
	if(uintn_t(_base + _delta * _mtimes) <= k)
		return *this;

	// _ -> (b, d, (k - b) / d
	else
		_mtimes = (k - uintn_t(_base)) / _delta;

	check();
	return *this;
}


void Value::eq(uintn_t k) {

}

void Value::ne(uintn_t k) {

}


/**
 * Find a file of 1 in a word.
 * @param w		Word to process.
 * @param n		Number of bits to one.
 * @return		True if one is found, false else.
 */
/*static bool findField(uint32_t w, int& n) {
	n = 0;
	while(!(w & (1 << n)))
		n++;
	return !(w & (0xffffffff << n));
}*/


/**
 * Threshold giving the maximum size of a CLP set
 * to apply AND explicitly on the whole set
 * (and rebuilding a new CLP value).
 */
int Value::and_threshold = 8;

/**
 * Perform AND on the current value.
 * @param val	Value to perform AND on.
 */
Value& Value::_and(const Value& val) {
#ifndef USE_ORIGINAL_AND
	// if both of them are all, return all
	if((*this == all) && (val == all))
		return *this;

	// _ & v = v & _ = _
	if(*this == none)
		return *this;
	if(val == none) {
		*this = none;
		return *this;
	}

	// check for any constant
	Value v; // the CLP value
	uintn_t k; // the constant value to apply the AND relation on v
	if(isConst()) {
		if(val.isConst()) {		// k1 & k2
			*this = val.lower() & lower();
			return *this;
		}
		k = lower();
		v = val;
	}
	else if(val.isConst()) {
		v = *this;
		k = val.lower();
	}
	else {						// no k : cannot compute
		*this = all;
		return *this;
	}

	// v & 0 = 0
	if(k == 0) {
		*this = 0;
		return *this;
	}


	// for any value, m is the least significant bit of 1, and n is the most significant bit of 1
	// 000000000111110000000
	//          n   m
	// 000000000000000100110
	//                n   m
	// 111111111111100000000
	// n           m

	// first find the m and n for the constant k
	int n_k = 0, m_k = 0;
	int mode = 0;
	intn_t temp = k;
	for (int i = 0; i < 32; i++) {
		if ((mode == 0) && ((temp & 1) == 1)) {
			mode = 1;
			m_k = i;
			n_k = i;
		}
		else if ((mode == 1) && ((temp & 1) == 1)) {
			n_k = i;
		}
		temp = temp >> 1;
	}

	// find the maximum bit for the v.upper() unsigned
	uintn_t vu = v.upper();
	int n_vu = 0;
	temp = 1 << 31;
	for (int i = 31; i >=0 ; i--) {
		if(vu & temp) {
			n_vu = i;
			break;
		}
		temp = temp >> 1;
	}

	// find the maximum bit for the v.lower() unsigned
	uintn_t vl = v.lower();
	int n_vl = 0;
	temp = 1 << 31;
	for (int i = 31; i >=0 ; i--) {
		if(vl & temp) {
			n_vl = i;
			break;
		}
		temp = temp >> 1;
	}

	// n_v is the possible significant bit of the v
	int n_v = 0;
	if(n_vl > n_vu)
		n_v = n_vl;
	else
		n_v = n_vu;

	// narrow down the effective range to lower n of v and k
	int n = 0;
	if(n_v > n_k)
		n = n_k;
	else
		n = n_v;

#ifndef COMPLEX_AND

	if(m_k > n_v) // none of the bit can be masked
		*this = Value(0);
	else // The delta is the lowest possible 1, i.e. the m bit. Because any value below the m bit will be 0.
		*this = Value(VAL, 0, (1 << m_k), (1 << (n - m_k + 1))-1);

	return *this;

#else
	// T & v = v & T = T
	if(v == all) {
		STAT_UINT x = 1;
		x = (x << (n_k + 1 - m_k));
		x = x - 1;
		*this = Value(VAL, 0, 1 << m_k, x);
		return *this;
	}

	// first get the m of m_v_base and m_v_delta
	// m_v_base
	int m_v_base = -1;
	intn_t v_base = v._base;
	for(int i = 0; i < 32; i++) {
		if(v_base & 1) {
			m_v_base = i;
			break;
		}
		v_base = v_base >> 1;
	}


	int m_v_delta = -1;
	intn_t v_delta = v._delta;
	for(int i = 0; i < 32; i++) {
		if(v_delta & 1) {
			m_v_delta = i;
			break;
		}
		v_delta = v_delta >> 1;
	}


	int m_v;
	if(m_v_base == -1) // if base is 0, then we take the m of delta
		m_v = m_v_delta;
	else if(m_v_base < m_v_delta) // f base has smaller m value, we take it
		m_v = m_v_base;
	else
		m_v = m_v_delta;


	// then we look for max(m_v, m)
	int m;
	if(m_v > m_k)
		m = m_v;
	else
		m = m_k;


	STAT_UINT v_max_cand1 = v._base;
	STAT_UINT v_max_cand2 = v._base + v._delta * v.mtimes();
	STAT_UINT v_max;
	if(v_max_cand1 > v_max_cand2)
		v_max = v_max_cand1;
	else
		v_max = v_max_cand2;


	// find n_v
	int n_v = 0;
	STAT_UINT v_max_comp = (STAT_UINT)1 << 63;
	for(int i = 63; i >= 0; i--) {
		if(v_max_comp & v_max) {
			n_v = i;
			break;
		}
		v_max_comp = v_max_comp >> 1;
	}


	// find the mtimes
	// first find the max possible value for the result
	STAT_UINT max_possible_value = 1;
	max_possible_value = max_possible_value << (n_v + 1);
	max_possible_value = max_possible_value - 1;
	STAT_UINT max_possible_value2 = 1;
	max_possible_value2 = max_possible_value2 << (n_k + 1);
	max_possible_value2 = max_possible_value2 - 1;
	max_possible_value = max_possible_value & max_possible_value2;


	// obtain the mtimes
	max_possible_value = max_possible_value >> m;


	// if mtimes is 0, then we set delta to 0 too, and the resulted value is 0
	if(max_possible_value == 0)
		*this = Value(VAL, 0, 0, 0);
	else
		*this = Value(VAL, 0, (1 << m), max_possible_value);

	return *this;
#endif

#else	

	// T & v = v & T = T
	if(*this == all)
		return *this;
	if(val == all) {
		*this = all;
		return *this;
	}

	// _ & v = v & _ = _
	if(*this == none)
		return *this;
	if(val == none) {
		*this = none;
		return *this;
	}

	// check for any constant
	Value v;
	uintn_t k;
	if(isConst()) {
		if(val.isConst()) {		// k1 & k2
			*this = val.lower() & lower();
			return *this;
		}
		k = lower();
		v = val;
	}
	else if(val.isConst()) {
		v = *this;
		k = val.lower();
	}
	else {						// no k : cannot compute
		*this = all;
		return *this;
	}

	// v & 0 = 0
	if(k == 0) {
		*this = 0;
		return *this;
	}

	// find the field of [n, m]
	int n, m;
	for(m = 0; !(k & (1 << m)); m++);
	for(n = m; n < 32 && (k & (1 << n)); n++);
	n--;
	if(k & ~((1 << n) - 1)) {	// pure field ? (no one after n)
		*this = all;
		return *this;
	}

	// base % (1 << m) = 0
	if(v.lower() % (1 << m) == 0) {

		// 1 << m <= delta && delta
		if((1 << m) <= v.delta()) {
			// delta % (1 << m) = 0 -> (base, delta, n)
			if(v.delta() % (1 << m) == 0) {
				*this = v;
				return *this;
			}
		}

		// 1 << m > delta
		else {
			// (1 << m) % delta = 0 -> (base, 1 << m, (n + 1) / ((1 << m) / delta - 1)
			if((1 << m) % v.delta() == 0) {
				*this = Value(VAL, v.lower(), 1 << m, (v.mtimes() + 1) / ((1 << m) / v.delta()) - 1);
				return *this;
			}
		}
	}

	// try to rebuild AND  if threshold not reached
	if(int(v.mtimes() + 1) < and_threshold) {
		*this = none;
		intn_t n = v.lower();
		for(uintn_t i = 0; i < v.mtimes(); i++, n += v.delta())
			join(Value(VAL, n & k, 0, 0));
		return *this;
	}

	// else (0, 1 << m, 1 << (n + 1 - m) - 1)
	*this = Value(VAL, 0, 1 << m, (1 << (n + 1 - m)) - 1);
	return *this;
#endif	
}


//inline io::Output& operator<<(io::Output& out, const Value& v) { v.print(out); return out; }
const Value Value::none(NONE), Value::all(ALL, 0, 1, UMAXn);
const Value Value::bot(NONE), Value::top(ALL, 0, 1, UMAXn);

/* *** State methods *** */

/**
 * @class State
 * state for the @ref clp analysis.
 * @ingroup clp
 */

/**
 * Change the state to be a copy of the given one
 * @param state the state to be copied
*/
void State::copy(const State& state) {
	TRACED(cerr << "copy("; print(cerr); cerr << ", "; state.print(cerr); cerr << ") = ");
	clear();
	// memory
	first = state.first;
	for(Node *prev = &first, *cur = state.first.next; cur; cur = cur->next) {
		prev->next = new Node(cur);
		prev = prev->next;
	}
	// registers
	registers.copy(state.registers);
	tmpreg.copy(state.tmpreg);

	TRACED(print(cerr); cerr << io::endl);
}

/**
 * Remove all nodes from the state
*/
void State::clear(void) {

	// registers
	registers.clear();
	tmpreg.clear();

	// memory
	for(Node *cur = first.next, *next; cur; cur = next) {
		next = cur->next;
		delete cur;
	}
	first.next = 0;
}


/**
 * Set to T the memories on the given area.
 * @param base	Base address of the area.
 * @param size	Size of the area.
 */
void State::clear(t::uint32 base, t::uint32 size) {
	if(first.val == Value::none)
		return;
	for(Node *prev = &first, *cur = first.getNext(), *next; cur; cur = next) {
		next = cur->getNext();
		if(base <= cur->getAddress() && cur->getAddress() < base + size) {
			prev->next = 0;
			delete cur;
		}
		else {
			prev->next = cur;
			prev = cur;
		}
	}
}

/**
 * Define a value into a register or the memory
 * @param addr a value of kind REG for a register, VAL for the memory.
 *		  The value must be a constant (only the lower() attribute will be
 *		  used) or all.
 * @param val the value to store at the given address
*/
void State::set(const Value& addr, const Value& val) {
	TRACED(cerr << "set("; print(cerr); cerr << ", " << addr << ", " << val << ") = ");
	Node *prev, *cur, *next;
	if(first.val == Value::none) {
		TRACED(print(cerr); cerr << io::endl);
		return;
	}

	// insert a none in the state (put the state to none)
	if (val == Value::none){
		clear();
		first.val = Value::none;
		return;
	}

	// we assume that addr is a constant... (or T) - not anymore, a non-constant is a memory range to wipe
	// ASSERT(addr.isConst() || addr == Value::all);

	// == Register ==
	if(addr.kind() == REG) {
		if (addr.lower() < 0){
			// temp ones
			if (-addr.lower() < tmpreg.length())
				tmpreg.set(-addr.lower(), val);
			else {
				for(int i = tmpreg.length(); i < -addr.lower(); i++)
					tmpreg.add(Value::all);
				tmpreg.add(val);
			}
		} else {
			// real ones
			if (addr.lower() < registers.length())
				registers.set(addr.lower(), val);
			else {
				for(int i = registers.length(); i < addr.lower(); i++)
					registers.add(Value::all);
				registers.add(val);
			}
		}
		return;
	}

	// == or Memory ==

	// consum all memory references
	if(addr == Value::all) {
		prev = &first;
		cur = first.next;
		while(cur) {
			next = cur->next;
			delete cur;
			cur = next;
		}
		prev->next = 0;
		return;
	}

	else if(!addr.isConst()) {
		prev = &first;
		cur = first.next;
		for(prev = &first, cur = first.next; cur && cur->addr < uintn_t(addr.lower()); prev = cur, cur = cur->next);


		for( ; cur && cur->addr <= uintn_t(addr.upper()); ) {
			prev->next = cur->next;
			Node* toDelte = cur;
			cur = cur->next;
			delete toDelte;
		}
	}

	// find a value
	else {
		for(prev = &first, cur = first.next; cur && cur->addr < uintn_t(addr.lower()); prev = cur, cur = cur->next);
		if(cur && cur->addr == uintn_t(addr.lower())) { // find the exact match
			if(val.kind() != ALL)
				cur->val = val;
			else {
				prev->next = cur->next;
				delete cur;
			}
		}
		else if(val.kind() != ALL) { // if not, insert the memory node
			next = new Node(addr.lower(), val);
			prev->next = next;
			prev->next->next = cur;
		}
	}
	TRACED(print(cerr); cerr << io::endl);
}

/**
 * @return if a state is equals to the current one
*/
bool State::equals(const State& state) const {

	// Registers
	if (registers.length() != state.registers.length())
		return false;

	for (int i=0; i < registers.length(); i++)
		if (registers[i] != state.registers[i])
			return false;
	// Tmp registers
	/*for (int i=0; i < tmpreg.length(); i++)
		if (tmpreg[i] != state.tmpreg[i])
			return false;*/

	// Memory
	if(first.val.kind() != state.first.val.kind())
		return false;

	Node *cur = first.next, *cur2 = state.first.next;
	while(cur && cur2) {
		if(cur->addr != cur2->addr)
			return false;
		if(cur->val != cur->val)
			return false;
		cur = cur->next;
		cur2 = cur2->next;
	}
	return cur == cur2;
}

/**
 * Merge a state with the current one.
*/
void State::join(const State& state) {
	TRACEJ(cerr << "join(\n\t"; print(cerr); cerr << ",\n\t";  state.print(cerr); cerr << "\n\t) = ");

	// test none states
	if(state.first.val == Value::none)
		return;
	if(first.val == Value::none) {
		copy(state);
		TRACED(print(cerr); cerr << io::endl;);
		return;
	}

	// registers
	for(int i=0; i<registers.length() && i<state.registers.length() ; i++)
		registers[i].join(state.registers[i]);
	if (registers.length() < state.registers.length())
		for(int i=registers.length(); i < state.registers.length(); i++)
			registers.add(state.registers[i]);
	// temp registers
#	ifdef JOIN_TEMP_REGISTERS
	for(int i=0; i<tmpreg.length() && i<state.tmpreg.length() ; i++)
		tmpreg[i].join(state.tmpreg[i]);
	if (tmpreg.length() < state.tmpreg.length())
		for(int i=tmpreg.length(); i < state.tmpreg.length(); i++)
			tmpreg.add(state.tmpreg[i]);
#	endif

	// memory
	Node *prev = &first, *cur = first.next, *cur2 = state.first.next, *next;
	while(cur && cur2) {

		// addr1 < addr2 -> remove cur1
		if(cur->addr < cur2->addr) {
			prev->next = cur->next;
			delete cur;
			cur = prev->next;
		}

		// equality ? remove if join result in all
		else if(cur->addr == cur2->addr) {
			cur->val.join(cur2->val);
			if(cur->val.kind() == ALL) {
				prev->next = cur->next;
				delete cur;
				cur = prev->next;
			}
			else {
				prev = cur;
				cur = cur->next;
				cur2 = cur2->next;
			}
		}

		// addr1 > addr2 => remove cur2
		else
			cur2 = cur2->next;
	}

	// remove tail
	prev->next = 0;
	while(cur) {
		next = cur->next;
		delete cur;
		cur = next;
	}
	TRACEJ(print(cerr); cerr << io::endl;);
}

/**
 * Perform a widening.
 * @param state the state of the next iteration
 * @param loopBound is the number of iteration of the loop. A different widening
 *        operation will be used if the loopBound is known (>=0) or not.
*/
void State::widening(const State& state, int loopBound) {
	TRACED(cerr << "widening(" << loopBound << "\n\t");
	TRACED(print(cerr); cerr << ",\n\t";  state.print(cerr); cerr << "\n\t) = ");

	// test none states
	if(state.first.val == Value::none)
		return;
	if(first.val == Value::none) {
		copy(state);
		TRACED(print(cerr); cerr << io::endl;);
		return;
	}

	// registers
	for(int i=0; i<registers.length() && i<state.registers.length() ; i++)
		if (loopBound >= 0)
			registers[i].ffwidening(state.registers[i], loopBound);
		else {
			registers[i].widening(state.registers[i]);
		}

	if (registers.length() < state.registers.length())
		for(int i=registers.length(); i < state.registers.length(); i++) {
			registers.add(state.registers[i]);
			//registers.add(clp::Value::top);
		}

	// memory
	Node *prev = &first, *cur = first.next, *cur2 = state.first.next, *next;
	while(cur && cur2) {
		// addr1 < addr2 -> remove cur1
		if(cur->addr < cur2->addr) {
			prev->next = cur->next;
			delete cur;
			cur = prev->next;
		}
		// equality ? remove if join result in all
		else if(cur->addr == cur2->addr) {
			if (loopBound >= 0)
				cur->val.ffwidening(cur2->val, loopBound);
			else
				cur->val.widening(cur2->val);
			if(cur->val.kind() == ALL) {
				prev->next = cur->next;
				delete cur;
				cur = prev->next;
			}
			else {
				prev = cur;
				cur = cur->next;
				cur2 = cur2->next;
			}
		}
		// addr1 > addr2 => remove cur2
		else
			cur2 = cur2->next;
	}

	// remove tail
	prev->next = 0;
	while(cur) {
		next = cur->next;
		delete cur;
		cur = next;
	}
	TRACED(print(cerr); cerr << io::endl;);
}


void State::augment(const State& state) {
	TRACEJ(cerr << "augment(\n\t"; print(cerr); cerr << ",\n\t";  state.print(cerr); cerr << "\n\t) = ");

	// nothing to augment
	if(state.first.val == Value::none)
		return;
	// nothing for *this, hence take all from the augment
	if(first.val == Value::none) {
		copy(state);
		TRACED(print(cerr); cerr << io::endl;);
		return;
	}

	// registers
	for(int i=0; i<registers.length() && i<state.registers.length() ; i++) {
		if((registers[i] != clp::Value::all) && (state.registers[i] != clp::Value::all))
			registers[i].join(state.registers[i]);
		else if (registers[i] == clp::Value::all)
			registers[i] = state.registers[i];
		else if (state.registers[i] == clp::Value::all)
			{}
		else
			registers[i].join(state.registers[i]);
	}

	if (registers.length() < state.registers.length())
		for(int i=registers.length(); i < state.registers.length(); i++)
			registers.add(state.registers[i]);
	// temp registers
#	ifdef JOIN_TEMP_REGISTERS
	for(int i=0; i<tmpreg.length() && i<state.tmpreg.length() ; i++)
		tmpreg[i].join(state.tmpreg[i]);
	if (tmpreg.length() < state.tmpreg.length())
		for(int i=tmpreg.length(); i < state.tmpreg.length(); i++)
			tmpreg.add(state.tmpreg[i]);
#	endif

	// memory
	Node *cur = first.next, *cur2 = state.first.next, *next; // *prev = &first,
	while(cur && cur2) {

		// addr1 < addr2 -> keep
		if(cur->addr < cur2->addr) {
			cur = cur->next;
		}

		// equality ? remove if join result in all
		else if(cur->addr == cur2->addr) {
			cur->val.join(cur2->val);
			cur = cur->next;
			cur2 = cur2->next;
		}

		// addr1 > addr2 => remove cur2
		else
			cur2 = cur2->next;
	}
	TRACEJ(print(cerr); cerr << io::endl;);
}


/**
 * Print the state, the printing does not include the newline at the end
*/
void State::print(io::Output& out, const hard::Platform *pf) const {
	if(first.val == Value::none)
		out << "None (bottom)";
	else {
		#ifdef STATE_MULTILINE
			#define CLP_START "\t"
			#define CLP_END "\n"
			//out << "{\n";
		#else
			#define CLP_START ""
			#define CLP_END ", "
			out << "{";
		#endif
		// tmp registers
		/*for(int i = 0; i < tmpreg.length(); i++){
			Value val = tmpreg[i];
			if (val.kind() == VAL)
				out << CLP_START << "t" << i << " = " << val << CLP_END;
		}*/
		bool fst = true;
		// registers
		for(int i = 0; i < registers.length(); i++){
			Value val = registers[i];
			if (val.kind() == VAL) {
				if(!fst)
					out << ", ";
				else
					fst = false;
				if(!pf)
					out << "r" << i;
				else
					out << pf->findReg(i)->name();
				out << " = " << val;
			}
		}
		// memory
		for(Node *cur = first.next; cur; cur = cur->next) {
			if(!fst)
				out << ", ";
			else
				fst = false;
			out << CLP_START << Address(cur->addr);
			out << " = " << cur->val;
		}
		// temp register, not in state
		for(int i = 0; i < tmpreg.length(); i++){
			Value val = tmpreg[i];
			if (val.kind() == VAL) {
				if(!fst)
					out << ", ";
				else
					fst = false;
				out << "t" << i;
				out << " = " << val;
			}
		}
		#ifndef STATE_MULTILINE
		out << "}";
		#endif
	}
}

/**
 * Return a stored value
 * @param addr is the addresse to get the value of. The kind of the value
 *        can be REG for a register, or VAL for memory. The address is
 *        considered as constant, the lower() attribute is the value.
 * @return the stored value
*/
const Value& State::get(const Value& addr) const {
	Node * cur;
	ASSERTP(addr.isConst(), "addr = " << addr << " is not a constant of the kind " << addr.kind()); // we assume that addr is a constant...
	if(addr.kind() == REG){
		// Tmp Registers
		if (addr.lower() < 0)
			if ((-addr.lower()) < tmpreg.length())
				return tmpreg[-addr.lower()];
			else
				return Value::all;
		// Real registers
		else if (addr.lower() < registers.length())
			return registers[addr.lower()];
		else
			return Value::all;
	} else {
		// Memory
		for(cur = first.next; cur && cur->addr < uintn_t(addr.lower()); cur = cur->next)
				;
		if(cur && cur->addr == uintn_t(addr.lower()))
			return cur->val;
		return first.val;
	}
}

const State State::EMPTY(Value::none), State::FULL(Value::all);
io::Output& operator<<(io::Output& out, const State& state) { state.print(out); return out; }

// internal
struct sorter { static inline int compare(Segment *s1, Segment *s2) { return s1->address().compare(s2->address()); } };

/**
 * Definition of the abstract interpretation problem to make a CLP analysis
 */
class ClpProblem {
public:
	typedef clp::State Domain;

	typedef ClpProblem Problem;
	Problem& getProb(void) { return *this; }

	ClpProblem(Process *proc)
	:	pc(0),
		has_if(false),
		has_branch(false),
		bb(0),
		currentInst(0),
		bBuildFilters(false),
		_process(proc),
		istate(0),
		currentClpStatePack(0),
		currentAccessAddress(0),
	 	_nb_inst(0),
	 	_nb_sem_inst(0),
	 	_nb_set(0),
	 	_nb_top_set(0),
	 	_nb_store(0),
	 	_nb_top_store(0),
	 	_nb_top_store_addr(0),
	 	_nb_load(0),
	 	_nb_load_top_addr(0),
	 	_nb_filters(0),
	 	_nb_top_filters(0),
	 	_nb_top_load(0),
		_nb_clp_bb_count(0)
{ }

	/*
	 *
	 */
	inline void setInitialState(dfa::State* ds) { istate = ds; }

#	ifdef HAI_JSON
		void dumpJSON(const Domain& dom, json::Saver& saver) {
			if(dom == Domain::EMPTY)
				saver.put("_");
			else if(dom == Domain::FULL)
				saver.put("T");
			else {
				saver.beginObject();
				for(Domain::Iter i(dom); i; i++)
					switch(i.id().kind()) {
					case NONE:
					case ALL:
						break;
					case REG:
						saver.addField(_ << i.id().lower());
						saver.put(_ << *i);
						break;
					case VAL:
						saver.addField(_ << "0x" << ot::address(i.id().lower()));
						saver.put(_ << *i);
						break;
					}
				saver.endObject();
			}
		}
#	endif

	/**
	 *  Initialize a register in the init state from an address.
	 *  @param reg		Register to initialize.
	 *  @param address	Address to set in register.
	 */
	void initialize(const hard::Register *reg, const Address& address) {
		Value v;
		v = Value(VAL, address.offset());
		TRACEP(cerr << "init:: r" << reg->platformNumber() << " <- " << v.lower() << "\n");
		set(_init, reg->platformNumber(), v);
	}

	/**
	 * Initialize a register from a DFA value.
	 * @param reg	Register to initialize.
	 * @param val	DFA value.
	 */
	void initialize(const hard::Register *reg, const dfa::Value val) {
		Value v(VAL, val.base(), val.delta(), val.count());
		TRACEP(cerr << "init:: r" << reg->platformNumber() << " <- " << v << "\n");
		set(_init, reg->platformNumber(), v);
	}

	/**
	 * Initialize a memory cell from a DFA value.
	 */
	void initialize(Address addr, const dfa::Value val) {
		Value v(VAL, val.base(), val.delta(), val.count());
		TRACEP(cerr << "init:: @" << addr << " <- " << v << "\n");
		_init.set(Value(REG, addr.offset()), v);
	}

	/** Provides the Bottom value of the Abstract Domain */
	inline const Domain& bottom(void) const { return State::EMPTY; }
	inline const Domain& top(void) const { return State::FULL; }

	/** Provides the entry state of the program */
	inline const Domain& entry(void) const {
		TRACEP(cerr << "entry() = " << _init << io::endl);
		return _init;
	}
	/**
	 * The Least Upper Bound of two elements a and b.
	 * This will be used, among other things, for performing the junction
	 * between two CFG paths.
	*/
	inline void lub(Domain &a, const Domain &b) const {
		a.join(b);
	}

	void checkWideningAlarm(Domain& d, Domain& a, Domain& b) const {
		State::Iter id(d), ia(a), ib(b);

		// check the state
		while(id && ia && ib) {
			Value idd = id.id(), ida = ia.id(), idb = ia.id();
			if(idd.kind() == REG && ida.kind() == REG && ida.kind() == REG) {
				if((*id).kind() == ALL && (*ia).kind() != ALL && (*ib).kind() != ALL) {
					cerr << "\t\t\tALARM! widening register " << idd << ": " << *ia << ", " << *ib << " -> " << *id << io::endl;
					return;
				}
				id++;
				ia++;
				ib++;
			}
			else if(idd.kind() == VAL && ida.kind() == VAL && ida.kind() == VAL) {
				t::uint32 ad = idd.lower(), aa = ida.lower(), ab = idb.lower();
				if(ad > aa && ad > ab && aa == ab) {
					cerr << "\t\t\tALARM! widening memory " << ida << ": " << *ia << ", " << *ib << io::endl;
					return;
				}
				if(ad < aa && ad < ab)
					id++;
				else if(aa < ad && aa < ab)
					ia++;
				else
					ib++;
			}
			else {
				if(idd.kind() == REG)
					id++;
				if(ida.kind() == REG)
					ia++;
				if(idb.kind() == REG)
					ib++;
			}
		}

		// check the end
		if(!id && ia && ib)
			cerr << "\t\t\tALARM! widening" << ia.id() << io::endl;
	}

	/**
	 * The windening operation
	 * This will be used to perform the junction between to iteration of
	 * a loop.
	*/
	inline void widening(Block *bb, Domain &a, Domain b) const{
		TRACEA(Domain di = a);
		TRACEP(cerr << "*** widening ****\n");
		TRACEP(cerr << "s1 = " << a << "\ns2 = " << b << ") = ");
		a.widening(b, MAX_ITERATION(bb));
		TRACEA(checkWideningAlarm(a, di, b));
		TRACEP(cerr << a << io::endl);
	}

	/**
	 * Update the domain in a way specific of the edge (for filtering purpose)
	*/
	inline void updateEdge(Edge *edge, Domain &dom){
		Block *source = edge->source();
		TRACEP(cerr << "\n*** Update edge from " << source
					<< " to " << edge->target()
					<< " [taken=" << edge->isTaken() << "] ***\n");
		TRACEP(cerr << "s = " << dom << io::endl);
		if(!edge->isBoth() && se::REG_FILTERS.exists(source)) {
			TRACEP(cerr << "\tApply filter on this edge!\n");

			Vector<se::SECmp *> reg_filters = se::REG_FILTERS(edge);
			Vector<se::SECmp *> addr_filters = se::ADDR_FILTERS(edge);

			// filter registers
			Domain all = Domain::EMPTY, init;
			init.copy(dom);
			for (int i=0; i < reg_filters.length(); i++){
				se::SECmp *filter = reg_filters[i];

				// case of OR or NONE
				if(filter->op() == se::NONE)
					continue;
				else if(filter->op() == se::OR) {
					all.join(dom);
					dom.copy(init);

					continue;
				}

				// apply the filter
				TRACEP(cerr << "\tFilter: " << filter->asString() << ' ');
				//FIXME: check in a() and b() are valid pointers
				Value rval = filter->a()->val();
				Value r = Value(REG, rval.lower(), 0, 0);
				Value v = dom.get(r);
				TRACEP(cerr << v);
				TRACEP(cerr << " -> ");
				applyFilter(v, filter->op(), filter->b()->val());
				TRACEP(cerr << v << io::endl);
				dom.set(r, v);
				TRACEP(cerr << "d = " << dom << io::endl);
				_nb_filters++;
				if (filter->b()->val() == Value::all)
					_nb_top_filters++;
			}

			// complete fitering
			dom.join(all);
			init.copy(dom);

			// filter addresses
			for (int i=0; i < addr_filters.length(); i++){
				se::SECmp *filter = addr_filters[i];

				// case of OR or NONE
				if(filter->op() == se::NONE)
					continue;
				else if(filter->op() == se::OR) {
					all.join(dom);
					dom.copy(init);
					continue;
				}

				// apply the filter
				TRACEP(cerr << "\tFilter: " << filter->asString() << ' ');
				//FIXME: check in a() and b() are valid pointers
				Value a = filter->a()->val();
				Value v = dom.get(a);
				TRACEP(v.print(cerr));
				// if we are from a loop header, widen the value
				TRACEP(cerr << " -> ");
				applyFilter(v, filter->op(), filter->b()->val());
				TRACEP(v.print(cerr));
				TRACEP(cerr << '\n');
				dom.set(a, v);
				_nb_filters++;
				if (filter->b()->val() == Value::all)
					_nb_top_filters++;
			}

			// complete computation
			dom.join(all);

//			// free filters if required
//			if(to_free) {
//				for(int i = 0; i < reg_filters.count(); i++)
//					delete reg_filters[i];
//				for(int i = 0; i < addr_filters.count(); i++)
//					delete addr_filters[i];
//			}
		}
		TRACEP(cerr << "s' = " << dom << io::endl);
	}

	/** This function does the assignation of a state to another. */
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	/** This functions tests two states for equality. */
	inline bool equals(const Domain &a, const Domain &b) const {
		return a.equals(b);
	}

	inline void enterContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }


	/**
	 * Read a value from the memory.
	 * @param address	Address to read.
	 * @param type		Type of the data.
	 * @return			Read data value.
	 */
	Value readFromMem(uintn_t address, sem::type_t type) {
		switch(type) {
		case sem::INT8: 	{ t::int8 d; _process->get(address, d); return Value(d); }
		case sem::INT16: 	{ t::int16 d; _process->get(address, d); return Value(d); }
		case sem::INT32: 	{ t::int32 d; _process->get(address, d); return Value(d); }
		case sem::UINT8: 	{ t::uint8 d; _process->get(address, d); return Value(d); }
		case sem::UINT16: 	{ t::uint16 d; _process->get(address, d); return Value(d); }
		case sem::UINT32: 	{ t::uint32 d; _process->get(address, d); return Value(d); }
		default:			return Value::all;
		}
	}

	Value getCurrentAccessAddress(void) {
		return currentAccessAddress;
	}

	/**
	 * Interpret one instruction.
	 * @param state		State to update.
	 */
	void update(State *state) {
		_nb_clp_bb_count++;
		TRACEI(cerr << "\t\t" << i << io::endl);
		sem::inst& i = b[pc];
		if(state->equals(Domain::EMPTY)) { // handles the bottom state input (possibly infeasible path)
			pc++;
			return;
		}

		switch(i.op) {
		case sem::BRANCH: {
			// pc = b.length(); // commented out because it only indicates that the PC of the processor to change, not the pc of the semanitc block
			TRACESI(cerr << "\t\t\tbranch(" << get(*state, i.d()) << ")\n");
			if(has_if)
				has_branch = true;
			break;
		}
		case sem::TRAP:
			pc = b.length(); // goes to the end of the semantic instruction block of the whole BB
			TRACESI(cerr << "\t\t\ttrap\n");
			break;
		case sem::CONT:
			pc = b.length(); // goes to the end of the semantic instruction block of the whole BB
			TRACESI(cerr << "\t\tcont\n");
			break;
		case sem::IF:
			TRACESI(cerr << "\t\t\tif(" << i.sr() << ", " << i.cond() << ", " << i.jump() << ")\n");
			listOfIFsToDo.push(pair(pc + i.b() + 1, new Domain(*state)));
			has_if = true;
			break;
		case sem::NOP: break;
		case sem::LOAD: {
			_nb_load++;
			Value addrclp = get(*state, i.a());
			currentAccessAddress = addrclp;
			TRACESI(cerr << "\t\t\tload(" << i.d() << ", " << addrclp << ") = ");
			// first try to read the values from CLP state
			if (addrclp == Value::all){
				set(*state, i.d(), addrclp);
				_nb_load_top_addr++;
				TRACESI(cerr << "T\n");
				TRACEA(cerr << "\t\t\tALARM ! Load at T !\n");
			} else if (addrclp.mtimes() < MEMORY_ACCESS_THRESHOLD){
				// if the addr is not cst, load only if
				// there is less than MEMORY_ACCESS_THRESHOLD values to join
				Value addr(VAL, addrclp.lower());
				set(*state, i.d(), state->get(addr));
				for(unsigned int m = 1; m <= addrclp.mtimes(); m++){
					//cerr << "load for m=" << m << '\n';
					// join other values with the first
					Value addr(VAL, addrclp.lower() + addrclp.delta() * m);
					Value val = get(*state, i.d());
					val.join(state->get(addr));
					set(*state, i.d(), val);
				}
				TRACESI(cerr << get(*state, i.d()) << io::endl);
			} else {
				set(*state, i.d(), Value::all);
				TRACESI(cerr << "T (too many)\n");
				TRACEA(cerr << "\t\t\tALARM! load too many\n");
			}

			// if the value is not available, read it from Initialized State
			if(get(*state, i.d()) == Value::all) {
				Value val = Value::none;
				// need to make sure the starting address to load is within the READ_ONLY_AREA
				bool addressInInitMem = false;
				if(istate && istate->isReadOnly(Address((uintn_t)addrclp.start())))
					addressInInitMem = true;

				bool warningFlag = true;
				for(unsigned int m = 0; (m <= addrclp.mtimes()) && addressInInitMem && (addrclp.mtimes() < MEMORY_ACCESS_THRESHOLD*2 /*!= clp::UMAXn*/); m++){
					if((addrclp.mtimes() > MEMORY_ACCESS_THRESHOLD) && warningFlag) {
						elm::cerr << "WARNING: accessing more than " << MEMORY_ACCESS_THRESHOLD << " locations in the initialized memory (" << addrclp.mtimes() << " times)" << io::endl;
						warningFlag = false;
					}
					Value addr(VAL, addrclp.lower() + addrclp.delta() * m);
					if(istate && istate->isReadOnly((uintn_t)addr.start())) {
						Value r = readFromMem(addr.lower(), i.type());
						val.join(r);
						set(*state, i.d(), val);
					} // end of each valid address to load
				} // for each address
			}

			if((i.d() >= 0) && (get(*state, i.d()) == Value::all))
				_nb_top_load++;
			} break;

		case sem::STORE: {
				Value addrclp = get(*state, i.a());
				currentAccessAddress = addrclp;
				TRACESI(cerr << "\t\t\tstore(" << get(*state, i.d()) << ", " << addrclp << ")\n");

				// store at T
				if (addrclp == Value::all) {
					// obtain the range

					Pair<Address, Address> accessRange = otawa::ACCESS_RANGE(currentInst);

					if(accessRange.fst != Address::null && accessRange.snd != Address::null)
						state->set(clp::Value(VAL, accessRange.fst.offset(), 1, accessRange.snd.offset() -accessRange.fst.offset()), get(*state, i.d()));
					else
						state->set(addrclp, get(*state, i.d()));






					_nb_store++; _nb_top_store ++;
					_nb_top_store_addr++;
					ALARM_STORE_TOP(warnStoreToTop());
#ifdef CATCH_STT
					assert(0);
#endif
#					ifdef HAI_JSON
						HAI_BASE->addEvent("store to T");
#					endif
				}

				// store all on the area (too many addresses)
				else if (addrclp.mtimes() >= MEMORY_ACCESS_THRESHOLD) {
					_nb_store++;
					_nb_top_store ++;
					if(addrclp.mtimes() < UMAXn) {
						// state->set(Value::all, get(*state, i.d())); // instead of setting all, we only clear a range of the memories
						state->clear(addrclp.start(), elm::abs(addrclp.delta()) * addrclp.mtimes());
					}
					else {
						Symbol *sym = this->_process->findSymbolAt(addrclp.lower());
						if(!sym) {
							_nb_top_store_addr++;
							ALARM_STORE_TOP(warnStoreToTop());
#						ifdef HAI_JSON
							HAI_BASE->addEvent("store to T");
#						endif
						}
						else
							state->clear(sym->address().offset(), sym->size());
					}
				}

				// simple store
				else {
					_nb_store++;
					if (get(*state, i.d()) == Value::all)
						_nb_top_store++;
					if(addrclp.mtimes())
					{
						for(unsigned int m = 0; m <= addrclp.mtimes(); m++){ // store to the list of addresses
							Value addr(VAL, addrclp.lower() + addrclp.delta() * m);
							// opt1: store the value to all the addresses
							//state->set(addr, get(*state, i.d()));
							// opt2: since not sure which address to store, but all these address are possible to change to a non-determined value
							//state->set(addr, Value::all);
							// opt3: get the original value and join together
							Value val = get(*state, i.d());
							val.join(state->get(addr));
							state->set(addr, val);
						}
					}
					else {
						Value addr(addrclp.lower());
						state->set(addr, get(*state, i.d()));
					}
				}
			} break;
		case sem::SETP:
			set(*state, i.d(), Value::all);
			TRACESI(cerr << "\t\t\tsetp(" << i.d() << ", " << i.cst() << ") = T\n");
			break;
		case sem::CMP:
			set(*state, i.d(), Value::all);
			TRACESI(cerr << "\t\t\tcmp(" << i.d() << ", " << i.a() << ", " << i.b() << ") = T\n");
			break;
		case sem::CMPU:
			set(*state, i.d(), Value::all);
			TRACESI(cerr << "\t\t\tcmpu(" << i.d() << ", " << i.a() << ", " << i.b() << ") = T\n");
			break;
		case sem::SCRATCH:
			set(*state, i.d(), Value::all);
			TRACESI(cerr << "\t\t\tscratch(" << i.d() << ")\n");
			break;
		case sem::SET: {
				Value v = get(*state, i.a());
				set(*state, i.d(), v);
				TRACESI(cerr << "\t\t\tset(" << i.d() << ", " << v << ")\n");
				_nb_set++;
				if (v == Value::all)
					_nb_top_set++;
			} break;
		case sem::SETI: {
				Value v(VAL, i.cst());
				set(*state, i.d(), v);
				TRACESI(cerr << "\t\t\tseti(" << i.d() << ", " << v << ")\n");
			} break;
		case sem::ADD: {
				Value v = get(*state, i.a());
				TRACESI(cerr << "\t\t\tadd(" << i.d() << ", " << v << ", " << get(*state, i.b()));
				v.add(get(*state, i.b()));
				TRACESI(cerr << ") = " << v << io::endl);
				TRACEA(if(get(*state, i.a()) != Value::all
					   && get(*state, i.b()) != Value::all
					   && v == Value::all) cerr << "\t\t\tALARM! add\n");
				set(*state, i.d(), v);
			} break;
		case sem::SUB: {
				Value v = get(*state, i.a());
				TRACESI(cerr << "\t\t\tsub(" << i.d() << ", " << v << ", " << get(*state, i.b()));
				v.sub(get(*state, i.b()));
				TRACESI(cerr << ") = " << v << io::endl);
				TRACEA(if(get(*state, i.a()) != Value::all
					   && get(*state, i.b()) != Value::all
					   && v == Value::all) cerr << "\t\t\tALARM! sub\n");
				set(*state, i.d(), v);
			} break;
		case sem::SHL: {
				Value v = get(*state, i.a());
				TRACESI(cerr << "\t\t\tshl(" << i.d() << ", " << v << ", " << get(*state, i.b()));
				v.shl(get(*state, i.b()));
				TRACESI(cerr << ") = " << v << io::endl);
				TRACEA(if(get(*state, i.a()) != Value::all
					   && get(*state, i.b()) != Value::all
					   && v == Value::all) cerr << "\t\t\tALARM! shl\n");
				set(*state, i.d(), v);
				if(v == Value::all)
					_nb_top_set++;
			} break;
		case sem::SHR: case sem::ASR: {
				Value v = get(*state, i.a());
				TRACESI(cerr << "\t\t\tshr(" << i.d() << ", " << v << ", " << get(*state, i.b()));
				v.shr(get(*state, i.b()));
				TRACESI(cerr << ") = " << v << io::endl);
				TRACEA(if(get(*state, i.a()) != Value::all
					   && get(*state, i.b()) != Value::all
					   && v == Value::all) cerr << "\t\t\tALARM! shr\n");
				set(*state, i.d(), v);
				if(v == Value::all)
					_nb_top_set++;
			} break;
		case sem::OR: {
				Value v = get(*state, i.a());
				TRACESI(cerr << "\t\t\tor(" << i.d() << ", " << v << ", " << get(*state, i.b()));
				v._or(get(*state, i.b()));
				TRACESI(cerr << ") = " << v << io::endl);
				TRACEA(if(get(*state, i.a()) != Value::all
					   && get(*state, i.b()) != Value::all
					   && v == Value::all) cerr << "\t\t\tALARM! or\n");
				set(*state, i.d(), v);
				if(v == Value::all)
					_nb_top_set++;
			} break;

		case sem::AND: {
				Value v = get(*state, i.a());
				TRACESI(cerr << "\t\t\tand(" << i.d() << ", " << v << ", " << get(*state, i.b()));
				v._and(get(*state, i.b()));
				if(v == Value::all)
					_nb_top_set++;
				TRACESI(cerr << ") = " << v << io::endl);
				set(*state, i.d(), v);
		} break;
		case sem::MUL: {
			Value va = get(*state, i.a());
			Value vb = get(*state, i.b());
			if(va.isTop() || vb.isTop())
				set(*state, i.d(), Value::all);
			else if(va.isConst() && vb.isConst())
				set(*state, i.d(), Value(va.lower()*vb.lower()));
			else
				set(*state, i.d(), Value::all);
		} break;
//		case sem::MUL: {
//				Value v = get(*state, i.a());
//				TRACESI(cerr << "\t\t\tmul(" << i.d() << ", " << v << ", " << get(*state, i.b()));
//				v.add(get(*state, i.b()));
//				TRACESI(cerr << ") = " << v << io::endl);
//				TRACEA(if(get(*state, i.a()) != Value::all
//					   && get(*state, i.b()) != Value::all
//					   && v == Value::all) cerr << "\t\t\tALARM! add\n");
//				set(*state, i.d(), v);
//			} break;
		default: {
				set(*state, i.d(), Value::all);
			} break;
		}
		TRACEI(cerr << "\t\t !!-> " << *state << io::endl);

		pc++;
	}

	/**
	 * Get a semantic instruction by its index.
	 * @param i		Index of the semantic instruction.
	 * @return		Semantic instruction at the given index.
	 */
	inline sem::inst sem(int i) const { return b[i]; }

	/**
	 * Get the PC value.
	 * @return	PC value.
	 */
	inline int getPC(void) const { return pc; }

	/**
	 * Test if the current path is ended.
	 * @return	True if it is ended, false else.
	 */
	inline bool isPathEnded(void) const { return pc >= b.count(); }

	/**
	 * Get state of the next path.
	 * @return Next path state or null.
	 */
	clp::State *nextPath(void) {
		if(!listOfIFsToDo)
			return 0;
		else {
			Pair<int, clp::State *> n = listOfIFsToDo.pop();
			pc = n.fst;
			return n.snd;
		}
	}

	/**
	 * Prepare for interpreting the given machine instruction.
	 * @param inst	Instruction to interpret.
	 */
	//void prepare(Inst *inst) {
	void prepare(const Bundle& bundle) {
		b.clear();
		//inst->semInsts(b);
		bundle.semInsts(b);
		currentInst = bundle.first();
		pc = 0;
		listOfIFsToDo.clear();
	}

	/**
	 * This function update the state by applying a basic block.
	 * It gives the output state, given the input state and a pointer to the
	 * basic block.
	*/
	void update(Domain& out, const Domain& in, Block* bb) {
		out.copy(in);

		// do nothing for an end block, just simply pass the in state to out state
		if(bb->isEnd())
			return;

		// the unknown block
		if(bb->isSynth() && !bb->toSynth()->callee()) {
			// UNKOWN_BLOCK_EVALUATION is an identifier associated with the process, if it is set to true
			// the unknown block will be evaluated to bottom. This is to prevent the unknown block brings the
			// state to top to wipe out the useful state....
			if(UNKOWN_BLOCK_EVALUATION(_process)) {
				out = bottom();
			}
			else {
				out = top();
			}
			return;
		}

		this->bb = bb;

		Domain *state; // the working state in this function, it points to the output state so that the output changes accordingly
		clp::ClpStatePack::InstPack *ipack = 0;
		TRACEP(cerr << "\n*** update(" << bb << ") ***\n");
		TRACEP(cerr << "s = " << in << io::endl);
		// save the input state in the basic block, join with an existing state
		// if needed
		if(!bBuildFilters) { // in general case
			if(clp::STATE_IN(bb).exists())
				clp::STATE_IN(bb).remove();

			clp::STATE_IN(bb) = in;
		}

		if(out.equals(Domain::EMPTY)) { // if the in state is bottom, then we don't have to evaluate this...
			return;
		}

#ifdef USE_INST
		for(BasicBlock::InstIter inst = bb->toBasic()->insts(); inst; inst++) {
#else // use bundle
		for(BasicBlock::BundleIter bundle(bb->toBasic()); bundle; bundle++) {
			has_if = false;
			has_branch = false;
#endif

#ifdef USE_INST
			this->currentInst = inst;
#else
			this->currentInst = (*bundle).first();
#endif
			TRACESI(cerr << '\t' << inst->address() << ": "; inst->dump(cerr); cerr << io::endl);
			_nb_inst++; // statistics

			// get semantic instructions
#ifdef USE_INST
			prepare(inst);
#else
			b.clear();
			(*bundle).semInsts(b);
			pc = 0;
			listOfIFsToDo.clear();
#endif

			state = &out;

			// initialize the InstPack for the current instruction/bundle
			ASSERT(!( (!bBuildFilters & (currentClpStatePack !=0)) | (bBuildFilters & (currentClpStatePack ==0)) )); // show that currentClpStatePack and bBuildFilters always true together
			if(bBuildFilters && currentClpStatePack) { // when building a filter
				ipack = currentClpStatePack->newPack(currentInst->address());
			}

			// perform interpretation of each semantic instructions of a given instruction bundle
			while(true) {

				// pc is the current index of the semantic instruction to process.
				// when pc equals to the b.length() that means we are reaching the end of the semantic instruction block
				while(pc < b.length()) {
					update(state);
					_nb_sem_inst++; // for statistics

					// When creating a filter, a data strucuture is created to hold the state for each semantic instruction of each addresses
					// currentClpStatePack of ClpStatePack:
					// |---------------------------------------------|
					// | ipack of InstPack for instruction/bundle 1  |
					// | .........                                   |
					// | ipack of InstPack for instruction/bundle n  |
					// |---------------------------------------------|
					//
					// For each ipack, it contains the state for each semantic instruction of the instruction/bundle
					// |----------------------------------------------|
					// | state of semInst1_1 for instruction/bundle 1 |
					// | .........                                    |
					// | state of semInstM_1 for instruction/bundle 1 |
					// |----------------------------------------------|
					// .........
					// |----------------------------------------------|
					// | state of semInst1_n for instruction/bundle n |
					// | .........                                    |
					// | state of semInstM_n for instruction/bundle n |
					// |----------------------------------------------|
					ASSERT(!( (!bBuildFilters & (currentClpStatePack !=0)) | (bBuildFilters & (currentClpStatePack ==0)) )); // show that currentClpStatePack and bBuildFilters always true together
					if (bBuildFilters && currentClpStatePack){
						ipack->append(*state);
					}
				}

				// this normally happens when processing other processing path formed by the IF sem. inst.
				// because two different paths for the condition, we need to merge the resulted states from both paths of the IF sem. inst.
				// so every possibility is covered
				if(state != &out) {
					out.join(*state);
					delete state;
				}

				// if there are alternative path, i.e. because of an IF sem, to handle.
				if(!listOfIFsToDo)
					break; // when all the forking states are processed
				else {
					Pair<int, Domain *> p = listOfIFsToDo.pop(); // popping the state prepared previous when an IF was encountered
					pc = p.fst;
					state = p.snd;
				}

			} // end of processing semantic instructions of a given instruction/bundle
		} // end of going through all the instructions/bundles of the basic block
		// reset tracking
		this->currentInst = 0;
		this->bb = 0;

		TRACEP(cerr << "s' = " << out << io::endl);
		if (bBuildFilters)
			return;

		// save the output state in the basic block
		if(clp::STATE_OUT(bb).exists())
			clp::STATE_OUT(bb).remove();

		clp::STATE_OUT(bb) = out;
		TRACEU(cerr << ">>>\tout = " << out << io::endl);

		// if the block has an IF instruction
		if(has_branch /*&& ! se::REG_FILTERS.exists(bb)*/){ // re-evaluate filters, because values can change!
			//TODO: delete 'old' reg_filters if needed
			//use Symbolic Expressions to get filters for this basic block
			TRACEP(cerr << "> IF+BRANCH detected, getting filters..." << io::endl);
			se::FilterBuilder builder(bb->toBasic(), *this);
		}
	}

	/**
	 * Get the content of a register
	 * @param state is the state from which we want information
	 * @param i is the identifier of the register
	 * @return the value of the register
	 */
	const clp::Value& get(const clp::State& state, int i) {
		clp::Value addr(clp::REG, i);
		return state.get(addr);
	}

	/**
	 * Set a register to the given value
	 * @param state is the initial state
	 * @param i is the regsiter identifier (i < 0 for temp registers)
	 * @param v is the value to set
	 * @return the new state
	*/
	const void set(clp::State& state, int i, const clp::Value& v) {
		clp::Value addr(clp::REG, i);
		return state.set(addr, v);
	}

	void fillPack(BasicBlock* bb, clp::ClpStatePack *empty_pack) {
		bBuildFilters = true; // currently only set to true when building filters
		currentClpStatePack = empty_pack;
		clp::State output;
		clp::State input = clp::STATE_IN(*bb);
		update(output, input, bb);
		currentClpStatePack = 0;
		bBuildFilters = false;
	}

	/**
	 * Return various statistics about the analysis
	*/
	inline clp::STAT_UINT get_nb_inst(void){ return _nb_inst; }
	inline clp::STAT_UINT get_nb_sem_inst(void){ return _nb_sem_inst; }
	inline clp::STAT_UINT get_nb_set(void){ return _nb_set; }
	inline clp::STAT_UINT get_nb_top_set(void){ return _nb_top_set; }
	inline clp::STAT_UINT get_nb_store(void){ return _nb_store; }
	inline clp::STAT_UINT get_nb_top_store(void){ return _nb_top_store; }
	inline clp::STAT_UINT get_nb_top_store_addr(void){return _nb_top_store_addr;}
	inline clp::STAT_UINT get_nb_load(void){return _nb_load;}
	inline clp::STAT_UINT get_nb_load_top_addr(void){return _nb_load_top_addr;}
	inline clp::STAT_UINT get_nb_filters(void){ return _nb_filters;}
	inline clp::STAT_UINT get_nb_top_filters(void){ return _nb_top_filters;}
	inline clp::STAT_UINT get_nb_top_load(void) const { return _nb_top_load; }
	inline clp::STAT_UINT get_nb_clp_bb_count(void) const { return _nb_clp_bb_count; }

private:
	clp::State _init;
	sem::Block b;
	Vector<Pair<int, Domain *> > listOfIFsToDo; // when encountering an IF sem. inst., the analysis has to take care of both taken and non-taken cases with filters
	int pc;
	bool has_if;
	bool has_branch;
	Block *bb; // use for tracking, nothing to do with the analysis itself
	Inst *currentInst; // use for tracking, nothing to do with the analysis itself

	/* attribute for specific analysis / packing */
	bool bBuildFilters; // currently only set to true when building filters
	Process* _process;
	dfa::State *istate;
	clp::ClpStatePack *currentClpStatePack;
	clp::Value currentAccessAddress;

	// attributes for statistics purpose
	clp::STAT_UINT _nb_inst;
	clp::STAT_UINT _nb_sem_inst;
	clp::STAT_UINT _nb_set;
	clp::STAT_UINT _nb_top_set;
	clp::STAT_UINT _nb_store;
	clp::STAT_UINT _nb_top_store;
	clp::STAT_UINT _nb_top_store_addr;
	clp::STAT_UINT _nb_load;
	clp::STAT_UINT _nb_load_top_addr;
	clp::STAT_UINT _nb_filters;
	clp::STAT_UINT _nb_top_filters;
	clp::STAT_UINT _nb_top_load;
	clp::STAT_UINT _nb_clp_bb_count;

	// store to T management
	List<Pair<Inst *, Block *> > top_stores;
	void warnStoreToTop(void) {
		if(!VERBOSE(_process))
			return;
		if(!currentInst || !bb)
			return;
		Pair<Inst *, Block *> p = pair(currentInst, bb);
		if(!top_stores.contains(p)) {
			top_stores.add(p);
			cerr << "WARNING: clp: (" << p.snd << "):" << p.fst->address() << ": " << p.fst << " store to T (unbounded address)\n";
		}
	}
};

// CLPStateCleaner
class CLPStateCleaner: public Cleaner {
public:
	inline CLPStateCleaner(WorkSpace* _ws) : ws(_ws) { }

	virtual void clean(void) {
		const CFGCollection *cfgc = INVOLVED_CFGS(ws);
		for(CFGCollection::Iter cfg(cfgc); cfg; cfg++) {
			for(CFG::BlockIter bbi = cfg->blocks(); bbi; bbi++){
				clp::STATE_IN(*bbi).remove();
				clp::STATE_OUT(*bbi).remove();

				if(se::REG_FILTERS(*bbi).exists()) {
					Vector<se::SECmp *> vse = se::REG_FILTERS(*bbi);
					for(Vector<se::SECmp *>::Iter vsei(vse); vsei; vsei++)
						delete *vsei;
					se::REG_FILTERS(*bbi).remove();
				}

				if(se::ADDR_FILTERS(*bbi).exists()) {
					Vector<se::SECmp *> vse = se::ADDR_FILTERS(*bbi);
					for(Vector<se::SECmp *>::Iter vsei(vse); vsei; vsei++)
						delete *vsei;
					se::ADDR_FILTERS(*bbi).remove();
				}
			}
		}
	}
private:
	WorkSpace* ws;
};

/**
 * @class ClpAnalysis
 *
 * This analyzer tracks values in the form of a CLP.
 *
 * @par Widening
 *
 * The widening of this analysis is performed using filtering from symbolic
 * expressions.
 *
 * @par Provided Features
 * @li @ref otawa::clp::CLP_ANALYSIS_FEATURE
 *
 * @par Required Features
 * @li @ref otawa::LOOP_INFO_FEATURE
 * @li @ref otawa::ipet::FLOW_FACTS_FEATURE
 * @li @ref otawa::VIRTUALIZED_CFG_FEATURE
 *
 * @ingroup clp
 */

p::declare Analysis::reg = p::init("otawa::clp::CLPAnalysis", Version(0, 1, 0))
	.maker<Analysis>()
	//.require(VIRTUALIZED_CFG_FEATURE)
	.require(COLLECTED_CFG_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(FLOW_FACTS_FEATURE)
	.require(dfa::INITIAL_STATE_FEATURE)
	.provide(clp::CLP_ANALYSIS_FEATURE);

Analysis::Analysis(p::declare& r)
: 	Processor(r),
	mem(0),
	_nb_inst(0),
	_nb_sem_inst(0),
	_nb_set(0),
	_nb_top_set(0),
	_nb_store(0),
	_nb_top_store(0),
	_nb_top_store_addr(0),
	_nb_load(0),
	_nb_top_load(0),
	_nb_load_top_addr(0),
	_nb_filters(0),
	_nb_top_filters(0),
	verbose(false)
{ }


/**
 */
void Analysis::setup(WorkSpace *ws) {
	mem = hard::MEMORY(ws);
}


/**
 * Perform the analysis by processing the workspace
 * @param ws the workspace to be processed
 */
void Analysis::processWorkSpace(WorkSpace *ws) {
	clock_t clockWorkSpace;
	clockWorkSpace = clock();

	sys::StopWatch watchWorkSpace;
	watchWorkSpace.start();

	typedef dfa::hai::WideningListener<ClpProblem> ClpListener;
	typedef dfa::hai::WideningFixPoint<ClpListener> ClpFP;
	typedef dfa::hai::HalfAbsInt<ClpFP> ClpAI;

	// get the entry
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0);

	// set the cleaner
	addCleaner(clp::CLP_ANALYSIS_FEATURE, new CLPStateCleaner(ws));

	VERBOSE(ws->process()) = logFor(LOG_BB);
	ClpProblem prob(ws->process());

	// initialize state with initial register values
	if(logFor(LOG_CFG))
		log << "FUNCTION " << cfg->label() << io::endl;
	for(int i = 0; i < inits.count(); i++)
		prob.initialize(inits[i].fst, inits[i].snd);

	// support initial state
	dfa::State *istate = dfa::INITIAL_STATE(ws);
	if(istate) {

		// initialize registers
		for(dfa::State::RegIter r(istate); r; r++)
			prob.initialize((*r).fst, (*r).snd);

		// initialize memory
		for(dfa::State::MemIter m(istate); m; m++)
			prob.initialize((*m).address(), (*m).value());

		prob.setInitialState(istate);
	}

	// look for a stack value
	const hard::Register *sp = ws->process()->platform()->getSP();
	if(!sp)
		warn("no stack pointer in the architecture.");
	else {
		Value v = prob.entry().get(Value(REG, sp->platformNumber()));
		if(v.isTop()) {
			bool found = false;
			if(mem) {
				warn("no initial for stack pointer: looking in memory.");
				Address addr;
				const Array< const hard::Bank * > &banks = mem->banks();
				for(int i = 0; i < banks.count(); i++)
					if(banks[i]->isWritable() && (addr.isNull() || banks[i]->address() > addr))
						addr = banks[i]->topAddress();
				if(addr.isNull()) {
					warn("no writable memory: reverting to loader stack address.");
					addr = ws->process()->defaultStack();
				}
				if(!addr.isNull()) {
					warn(_ << "setting stack at " << addr);
					prob.initialize(sp, addr);
					found = true;
				}
			}
			if(!found)
				warn("no value for the initial stack pointer");
		}
	}

	// perform analysius
	ClpListener list(ws, prob);
	ClpFP fp(list);
	ClpAI cai(fp, *ws);
	cai.solve(cfg);

	// the states actually stored in the listener!
	for(CFGCollection::Iter cfg(coll); cfg; cfg++) {
		for(CFG::BlockIter bb = cfg->blocks(); bb; bb++) {
			STATE_IN(bb) = *(list.results[cfg->index()][bb->index()]);
		}
	}

	// process stats
	_nb_inst = prob.get_nb_inst();
	_nb_sem_inst = prob.get_nb_sem_inst();
	_nb_set = prob.get_nb_set();
	_nb_top_set = prob.get_nb_top_set();
	_nb_store = prob.get_nb_store();
	_nb_top_store = prob.get_nb_top_store();
	_nb_top_store_addr = prob.get_nb_top_store_addr();
	_nb_load = prob.get_nb_load();
	_nb_load_top_addr = prob.get_nb_load_top_addr();
	_nb_filters = prob.get_nb_filters();
	_nb_top_filters = prob.get_nb_top_filters();
	_nb_top_load = prob.get_nb_top_load();

	clockWorkSpace = clock() - clockWorkSpace;
	if(verbose)
		elm::cerr << "CLP Analyse takes " << clockWorkSpace << " micro-seconds for processing " << prob.get_nb_clp_bb_count() << " blocks" << io::endl;

//	watchWorkSpace.stop();
//	otawa::ot::time t = watchWorkSpace.delay();
//	elm::cerr << "CLP Analysew takes " << t << " micro-seconds" << io::endl;
}


/**
 * Build the initial configuration of the Analysis fro a property list
 * @param props the property list
 */
void Analysis::configure(const PropList &props) {
	Processor::configure(props);
	for(Identifier<init_t>::Getter init(props, INITIAL); init; init++)
		inits.add(init);
	verbose = VERBOSE(props);
}


/**
 * This features ensure that the clp analysis has been identified.
 *
 * @par Default Processor
 * @li @ref otawa::ClpAnalysis
 *
 * @par Hooked Propertues
 * @li @ref otawa::clp::STATES_IN
 * @li @ref otawa::clp::STATE_OUT
 *
 * @ingroup clp
 */
p::feature CLP_ANALYSIS_FEATURE("otawa::clp::CLP_ANALYSIS_FEATURE", p::make<Analysis>());

/**
 * Put on a basic block, it's the CLP state at the beginning of the block
 * @ingroup clp
*/
Identifier<clp::State> STATE_IN("otawa::clp::STATE_IN");

/**
 * Put on a basic block, it's the CLP state at the end of the block
 * @ingroup clp
*/
Identifier<clp::State> STATE_OUT("otawa::clp::STATE_OUT");

/**
 * How to treat the unknown block: true - generate bottoms value, false - generates top value
 * @ingroup clp
*/
Identifier<bool> UNKOWN_BLOCK_EVALUATION("otawa::clp::UNKOWN_BLOCK_EVALUATION", false);


/**
 * @class ClpStatePack::Context
 * A context allows to share a CLP problem through different constructions
 * of ClpStatePack. ClpStatePack works at the basic block level and,
 * when one has a lot of basic block to process (like in CFG),
 * this Context object allows to factor a part of the initialization.
 * @ingroup clp
 */

/**
 * Buid a ClpPack context.
 * @param process	Analyzed process.
 */
ClpStatePack::Context::Context(Process *process) {
	ASSERT(process);
	prob = new ClpProblem(process);
	to_free = true;
}

/**
 * Build a ClpPacl context from an existing problem.
 * @param problem	The problem.
 */
ClpStatePack::Context::Context(ClpProblem& problem) {
	prob = &problem;
	to_free = false;
}

/**
 */
ClpStatePack::Context::~Context(void) {
	if(to_free)
		delete prob;
}


/**
 * @class ClpStatePack
 *	A ClpStatePack must be constructed after the run of the ClpAnalysis.
 *	This constructor will use the input state of the BasicBlock, and run again
 *	the analysis until the end of the block.
 *
 *	The state for each instruction and semantic instruction will be saved inside
 *	the pack.
 */

/**
 * Constructor of a new ClpStatePack.
 *	@param bb 		BasicBlock to be analysed.
 *	@param process	Current process.
 */
ClpStatePack::ClpStatePack(BasicBlock *bb, Process *process): _bb(bb), _packs(){
	ASSERT(STATE_IN.exists(*bb));
	ClpProblem prob(process);
	prob.fillPack(_bb, this);
}

/**
 * Build a CLP state pack from a context.
 * @param bb		BB to analyze.
 * @param context	Context to use.
 */
ClpStatePack::ClpStatePack(BasicBlock *bb, const Context& context): _bb(bb), _packs() {
	ASSERT(STATE_IN.exists(*bb));
	context.problem().fillPack(_bb, this);
}


/** Destructor for ClpStatePack */
ClpStatePack::~ClpStatePack(void){
	while(!_packs.isEmpty()){
		InstPack *p = _packs.pop();
		delete p;
	}
}
/** Destructor for InstPack */
ClpStatePack::InstPack::~InstPack(void){
	while(!_states.isEmpty()){
		clp::State *st = _states.pop();
		delete st;
	}
}
/** Add a new state at the end of this pack.
*	@param state the state to be added.
*/
void ClpStatePack::InstPack::append(clp::State &state){
	clp::State *st = new clp::State(state);
	_states.add(st);
}
/** @return the CLP state after the given instruction
*	@param instruction is the address of the instruction to get the state of.
*/
clp::State ClpStatePack::state_after(address_t instruction){
	for(PackIterator packs = getIterator(); packs; packs++){
		InstPack *ipack = (*packs);
		if (ipack->inst_addr() == instruction)
			return ipack->outputState();
	}
	return clp::State::EMPTY; // FIXME: we should raise an exception?
}
/** @return the CLP state after the given semantic instruction
*	@param instruction is the address of the instruction where the semantic
*		instruction is.
*	@param sem is the index (starting from 0) of the semantic instruction inside
*		the block corresponding to the machine instruction.
*/
clp::State ClpStatePack::state_after(address_t instruction, int sem){
	for(PackIterator packs = getIterator(); packs; packs++){
		InstPack *ipack = (*packs);
		if (ipack->inst_addr() == instruction)
			return *(ipack->_states[sem]);
	}
	return clp::State::EMPTY; // FIXME: we should raise an exception?
}
/** @return the CLP state before the given instruction
*	@param instruction is the address of the instruction to get the state before.
*/
clp::State ClpStatePack::state_before(address_t instruction){
	clp::State last_state = STATE_IN(_bb);
	for(PackIterator packs = getIterator(); packs; packs++){
		InstPack *ipack = (*packs);
		if (ipack->inst_addr() == instruction)
			return last_state;
		last_state = ipack->outputState();
	}
	return last_state;
}
/** @return the CLP state before the given semantic instruction
*	@param instruction is the address of the instruction where the semantic
*		instruction is.
*	@param sem is the index (starting from 0) of the semantic instruction inside
*		the block corresponding to the machine instruction.
*/
clp::State ClpStatePack::state_before(address_t instruction, int sem){
	clp::State last_state = STATE_IN(_bb);
	for(PackIterator packs = getIterator(); packs; packs++){
		InstPack *ipack = (*packs);
		if (ipack->inst_addr() == instruction){
			if ( sem == 0)
				return last_state;
			else
				return *(ipack->_states[sem - 1]);
		}
		if (! ipack->isEmpty())
			last_state = ipack->outputState();
	}
	return last_state;
}

/** Add a new instruction pack inside the ClpStatePack. */
ClpStatePack::InstPack* ClpStatePack::newPack(address_t inst){
	InstPack *ipack = new InstPack(inst);
	_packs.add(ipack);
	return ipack;
}

} //clp


/**
 * @class DeadCodeAnalysis
 *
 * This analyzer add the NEVER_TAKEN identifer on edges which are never taken.
 *
 * @par Provided Features
 * @li @ref otawa::DEAD_CODE_ANALYSIS_FEATURE
 *
 * @par Required Features
 * @li @ref otawa::CLP_ANALYSIS_FEATURE
 */
DeadCodeAnalysis::DeadCodeAnalysis(void): Processor("otawa::DeadCodeAnalysis", Version(0, 1, 0)) {
	require(clp::CLP_ANALYSIS_FEATURE);
	provide(DEAD_CODE_ANALYSIS_FEATURE);
}

void DeadCodeAnalysis::processWorkSpace(WorkSpace *ws){
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0);
	/* for each bb */
	for(CFG::BlockIter bbi = cfg->blocks(); bbi; bbi++){
		clp::State st = clp::STATE_OUT(*bbi);
		/* if the bb state is None : */
		if (st == clp::State::EMPTY){
			/* mark all (in|out) edges as never taken */
			for(Block::EdgeIter edge = bbi->ins(); edge; edge++)
				NEVER_TAKEN(*edge) = true;
			for(Block::EdgeIter edge = bbi->outs(); edge; edge++)
				NEVER_TAKEN(*edge) = true;
		} else {
			/* mark all (in|out) edges as not never taken */
			for(Block::EdgeIter edge = bbi->ins(); edge; edge++)
				if (!NEVER_TAKEN.exists(*edge))
					NEVER_TAKEN(*edge) = false;
			for(Block::EdgeIter edge = bbi->outs(); edge; edge++)
				if (!NEVER_TAKEN.exists(*edge))
					NEVER_TAKEN(*edge) = false;
		}
	}
}

Feature<DeadCodeAnalysis> DEAD_CODE_ANALYSIS_FEATURE("otawa::DEAD_CODE_ANALYSIS_FEATURE");
Identifier<bool> NEVER_TAKEN("otawa::NEVER_TAKEN");

namespace clp {

/**
 * @class Manager
 * This class allows to exploit the result of a CLP analysis.
 * Basically, it provide facility to traverse a basic block, semantic instruction by semantic instruction
 * and to lookup the state.
 * @ingroup clp
 */


/**
 * Create a manager for the current workspace.
 * @param ws	Workspace to work with.
 */
Manager::Manager(WorkSpace *ws) {
	p = new ClpProblem(ws->process());
	p->setInitialState(dfa::INITIAL_STATE(ws));
}

Manager::~Manager() {
	delete p;
}

/**
 * Start the interpretation of a basic block.
 * @param bb	Basic block to interpret.
 */
Manager::step_t Manager::start(BasicBlock *bb) {
	mi = BasicBlock::BundleIter(bb);
	s = STATE_IN(bb);
	cs = &s;
	p->prepare(*mi);
	i = 0;
	//p->update(cs);
	//return NEW_INST | NEW_PATH | NEW_SEM;
	return next();
}


/**
 * Go to the next step in the interpretation of the basic block.
 * This may causes:
 * @li @ref Manager::NEW_SEM -- just interpreting the next semantic instruction,
 * @li @ref Manager::NEW_PATH -- starting the interpretation of the next semantic path,
 * @li @ref Manager::NEW_INST -- starting the interpretation of the next machine instruction,
 * @li @ref Manager::ENDED -- to have exhausted any interpretation.
 * @return	A OR'ed combination of NEW_SEM, NEW_PATH, NEW_INST, or the value ENDED.
 */
Manager::step_t Manager::next(void) {
	step_t r = NEW_SEM;
	while(p->isPathEnded()) {
		if(cs != &s) {
			p->lub(s, *cs);
			delete cs;
		}
		cs = p->nextPath();
		r |= NEW_PATH;
		if(!cs) {
			r |= NEW_INST;
			mi++;
			if(!mi)
				return false;
			cs = &s;
			p->prepare(mi);
			i = 0;
		}
	}
	i = p->getPC();
	p->update(cs);

	// Merge the state so that it can be used right away.
	// If there are further path to take, the following call of this function will re-assign the cs (current state).
	if(p->isPathEnded()) {
		if(cs != &s) {
			p->lub(s, *cs);
			delete cs;
			cs = &s;
		}
	}
	return r;
}


/**
 * Get the last interpreted semantic instruction.
 * @return	Current semantic instruction.
 */
sem::inst Manager::sem(void) {
	return p->sem(i);
}


/**
 * Get the current interpreted machine instruction.
 * @return	Current machine instruction.
 */
Inst *Manager::inst(void) {
	return *mi;
}


/**
 * Get the state result of the last interpretation.
 * @return	Result state.
 */
State *Manager::state(void) {
	return cs;
}


Value Manager::getCurrentAccessAddress(void) {
	return p->getCurrentAccessAddress();
}

Manager::step_t Manager::rewind(State& rState) {
	s = rState;
	cs = &s;
	p->prepare(mi);
	i = p->getPC();
	step_t r = NEW_SEM | NEW_INST;
	return r;
}

/**
 * @fn int Manager::ipc(void);
 * Get the current semantic instruction PC.
 * @return	Current semantic instruction PC.
 */

/**
 * @fn bool Manager::newSem(step_t s);
 * Test if the given step result contains the flag @ref Manager::NEW_SEM.
 * @return	True if @ref Manager::NEW_SEM is set, false else.
 */

/**
 * @fn bool Manager::newPath(step_t s);
 * Test if the given step result contains the flag @ref Manager::NEW_PATH.
 * @return	True if @ref Manager::NEW_PATH is set, false else.
 */

/**
 * @fn bool Manager::newInst(step_t s);
 * Test if the given step result contains the flag @ref Manager::NEW_INST.
 * @return	True if @ref Manager::NEW_INST is set, false else.
 */

class Plugin: public ProcessorPlugin {
public:
	//typedef Array<AbstractRegistration * > procs_t;

	Plugin(void): ProcessorPlugin("otawa::clp", Version(0, 1, 0), OTAWA_PROC_VERSION) { }
	//virtual procs_t& processors (void) const { return procs_t::EMPTY; };
};


}	// clp

} //otawa


otawa::clp::Plugin otawa_clp_plugin;
ELM_PLUGIN(otawa_clp_plugin, OTAWA_PROC_HOOK);


