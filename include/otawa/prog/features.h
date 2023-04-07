/*
 *	$Id$
 *	Features for the prog module.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_PROG_FEATURES_H_
#define OTAWA_PROG_FEATURES_H_

#include <otawa/proc/AbstractFeature.h>
#include <otawa/prog/sem.h>

namespace otawa {

class Inst;
	
namespace hard { class Register; }

// delayed_t type
typedef enum delayed_t {
	DELAYED_None = 0,
	DELAYED_Always = 1,
	DELAYED_Taken = 2
} delayed_t;

// DelayedInfo class
class DelayedInfo {
public:
	virtual ~DelayedInfo(void);
	virtual delayed_t type(Inst *inst) = 0;
	virtual int count(Inst *inst);
};
extern Identifier<DelayedInfo *> DELAYED_INFO;


// Conditional type
class Condition {
public:
	typedef t::uint8 cond_t;
	static const cond_t
		EQ  = 0b001,
		LT  = 0b010,
		GT  = 0b100,
		ANY = 0b111;

	Condition();
	Condition(sem::cond_t cond, hard::Register *reg);
	Condition(bool unsigned_, cond_t cond, hard::Register *reg);

	inline bool isEmpty() const { return _cond == 0; }
	inline bool isAny() const { return _cond == ANY; }
	inline bool isUnsigned() const { return _unsigned; }
	inline bool isSigned() const { return !_unsigned; }
	inline cond_t cond() const { return _cond; }
	inline hard::Register *reg() const { return _reg; }
	sem::cond_t semCond() const;
	bool equals(const Condition& c) const;
	bool subsetOf(const Condition& c) const;

	Condition complementOf(const Condition& c) const;
	Condition inverse() const;
	Condition meet(const Condition& c) const;
	Condition join(const Condition& c) const;

	inline Condition operator~(void) const { return inverse(); }
	inline Condition operator&(const Condition& c) const { return meet(c); }
	inline Condition operator|(const Condition& c) const { return join(c); }
	inline Condition operator-(const Condition& c) const { return complementOf(c); }
	inline bool operator==(const Condition& c) const { return equals(c); }
	inline bool operator!=(const Condition& c) const { return !equals(c); }
	inline bool operator<=(const Condition& c) const { return subsetOf(c); }
	inline bool operator<(const Condition& c) const { return subsetOf(c) && !equals(c); }
	inline bool operator>=(const Condition& c) const { return c.subsetOf(*this); }
	inline bool operator>(const Condition& c) const { return c.subsetOf(*this) && !equals(c); }

private:
	bool _unsigned;
	cond_t _cond;
	hard::Register *_reg;
};
io::Output& operator<<(io::Output& out, const Condition& c);

// symbols and labels
class Symbol;
class LabelInfo {
public:
	~LabelInfo();
	virtual string labelFor(Inst *inst) = 0;
	virtual Address addressOf(string name) = 0;
};
extern p::interfaced_feature<LabelInfo> LABEL_FEATURE;
extern p::id<Symbol *> SYMBOL;
extern p::id<string> LABEL;
extern p::id<string> FUNCTION_LABEL;

// Process information
extern p::id<Address> ARGV_ADDRESS;
extern p::id<Address> ENVP_ADDRESS;
extern p::id<Address> AUXV_ADDRESS;
extern p::id<Address> SP_ADDRESS;
extern p::id<delayed_t> DELAYED;

// ABI featues
extern p::feature CONTROL_DECODING_FEATURE;
extern p::feature DELAYED_FEATURE;
extern p::feature DELAYED2_FEATURE;
extern p::feature FLOAT_MEMORY_ACCESS_FEATURE;
extern p::feature MEMORY_ACCESS_FEATURE;
extern p::feature MEMORY_ACCESSES;
extern p::feature REGISTER_USAGE_FEATURE;
extern p::feature SEMANTICS_INFO_EXTENDED;
extern p::feature SEMANTICS_INFO_FLOAT;
extern p::feature SEMANTICS_INFO;
extern p::feature SOURCE_LINE_FEATURE;
extern p::feature VLIW_SUPPORTED;
extern p::feature CONDITIONAL_INSTRUCTIONS_FEATURE;

// task identification
class TaskInfo {
public:
	virtual ~TaskInfo();
	virtual Path workDirectory() = 0;
	virtual string entryName() = 0;
	virtual Inst *entryInst() = 0;
};

extern p::id<string> TASK_ENTRY;
extern p::id<Address> TASK_ADDRESS;
extern p::interfaced_feature<TaskInfo> TASK_INFO_FEATURE;

} // otawa

#endif /* OTAWA_PROG_FEATURES_H_ */
