/*
 *	LoopIdentifier class interface
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
#ifndef OTAWA_SGRAPH_LOOPINDENTIFIER_H_
#define OTAWA_SGRAPH_LOOPINDENTIFIER_H_

#include "../graph/DiGraph.h"

namespace otawa { namespace graph {

class LoopIdentifier {
	typedef t::uint8 flags_t;
	static const flags_t
		HEADER		= 0x01,
		REENTRY		= 0X02,
		TRAVERSED	= 0x04,
		IRREDUCIBLE	= 0x08;

public:
	LoopIdentifier(const DiGraph& graph, Vertex *entry);
	inline bool isHeader(Vertex *v) const { return _flags[v->index()] & HEADER; }
	inline bool isReentry(Vertex *v) const { return _flags[v->index()] & REENTRY; }
	inline bool isIrreducible(void) const { return _irred; }
	inline bool isIrreducible(Vertex *v) const { return _flags[v->index()] & IRREDUCIBLE; }
	inline Vertex *immediateLoop(Vertex *v) const { return _iloop[v->index()]; }
	bool contains(Vertex *h, Vertex *v) const;
	bool isBack(Edge *e) const;
	bool isEntry(Edge *e) const;
	inline Vertex *loopOf(Vertex *v) const
		{ if(isHeader(v)) return v; else return iloop(v); }

private:
	inline bool isTraversed(Vertex *v) const { return _flags[v->index()] & TRAVERSED; }
	inline t::uint32 DFSP(Vertex *v) const { return _DFSP[v->index()]; }
	inline Vertex *iloop(Vertex *v) const { return _iloop[v->index()]; }

	inline void setHeader(Vertex *v) { _flags[v->index()] |= HEADER; }
	inline void setReentry(Vertex *v) { _flags[v->index()] |= REENTRY; }
	inline void setTraversed(Vertex *v) { _flags[v->index()] |= TRAVERSED; }
	inline void setIrreducible(Vertex *v)
		{ _flags[v->index()] |= IRREDUCIBLE; _irred = true; }
	inline void setDFSP(Vertex *v, t::uint32 nv) { _DFSP[v->index()] = nv; }
	inline void setILoop(Vertex *v, Vertex *h) { _iloop[v->index()] = h; }

	Vertex *DFS(Vertex *v, int p);
	void tagHead(Vertex *v, Vertex *nh);

	const DiGraph& g;
	flags_t *_flags;
	t::uint32 *_DFSP;
	Vertex **_iloop;
	bool _irred;
	Vertex *_entry;
};

} }	// otawa::sgraph

#endif /* OTAWA_SGRAPH_LOOPINDENTIFIER_H_ */
