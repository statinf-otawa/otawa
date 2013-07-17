#include "BSets.h"
#include <elm/string/StringBuffer.h>

using namespace elm;
using namespace otawa;

namespace otawa { namespace bpred {

BSets::BSets(){
}

BSets::~BSets(){
}

void BSets::add(int addr,int idBB) {
	int id_set = -1;
	for(int i = 0 ; i<this->v_idsets.length(); ++i ) {
		if(this->v_idsets[i].addr == addr) {
			id_set = this->v_idsets[i].id_set;
			break;
		}
	}
	
	if(id_set == -1) {
		// dans ce cas il faut creer un nouveau set
		id_set = this->v_BBsets.length();

		elm::genstruct::Vector<int> v;
		this->v_BBsets.add(v);
		BSets::s_idx s;
		s.addr = addr;
		s.id_set = id_set;
		this->v_idsets.add(s);
	}

	this->v_BBsets[id_set].add(idBB);

}

bool BSets::get_vector(int addr, elm::genstruct::Vector<int>& vset) {
	int id_set = -1;
	for(int i = 0 ; i<this->v_idsets.length(); ++i ) {
		if(this->v_idsets[i].addr == addr) {
			id_set = this->v_idsets[i].id_set;
			break;
		}
	}
	
	if(id_set == -1) {
		cerr << "[BSets] ERROR : unmatched address '" << addr << "'" << endl;

		return false;
	}
	else {
		vset = (this->v_BBsets[id_set]);
		return true;
	}
}

elm::String BSets::toString() {
	elm::StringBuffer bf;
	int id_set;
	bf 	<< "[Sets]\n";
	for(int i = 0 ; i<this->v_idsets.length(); ++i ) {
		bf 	<< "\t[Set@" << this->v_idsets[i].addr << "]\n";
		id_set = this->v_idsets[i].id_set;
		for(int j = 0 ; j<this->v_BBsets[id_set].length(); ++j) {
			bf << "\t\tBloc " << (this->v_BBsets[id_set])[j] << "\n"; 
		}
		bf << "\t[/Set]\n";
	}
	bf 	<< "[/Sets]\n";
	return bf.toString();

}

int BSets::get_addr(int idBB) {
	int id_set;
	bool found = false;
	for(int i = 0 ; i<this->v_idsets.length(); ++i ) {
		id_set = this->v_idsets[i].id_set;
		for(int j = 0 ; j<this->v_BBsets[id_set].length(); ++j) {
			if((this->v_BBsets[id_set])[j] == idBB) {
				found = true;
				break;
			}
		}
		if(found) {
			return this->v_idsets[i].addr;
			break;
		}
	}
	return -1;
}

void BSets::get_all_addr(elm::genstruct::Vector<int>& lst_addr) {
	lst_addr.clear();
	for(int i = 0 ; i<this->v_idsets.length(); ++i ) {
		lst_addr.add(this->v_idsets[i].addr);
	}
}

} }		// otawa::bpred

