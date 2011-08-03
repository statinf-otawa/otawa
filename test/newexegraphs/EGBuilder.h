/*
 *	$Id$
 *	Interface to the EGBuilder class.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#ifndef _EGBUILDER_H
#define _EGBUILDER_H_

#include "ExecutionGraph.h"
#include "EGBlockSeq.h"

namespace otawa { namespace exegraph2 {


class EGBuilder {
protected:
	EGNodeFactory * _node_factory;
	EGProc *_proc;

public:
	EGBuilder(WorkSpace * ws,
			EGNodeFactory *node_factory=NULL);
	virtual void build(ExecutionGraph *graph) = 0;
};

class EGGenericBuilder : public EGBuilder{
private:
	WorkSpace * _ws;
	uint32_t _branch_penalty;
	uint32_t _cache_line_size;
	typedef struct rename_table_t {
		otawa::hard::RegBank * reg_bank;
		elm::genstruct::AllocatedTable<EGNode *> *table;
	} rename_table_t;
	EGInstSeq *_inst_seq;
	ExecutionGraph *_graph;

public:
	EGGenericBuilder(WorkSpace * ws, EGNodeFactory *node_factory);

	void build(ExecutionGraph *graph);
	void createNodes();
	void findDataDependencies();
	void addEdgesForPipelineOrder();
	void addEdgesForFetch();
	void addEdgesForFetchWithDecomp();
	void addEdgesForProgramOrder(elm::genstruct::SLList<EGStage *> *list_of_stages = NULL);
	void addEdgesForMemoryOrder();
	void addEdgesForDataDependencies();
	void addEdgesForQueues();
};


} // namespace exegraph2
} // namespace otawa
#endif // _EGBUILDER_H_

