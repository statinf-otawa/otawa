/*
 *	StandardXGraphBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2020, IRIT UPS.
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

#ifndef OTAWA_ETIME_STANDARDXGRAPHBUILDER_H_
#define OTAWA_ETIME_STANDARDXGRAPHBUILDER_H_
#include <otawa/etime/AbstractTimeBuilder.h>

namespace otawa { namespace etime {

class StandardXGraphBuilder: public XGraphBuilder {
public:
	StandardXGraphBuilder(Monitor& mon);
	ParExeGraph *build(ParExeSequence *seq) override;

protected:
	inline string comment(string com) {
		if(isExplicit())
			return com;
		else
			return "";
	}

	virtual void createNodes(ParExeGraph *g, ParExeSequence *seq);
	virtual void addEdgesForPipelineOrder(ParExeGraph *g, ParExeSequence *seq);
	virtual void addEdgesForFetch();
	virtual void addEdgesForProgramOrder();
	virtual void addEdgesForMemoryOrder();
	virtual void addEdgesForDataDependencies();
	virtual void addEdgesForQueues();
	virtual void init();
	virtual ParExeNode *makeNode(ParExeGraph *g, ParExeInst *i, ParExeStage *stage);
	virtual ParExeEdge *makeEdge(ParExeNode *src, ParExeNode *snk, ParExeEdge::edge_type_t_t type = ParExeEdge::SOLID, int latency = 0, string name = "");

	WorkSpace *_ws = nullptr;
	int _cache_line_size = 0;
	ParExeProc *_proc = nullptr;
	ParExeStage *fetch_stage = nullptr;
	ParExeStage *branch_stage = nullptr;
};

} }	// otawa::etime

#endif  /* OTAWA_ETIME_STANDARDXGRAPHBUILDER_H_ */
