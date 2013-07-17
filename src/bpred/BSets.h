/*
 *	$Id$
 *	BSets class interface
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

#ifndef BSETS_H_
#define BSETS_H_
#include <elm/string/String.h>
#include <elm/genstruct/Vector.h>
#include <otawa/otawa.h>

namespace otawa { namespace bpred {

class BSets
{
private:
	typedef struct {
		int addr;
		int id_set;
	}s_idx;
	elm::genstruct::Vector<s_idx> v_idsets;
	elm::genstruct::Vector< elm::genstruct::Vector<int> > v_BBsets;

public:
	BSets();
	virtual ~BSets();
	void add(int addr, int idBB);
	bool get_vector( int addr, elm::genstruct::Vector<int>& vset);
	elm::String toString();
	int get_addr(int idBB);
	int nb_addr();
	void get_all_addr(elm::genstruct::Vector<int> &lst_addr);
};

inline int BSets::nb_addr() {
	return this->v_idsets./*size*/length();
}

} }		// otawa::bpred

#endif /*BSETS_H_*/
