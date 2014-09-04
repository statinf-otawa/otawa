/*
 *	ostat command
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
 
#include <otawa/otawa.h>
#include <otawa/app/Application.h>
#include <otawa/cfg.h>
#include <elm/options.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/VectorQueue.h>
#include <otawa/cfg/features.h>

using namespace otawa;
using namespace elm;
using namespace elm::genstruct;


/**
 * @addtogroup commands
 * @section ostat ostat Command
 * 
 * OStat displays some statistics - average, maximum - about a binary file:
 * @li basic block count,
 * @li basic block sizes,
 * @li memory access instructions count,
 * @li branch instructions count.
 * 
 * @par SYNTAX
 * @code
 * $ ostat binary_file function1 function2 ...
 * @endcode
 * 
 * OStat displays statistics about the given binary file for the requested
 * functions. If no function is provided, the @e main() function is used.
 * 
 * Options consists of :
 * @li -t: scan the full calling tree whose root is the processed function,
 * @li -s: display statistics in short form (no more information labels),
 * @li -o: display overall statistics including all processed functions.
 * 
 * @par Example
 * @code
 * $ ostat crc
 * FUNCTION main
 * BB count = 4
 * type = total count, average/bb, max/bb, ratio
 * instructions = 56, 14, 27, 100%
 * memory instructions = 20, 5, 9, 35.7143%
 * branch instructions = 4, 1, 1, 7.14286%
 * @endcode
 */

// Statistics class
class Statistics {
public:
	Statistics(void)
		: bb_cnt(0), inst_cnt(0), inst_max(0), mem_cnt(0), mem_max(0), bra_cnt(0), bra_max(0)
		{ }

	virtual ~Statistics(void) { }

	inline long long bbCount(void) const { return bb_cnt; };
	
	inline int instCount(void) const { return inst_cnt; };
	inline double averageInstCount(void) const { return (double)inst_cnt / bb_cnt; };
	inline long long maxInstCount(void) const { return inst_max; };
	
	inline long long memAccessCount(void) const { return mem_cnt; };
	inline double averageMemAccessCount(void) const { return (double)mem_cnt / bb_cnt; };
	inline long long maxMemAccessCount(void) const { return mem_max; };
	inline double memAccessRatio(void) const { return (double)mem_cnt * 100 / inst_cnt; }
	
	inline long long branchCount(void) const { return bra_cnt; };
	inline double averageBranchCount(void) const { return (double)bra_cnt / bb_cnt; };
	inline long long maxBranchCount(void) const { return bra_max; };
	inline double branchRatio(void) const { return (double)bra_cnt * 100 / inst_cnt; }

	void addBB(BasicBlock *bb) {

		// do not process virtual
		if(bb->isEnd())
			return;

		// initialize statistics
		long long insts = 0;
		long long mems = 0;
		long long bras = 0;
		bb_cnt++;

		// Count instructions
		for(BasicBlock::InstIter inst(bb); inst; inst++) {
			//if(!inst->isPseudo()) {
				insts++;
				if(inst->isMem())
					mems++;
				if(inst->isControl())
					bras++;
			//}
		}

		// Record computations
		if(insts > inst_max)
			inst_max = insts;
		if(mems > mem_max)
			mem_max = mems;
		if(bras > bra_max)
			bra_max = bras;
		inst_cnt += insts;
		mem_cnt += mems;
		bra_cnt += bras;
	}

	/**
	 * Print the statistics.
	 * @param out	Output to use (default cout).
	 */
	void print(io::Output& out = cout) {
		printLabel(out);
		out << "BB count = " << bbCount() << "\n";
		out << "type = total count, average/bb, max/bb, ratio\n";
		out << "instructions = "
			 << instCount() << ", "
			 << averageInstCount() << ", "
			 << maxInstCount() << ", 100%\n";
		out << "memory instructions = "
			 << memAccessCount() << ", "
			 << averageMemAccessCount() << ", "
			 << maxMemAccessCount() << ", "
			 << memAccessRatio() << "%\n";
		out << "branch instructions = "
			 << branchCount() << ", "
			 << averageBranchCount() << ", "
			 << maxBranchCount() << ", "
			 << branchRatio() << "%\n";
	}

	/**
	 * Print the statistics in short form.
	 * @param out	Output to use (default cout).
	 */
	void printShort(io::Output& out = cout) {
		out << bbCount() << '\t'
			<< instCount() << ','
			<< averageInstCount() << ','
			<< maxInstCount() << '\t'
			<< memAccessCount() << ','
			<< averageMemAccessCount() << ','
			<< maxMemAccessCount() << '\t'
			<< branchCount() << ','
			<< averageBranchCount() << ','
			<< maxBranchCount() << '\t';
		printShortLabel(out);
		out << '\n';
	}

	/**
	 * Print the header lines.
	 * @param out	Where to output to.
	 */
	void printHeader(io::Output& out = cout) {
		out << "bb   \tinst       \tmem        \tbranch     \n";
		out << "count\tcnt,avg,max\tcnt,avg,max\tcnt,avg,max\n";
	}

protected:
	virtual void printLabel(io::Output& out = cout) { }
	virtual void printShortLabel(io::Output& out = cout) { }

	long long bb_cnt;
	long long inst_cnt;
	long long inst_max;
	long long mem_cnt;
	long long mem_max;
	long long bra_cnt;
	long long bra_max;
};


// CFGStatistics class
class CFGStatistics: public Statistics {
public:
	CFGStatistics(CFG *cfg): _cfg(cfg) {
		for(CFG::BBIterator bb(cfg); bb; bb++)
			addBB(bb);
	}
	inline CFG *cfg(void) { return _cfg; };

protected:
	virtual void printLabel(io::Output& out = cout) { out << "FUNCTION " << _cfg->label() << "\n"; }
	virtual void printShortLabel(io::Output& out = cout) { out << _cfg->label(); }

private:
	CFG *_cfg;
};


// StatAccumulator class
class StatAccumulator: public Statistics {
public:
	void add(Statistics& stat) {
		bb_cnt += stat.bbCount();
		inst_cnt += stat.instCount();
		inst_max = max(inst_max, stat.maxInstCount());
		mem_cnt += stat.memAccessCount();
		mem_max = max(mem_max, stat.maxMemAccessCount());
		bra_cnt += stat.branchCount();
		bra_max = max(bra_max, stat.maxBranchCount());
	}

protected:
	virtual void printLabel(io::Output& out = cout) { out << "TOTAL\n"; }
	virtual void printShortLabel(io::Output& out = cout) { out << "TOTAL"; }
};


// Command class
class Command: public Application {
	WorkSpace *fw;
	CFGInfo *info;
public:
	Command(void):
		Application(
			"ostat",
			Version(1, 0, 1),
			"Compute statistics on a binary file.\nIf no function name is given, the main() function is used.",
			"Hugues Casse <casse@irit.fr>",
			"Copyright (c) 2006, IRIT-UPS France"),
		tree_option(option::SwitchOption::Make(*this).cmd("-t").cmd("--tree").description("Scan the whole calling tree.")),
		short_option(option::SwitchOption::Make(*this).cmd("-s").cmd("--short").description("Perform short display.")),
		overall_option(option::SwitchOption::Make(*this).cmd("-o").cmd("--overall").description("Display only overall statistics."))
	{ }

	virtual void work(const string &entry, PropList &props) throw (elm::Exception) {
		require(COLLECTED_CFG_FEATURE);
		const CFGCollection *coll = otawa::INVOLVED_CFGS(workspace());
		collect(coll->get(0));
		if(*tree_option)
			for(int i = 1; i < coll->count(); i++)
				collect(coll->get(i));
	}

	virtual void work(PropList &props) throw (elm::Exception) {
		Application::work(props);
		StatAccumulator accu;

		// print header if needed
		if(*short_option)
			accu.printHeader();

		// display the statistics
		bool fst = true;
		for(genstruct::HashTable<Address, Statistics *>::Iterator it(stats); it; it++) {

			// accumulate if needed
			if(*overall_option)
				accu.add(**it);

			// blank line
			if(!*short_option && !fst)
				cout << '\n';
			else
				fst = false;

			// display the statistics
			if(*short_option)
				it->printShort();
			else
				it->print();
		}

		// display the total if needed
		if(*overall_option) {
			if(*short_option)
				accu.printShort();
			else {
				cout << '\n';
				accu.print();
			}
		}
	}

private:

	void collect(CFG *cfg) {
		Statistics *s = stats.get(cfg->address(), 0);
		if(s)
			return;
		s = new CFGStatistics(cfg);
		stats.put(cfg->address(), s);
	}

	option::SwitchOption tree_option, short_option, overall_option;
	genstruct::HashTable<Address, Statistics *> stats;
};

OTAWA_RUN(Command)
