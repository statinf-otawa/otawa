/*
 *	Config and ConfigSet class interfaces
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
#ifndef OTAWA_ETIME_CONFIG_H_
#define OTAWA_ETIME_CONFIG_H_

#include <elm/types.h>

namespace otawa { namespace etime {

using namespace elm;

class Config {
public:
	inline Config(void): b(0) { }
	inline Config(t::uint32 bits): b(bits) { }
	inline Config(const Config& conf): b(conf.b) { }
	inline t::uint32 bits(void) const { return b; }
	inline void set(int n) { b |= 1 << n; }
	inline void clear(int n) { b &= ~(1 << n); }
	inline bool bit(int n) const { return b & (1 << n); }
	string toString(int n) const;

private:
	t::uint32 b;
};


class ConfigSet {
public:
	ConfigSet(void): t(0) { }
	ConfigSet(ot::time time): t(time) { }
	ConfigSet(const ConfigSet& set): t(set.t), confs(set.confs) { }
	inline ot::time time(void) const { return t; }
	inline int count(void) const { return confs.length(); }
	inline void add(Config conf) { confs.add(conf); }
	inline void add(const ConfigSet& set) { t = max(t, set.time()); confs.addAll(set.confs); }

	inline void push(const ConfigSet& set) { t = max(t, set.time()); confs.addAll(set.confs); }
	inline void pop(const ConfigSet& set) { for(int i = 0; i < set.confs.length(); i++) confs.pop(); }

	t::uint32 posConst(void) const;
	t::uint32 negConst(void) const;
	t::uint32 unused(t::uint32 neg, t::uint32 pos, int n) const;
	t::uint32 complex(t::uint32 neg, t::uint32 pos, t::uint32 unused, int n) const;
	void scan(t::uint32& pos, t::uint32& neg, t::uint32& unus, t::uint32& comp, int n) const;
	bool isFeasible(int n);
	void dump(io::Output& out, int n);

	class Iter: public Vector<Config>::Iter {
	public:
		inline Iter(const ConfigSet& set): Vector<Config>::Iter(set.confs) { }
	};

private:
	ot::time t;
	Vector<Config> confs;
};


} }		// otawa::etime

#endif /* OTAWA_ETIME_CONFIG_H_ */
