/*
 *	$Id$
 *	exegraph module interface
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
#ifndef _LI_EXEGRAPH_H_
#define _LI_EXEGRAPH_H_

#include "ExeGraph.h"

namespace otawa { 
	
#define INFINITE_TIME 0x0FFFFFF

class LiExeGraph;
	
class LiExeNode : public ExeGraph<LiExeNode>::ExeNode {
	private:
		int _time[ExeGraph<LiExeNode>::STEPS][ExeGraph<LiExeNode>::BOUNDS];
		bool _shaded;
		LiExeGraph * _graph;
	public:
  		inline  LiExeNode(ExeGraph<LiExeNode> *graph, PipelineStage<LiExeNode> *stage, ExeInst<LiExeNode> *inst) ;
		inline int time(ExeGraph<LiExeNode>::time_step_t step, ExeGraph<LiExeNode>::time_bound_t bound)
			{return _time[step][bound];}
		inline void setTime(ExeGraph<LiExeNode>::time_step_t step, ExeGraph<LiExeNode>::time_bound_t bound, int time);
		inline void shade(void)
			{_shaded = true;}
		inline bool isShaded(void)
			{return _shaded;}		
};

class LiExeGraph : public ExeGraph<LiExeNode> {
	public:
		typedef class LiExeNode Node;
	private:
		bool _times_changed;	
		void latestTimes();
		void earliestTimes();
		bool maxTimeToNode(LiExeNode *source, LiExeNode *target, int *time);			
		void shadeNodes(LiExeNode * node);
		void nodeLatestTimes(LiExeNode *node);
		void nodeEarliestTimes(LiExeNode *node);
		bool separated(LiExeNode *u, LiExeNode *v);		
		inline elm::String timeName(int time);
	public:
		inline LiExeGraph(WorkSpace *fw,  Microprocessor<LiExeNode> * microprocessor, ExeSequence<LiExeNode> *sequence)	
			: ExeGraph<LiExeNode>::ExeGraph() {
			build(fw, microprocessor, sequence);
		}
		int analyze();
		inline void setTimesChanged() 
			{_times_changed = true;}
		void dump(elm::io::Output& dotFile);
};

inline  LiExeNode::LiExeNode(ExeGraph<LiExeNode> *graph, PipelineStage<LiExeNode> *stage, ExeInst<LiExeNode> *inst) 
	: ExeNode(graph, stage, inst), _shaded(false), _graph((LiExeGraph *)graph)  {
	 if (inst->codePart() >= ExeGraph<LiExeNode>::BODY) {
		setTime(ExeGraph<LiExeNode>::READY,ExeGraph<LiExeNode>::MIN,0);
		setTime(ExeGraph<LiExeNode>::START,ExeGraph<LiExeNode>::MIN,0);
		setTime(ExeGraph<LiExeNode>::FINISH,ExeGraph<LiExeNode>::MIN,stage->minLatency());
	}
	else {
		setTime(ExeGraph<LiExeNode>::READY,ExeGraph<LiExeNode>::MIN,-INFINITE_TIME);
		setTime(ExeGraph<LiExeNode>::START,ExeGraph<LiExeNode>::MIN,-INFINITE_TIME);
		setTime(ExeGraph<LiExeNode>::FINISH,ExeGraph<LiExeNode>::MIN,-INFINITE_TIME);
	}
	setTime(ExeGraph<LiExeNode>::READY,ExeGraph<LiExeNode>::MAX,0);
	setTime(ExeGraph<LiExeNode>::START,ExeGraph<LiExeNode>::MAX, INFINITE_TIME);
	setTime(ExeGraph<LiExeNode>::FINISH,ExeGraph<LiExeNode>::MAX,INFINITE_TIME);
}

inline void LiExeNode::setTime(ExeGraph<LiExeNode>::time_step_t step, ExeGraph<LiExeNode>::time_bound_t bound, int time) {
//	elm::cout << "Setting time for node " << name() << " to " << time << " \n";
	if (_time[step][bound] != time) {
		_graph->setTimesChanged();
		_time[step][bound] = time;
	}
}

#define EPSILON 10
inline elm::String LiExeGraph::timeName(int time) {
	StringBuffer _buffer;
	if (time < -INFINITE_TIME+EPSILON)	
		_buffer << "-INF";
	else {
		if (time > INFINITE_TIME-EPSILON)
			_buffer << "-INF";
		else
			_buffer << time;
   }
   return _buffer.toString();
}
 

}; // namespace otawa

#endif //_LI_EXEGRAPH_H_
