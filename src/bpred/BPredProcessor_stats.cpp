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

#include <otawa/cfg.h>
#include <otawa/bpred/BPredProcessor.h>
#include <otawa/ipet/BasicConstraintsBuilder.h> // Contraintes de base feature
#include <otawa/cfg/CFGCollector.h> // CFG feature (?)
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/ilp/Constraint.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ilp/System.h>



namespace otawa {
namespace bpred {


using namespace otawa::ilp;
using namespace elm;

/**
 * Creates a string that corresponds to the predictor used.
 * 
 * @return a @ref elm::String containing the name of the predictor.
 */  
String BPredProcessor::predictorToString() {
	switch(this->method) {
	case NO_CONFLICT_2BITS_COUNTER:
		{
			return String("No conflict 2bits counter");
		}
		break;
	case BI_MODAL:
		{
			return String("Bimodal");
		}
		break;
	case GLOBAL_2B:
		{
			return String("Global 2bits");
		}
		break;
	case GLOBAL_1B:
		{
			return String("Global 1bit");
		}
		break;
	}
}

/**
 * Counts the branches contained in a CFG.
 * 
 * @param cfg The pointer to the CFG.
 * 
 * @return The number of branches in the specified CFG.
 */
int countBranches(CFG *cfg) {
	int res = 0;
	for(CFG::BBIterator bb(cfg); bb; bb++) {

		unsigned int nb_OE = 0;
		// Parcours des OutEdges
		for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
			// on incremente que s'il s'agit d'un edge TAKEN ou NOT_TAKEN
			if(edge->kind() == Edge::TAKEN) nb_OE++;
			else if(edge->kind() == Edge::NOT_TAKEN) nb_OE++;
		}

		// Si un branchement a ete trouve ...
		if(nb_OE == 2 ) {
			++res;
		}

	}
	return res;
}

/**
 * Generates a String containing stats about the constraints system.
 * 
 * @param fw The current workspace.
 * @param cfg The CFG for which the system has been generated.
 * 
 * @return A String containing the stats.
 */
String BPredProcessor::genStats(WorkSpace *fw,CFG *cfg) {
	System *system = ipet::SYSTEM(fw);
	int nb_cons = system->countConstraints();
	int nb_vars = system->countVars();
	io::BlockOutStream bf;
	
	system->dump(bf);
	long int max = 0;
	long int deb=0,fin=-1;			// HACK
	const char *dmp = bf.block();

	//////////////////////////////////
	//SUPRESSION DE LA FONCTION OBJET
	//////////////////////////////////
	for(deb=0;deb<bf.size();deb++) {
		if(dmp[deb]=='m' && dmp[deb+1]=='a' && dmp[deb+2]=='x') {
			while(dmp[deb]!='\n')deb++;
			break;
		}
		else {
			while(dmp[deb]!='\n')deb++;			
		}
	}
	long int k= deb+1;
	/////////////////////////////////

	long int d=k;
	for( long int i = k,tmp_max=0 ; i<bf.size();++i) {
		if(dmp[i]=='\n') {
			if(tmp_max>max) {
				max= tmp_max;
			}
			d=i;
			tmp_max=0;
		}
		else {
			if(dmp[i]=='+') tmp_max++;
		}
	}
	


	StringBuffer f_out;
	f_out << "Predicteur  : " << predictorToString() <<"\n";
	f_out << "Taille BHT  : " << ((this->BHT+1)/4) << "\n";
	f_out << "Taille Hist.: " << (1 << (this->BHG_history_size)) << "\n";
	f_out << "Nb Blocs    : " << cfg->countBB() << "\n";
	f_out << "Nb Brcht    : " << countBranches(cfg) << "\n";
	f_out << "Conflits : \n";
	if(this->method == BI_MODAL) {
		for(int i=0;i<this->stat_addr.length();++i)
			f_out << "\t@" << this->stat_addr[i] << " => " << this->stat_nbbr[i] <<" brcht(s) \n";
	}
	else if (this->method == GLOBAL_1B || this->method == GLOBAL_2B) {
		for(int i=0;i<this->stat_hist.length();++i)
			f_out << "\t@" << BitSet_to_String(*(this->stat_hist[i])) << " => " << this->stat_nbbr[i] <<" brcht(s) \n";
	}
	else {
		f_out << "\tno conflicts" << "\n";
	}
	f_out << "\nComplexite  : ( Nb contraintes=" << nb_cons << ", Nb variables=" << nb_vars << ", Arite=" << (max+1) << ")\n";

	return f_out.toString();
}


/**
 * The public accessor to the string containing the stats generated. 
 * @return A String containing the stats generated.
 */
String BPredProcessor::getStats() {
	return this->stats_str;
}

} // ::ipet
} // ::otawa
