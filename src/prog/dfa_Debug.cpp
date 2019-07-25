/*
 *	dfa.Debug class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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

#include <otawa/cfg/features.h>
#include <otawa/cfg/Edge.h>
#include <otawa/dfa/Debug.h>

namespace otawa { namespace dfa {

static string id(CFG *cfg, Block *bb) {
	return _ << cfg->index() << "-" << bb->index();
}

/**
 * @class Debug
 * This class is an helper allowing to output debugging information
 * for data flow analysis in a JSON format for subsequent exploration
 * of the computed states.
 *
 * The format is the following:
 *
 * For each produced state, an addState() call must be done. The returned JSON
 * saver allow adding a structured data to the current JSON file (a single string
 * or number, an object or an array). In addition, calls to addEvent() allows
 * to record in the current specific event keywords produced by the analysis
 * for easier retrieval at exploration time. Typical use of these events
 * is to mark error states, unusual operations or negative cases for the analysis.
 * At exploration time, these events allows to retrieve states where they happen
 * and to understand more easily the causes of the problem.
 */

/**
 * Build the debug handle.
 * @param ws	Current workspace.
 * @param path	Path to save the debug log.
 */
Debug::Debug(WorkSpace *ws, sys::Path path): saver(path), in_state(false), failed(false) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERTP(coll, "no CFG collection available!");
	saver.setReadable(true);

	// output the graph form
	saver.beginObject();
	saver.addField("graph");
	saver.beginArray();
	for(CFGCollection::Iter cfg(coll); cfg(); cfg++)
		for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {
			saver.beginObject();
			saver.addField("id");
			saver.put(id(*cfg, *bb));
			saver.addField("text");
			saver.put(_ << *cfg << " / " << *bb);
			saver.addField("to");
			saver.beginArray();
			cerr << "DEBUG: " << *bb << io::endl;
			if(bb->isExit())
				for(auto call: bb->cfg()->callers()) {
					ASSERT(call->outs());
					cerr << "DEBUG: exit found!\n";
					saver.put(id(call->cfg(), call->outs()->sink()));
					cerr << "DEBUG: call from " << *call << io::endl;
				}
			else if(bb->isBasic())
				for(Block::EdgeIter e = bb->outs(); e(); e++) {
					if(!e->sink()->isSynth())
						saver.put(id(*cfg, e->target()));
					else {
						SynthBlock *sb = e->sink()->toSynth();
						saver.put(id(sb->callee(), sb->callee()->entry()));
					}
				}
			saver.endArray();
			saver.endObject();
		}
	saver.endArray();

	// start state list
	saver.addField("states");
	saver.beginArray();
}


/**
 */
Debug::~Debug(void) {
	if(in_state)
		complete();
	saver.endArray();
	saver.endObject();
}


/**
 * Complete the current state.
 */
void Debug::complete(void) {
	saver.addField("event");
	saver.put(events.toString());
	events.reset();
	saver.endObject();
}


/**
 * Add a new state to the list.
 * @param bb	Basic block where the state is made.
 * @return		JSON saver to save the state.
 */
json::Saver& Debug::addState(Block *bb) {
	if(in_state) {
		complete();
		cerr << "DEBUG: complete state!\n";
	}
	cerr << "DEBUG: add state!\n";
	saver.beginObject();
	saver.addField("at");
	if(bb)
		saver.put(id(bb->cfg(),bb));
	else
		saver.put("");
	saver.addField("content");
	in_state = true;
	return saver;
}


/**
 * Add an event in the current state.
 * @param event	Added event.
 */
void Debug::addEvent(string event) {
	events << ' ' << event;
}

} } // otawa::dfa



