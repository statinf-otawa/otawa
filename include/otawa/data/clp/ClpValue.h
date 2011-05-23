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

#include <limits>

namespace otawa {

namespace clp {
	// Here we define the number of bits the CLP will be (here modulo 2^32)
	#define OCLP_NBITS 32
	#define OCLP_intn_t int32_t
	#define OCLP_uintn_t uint32_t
	#define OCLP_UMAXn std::numeric_limits<OCLP_uintn_t>::max()
	#define OCLP_MAXn std::numeric_limits<OCLP_intn_t>::max()
	#define OCLP_MINn std::numeric_limits<OCLP_intn_t>::min()
	
	// This macro allow to make asserts over clp that must be constants
	#define OCLP_IS_CST(clp) (clp.delta() == 0 || clp.mtimes() == 0)
	
	#define OCLP_STAT_UINT unsigned long long int
	
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
		/** Constructors **/
		inline Value(kind_t kind=VAL, OCLP_intn_t lower=0, OCLP_intn_t delta=0,
			OCLP_uintn_t mtimes=0): _kind(kind), _lower(lower), _delta(delta),
			_mtimes(mtimes) { }
		inline Value(const Value& val):
			_kind(val._kind), _lower(val._lower), _delta(val._delta),
			_mtimes(val._mtimes) { }
		inline Value(const int val):
			_kind(VAL), _lower(val), _delta(0), _mtimes(0) {}
		
		/** Operators **/
		
		inline Value& operator=(const Value& val){
			_kind = val._kind;
			_lower = val._lower;
			_delta = val._delta;
			_mtimes = val._mtimes;
			return *this;
		}
		
		inline bool operator==(const Value& val) const {
			return (
						(_kind == val._kind &&
						 _lower == val._lower &&
						 _delta == val._delta &&
						 _mtimes == val._mtimes) ||
						( (_delta == 1 || _delta == -1) &&
						  (val._delta == 1 || val._delta == -1) &&
						 _mtimes == OCLP_UMAXn && val._mtimes == OCLP_UMAXn)
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
		
		/** Accessors **/
		
		inline kind_t kind(void) const { return _kind; }
		inline OCLP_intn_t lower(void) const { return _lower; }
		inline OCLP_intn_t delta(void) const { return _delta; }
		inline OCLP_uintn_t mtimes(void) const { return _mtimes; }
		
		inline OCLP_intn_t start(void) const {
			if (_delta < 0)
				return _lower + _delta * _mtimes;
			else
				return _lower;
		}
		inline OCLP_intn_t stop(void) const {
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
		/**
		 * Join another set to the current one
		 * @param val the value to be joined with
		 */
		void join(const Value& val);
		/**
		 * Perform a widening
		 * @param val the value of the next iteration state
		*/
		void widening(const Value& val);
		/**
		 * Intersection with the current value.
		 * @param val the value to do the intersection with
		 */
		void inter(const Value& val);
	static const Value none, all;
	
		/** 
		 * Set the values for the current object
		 * @param kind the kind of the object
		 * @param lower the lower bound of the CLP
		 * @param delta the step of the CLP
		 * @param mtimes the number of times delta need to be added to get the
		 *				 max bound of the CLP
		 */
		inline void set(kind_t kind, OCLP_intn_t lower, OCLP_intn_t delta,
						OCLP_uintn_t mtimes){
			_kind = kind;
			_lower = lower;
			_delta = delta;
			_mtimes = mtimes;
		}
	private:
		kind_t _kind;
		OCLP_intn_t _lower;
		OCLP_intn_t _delta;
		OCLP_uintn_t _mtimes;
	};
	
	// output
	inline elm::io::Output& operator<<(elm::io::Output& out, Value &val) {
		val.print(out);
		return out;
	}
	
	
}	// clp

}	// otawa

#endif /* OTAWA_DATA_CLP_VALUE_H_ */
