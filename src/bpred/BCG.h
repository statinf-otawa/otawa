#ifndef BCG_H_
#define BCG_H_

#include <otawa/util/GenGraph.h>
#include <otawa/otawa.h>
#include <otawa/dfa/BitSet.h>

class BCGNode;
class BCGEdge;




class BCG : public otawa::GenGraph<BCGNode,BCGEdge> {
private:
	int m_class;
	otawa::dfa::BitSet *m_history;
public:
	BCG(int _class);
	BCG(const otawa::dfa::BitSet& _hist);
	~BCG();
	int getClass();
	otawa::dfa::BitSet& getHistory(); 
};




class BCGEdge : public otawa::GenGraph<BCGNode,BCGEdge>::Edge {
private:
	bool m_edge_taken;
public:
	BCGEdge(BCGNode *source, BCGNode *target, bool taken);
	~BCGEdge();
	bool isTaken();
};




class BCGNode : public otawa::GenGraph<BCGNode,BCGEdge>::Node{
private:
	bool m_entry;
	bool m_exit;
	bool m_exit_T; // exits with Taken
	bool m_exit_NT; // exits with NotTaken
	otawa::dfa::BitSet *m_history;
	int m_bb;
public:
	BCGNode(int cfg_bb, bool entry, bool exit, bool exit_T=false, bool exit_NT=false);
	BCGNode(int cfg_bb, bool entry, bool exit,const otawa::dfa::BitSet *history, bool exit_T=false, bool exit_NT=false);
	~BCGNode();
	
	int getCorrespondingBBNumber();
	bool isEntry();
	bool isExit();
	bool exitsWithT();
	bool exitsWithNT();
	bool isSuccessor(BCGNode* succ,bool& withT, bool& withNT);
	otawa::dfa::BitSet& getHistory(); 

};



#endif /*BCG_H_*/

