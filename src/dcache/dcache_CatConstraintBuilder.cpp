/*
 *	dcache::CatConstraintBuilder class interface
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

#include <otawa/ilp.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/util/Dominance.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/TrivialDataCacheManager.h>
#include <otawa/hard/Platform.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/dcache/CatBuilder.h>
#include <otawa/dcache/CatConstraintBuilder.h>
#include <otawa/cache/categories.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/stats/BBStatCollector.h>

namespace otawa { namespace dcache {

using namespace otawa::cache;


class StatCollector: public BBStatCollector {
public:
	StatCollector(WorkSpace *ws, cstring name, cstring unit)
	: BBStatCollector(ws), _name(name), _unit(unit) {
		system = ipet::SYSTEM(ws);
		ASSERT(system);
	}

	virtual cstring name(void) const { return _name; }
	virtual cstring unit(void) const { return _unit; }
	virtual bool isEnum(void) const { return false; }
	virtual const cstring valueName(int value) { return ""; }

	virtual int total(BasicBlock *bb) {
		Pair<int, BlockAccess *> dbs = dcache::DATA_BLOCKS(bb);
		ilp::Var *var = ipet::VAR(bb);
		ASSERT(var);
		return dbs.fst * int(system->valueOf(var));
	}

	void collect(Collector& collector, BasicBlock *bb) {
		if(bb->isEnd())
			return;
		collector.collect(bb->address(), bb->size(), count(bb));
	}

	virtual int mergeContext(int v1, int v2) { return v1 + v2; }
	virtual int mergeAgreg(int v1, int v2) { return v1 + v2; }

protected:
	virtual int count(BasicBlock *bb) = 0;

	ilp::System *system;
private:
	cstring _name, _unit;
};


// BB hit collector
class HitCollector: public StatCollector {
public:
	HitCollector(WorkSpace *ws): StatCollector(ws, "L1 Data Cache Hit Number", "cache access")  { }

protected:
	virtual int count(BasicBlock *bb) {
		int sum = 0;
		Pair<int, BlockAccess *> dbs = dcache::DATA_BLOCKS(bb);
		for(int i = 0; i < dbs.fst; i++) {
			ilp::Var *bb_var = ipet::VAR(bb);
			ASSERT(bb_var);
			ilp::Var *var = dcache::MISS_VAR(dbs.snd[i]);
			ASSERT(var);
			sum +=  int(system->valueOf(bb_var))  - int(system->valueOf(var));
		}
		return sum;
	}
};


// BB miss collector
class MissCollector: public StatCollector {
public:
	MissCollector(WorkSpace *ws): StatCollector(ws, "L1 Data Cache Miss Number", "cache accesses") { }

protected:
	virtual int count(BasicBlock *bb) {
		int sum = 0;
		Pair<int, BlockAccess *> dbs = dcache::DATA_BLOCKS(bb);
		for(int i = 0; i < dbs.fst; i++) {
			ilp::Var *bb_var = ipet::VAR(bb);
			ASSERT(bb_var);
			ilp::Var *var = MISS_VAR(dbs.snd[i]);
			ASSERT(var);
			sum +=  int(system->valueOf(var));
		}
		return sum;
	}
};


class NCCollector: public StatCollector {
public:
	NCCollector(WorkSpace *ws): StatCollector(ws, "L1 Data Cache NC Number", "cache access") { }


protected:
	virtual int count(BasicBlock *bb) {
		int sum = 0;
		Pair<int, BlockAccess *> dbs = dcache::DATA_BLOCKS(bb);
		ilp::Var *xi = ipet::VAR(bb);
		ASSERT(xi);
		int xiv = int(system->valueOf(xi));
		for(int i = 0; i < dbs.fst; i++)
			if(dcache::CATEGORY(dbs.snd[i]) == otawa::NOT_CLASSIFIED)
				sum += xiv;
		return sum;
	}
};




static SilentFeature::Maker<CatConstraintBuilder> maker;
/**
 * This feature ensures that the constraints associated with each data cache block categories
 * has been translated to ILP constraints and that miss count variables are declared.
 *
 * @p Default processor
 * @li @ref CatConstraintBuilder
 *
 * @p Properties
 * @li @ref MISS_VAR
 * @ingroup dcache
 */
SilentFeature CONSTRAINTS_FEATURE("otawa::dcache::CONSTRAINTS_FEATURE", maker);


/**
 * This property gives the variable counting the number of misses of a BlockAccess
 *
 * @p Hook
 * @li @ref BlockAccess
 * @ingroup dcache
 */
Identifier<ilp::Var *> MISS_VAR("otawa::dcache::MISS_VAR", 0);


/**
 * @class CatConstraintBuilder
 * This processor allocates the variable to count misses of a data cache and
 * makes the constraints for these variables.
 *
 * @p Provided features
 * @li @ref ipet::DATA_CACHE_SUPPORT_FEATURE
 *
 * @p Required features
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 * @li @ref dcache::CATEGORY_FEATURE
 * @li @ref DOMINANCE_FEATURE
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 *
 * @p Configuration
 * @li @ref ipet::EXPLICIT
 * @ingroup dcache
 */

p::declare CatConstraintBuilder::reg = p::init("otawa::dcache::CatConstraintBuilder", Version(1, 0, 0))
	.base(Processor::reg)
	.maker<CatConstraintBuilder>()
	.require(ipet::ASSIGNED_VARS_FEATURE)
	.require(dcache::CATEGORY_FEATURE)
	.require(DOMINANCE_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.provide(ipet::DATA_CACHE_SUPPORT_FEATURE)
	.provide(dcache::CONSTRAINTS_FEATURE);


/**
 */
CatConstraintBuilder::CatConstraintBuilder(p::declare& r): Processor(r), _explicit(false) {
}


/**
 */
void CatConstraintBuilder::configure(const PropList& props) {
	Processor::configure(props);
	_explicit = ipet::EXPLICIT(props);
}


/**
 */
void CatConstraintBuilder::processWorkSpace(otawa::WorkSpace *ws) {
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(ws)->dataCache();
	ilp::System *system = ipet::SYSTEM(ws);
	int penalty = cache->missPenalty();
	//LBlockSet **lbsets = LBLOCKS(fw);

	// traverse CFG
	const CFGCollection *cfgs = INVOLVED_CFGS(ws);
	ASSERT(cfgs);
	for(int i = 0; i < cfgs->count(); i++) {
		CFG *cfg = cfgs->get(i);
		if(logFor(LOG_CFG))
			log << "\tprocess CFG " << cfg->label() << io::endl;

		// traverse basic blocks
		for(CFG::BBIterator bb(cfg); bb; bb++) {
			if(logFor(LOG_BB))
				log << "\t\tprocess BB " << bb->number()
					<< " (" << ot::address(bb->address()) << ")\n";

			// traverse blocks accesses
			Pair<int, BlockAccess *> ab = DATA_BLOCKS(bb);
			for(int j = 0; j < ab.fst; j++) {
				BlockAccess& b = ab.snd[j];

                // Create x_miss variable
                ilp::Var *miss;
                if(!_explicit)
                        miss = system->newVar();
                else
                        miss = system->newVar(_ << "xm_data_bb" << bb->number() << "_i" << b.instruction()->address() << "_" << j);
                MISS_VAR(b) = miss;

                // Add the constraint depending on the block access category
                switch(dcache::CATEGORY(b)) {
                	case cache::ALWAYS_HIT: { // Add constraint: xmiss = 0
	                		ilp::Constraint *cons2 = system->newConstraint(ilp::Constraint::EQ,0);
    	            		cons2->addLeft(1, miss);
						}
                		break;
					case cache::FIRST_HIT:
					case cache::NOT_CLASSIFIED: { // Add constraint: xmiss <= x
							ilp::Constraint *cons3 = system->newConstraint(ilp::Constraint::LE);
    	            		cons3->addLeft(1, miss);
        	        		cons3->addRight(1, ipet::VAR(bb));
						}
					break;
                	case cache::ALWAYS_MISS: { // Add constraint: xmiss = x
							ilp::Constraint *cons3 = system->newConstraint(ilp::Constraint::EQ);
    	            		cons3->addLeft(1, miss);
        	        		cons3->addRight(1, ipet::VAR(bb));
						}
                		break;
					case cache::FIRST_MISS: {
							BasicBlock *header = dcache::CATEGORY_HEADER(b);
							ASSERT(header);

							// Add constraint: xmiss <= sum of entry-edges of the loop
							ilp::Constraint *cons5a = system->newConstraint(ilp::Constraint::LE);
						 	cons5a->addLeft(1, miss);
						 	for(BasicBlock::InIterator inedge(header); inedge; inedge++)
						 		if (!Dominance::dominates(header, inedge->source()))
						 			cons5a->addRight(1, ipet::VAR(*inedge));

						 	// Add constraint: xmiss <= x
						 	ilp::Constraint *cons1 = system->newConstraint(ilp::Constraint::LE);
							cons1->addRight(1, ipet::VAR(bb));
							cons1->addLeft(1, miss);
						}
						break;

                	default:
                		ASSERT(false);
				break;
                }

			}
		}
	}
}

/**
 */
void CatConstraintBuilder::collectStats(WorkSpace *ws) {
	recordStat(CONSTRAINTS_FEATURE, new HitCollector(ws));
	recordStat(CONSTRAINTS_FEATURE, new MissCollector(ws));
	recordStat(CONSTRAINTS_FEATURE, new NCCollector(ws));
}

} }		// otawa::dcache
