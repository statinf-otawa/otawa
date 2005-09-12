#ifndef _LBLOCKSET_H_
#define _LBLOCKSET_H_


#include <otawa/instruction.h>
#include <elm/Collection.h> 
#include <elm/genstruct/Vector.h>
#include <otawa/cache/LBlock.h>
#include <elm/Iterator.h>
#include <otawa/cache/ccg/CCGDFA.h>


namespace otawa {


class LBlockSet {
		
	int cptr ;
	static int counter;
	int linenumber;
	 elm::genstruct::Vector<LBlock *> listelbc;
		class Iterator: public IteratorInst<LBlock *> {
		 elm::genstruct::Vector<LBlock *> & lbs;
		 int pos;
		 public:
		 inline Iterator(elm::genstruct::Vector<LBlock *> &vector): lbs(vector) {
			pos = 0;
			};
		virtual bool ended(void) const;
		virtual LBlock *item(void) const;
		virtual void next(void);
	};	
	public:	 
	static Identifier ID_LBlockSet;
	friend class CCGDFA;
	inline IteratorInst<LBlock *> *visitLBLOCK(void) {
			 return new Iterator(listelbc); 
		 };
	 
	inline LBlockSet(){
	 	cptr = 0;
	 	linenumber = counter;
	 	counter = counter + 1;
		};
	int addLBLOCK (LBlock *node);
	int returnCOUNTER(void);
	LBlock *returnLBLOCK(int i);
	int cacheline(void);
};

} //otawa

#endif //_LBLOCKSET_H_
