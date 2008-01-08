#ifndef BBHG_H_
#define BBHG_H_
#include <otawa/util/GenGraph.h>
#include <otawa/otawa.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/otawa.h>

class BBHGNode;
class BBHGEdge;




class BBHG : /*public otawa::PropList,*/ public otawa::GenGraph<BBHGNode,BBHGEdge> {
protected:
	friend class BHGNode;
	friend class BHGEdge;
	int m_history_size;
public:
	BBHG(int history_size);
	int getClass();
	void add(BBHGNode* node);
};




class BBHGEdge : public otawa::GenGraph<BBHGNode,BBHGEdge>::Edge {
private:
	bool m_edge_taken;
	bool m_from_branch;
public:
	BBHGEdge(BBHGNode *source, BBHGNode *target, bool taken=false, bool from_branch=false);
	~BBHGEdge();
	bool isTaken();
	bool isFromBranch();
};




class BBHGNode : public otawa::GenGraph<BBHGNode,BBHGEdge>::Node{
private:
	bool m_entry;
	bool m_exit;
	bool m_exit_T; // exits with Taken
	bool m_exit_NT; // exits with NotTaken
	bool m_branch;
	otawa::dfa::BitSet *m_history;
	otawa::BasicBlock *m_bb;
	int m_history_size;
public:
	BBHGNode(otawa::BasicBlock* cfg_bb,const otawa::dfa::BitSet& bs, bool branch=false, bool entry=false, bool exit=false, bool exit_T=false, bool exit_NT=false);
	~BBHGNode();
	
	otawa::BasicBlock* getCorrespondingBB();
	otawa::dfa::BitSet& getHistory();
	bool isEntry();
	bool isExit();
	bool exitsWithT();
	bool exitsWithNT();
	bool isBranch();
	bool isSuccessor(BBHGNode* succ,bool& withT, bool& withNT);
	bool equals(const BBHGNode& b);
	void setExit(bool isExit, bool withT = false, bool withNT = false);
};
#endif /*BBHG_H_*/
