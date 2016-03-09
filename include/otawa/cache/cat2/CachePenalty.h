/*
 *	$Id$
 *	
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
#ifndef CACHE_PENALTY_H_
#define CACHE_PENALTY_H_

#include <otawa/cfg/CFG.h>
#include <otawa/prop/Identifier.h>


namespace otawa {
   	
	class CachePenalty {
	public:
		typedef enum {MISS, MISS_MISS, HIT_MISS, x_HIT} cache_penalty_type_t;
	private:
		Block *_header[2];
		int _penalty[4];  
	public:
		CachePenalty(){
			_header[0] = NULL;
			_header[1] = NULL;
			for (int i=0 ; i<4 ; i++){
				_penalty[i] = 0;
			}
		}
		inline void setHeader(int index, Block *bb)
		{ _header[index] = bb;}
		inline Block *header(int index){
			return _header[index];
		}
		inline void setPenalty(cache_penalty_type_t type, int value)
		{ _penalty[type] = value; }
		inline int penalty(cache_penalty_type_t type){
			return _penalty[type];
		}
		void dump(elm::io::Output &out){
			if (_header[0]){
				out << "h0=b" << _header[0]->index();
				if (_header[1])
					out << " - h1=b" << _header[1]->index();
				if (!_header[1])
					out << " - p[MISS]=" << _penalty[MISS];
				else {
					out << " - p[MISS_MISS]=" << _penalty[MISS_MISS];
					out << " - p[HIT_MISS]=" << _penalty[HIT_MISS];
					out << " - p[x_HIT]=" << _penalty[x_HIT];
				}
			}
		}
	};

	extern Identifier<CachePenalty *> ICACHE_PENALTY;


}

#endif /*CACHE_PENALTY_H_*/
