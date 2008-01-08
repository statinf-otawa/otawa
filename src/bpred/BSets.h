#ifndef BSETS_H_
#define BSETS_H_
#include <elm/string/String.h>
#include <elm/genstruct/Vector.h>
#include <otawa/otawa.h>

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
#endif /*BSETS_H_*/
