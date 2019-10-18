/*
 *	StepGraphBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2019, IRIT UPS.
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
#ifndef OTAWA_ETIME_STEPGRAPHBUILDER_H_
#define OTAWA_ETIME_STEPGRAPHBUILDER_H_

#include <otawa/etime/features.h>
#include <otawa/etime/AbstractTimeBuilder.h>

namespace otawa { namespace etime {

class StepGraphBuilder: public XGraphBuilder {
public:
	StepGraphBuilder(Monitor& mon);
	~StepGraphBuilder();
	ParExeGraph *build(ParExeSequence *seq);

private:

	void reset();
	void nextNode(ParExeInst *inst, const hard::PipelineUnit *u);
	void managePipelineOrder();
	void manageFetchOrder();
	void manageInOrder();
	void manageBranch();
	void manageRead(const hard::Register *r);
	void manageWrite(const hard::Register *r);
	void manageLoadIssue(bool cached);
	void manageStoreIssue(bool cached);
	void manageLoadWait();
	void manageStoreWait();
	void manageResourceUse(const hard::Queue *q);
	void manageResourceRelease(const hard::Queue *q);

	inline string comment(string com)
		{ if(isExplicit()) return com; else return ""; }

	class CircularQueue {
	public:
		inline CircularQueue() { }
		CircularQueue(const CircularQueue& q);
		~CircularQueue() { if(ns != nullptr) delete [] ns; };
		void configure(int size);
		void reset();
		inline void put(ParExeNode *n)
			{ i++; if(i >= s) i = 0; ns[i] = n; }
		inline ParExeNode *get() const { return ns[i]; }
		inline ParExeNode *last() const { int j = i + 1; if (j >= s) j = 0; return ns[j]; }
	private:
		int i = 0, s = 0;
		ParExeNode **ns = nullptr;
	};

	class Stage: public CircularQueue {
	public:
		Stage();
		Stage(const Stage& q);
		void configure(ParExeStage *s);
		inline ParExeStage *stage() const { return st; }
	private:
		ParExeStage *st = nullptr;
	};

	class ProcQueue: public CircularQueue {
	public:
		ProcQueue();
		ProcQueue(const ProcQueue& q);
		void configure(ParExeQueue *q);
		inline ParExeQueue *queue() const { return pq; }
	private:
		ParExeQueue *pq = nullptr;
	};

	WorkSpace *_ws = nullptr;
	int _icache_shift = 0;
	ParExeProc *_proc = nullptr;

	ParExeGraph *graph = nullptr;
	ParExeNode
		*last_node = nullptr,
		*last_branch = nullptr,
		*first_in_line = nullptr,
		*prev = nullptr,
		*node = nullptr,
		*prev_load = nullptr,
		*prev_store = nullptr;
	Stage *stage = nullptr;

	Vector<Stage> stages;
	Vector<ProcQueue> queues;
	Vector<ParExeNode *> prods;
	Vector<RegResource *> rres;
};

} }		// otawa::etime

#endif /* OTAWA_ETIME_STEPGRAPHBUILDER_H_ */
