/*
 *	$Id$
 *	Interface to the EGBBTime classes.
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

#ifndef _EG_BBTIME_H_
#define _EG_BBTIME_H_

#include <otawa/prop/Identifier.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include "ExecutionGraph.h"
#include <elm/io/OutFileStream.h>
#include <otawa/proc/BBProcessor.h>
#include "EGBlockSeq.h"
#include "EGSolver.h"
#include "EGBuilder.h"
#include "EGDisplay.h"

namespace otawa { namespace exegraph2 {
    extern Identifier<String> GRAPHS_DIR;
 //   extern Identifier<int> TIME;

    using namespace elm::genstruct;

    class EGBBTime;

    class EGBBTimeConfig {
    	friend class EGBBTime;
    protected:
    	EGBlockSeqListFactory *	_block_seq_list_factory;
    	EGBuilder *_builder;
    	EGSolver *_solver;
    public:
    	EGBBTimeConfig()
    	: _block_seq_list_factory(NULL), _builder(NULL), _solver(NULL){}
    };

    // -- class EGBBTime ----------------------------------------------------------------------------------

    class EGBBTime: public BBProcessor {
    private:
    	EGBBTimeConfig *_config;
		WorkSpace *_ws;
		EGProc *_proc;
		PropList _props;
		int _prologue_depth;
		OutStream *_output_stream;
		elm::io::Output *_output;
		String _graphs_dir_name;
		bool _do_output_graphs;
		EGBlockSeqListFactory * _block_seq_list_factory;
		EGBuilder * _builder;
		EGSolver * _solver;

    public:
		EGBBTime(EGBBTimeConfig *config=NULL, const PropList& props = PropList::EMPTY);
		EGBBTime(const PropList& props = PropList::EMPTY);
		EGBBTime(AbstractRegistration& reg);
		~EGBBTime();
		virtual void configure(const PropList& props);

		void processWorkSpace(WorkSpace *ws);
		void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
		void analyzeBlockSequence(EGBlockSeq *seq, int context_index);

    };


} // namespace exegraph2
} // namespace otawa

#endif // _EG_BBTIME_H_




