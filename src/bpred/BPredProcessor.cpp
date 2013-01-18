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


namespace otawa {
namespace ipet {

using namespace otawa::ilp;
using namespace elm;



/**
 * @class BPredProcessor
 * This is a specialization of the CFGProcessor class dedicated to branch
 * prediction. The 
 * 
 * It accepts in configuration the following properties:
 * @li @ref BP__METHOD: the method used for branch prediction,
 * @li @ref BP__HISTORY_SIZE: the history size to use for Global2b and Global1b methods,
 * @li @ref BP__INIT_HISTORY_BINARYVALUE: the inital value for history,
 * @li @ref BP__BHT_SIZE: the BHT's size to use with the bimodal method,
 * @li @ref BP__DUMP_BCG: to dump or not the BCGs,
 * @li @ref BP__DUMP_BHG: to dump or not the BBHGs,
 * @li @ref BP__DUMP_BBHG: to dump or not the BHGs,
 * @li @ref BP__WITH_MITRA: to use or not some constraints from mitra's,
 * @li @ref BP__WITH_MITRA: to generate or not stats.
 *
 *
 * @note This processor automatically call @ref BasicConstraintsBuilder, @ref Virtualizer, @ref BasicObjectFunctionBuilder, @ref FlowFactConstraintBuilder.
 */

Feature<BPredProcessor> BRANCH_PREDICTION_FEATURE("BRANCH_PREDICTION_FEATURE");

// Configuration Properties
Identifier<BPredProcessor::Methods> BP__METHOD(			"otawa::ipet::bpred::BPredProcessor::method",				BPredProcessor::NO_CONFLICT_2BITS_COUNTER);
Identifier<int> 		BP__BHT_SIZE(					"otawa::ipet::bpred::BPredProcessor::bht_size",				4);
Identifier<int> 		BP__HISTORY_SIZE(				"otawa::ipet::bpred::BPredProcessor::history_size",			4);
Identifier<bool> 		BP__DUMP_BCG(					"otawa::ipet::bpred::BPredProcessor::dump_bcg",				false); 
Identifier<bool>		BP__DUMP_BHG(					"otawa::ipet::bpred::BPredProcessor::dump_bhg",				false); 
Identifier<bool> 		BP__DUMP_BBHG(					"otawa::ipet::bpred::BPredProcessor::dump_bbhg",			false);
Identifier<bool> 		BP__WITH_MITRA(					"otawa::ipet::bpred::BPredProcessor::with_mitra",			false);
Identifier<bool> 		BP__WITH_STATS(					"otawa::ipet::bpred::BPredProcessor::with_stats",			false);
Identifier<const char*> BP__INIT_HISTORY_BINARYVALUE(	"otawa::ipet::bpred::BPredProcessor::initial_history_value",NULL);
Identifier<bool> 		BP__EXPLICIT_MODE(				"otawa::ipet::bpred::BPredProcessor::explicit_mode",		true);


/**
 * Build a new branch prediction processor.
 */
BPredProcessor::BPredProcessor(): CFGProcessor("otawa::ipet::bpred::BPredProcessor", elm::Version(1,0,0)) {
	require(VIRTUALIZED_CFG_FEATURE); 
	require(CONTROL_CONSTRAINTS_FEATURE); // Contraintes de base
	require(ipet::OBJECT_FUNCTION_FEATURE);
	require(ipet::FLOW_FACTS_CONSTRAINTS_FEATURE);
	provide(BRANCH_PREDICTION_FEATURE);
	this->mitraInit=NULL;
}



/**
 * The destructor.
 */
BPredProcessor::~BPredProcessor() {
	if(this->mitraInit!=NULL) delete this->mitraInit;
}


/**
 * Converts a string containing a binary number into a bitset and sets the class parameter mitraInit.
 * 
 * @param binary_histo	A constant char array containing the binary number, terminated by \0.
 */
void BPredProcessor::setMitraInit(const char* binary_histo)
{
	if(binary_histo!=NULL) {
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
			case BPredProcessor::NO_CONFLICT_2BITS_COUNTER:
				processCFG__NoConflict_2bCounter(fw,cfg);
				break;
			case BPredProcessor::BI_MODAL:
				processCFG__Bimodal(fw,cfg);
				break;
			case BPredProcessor::GLOBAL_2B:
				processCFG__Global2B(fw,cfg);
				break;
			case BPredProcessor::GLOBAL_1B:
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
	this->BHG_history_size 	= BP__HISTORY_SIZE(props);
	this->BHT 				= (1 << ((BP__BHT_SIZE(props))+2)) -1 ;
	this->dumpBCG 			= BP__DUMP_BCG(props);
	this->dumpBHG 			= BP__DUMP_BHG(props);
	this->dumpBBHG 			= BP__DUMP_BBHG(props);
	this->method 			= BP__METHOD(props);
	this->withStats 		= BP__WITH_STATS(props);
	this->withMitra 		= BP__WITH_MITRA(props);
	this->explicit_mode		= BP__EXPLICIT_MODE(props);
	//ipet::EXPLICIT(props) 	= this->explicit_mode ;	
	this->mitraInit 		= new dfa::BitSet(this->BHG_history_size);
	setMitraInit(BP__INIT_HISTORY_BINARYVALUE(props));
}

} // ::ipet
} // ::otawa

