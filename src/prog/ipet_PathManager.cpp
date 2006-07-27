/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/ipet/PathManager.cpp -- PathManager class implementation.
 */
#include <otawa/ipet/PathManager.h>

using namespace elm::genstruct;

namespace otawa { namespace ipet {

/**
 * Returns the path of length 1 composed by only the given basic block.
 * If this path doesn't exist, creates it.
 * @param start basic block
 * @return BBPath corresponding to the given basic block
 */
BBPath *PathManager::getBBPath(BasicBlock *start){
	TreePath<BasicBlock*,BBPath*> *tree = ID_Tree(start);
	if(!tree){
		BBPath *bbp = new BBPath(this, start); 
		tree = new TreePath<BasicBlock*,BBPath*>(start,bbp);
		ID_Tree(start) = tree;
		return bbp;
	}
	return tree->rootData();
}



/**
 * Returns the path composed by the given path.
 * If this path doesn't exist, creates it.
 * @param path vector of basic blocks composing the path
 * @return BBPath corresponding to the given path
 */
BBPath *PathManager::getBBPath(Vector<BasicBlock*> *path){
	assert(path);
	BasicBlock *bb = path->get(0);
	BBPath *bbp;
	TreePath<BasicBlock*,BBPath*> *tree = ID_Tree(bb);
	if(!tree){
		bbp = new BBPath(this, path);
		tree = new TreePath<BasicBlock*,BBPath*>(*path,bbp);
		ID_Tree(bb) = tree;
		return bbp;
	}
	elm::Option<BBPath*> option = tree->get(*path,1);
	if(!option){
		bbp = new BBPath(this, path);
		tree->add(path,bbp,1);
		return bbp;
	}
	return *option;
}

/**
 * This identifier is used for storing in a BasicBlock a TreePath
 * storing all BBPath starting from this basic block
 */
GenericIdentifier<TreePath<BasicBlock*,BBPath*>*> PathManager::ID_Tree("pathmanager.tree",0);

} }

