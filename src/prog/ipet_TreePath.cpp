/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/prog/TreePath.cpp -- implementation of tree holding sequences.
 */
#include <otawa/tsim/TreePath.h>

namespace otawa { namespace tsim {

/**
 * @author G. Cavaignac
 * @class TreePath
 * This class is used for holding unique sequences in a tree so that it is
 * very fast to retrieve the matched sequence with a list of elements
 * @param T1 element of the sequence
 * @param T2 type holding the data of the sequence
 */

/**
 * @fn TreePath::TreePath(int max_childs = DEFAULT_MAX_CHILDS);
 * Builds a new empty TreePath
 * @param max_childs for speed only: gives this initial capacity for the size of the vector holding childs
 */

/**
 * @fn TreePath::TreePath(const T1 label, const T2 data, int max_childs = 2);
 * Builds a new TreePath with initial values for its root
 * @param label element of the sequence
 * @param data data attached to the sequence [label] (length=1)
 * @param max_childs for speed only: gives this initial capacity for the size of the vector holding childs
 */

/**
 * @fn TreePath::TreePath(elm::genstruct::Vector<T1> &path, const T2 data, int max_childs = 2);
 * Builds a new TreePath with initial values for an initial path
 * @param path first path in the tree
 * @param data data attached to the path
 * @param max_childs for speed only: gives this initial capacity for the size of the vector holding childs
 */

/**
 * @fn TreePath::TreePath(elm::genstruct::Vector<T1> *path, const T2 data, int max_childs = 2);
 * Builds a new TreePath with initial values for an initial path
 * @param path first path in the tree
 * @param data data attached to the path
 * @param max_childs for speed only: gives this initial capacity for the size of the vector holding childs
 */

/**
 * @fn TreePath::~TreePath();
 * Destroys all his childs
 */

/**
 * @fn T1 TreePath::rootLabel() const;
 * @return label for the root
 * @attention If we don't use the root, this function gives a meaningless value
 */

/**
 * @fn T2 TreePath::rootData() const;
 * @return label for sequence (length=1) attached to the root.
 * @attention If we don't use the root, this function gives a meaningless value
 */

/**
 * @fn elm::Option<T2> TreePath::get(const elm::genstruct::Vector<T1> &path, int from_index = 0);
 * Gives an optional value attached to the path given
 * @param path path searched
 * @param from_index if we don't want to start searching the path with the index 0 of the Vector
 * @return optional value. If this value exists, the path have a data attached
 */

/**
 * @fn void TreePath::add(elm::genstruct::Vector<T1> &path, const T2 data, int from_index = 0);
 * Adds a value attached to the path given
 * @param path the path wich we want to attach some data
 * @param data data to attach to the path
 * @param from_index if we don't want to start searching the path with the index 0 of the Vector
 */

/**
 * @fn void TreePath::add(elm::genstruct::Vector<T1> *path, const T2 data, int from_index = 0);
 * Adds a value attached to the path given
 * @param path the path wich we want to attach some data
 * @param data data to attach to the path
 * @param from_index if we don't want to start searching the path with the index 0 of the Vector
 */

} } // namespace otawa::ipet

/* Example
#include <iostream>
#include <elm/genstruct/Vector.h>
using namespace elm::genstruct;

enum Direction {LEFT, RIGHT};

int main(){
	Vector<enum Direction> vect;
	
	vect.add(LEFT); // the root of this tree : LEFT
	vect.add(RIGHT);
	vect.add(RIGHT);
	vect.add(RIGHT);
	TreePath<enum Direction,char*> tree1(vect,"The Green House");
	
	vect.clear();
	// We use the same tree with root=LEFT
	vect.add(RIGHT);
	vect.add(LEFT);
	vect.add(RIGHT);
	tree1.add(vect,"The Blue House");
	
	// Now we search the houses
	vect.clear();
	// We use the same tree with root=LEFT
	vect.add(RIGHT);
	vect.add(RIGHT);
	vect.add(RIGHT);
	if(tree1.get(vect))
		std::cout << "< > > > : " << *tree1.get(vect) << '\n';
	
	vect.clear();
	// We use the same tree with root=LEFT
	vect.add(RIGHT);
	vect.add(LEFT);
	vect.add(RIGHT);
	if(tree1.get(vect))
		std::cout << "< > < > : " << *tree1.get(vect) << '\n';

	// If you don't want to use the root of the tree, it's the same:
	vect.clear();
	TreePath<enum Direction, char*> tree2;
	vect.add(LEFT); // the root of this tree : LEFT
	vect.add(RIGHT);
	vect.add(RIGHT);
	vect.add(RIGHT);
	tree2.add(vect,"The Green House");

	vect.clear();
	vect.add(LEFT);
	vect.add(RIGHT);
	vect.add(LEFT);
	vect.add(RIGHT);
	tree2.add(vect,"The Blue House");

	// Now we search the houses
	vect.clear();
	vect.add(LEFT);
	vect.add(RIGHT);
	vect.add(RIGHT);
	vect.add(RIGHT);
	if(tree2.get(vect))
		std::cout << "< > > > : " << *tree2.get(vect) << '\n';
	
	vect.clear();
	vect.add(LEFT);
	vect.add(RIGHT);
	vect.add(LEFT);
	vect.add(RIGHT);
	if(tree2.get(vect))
		std::cout << "< > < > : " << *tree2.get(vect) << '\n';
	
	return 0;
}*/

