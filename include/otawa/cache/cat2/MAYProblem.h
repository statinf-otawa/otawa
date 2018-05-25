/*
 *	$Id$
 *	This describes the "MAY" cache problem.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#ifndef CACHE_MAYPROBLEM_H_
#define CACHE_MAYPROBLEM_H_

#include <otawa/dfa/hai/HalfAbsInt.h>
#include <elm/assert.h>
#include <elm/io.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {


class MAYProblem {
	
	// Types
	public:

	class Domain {
			int A, size;
						
			public:		
					/**
			 * @class Result
			 *
			 * Class to hold the results (for each basic block) of the abstract interpretation.
			 */
			
		
			inline Domain(const int _size, const int _A)
			: A (_A), size(_size)
			{
				age = new int [size];
				for (int i = 0; i < size; i++)
					age[i] = -1;
			}
			
			inline ~Domain() {
				delete [] age;
			}
			
			inline Domain(const Domain &source) : A(source.A), size(source.size) {
				age = new int [size];
				for (int i = 0; i < size; i++)
					age[i] = source.age[i];

			} 
		
			inline Domain& operator=(const Domain &src) {
				ASSERT((A == src.A) && (size == src.size));
				for (int i = 0; i < size ; i++)
					age[i] = src.age[i];
				return(*this);
				
			}
			 
			inline void glb(const Domain &dom) {
				/* not implemented */
				ASSERT(false);
			}
			
			inline void lub(const Domain &dom) {
				ASSERT((A == dom.A) && (size == dom.size));

				for (int i = 0; i < size; i++) {
					if (((age[i] > dom.age[i]) && (dom.age[i] != -1)) || (age[i] == -1)) 
						age[i] = dom.age[i];
				}
			}
			
			inline int getSize(void) {
				return size;
			}
			
			inline void addDamage(const int id, const int damage) {
				ASSERT((id >= 0) && (id < size));
				if (age[id] == -1)
					return;
				age[id] += damage;
				if (age[id] >= A)
					age[id] = -1;
			}
			
			inline bool equals(const Domain &dom) const {
				ASSERT((A == dom.A) && (size == dom.size));
				for (int i = 0; i < size; i++)
					if (age[i] != dom.age[i])
						return false;
				return true;
			}
			
			inline void empty() {
				for (int i = 0; i < size; i++)
					age[i] = -1;
				
			}
			
			inline bool contains(const int id) {
				ASSERT((id < size) && (id >= 0));
				return(age[id] != -1);				
			}
			
			
			inline void inject(const int id) {
				if (contains(id)) {
					for (int i = 0; i < size; i++) {
						if ((age[i] <= age[id]) && (age[i] != -1))
							age[i]++;						
					}
					age[id] = 0;
				} else {
					for (int i = 0; i < size; i++) {
						if (age[i] != -1) 
							age[i]++;
						if (age[i] == A)
							age[i] = -1;
					}
				}
				age[id] = 0;				
			}
			
			inline void print(elm::io::Output &output) const {
				bool first = true;
				output << "[";
				for (int i = 0; i < size; i++) {
					if (age[i] != -1) {
						if (!first) {
							output << ", ";
						}
						// output << i << ":" << age[i];
						output << i;
						output << ":";
						output << age[i];
						
						first = false;
					}
				}
				output << "]";
				
			}
			
			inline int getAge(int id) const {
				ASSERT(id < size);
				return(age[id]);
			}
			
			
			inline void setAge(const int id, const int _age) {
				ASSERT(id < size);
				ASSERT((_age < A) || (_age == -1));
				age[id] = _age;
			}
		
			/*
			 * For each cache block belonging to the set: 
			 * age[block] represents its age, from 0 (newest) to A-1 (oldest).
			 * The value -1 means that the block is not in the set.
			 */  
			int *age;
	};
	
	private:
	
	// Fields
	LBlockSet *lbset;
	WorkSpace *fw;
	const int line;
	const hard::Cache *cache;
	Domain bot;
	Domain _top;
	Domain ent;
	
	public:
	Domain callstate;

	// Public fields
	
	// Constructors
	MAYProblem(const int _size, LBlockSet *_lbset, WorkSpace *_fw, const hard::Cache *_cache, const int _A);
	
	// Destructors
	~MAYProblem();
	
	// Problem methods
	const Domain& bottom(void) const;
	const Domain& top(void) const;
	const Domain& entry(void) const;
	
		
	inline void lub(Domain &a, const Domain &b) const {
		a.lub(b);
	}
	inline void assign(Domain &a, const Domain &b) const {
		a = b;
	}
	inline bool equals(const Domain &a, const Domain &b) const {
		return (a.equals(b));
	}
	

	void update(Domain& out, const Domain& in, Block* bb);
	inline void enterContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) {

	}

	inline void leaveContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) {

	}		

};

elm::io::Output& operator<<(elm::io::Output& output, const MAYProblem::Domain& dom);

}

#endif /*CACHE_MAYPROBLEM_H_*/
