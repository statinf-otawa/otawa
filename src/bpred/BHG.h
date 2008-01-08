#ifndef BHG_H_
#define BHG_H_

#include <otawa/util/GenGraph.h>
#include <otawa/otawa.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/otawa.h>

class BHGNode;
class BHGEdge;




class BHG : /*public otawa::PropList,*/ public otawa::GenGraph<BHGNode,BHGEdge> {
protected:
	friend class BHGNode;
	friend class BHGEdge;
	int m_history_size;
public:
	BHG(int history_size);
	int getClass();
	void add(BHGNode* node);
};




class BHGEdge : public otawa::GenGraph<BHGNode,BHGEdge>::Edge {
private:
	bool m_edge_taken;
public:
	BHGEdge(BHGNode *source, BHGNode *target, bool taken);
	~BHGEdge();
	bool isTaken();
};




class BHGNode : public otawa::GenGraph<BHGNode,BHGEdge>::Node{
private:
	bool m_entry;
	bool m_exit;
	bool m_exit_T; // exits with Taken
	bool m_exit_NT; // exits with NotTaken
	otawa::dfa::BitSet *m_history;
	otawa::BasicBlock *m_bb;
	int m_history_size;
public:
	BHGNode(otawa::BasicBlock* cfg_bb,const otawa::dfa::BitSet& bs, bool entry=false, bool exit=false, bool exit_T=false, bool exit_NT=false);
	~BHGNode();
	
	otawa::BasicBlock* getCorrespondingBB();
	otawa::dfa::BitSet& getHistory();
	bool isEntry();
	bool isExit();
	bool exitsWithT();
	bool exitsWithNT();
	bool isSuccessor(BHGNode* succ,bool& withT, bool& withNT);
	bool equals(const BHGNode& b);
	void setExit(bool withT = false, bool withNT = false);
};


#endif /*BHG_H_*/
