/*
 *	$Id$
 *	BPredProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#include <otawa/bpred/BPredProcessor.h>
#include <otawa/ipet/BasicConstraintsBuilder.h> // Contraintes de base feature
#include <otawa/ipet.h>
#include <otawa/cfg/CFGCollector.h> // CFG feature (?)
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/ilp/Constraint.h>
#include <otawa/ipet/IPET.h>

#include <otawa/ipet/VarAssignment.h>
#include <otawa/ipet/FlowFactConstraintBuilder.h>
#include <otawa/proc/ProcessorPlugin.h>


namespace otawa {
namespace bpred {

using namespace otawa::ilp;
using namespace elm;


/**
 * @defgroup bpred bpred Plugin
 *
 * This module implements a branch prediction analysis based on:
 *
 * C. Burguiere, C. Rochange. History-based Schemes and Implicit Path Enumeration.
 * International Workshop on Worst-Case Execution Time Analysis (WCET 2006), Dresden, July 2006.
 *
 * The possible states of the branch predictor on the program are modelled by a graph
 * translated into the IPET system. Mispredictions are converted as time added to the objective
 * function producing the WCET.
 *
 * To invoke it, just require the feature @ref otawa::bpred::BRANCH_PREDICTION_FEATURE.
 * To configure the computation, pass properties to the configuration property list:
 * @ref otawa::bpred::METHOD, @ref otawa::bpred::BHT_SIZE, @ref otawa::bpred::HISTORY_SIZE .
 *
 * @par Plugin Information
 * @li name: otawa/bpred
 * @li header: <otawa/bpred/features.h>
 */

static SilentFeature::Maker<BPredProcessor> maker;
/**
 * @class BPredProcessor
 * This is a specialization of the CFGProcessor class dedicated to branch
 * prediction.
 * 
 * @p Configuration Properties
 * @li @ref METHOD
 * @li @ref HISTORY_SIZE
 * @li @ref INIT_HISTORY_BINARYVALUE
 * @li @ref BHT_SIZE
 * @li @ref DUMP_BCG
 * @li @ref DUMP_BHG
 * @li @ref DUMP_BBHG
 * @li @ref WITH_MITRA
 *
 *
 * @note This processor automatically call @ref BasicConstraintsBuilder, @ref Virtualizer, @ref BasicObjectFunctionBuilder, @ref FlowFactConstraintBuilder.
 *
 * @ingroup bpred
 */
SilentFeature BRANCH_PREDICTION_FEATURE("otawa::bpred::BRANCH_PREDICTION_FEATURE", maker);


/**
 * Configuration property to select the method used for branch prediction.
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<method_t> METHOD("otawa::bpred::METHOD",	NO_CONFLICT_2BITS_COUNTER);

/**
 * Configuration property to select the BHT's size to use with the bimodal metho.
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<int> BHT_SIZE("otawa::bpred::BHT_SIZE", 4);

/**
 * Configuration property to select the history size to use for Global2b and Global1b methods.
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<int> HISTORY_SIZE("otawa::bpred::HISTORY_SIZE", 4);

/**
 * Configuration property to dump or not the BCGs.
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<bool> DUMP_BCG("otawa::bpred::DUMP_BCG",	false);

/**
 * Configuration property to select to dump or not the BHGs.
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<bool> DUMP_BHG("otawa::bpred::DUMP_BHG",	false);

/**
 * Configuration property to dump or not the BBHGs.
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<bool> DUMP_BBHG("otawa::bpred::DUMP_BBHG", false);

/**
 * Configuration property to use or not some constraints from mitra's .
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<bool> WITH_MITRA("otawa::bpred::WITH_MITRA",	false);

/**
 * Configuration property to generate or not stats.
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<bool> WITH_STATS("otawa::bpred::WITH_STATS",	false);

/**
 * Configuration property to select the inital value for history.
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<const char*> INIT_HISTORY_BINARYVALUE("otawa::bpred::INIT_HISTORY_BINARYVALUE",	0);

/**
 * Configuration property to select .
 *
 * @p Feature
 * @li otawa::bpred::BRANCH_PREDICTION_FEATURE
 * @ingroup bpred
 */
Identifier<bool> EXPLICIT_MODE("otawa::bpred::sEXPLICIT_MODE", true);


p::declare BPredProcessor::reg = p::init("otawa::bpred::BPredProcessor", elm::Version(1,1,0))
	.require(VIRTUALIZED_CFG_FEATURE)
	.require(ipet::CONTROL_CONSTRAINTS_FEATURE)
	.require(ipet::OBJECT_FUNCTION_FEATURE)
	.require(ipet::FLOW_FACTS_CONSTRAINTS_FEATURE)
	.provide(BRANCH_PREDICTION_FEATURE)
	.maker<BPredProcessor>()
	.base(CFGProcessor::reg);

/**
 * Build a new branch prediction processor.
 */
BPredProcessor::BPredProcessor(p::declare& r): CFGProcessor(r) {
	this->mitraInit = 0;
}



/**
 * The destructor.
 */
BPredProcessor::~BPredProcessor() {
	if(this->mitraInit != 0) delete this->mitraInit;
}


/**
 * Converts a string containing a binary number into a bitset and sets the class parameter mitraInit.
 * 
 * @param binary_histo	A constant char array containing the binary number, terminated by \0.
 */
void BPredProcessor::setMitraInit(const char* binary_histo)
{
	if(binary_histo != 0) {
		char c;
		int i =0, j = this->BHG_history_size -1;
		while((c=binary_histo[i])!='\0') {
			if(c=='1') this->mitraInit->add(j);
			i++;j--;
		}
	}
}


/**
 * Converts a BitSet into a String.
 * 
 * @param bs	A constant reference to the BitSet.
 * 
 * @return A String containing the binary number that corresponds to the given BitSet.
 */
String BPredProcessor::BitSet_to_String(const dfa::BitSet& bs) {
	StringBuffer bf;
	for(int i=bs.size()-1;i>=0;--i)
		bf << ((bs.contains(i)) ? "1":"0");
	return bf.toString();
}


/**
 * Operates a left-shift to the BitSet, filling the new free bit(s) with the given state (true or false).
 * 
 * @param bs		BitSet to apply the left-shift from.
 * @param dec		The number of bits to shift.
 * @param val_in	The state to set the new bits.
 * 
 * @return A new BitSet corresponding to the left shift applied to the given BitSet.
 */
dfa::BitSet BPredProcessor::lshift_BitSet(dfa::BitSet bs,int dec,bool val_in) {
//	cerr << "{ " << BitSet_to_String(bs) << "<<" << dec << " avec " << ((val_in)?"1":"0") << " => ";
	dfa::BitSet _bs=bs;
	bs.empty();
	for(int i = _bs.size()-1,j = _bs.size()-1-dec;j>=0;--i,--j) {
		if(_bs.contains(j)) {
			bs.add(i);
		}
	}
	for(int i=dec-1;i>=0;--i) {
		if(val_in)
			bs.add(i);
	}
//	cerr << BitSet_to_String(bs) << " }\n";
	return bs;
}

/**
 * That's the overloaded method that creates the new constraint system defined by the given method.
 * 
 * @param fw	Current workspace.
 * @param cfg	CFG from which the constraints system must be created.
 */
void BPredProcessor::processCFG(WorkSpace *fw, CFG *cfg) {

	if(cfg != 0) {
		switch(this->method) {
			case NO_CONFLICT_2BITS_COUNTER:
				processCFG__NoConflict_2bCounter(fw,cfg);
				break;
			case BI_MODAL:
				processCFG__Bimodal(fw,cfg);
				break;
			case GLOBAL_2B:
				processCFG__Global2B(fw,cfg);
				break;
			case GLOBAL_1B:
				processCFG__Global1B(fw,cfg);
				break;
		}
	}
	
    if(this->withStats) {
    	this->stats_str = genStats(fw,cfg);
    }

}

/**
 * Configures the parameters of the class from the given PropList.
 * 
 * @param props	PropList containg the parameters.
 */
void BPredProcessor::configure(const PropList& props) {
	CFGProcessor::configure(props);
	this->BHG_history_size 	= HISTORY_SIZE(props);
	this->BHT 				= (1 << ((BHT_SIZE(props))+2)) -1 ;
	this->dumpBCG 			= DUMP_BCG(props);
	this->dumpBHG 			= DUMP_BHG(props);
	this->dumpBBHG 			= DUMP_BBHG(props);
	this->method 			= METHOD(props);
	this->withStats 		= WITH_STATS(props);
	this->withMitra 		= WITH_MITRA(props);
	this->explicit_mode		= EXPLICIT_MODE(props);
	//ipet::EXPLICIT(props) 	= this->explicit_mode ;	
	this->mitraInit 		= new dfa::BitSet(this->BHG_history_size);
	setMitraInit(INIT_HISTORY_BINARYVALUE(props));
}


String BPredProcessor::bin_to_str(int i) {
	switch(i){
	case 0:
		return String("00");
		break;
	case 1:
		return String("01");
		break;
	case 2:
		return String("10");
		break;
	case 3:
		return String("11");
		break;
	default:
		return String("#ERROR: bin_to_str#");
	}
}

// Plugin declaration
class Plugin: public ProcessorPlugin {
public:
	typedef genstruct::Table<AbstractRegistration * > procs_t;

	Plugin(void): ProcessorPlugin("otawa::bpred", Version(1, 1, 0), OTAWA_PROC_VERSION) { }
	virtual procs_t& processors(void) const { return procs_t::EMPTY; };
};

} } // otawa::bpred

otawa::bpred::Plugin OTAWA_PROC_HOOK;
otawa::bpred::Plugin& otawa_bpred = OTAWA_PROC_HOOK;
