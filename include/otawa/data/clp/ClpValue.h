/*
 *	$Id$
 *	CLP Value definition
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#ifndef OTAWA_DATA_CLP_VALUE_H_
#define OTAWA_DATA_CLP_VALUE_H_

#include <elm/types.h>
#include <elm/type_info.h>
#include <elm/io.h>

namespace otawa { namespace clp {

using namespace elm;

// Here we define the number of bits the CLP will be (here modulo 2^32)
const int NBITS = 32;
typedef elm::t::int32 intn_t;
typedef elm::t::uint32 uintn_t;
typedef t::uint64 STAT_UINT;
const uintn_t UMAXn = elm::type_info<uintn_t>::max;
const intn_t MAXn = elm::type_info<intn_t>::max;
const intn_t MINn = elm::type_info<intn_t>::min;
	
	
/**
 * Allowed types for values:
 * NONE represents nothing;
 * REG is only used for addresses, and represents a register;
 * VAL represents some values (either a constant or an interval);
 * ALL is the Top element.
 */
typedef enum {
	NONE,
	REG,	// only used for addresses
	VAL,
	ALL
} kind_t;
	
	/**
	 * A set of values represented by a Circular Linear Progression.
	 * Values are defined by a 3-tuple (lower, delta, mtimes) for 32bits numbers.
	 * A constant integer k is represented by (k, 0, 0)
	 * The Top element is represented by (0, 1, 2^32 - 1)
	 */
	class Value {
	public:
		static int and_threshold;

		inline bool isConst(void) const { return delta() == 0 || mtimes() == 0; }

		/** Constructor for a new CLP
		*	@param kind the kind of the CLP
		*	@param lower the lower bound
		*	@param delta the delta
		*	@param mtimes the number of time the delta is applied
		*/
		inline Value(kind_t kind=VAL, intn_t lower=0, intn_t delta=0,
			uintn_t mtimes=0): _kind(kind), _lower(lower), _delta(delta),
			_mtimes(mtimes) { }
		/** Copy constructor */
		inline Value(const Value& val):
			_kind(val._kind), _lower(val._lower), _delta(val._delta),
			_mtimes(val._mtimes) { }
		/** Singleton constructor
		*	@param val the single integer element of the CLP
		*	@return the CLP (val, 0, 0)
		*/
		inline Value(const int val):
			_kind(VAL), _lower(val), _delta(0), _mtimes(0) {}
		
		inline Value& operator=(const Value& val){
			_kind = val._kind;
			_lower = val._lower;
			_delta = val._delta;
			_mtimes = val._mtimes;
			return *this;
		}
		
		inline bool isTop(void) const { return _kind == ALL; }

		inline bool operator==(const Value& val) const {
			return (
						(_kind == val._kind &&
						 _lower == val._lower &&
						 _delta == val._delta &&
						 _mtimes == val._mtimes)
						 /* WARNING: keep in order to verify it does not break something else
						  ||
						( (_delta == 1 || _delta == -1) &&
						  (val._delta == 1 || val._delta == -1) &&
						 _mtimes == UMAXn && val._mtimes == UMAXn)*/
					);
		}
		inline bool operator!=(const Value& val) const {
			return ! operator==(val);
		}
		
		Value operator+(const Value& val) const;
		Value operator-(const Value& val) const;
		inline bool operator>=(const int val) const {
			return _lower >= val;
		}
		
		inline kind_t kind(void) const { return _kind; }
		inline intn_t lower(void) const { return _lower; }
		inline intn_t upper(void) const { return _lower + _delta * _mtimes; }
		inline intn_t delta(void) const { return _delta; }
		inline uintn_t mtimes(void) const { return _mtimes; }
		
		/** @return the "start" of the CLP, i.e. the lower bound if delta >= 0,
		*	lower + delta * mtimes else.
		*/
		inline intn_t start(void) const {
			if (_delta < 0)
				return _lower + _delta * _mtimes;
			else
				return _lower;
		}

		/** @return the "stop" of the CLP, i.e. the upper bound if delta >= 0,
		*	lower else.
		*/
		inline intn_t stop(void) const {
			if (_delta < 0)
				return _lower;
			else
				return _lower + _delta * _mtimes;
		}
		
		/**
		 * Add another set to the current one
		 * @param val the value to add
		 */
		void add(const Value& val);
		/**
		 * Subtract anothe set to the current one
		 * @param val the value to subtract
		 */
		void sub(const Value& val);
		/**
		 * Print a human representation of the CLP
		 * @param out the stream to output the representation
		 */
		void print(io::Output& out) const;
		/**
		 * Left shift the current value.
		 * @param val the value to shift the current one with. Must be a
		 *            positive constant.
		*/
		void shl(const Value& val);
		/**
		 * Right shift the current value.
		 * @param val the value to shift the current one with. Must be a
		 *            positive constant.
		*/
		void shr(const Value& val);

		void _or(const Value& val);

		/**
		 * Join another set to the current one
		 * @param val the value to be joined with
		 */
		void join(const Value& val);
		/**
		 * Perform a widening to the infinite (to be filtred later)
		 * @param val the value of the next iteration state
		*/
		void widening(const Value& val);
		/**
		 * Perform a widening, knowing flow facts for the loop
		 * @param val the value of the next iteration state
		 * @param loopBound the maximum number of iteration of the loop
		*/
		void ffwidening(const Value& val, int loopBound);
		/**
		 * Intersection with the current value.
		 * @param val the value to do the intersection with
		 */
		void inter(const Value& val);
		/**
		 * Reverse the CLP direction (swap upper and lower bounds, and use
		 * the opposite of delta as new delta).
		*/
		void reverse(void);

		inline bool swrap(void) const
			{ return _delta != 0 && _mtimes > (MAXn - _lower) / elm::abs(_delta); }
		inline bool uwrap(void) const
			{ return _delta != 0 && _mtimes > (UMAXn - _lower) / elm::abs(_delta); }
		
		void ge(intn_t k);
		void geu(uintn_t k);
		void le(intn_t k);
		void leu(uintn_t k);
		void eq(uintn_t k);
		void ne(uintn_t k);
		void _and(const Value& val);

		/** Represents the bottom element */
		static const Value none;
		/** Represents the top element */
		static const Value all;
		
		/** 
		 * Set the values for the current object
		 * @param kind the kind of the object
		 * @param lower the lower bound of the CLP
		 * @param delta the step of the CLP
		 * @param mtimes the number of times delta need to be added to get the
		 *				 max bound of the CLP
		 */
		inline void set(kind_t kind, intn_t lower, intn_t delta,
						uintn_t mtimes){
			_kind = kind;
			_lower = lower;
			_delta = delta;
			_mtimes = mtimes;
		}
	private:
		kind_t _kind;
		intn_t _lower;
		intn_t _delta;
		uintn_t _mtimes;
	};

	inline elm::io::Output& operator<<(elm::io::Output& out, const otawa::clp::Value &val)
		{ val.print(out); return out; }
	
}	// clp

}	// otawa

#endif /* OTAWA_DATA_CLP_VALUE_H_ */
