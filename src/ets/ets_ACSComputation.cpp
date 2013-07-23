/*
 *	ACSComputation class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
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
#include <otawa/ets/features.h>
#include <otawa/ast.h>
#include <otawa/ets/ACSComputation.h>
#include <otawa/instruction.h>
#include <otawa/hard/CacheConfiguration.h>

//#define AC_OUT(txt) txt	//with debuging
#define AC_OUT(txt)	//without debuging

namespace otawa { namespace ets {

using namespace ast;

/**
 * @class ACSComputation
 * This processor is used to simule cache states. 
 * <br>Currently: 
 * 	- One level, 
 * 	- Direct mapped, FullyAssociative and SetAssociative, 
 * 	- None and LRU.<br><br>
 * @remarks For each node I have adapted the Muller's Algorithm to calculate Cache States.
 *
 * @p Provided Features
 * @li @ref ACS_FEATURE
 *
 * @p Required Features
 * @li @ref hard::CACHE_CONFIGURATION_FEATURE
 */


p::declare ACSComputation::reg = p::init("otawa::ets::ACSComputation", Version(1, 2, 0))
	.base(ASTProcessor::reg)
	.maker<ACSComputation>()
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.provide(ACS_FEATURE);


/**
 */
ACSComputation::ACSComputation(p::declare& r): ASTProcessor(r) {
}


/** 
 * @var ACSComputation::cache_line_length;
 * It is the length of the current cache line, that is to say the number of l-blocks in this line.
 */

/** 
 * @var ACSComputation::cache_size;
 * It is the cache size, that is to say the number of cache lines.
 */


/**
 */
void ACSComputation::processWorkSpace(WorkSpace *ws) {
	const hard::CacheConfiguration *caches = hard::CACHE_CONFIGURATION(ws);
	if(!caches->hasInstCache())
		throw ProcessorException(*this, "no instruction cache");
	if(caches->isUnified())
		throw ProcessorException(*this, "unified cache unsupported");
	cache_size = caches->instCache()->rowCount();
	cache_line_length = 0;
	ASTProcessor::processWorkSpace(ws);
}


/**
 */
ACSComputation::~ACSComputation(void) {
}


/**
 * @fn void ACSComputation::processAST(FrameWork *ws, AST *ast);
 * Process each cache line.
 * @param ws	Container framework.
 * @param ast	AST to process.
 */	
void ACSComputation::processAST(WorkSpace *ws, AST *ast) {
	for(int j=0;j<cache_size;j++){
		AbstractCacheState *acs= new AbstractCacheState(j);
		AC_OUT(cout <<"||||||- "<<j<<" -||||||\n");
		initialization(ws, ast, acs);
		AC_OUT(cout <<"|| length["<<j<<"] : "<<cache_line_length<<'\n');
		AC_OUT(cout <<"|| htable["<<j<<"] : "<<acs->htable.count()<<'\n');
		if (cache_line_length != 0){
			for(int i = 0; i<ws->cache().instCache()->wayCount(); i++){
				acs->cache_state.add(new BitVector(cache_line_length));
			}
			/*AbstractCacheState::AbstractCacheState *tmp =*/ applyProcess(ws, ast, acs);
			cache_line_length = 0;
		}
	}
}

/**
 * @fn void ACSComputation::initialization(FrameWork *ws, AST *ast, AbstractCacheState *acs);
 * Initialize the abstract cache.
 * @param ws	Container framework.
 * @param ast	AST to process.
 * @param acs	AbstractCacheState of preceding nodes.
 */
void ACSComputation::initialization(WorkSpace *ws, AST *ast, AbstractCacheState *acs) {
	if (HITS(ast) == -1){
		HITS(ast) = 0;
		MISSES(ast) = 0;
		FIRST_MISSES(ast) = 0;
		CONFLICTS(ast) = 0;
	}
	
	switch (ast->kind()){
		case AST_Call:	{	
			ASTInfo *ast_info = ws->getASTInfo();
			Option< FunAST *> fun_res = ast_info->get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				initialization(ws, fun_ast, acs);
			}
			break;
		}
		case AST_Block: {
			address_t last = ast->toBlock()->block()->address() + ast->toBlock()->size();
			int same_lblock = 0;
			for(Inst *inst = ast->toBlock()->block(); inst->address() < last; inst = inst->nextInst()){
				if(ws->cache().hasInstCache() && !ws->cache().isUnified()) {
					int which_line = ws->cache().instCache()->line(inst->address());
					if ((which_line == acs->cache_line)&&(!acs->htable.exists(inst->address()))){
						if (same_lblock == 0 ){
							AC_OUT(cout <<"|| "<< inst->address() <<" ~ "<< which_line <<" indice : "<<cache_line_length<<'\n');
							acs->htable.put(inst->address(), cache_line_length);
							cache_line_length++;
						}
						if (same_lblock < (int)(ws->cache().instCache()->blockSize()/inst->size())-1)
							same_lblock++;
						else 
							same_lblock = 0;
					}
				}
			}
			break;
		}
		case AST_Seq: 	
			initialization(ws, ast->toSeq()->child1(), acs);
			initialization(ws, ast->toSeq()->child2(), acs);
			break;
		case AST_If:
			initialization(ws, ast->toIf()->condition(), acs);
			initialization(ws, ast->toIf()->thenPart(), acs);
			initialization(ws, ast->toIf()->elsePart(), acs);
			break;
		case AST_While:
			initialization(ws, ast->toWhile()->condition(), acs);
			initialization(ws, ast->toWhile()->body(), acs);
			break;
		case AST_For:		
			initialization(ws, ast->toFor()->initialization(), acs);
			initialization(ws, ast->toFor()->condition(), acs);
			initialization(ws, ast->toFor()->body(), acs);
			initialization(ws, ast->toFor()->incrementation(), acs);
			break;
		case AST_DoWhile:	
			initialization(ws, ast->toDoWhile()->body(), acs);
			initialization(ws, ast->toDoWhile()->condition(), acs);
			break;
		default :
			break;
	}
}

/**
 * @fn AbstractCacheState * ACSComputation::applyProcess(FrameWork *ws, AST *ast, AbstractCacheState *state);
 * Simulate the cache and put of each AST node its input AbstractCacheState.
 * @param ws	Container framework.
 * @param ast	AST to process.
 * @param state	AbstractCacheState of preceding nodes.
 * @return AbstractCacheState of the current AST.
 * @remarks I use in this method the C. Ferdinand's algorithme (Update - Must) to load one l-block in the ACS. 
 */
AbstractCacheState * ACSComputation::applyProcess(WorkSpace *ws, AST *ast, AbstractCacheState *state){
	switch (ast->kind()){
		case AST_Call:{	
			AC_OUT(cout << ".:Call : "<< " :.\n");
			ASTInfo *ast_info = ws->getASTInfo();
			Option< FunAST *> fun_res = ast_info->get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				AbstractCacheState *acs = new AbstractCacheState(state);
				ACS(ast->toCall()) = acs;
				
				//Algorithme to calculate cache state for Call.
				acs->assignment(applyProcess (ws, fun_ast, state));
				
				return acs;
			}
			else
				return state;
		}
		case AST_Block: {
			AC_OUT(cout << ".:Block : "<< ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ")<<" :.\n");
			AbstractCacheState *acs = new AbstractCacheState(state);
			ACS(ast->toBlock()) = acs;
			address_t last = ast->toBlock()->block()->address() + ast->toBlock()->size();
			for(Inst *inst = ast->toBlock()->block(); inst->address() < last; inst = inst->nextInst()){
				if(ws->cache().hasInstCache() && !ws->cache().isUnified()) {
					if (acs->htable.exists(inst->address())){
							
						AC_OUT(cout<<"|| en entree : "<<"\n";
						for(int j=0;j<acs->cache_state.length();j++){
							cout<<"||--><"<<j<<">";
							for(int i=0;i<acs->cache_state[j]->size();i++){
								cout<<acs->cache_state[j]->bit(i)<<' ';
							}
							cout<<"\n";
						}
						cout<<'\n');
							
						//Ferdinand's algorithme (Update - Must).
						bool stop = false;
						genstruct::Vector<BitVector *> cache_tmp;
						cache_tmp.add(new BitVector(acs->cache_state[0]->size()));
						cache_tmp[0]->set(acs->htable.get(inst->address(), -1), true);
						for(int x=0;x<acs->cache_state.length();x++){
							if(!stop){
								if(acs->cache_state[x]->bit(acs->htable.get(inst->address(), -1))){
									acs->cache_state[x]->set(acs->htable.get(inst->address(), -1), false);
									cache_tmp[x]->applyOr(*acs->cache_state[x]);
									stop = true;
								}
								else{
									if(x<acs->cache_state.length()-1){
										cache_tmp.add(new BitVector(*acs->cache_state[x]));
									}
								}
							}
							else{
								cache_tmp.set(x, acs->cache_state[x]);
							}
						}
						for(int y=0;y<acs->cache_state.length();y++){
							acs->cache_state.set(y, new BitVector(*cache_tmp.get(y)));
						}
							
						//Muller's categorisations in 1994.
						if(stop==true){
							if(!acs->byConflict()){
								if(!acs->hcat.exists(inst->address())){
									//ALWAYS_HIT.
									acs->hcat.put(inst->address(), AbstractCacheState::ALWAYS_HIT);
									HITS(ast->toBlock()) = HITS(ast->toBlock()) +1;
									AC_OUT(	cout << "|| hit avec "<<inst->address()<<"\n";
											cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toBlock()->get<int>(ETS::ID_HITS, -2)<< '\n');
								}
								else{
									if((acs->hcat.get(inst->address(), -1) == AbstractCacheState::ALWAYS_MISS)&&(MISSES(ast->toBlock()) > 0)){
										//FIRST_MISS.
										MISSES(ast->toBlock()) = MISSES(ast->toBlock()) - 1;
										acs->hcat.remove(inst->address());
										acs->hcat.put(inst->address(), AbstractCacheState::FIRST_MISS);
										FIRST_MISSES(ast->toBlock()) = FIRST_MISSES(ast->toBlock()) + 1;
										AC_OUT(	cout << "|| first_miss avec "<<inst->address()<<"\n";
												cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toBlock()->get<int>(ETS::ID_FIRST_MISSES, -2)<< '\n');
									}	
								}
							}
							else{
								//CONFLICT.
								acs->hcat.put(inst->address(), AbstractCacheState::CONFLICT);
								CONFLICTS(ast->toBlock()) = CONFLICTS(ast->toBlock()) + 1;
								AC_OUT(	cout << "|| conflict avec "<<inst->address()<<"\n");
							}
						}
						else{
							//ALWAYS_MISS
							if(!acs->hcat.exists(inst->address())){
								acs->hcat.put(inst->address(), AbstractCacheState::ALWAYS_MISS);
								
								MISSES(ast->toBlock()) = MISSES(ast->toBlock()) + 1;
								AC_OUT(	cout << "|| miss avec "<<inst->address()<<"\n";
										cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de miss : " << ast->toBlock()->get<int>(ETS::ID_MISSES, -2)<< '\n';
										cout << "|| -rajout de :"<< inst->address()<<" a lindex : "<<acs->htable.get(inst->address(), -1)<<"\n");
								}
						}
							
						AC_OUT(cout<<"|| en sortie : "<<"\n";
						for(int j=0;j<acs->cache_state.length();j++){
							cout<<"||--><"<<j<<">";
							for(int i=0;i<acs->cache_state[j]->size();i++){
								cout<<acs->cache_state[j]->bit(i)<<' ';
							}
							cout<<"\n";
						}
						cout<<'\n');
						
					}
				}
			}
			return acs;
			break;
		}
		case AST_Seq: {	
			AC_OUT(cout << ".:Seq : "<< " :.\n");
			AbstractCacheState *acs = new AbstractCacheState(state);
			//cout <<"test1\n";
			ACS(ast->toSeq()) = acs;
			//cout <<"test2\n";
			
			//Algorithme to calculate cache state for Seq.
			acs->assignment(applyProcess (	ws,
											ast->toSeq()->child2(), 
											applyProcess (	ws,
															ast->toSeq()->child1(), 
															state)));
			
			return acs;
			break;
		}
		case AST_If: {
			AC_OUT(cout << ".:If : "<< " :.\n");
			AbstractCacheState *acs = new AbstractCacheState(state);
			ACS(ast->toIf()) = acs;
			
			//Algorithme to calculate cache state for If.
			AbstractCacheState *tmp = new AbstractCacheState(applyProcess (ws, ast->toIf()->condition(), state));
			if(ast->toIf()->elsePart()->kind() != AST_Nop)
				acs->join(	applyProcess (	ws,
											ast->toIf()->thenPart(), 
											tmp),
							applyProcess (	ws,
				 							ast->toIf()->elsePart(), 
				 							tmp));
			else
				acs->assignment(	applyProcess (	ws,
													ast->toIf()->thenPart(), 
													tmp));
			
			return acs;
			break;
		}
		case AST_While:	{
			AC_OUT(cout << ".:While : "<< " :.\n");
			int N = LOOP_COUNT(ast->toWhile()); 
			int i = 0;
			bool is_start = true;
			AbstractCacheState *acs = new AbstractCacheState(state);
			ACS(ast->toWhile()) = acs;
			
			//Algorithme to calculate cache state for While.
			acs->assignment(applyProcess (ws, ast->toWhile()->condition(), state));
			AbstractCacheState *tmp = new AbstractCacheState(state);
			while (((acs->areDifferent(tmp))||(is_start))&&(i < N)){
				tmp->assignment(acs);
				acs->assignment(applyProcess (	ws,
												ast->toWhile()->condition(), 
												applyProcess (ws, ast->toWhile()->body(), acs)));
				is_start = false;
				i++;
			}
			
			return acs;
			break;
		}
		case AST_For:	{	
			AC_OUT(cout << ".:For : "<< " :.\n");
			int N = LOOP_COUNT(ast->toFor()); 
			int i = 0;
			bool is_start = true;
			AbstractCacheState *acs = new AbstractCacheState(state);
			ACS(ast->toFor()) = acs;
			
			//Algorithme to calculate cache state for For.
			acs->assignment(applyProcess (	ws,
											ast->toFor()->condition(),
											applyProcess (ws, ast->toFor()->initialization(), state)));
			AbstractCacheState *tmp = new AbstractCacheState(state);
			while (((acs->areDifferent(tmp))||(is_start))&&(i < N)){
				tmp->assignment(acs);
				acs->assignment(applyProcess (	ws,
												ast->toFor()->condition(),
												applyProcess (	ws,
																ast->toFor()->incrementation(), 
																applyProcess (ws, ast->toFor()->body(), acs))));
				is_start = false;
				i++;
			}
			
			return acs;
			break;
		}
		case AST_DoWhile:	{
			AC_OUT(cout << ".:DoWhile : "<< " :.\n");
			bool is_start = true;
			int N = LOOP_COUNT(ast->toDoWhile()); 
			int i = 0;
			AbstractCacheState *acs = new AbstractCacheState(state);
			ACS(ast->toDoWhile()) = acs;
			
			//Algorithme to calculate cache state for DoWhile.
			acs->assignment(applyProcess (ws, ast->toDoWhile()->body(), state));
			AbstractCacheState *tmp = new AbstractCacheState(state);
			while (((acs->areDifferent(tmp))||(is_start))&&(i < N)){
				tmp->assignment(acs);
				acs->assignment(applyProcess (	ws,
												ast->toDoWhile()->body(), 
												applyProcess (ws, ast->toDoWhile()->condition(), acs)));
				is_start = false;
				i++;
			}
			
			return acs;
			break;
		}
		default :{
			AC_OUT(cout << ".:Default : "<< " :.\n");
			AbstractCacheState *tmp = new AbstractCacheState(state->cache_line);
			tmp->cache_state[0] = new BitVector(cache_line_length);
			return tmp;
		}
	}
}

} }// otawa::ets
