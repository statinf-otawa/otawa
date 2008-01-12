/*
 *	$Id$
 *	BPredProcessor class interface
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
#ifndef OTAWA_IPET_BRANCHINSPECTION_
#define OTAWA_IPET_BRANCHINSPECTION_

#include <otawa/otawa.h>
#include <otawa/dfa/BitSet.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/ipet/IPET.h>


#define CLAIRE_INFOS 0



class BSets;

class BCG;

class BHG;
class BHGEdge;
class BHGNode;

class BBHG;
class BBHGNode;

namespace otawa {
namespace ipet {

class BPredProcessor: public CFGProcessor {
public:
	enum Methods { 	NO_CONFLICT_2BITS_COUNTER=0, 
					BI_MODAL=1,
					GLOBAL_2B=2,
					GLOBAL_1B=3};
private:;
	unsigned int BHT;
	bool dumpBCG,dumpBHG,dumpBBHG;
	Methods method;
	unsigned int BHG_history_size;
	bool withStats,withMitra;
	elm::genstruct::Vector<int> stat_addr,stat_nbbr;
	String stats_str;
	elm::genstruct::Vector<dfa::BitSet*> stat_hist;
	dfa::BitSet *mitraInit;
	bool explicit_mode;
	// STATS
	String genStats(WorkSpace *ws, CFG *cfg);
	String predictorToString();

	// GENERAL
	BasicBlock* getBB(int id,CFG* cfg);
	virtual void processCFG(WorkSpace *ws,CFG* cfg);
	inline String bin_to_str(int i);
	inline int getBit(int v,int n);
	inline void setBit(int&v, int n);
	String BitSet_to_String(const dfa::BitSet& bs);
	dfa::BitSet lshift_BitSet(dfa::BitSet bs,int dec,bool val_in=false);
	void setMitraInit(const char* binary_histo);
	
	// Global 1bit
	void CS__Global1b_mitra(WorkSpace *fw, CFG *cfg, BBHG *bhg, elm::genstruct::HashTable<String ,ilp::Var*>& ht_vars );
	void CS__Global1b(WorkSpace *fw, CFG *cfg, BHG *bhg, BBHG *bbhg,elm::genstruct::Vector<BCG*> &bcgs, elm::genstruct::HashTable<String ,ilp::Var*>& ht_vars );
	void processCFG__Global1B(WorkSpace *ws,CFG* cfg);
	void generateBBHG(CFG* cfg,BBHG& bbhg);
	bool contains(const elm::genstruct::Vector< BBHGNode* >& v, BBHGNode& n, BBHGNode * &contained);
	bool isBranch(BasicBlock* bb);
	
	// Global 2bits
	void CS__Global2b_not_mitra(WorkSpace *fw, CFG *cfg, BHG* bhg, elm::genstruct::Vector<BCG*> &graphs ,	elm::genstruct::HashTable<String ,ilp::Var*>& ht_vars) ;
	void CS__Global2b(WorkSpace *fw, CFG *cfg, BHG *bhg, elm::genstruct::Vector<BCG*> &graphs,elm::genstruct::HashTable<String ,ilp::Var*>& ht_vars );
	bool isClassEntry(BHG* bhg, BHGNode* src);
	bool isClassExit(BHG* bhg, BHGNode* src, bool& src_withT,bool& src_withNT);
	bool contains(const elm::genstruct::Vector< BHGNode* >& v, BHGNode& n, BHGNode * &contained);
	void historyPlusOne(dfa::BitSet& h);
	bool isLinked(BHGEdge* dir, BHGNode* dest, dfa::BitSet& h, elm::genstruct::HashTable<BHGNode* , BHGNode*>& visited_nodes);
	BasicBlock* getFirstBranch(BasicBlock *bb, CFG* cfg);
	void getBranches(BasicBlock* bb,dfa::BitSet bs, elm::genstruct::Vector<BHGNode* >& suivants, CFG* cfg, BasicBlock* entryBr);
	void generateBHG(CFG* cfg,BHG& bhg);
	void generateBCGs(elm::genstruct::Vector<BCG*>& bcgs, BHG& bhg);
	void processCFG__Global2B(WorkSpace *ws,CFG* cfg);

	
	// No Conflict 2bits Counter
	void CS__NoConflict_2bCounter(WorkSpace* fw,BasicBlock* bb);
	void processCFG__NoConflict_2bCounter(WorkSpace *ws,CFG* cfg);

	// Bi Modal
	void CS__BiModal(WorkSpace *fw, CFG *cfg, BSets& bs, elm::genstruct::Vector<BCG*> &graphs);
	void generateClasses(CFG *cfg, BSets& bs);
	void computePredecessors(CFG 				*cfg, 
						BasicBlock			*bb, 
						elm::genstruct::Vector<int> 	*bit_sets, 
						elm::genstruct::Vector<int> 	&in_outs, 
						BSets 				&bs,
						int 				addr, 
						int 				visited[]);
	void simplifyCFG(CFG* cfg, BSets& bs, int addr, elm::genstruct::Vector<BCG*> &graphs);
	void processCFG__Bimodal(WorkSpace *ws,CFG* cfg);




public:
	String getStats();
	BPredProcessor();
	~BPredProcessor();
	void configure(const PropList &props);
};

inline String BPredProcessor::bin_to_str(int i) {
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
	
inline int BPredProcessor::getBit(int v,int n) {
	return ((v & 1 << n)>0)?1:0;
}


inline void BPredProcessor::setBit(int& v, int n) {
	v |= (1<<n);
}


extern Feature<BPredProcessor> BRANCH_PREDICTION_FEATURE;


// Configuration Properties
extern Identifier<BPredProcessor::Methods> BP__METHOD;
extern Identifier<int> 			BP__BHT_SIZE;
extern Identifier<int> 			BP__HISTORY_SIZE;
extern Identifier<bool> 		BP__DUMP_BCG; 
extern Identifier<bool>			BP__DUMP_BHG; 
extern Identifier<bool> 		BP__DUMP_BBHG;
extern Identifier<bool> 		BP__WITH_MITRA;
extern Identifier<bool> 		BP__WITH_STATS;
extern Identifier<const char*> 	BP__INIT_HISTORY_BINARYVALUE;
extern Identifier<bool> 		BP__EXPLICIT_MODE;


} // ::ipet
} // ::otawa




#endif /* OTAWA_IPET_BRANCHINSPECTION_ */
