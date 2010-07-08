/*
 *	$Id$
 *	ForwardCFGAdapter and BackwardCFGAdapter classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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
#ifndef OTAWA_CFG_CFGADAPTER_H_
#define OTAWA_CFG_CFGADAPTER_H_

#include <otawa/cfg.h>

namespace otawa {

class ForwardCFGAdapter {
public:

	typedef BasicBlock *Vertex;

	inline ForwardCFGAdapter(CFG *_cfg): cfg(_cfg) { }
	inline int count(void) const { return cfg->countBB(); }
	inline Vertex entry(void) const { return cfg->entry(); }
	inline int index(Vertex v) const { return v->number(); }

	class Predecessor: public PreIterator<Predecessor, Vertex> {
	public:
		inline Predecessor(const ForwardCFGAdapter& g, const Vertex& v): iter(v) { }
		inline bool ended (void) const { return iter.ended(); }
		const Vertex item (void) const { return iter->source(); }
		void next(void) { iter++; }
	private:
		BasicBlock::InIterator iter;
	};

	class Successor: public PreIterator<Successor, Vertex> {
	public:
		inline Successor(const ForwardCFGAdapter& g, const Vertex& v): iter(v) { step(); }
		inline bool ended (void) const { return iter.ended(); }
		const Vertex item (void) const { return iter->target(); }
		void next(void) { iter++; step(); }
	private:
		inline void step(void) {
			while(iter && iter->kind() == Edge::CALL)
				iter++;
		}
		BasicBlock::OutIterator iter;
	};

private:
	CFG *cfg;
};


class BackwardCFGAdapter {
public:

	typedef BasicBlock *Vertex;

	inline BackwardCFGAdapter(CFG *_cfg): cfg(_cfg) { }
	inline int count(void) const { return cfg->countBB(); }
	inline Vertex entry(void) const { return cfg->exit(); }
	inline int index(Vertex v) const { return v->number(); }

	class Successor: public PreIterator<Successor, Vertex> {
	public:
		inline Successor(const BackwardCFGAdapter& g, const Vertex& v): iter(v) { }
		inline bool ended (void) const { return iter.ended(); }
		const Vertex item (void) const { return iter->source(); }
		void next(void) { iter++; }
	private:
		BasicBlock::InIterator iter;
	};

	class Predecessor: public PreIterator<Predecessor, Vertex> {
	public:
		inline Predecessor(const BackwardCFGAdapter& g, const Vertex& v): iter(v) { step(); }
		inline bool ended (void) const { return iter.ended(); }
		const Vertex item (void) const { return iter->target(); }
		void next(void) { iter++; step(); }
	private:
		inline void step(void) {
			while(iter && iter->kind() == Edge::CALL)
				iter++;
		}
		BasicBlock::OutIterator iter;
	};

private:
	CFG *cfg;
};

}	//otawa


#endif /* OTAWA_CFG_CFGADAPTER_H_ */
