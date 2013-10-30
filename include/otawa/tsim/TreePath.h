/*
 *	TreePath class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-07, IRIT UPS.
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_TSIM_TREEPATH_H
#define OTAWA_TSIM_TREEPATH_H
#include <elm/util/Option.h>
#include <elm/genstruct/Vector.h>

#define DEFAULT_MAX_CHILDS 8

namespace otawa { namespace tsim {

template <class T1, class T2>
class TreePath
{
	T1 _label;
	elm::Option<T2> _data;
	int _max_childs;
	elm::genstruct::Vector<TreePath<T1,T2>*> _childs;
	
	inline TreePath(const T1 label, int max_childs = DEFAULT_MAX_CHILDS);
	inline TreePath<T1,T2>* searchChild(const T1 &label) const;
	
public:
	inline TreePath(int max_childs = DEFAULT_MAX_CHILDS);
	inline TreePath(const T1 label, const T2 data, int max_childs = DEFAULT_MAX_CHILDS);
	inline TreePath(elm::genstruct::Vector<T1> &path, const T2 data, int max_childs = DEFAULT_MAX_CHILDS);
	inline TreePath(elm::genstruct::Vector<T1> *path, const T2 data, int max_childs = DEFAULT_MAX_CHILDS);
	inline ~TreePath();
	
	// Accessors
	inline T1 rootLabel() const;
	inline T2 rootData() const;
	inline elm::Option<T2> get(const elm::genstruct::Vector<T1> &path, int from_index = 0);

	// Mutators
	inline void add(elm::genstruct::Vector<T1> &path, const T2 data, int from_index = 0);
	inline void add(elm::genstruct::Vector<T1> *path, const T2 data, int from_index = 0);
	
	// Iterator
	class Iterator: public elm::genstruct::Vector<TreePath<T1, T2> *>::Iterator {
	public:
		inline Iterator(const TreePath<T1, T2> *tree);
		inline Iterator(const TreePath<T1, T2>::Iterator& iter);
	};
};


// TreePath private methods
template <class T1, class T2>
inline TreePath<T1,T2>::TreePath(const T1 label, int max_childs)
:  _max_childs(max_childs), _childs(max_childs) {
	_label = label;
	_data = elm::none;
}

template <class T1, class T2>
inline TreePath<T1,T2>* TreePath<T1,T2>::searchChild(const T1 &label) const{
	int l = _childs.length();
	for(int i=0 ; i < l ; i++){
		TreePath<T1,T2> *child = _childs[i];
		if(child->_label == label)
			return child;
	}
	return 0;
}

// TreePath public methods
template <class T1, class T2>
inline TreePath<T1,T2>::TreePath(int max_childs)
:_childs(max_childs), _max_childs(max_childs){
	_data = elm::none;
}

template <class T1, class T2>
inline TreePath<T1,T2>::TreePath(const T1 label, const T2 data, int max_childs)
: _max_childs(max_childs), _childs(max_childs) {
	_label = label;
	_data = data;
}

template <class T1, class T2>
inline TreePath<T1,T2>::TreePath(elm::genstruct::Vector<T1> &path, const T2 data, int max_childs)
: _max_childs(max_childs), _childs(max_childs){
	_label = path[0];
	_data = elm::none;
	add(path,data,1);
}

template <class T1, class T2>
inline TreePath<T1,T2>::TreePath(elm::genstruct::Vector<T1> *path, const T2 data, int max_childs)
:_childs(max_childs), _max_childs(max_childs){
	_label = path[0];
	_data = elm::none;
	add(*path,data,1);
}

template <class T1, class T2>
inline TreePath<T1,T2>::~TreePath(){
	int l = _childs.length();
	for(int i=0 ; i < l ; i++){
		delete _childs[i];
	}
}

template <class T1, class T2>
inline void TreePath<T1,T2>::add(elm::genstruct::Vector<T1> &path, const T2 data, int from_index){
	TreePath<T1,T2> *cur = this;
	int l = path.length();
	for(int i = from_index ; i < l ; i++){
		T1 &label = path[i];
		elm::genstruct::Vector<TreePath<T1,T2>*> &childs = cur->_childs;
		cur = cur->searchChild(label);
		if(!cur){
			cur = new TreePath<T1,T2>(label,_max_childs);
			childs.add(cur);
		}
	}
	cur->_data = data;
}

template <class T1, class T2>
inline void TreePath<T1,T2>::add(elm::genstruct::Vector<T1> *path, const T2 data, int from_index){
	add(*path, data, from_index);
}

template <class T1, class T2>
inline elm::Option<T2> TreePath<T1,T2>::get(const elm::genstruct::Vector<T1> &path, int from_index){
	TreePath<T1,T2> *cur = this;
	int l = path.length();
	for(int i=from_index ; i < l ; i++){
		cur = cur->searchChild(path[i]);
		if(!cur)
			return elm::none;
	}
	return cur->_data;
}

template <class T1, class T2>
inline T1 TreePath<T1,T2>::rootLabel() const {return _label;}

template <class T1, class T2>
inline T2 TreePath<T1,T2>::rootData() const {return _data;}


// TreePath<T1, T2>::Iterator Inlines
template <class T1, class T2>
inline TreePath<T1, T2>::Iterator::Iterator(const TreePath<T1, T2> *tree)
: elm::genstruct::Vector<TreePath<T1, T2> *>::Iterator(tree->_childs) {
	ASSERT(tree);
}

template <class T1, class T2>
inline TreePath<T1,T2>::Iterator::Iterator(const TreePath<T1, T2>::Iterator& iter)
: elm::genstruct::Vector<TreePath<T1, T2> *>::Iterator(iter) {
}

} } //namespace otawa::tsim

#endif /*OTAWA_IPET_TSIM_TREEPATH_H*/
