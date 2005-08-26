/*
 *	$$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_ACSComputation.cpp -- ACSComputation class implementation.
 */

#include <otawa/ets/ETS.h>
#include <otawa/ast.h>
#include <otawa/ets/ACSComputation.h>
#include <otawa/instruction.h>

#define AC_OUT(txt) txt	//with debuging
//#define AC_OUT(txt)	//without debuging

namespace otawa { namespace ets {

/**
 * @class ACSComputation
 * This processor is used to simule cache states. 
 * <br>Currently: 
 * 	- One level, 
 * 	- Direct mapped, FullyAssociative and SetAssociative, 
 * 	- None and LRU.<br><br>
 * @remarks For each node I have adapted the Muller's Algorithme to calculate Cache States:
 */
 
/** 
 * @var ACSComputation::cache_line_length;
 * It is the length of the current cache line, that is to say the number of l-blocks in this line.
 */

/** 
 * @var ACSComputation::cache_size;
 * It is the cache size, that is to say the number of cache lines.
 */

/**
 * @fn void ACSComputation::processAST(FrameWork *fw, AST *ast);
 * Process each cache line.
 * @param fw	Container framework.
 * @param ast	AST to process.
 */	
void ACSComputation::processAST(FrameWork *fw, AST *ast) {
	for(int j=0;j<cache_size;j++){
		AbstractCacheState::AbstractCacheState *acs= new AbstractCacheState::AbstractCacheState(j);
		AC_OUT(cout <<"||||||- "<<j<<" -||||||\n");
		initialization(fw, ast, acs);
		AC_OUT(cout <<"|| length["<<j<<"] : "<<cache_line_length<<'\n');
		AC_OUT(cout <<"|| htable["<<j<<"] : "<<acs->htable.count()<<'\n');
		if (cache_line_length != 0){
			for(int i = 0; i<fw->caches().get(0)->wayCount(); i++){
				acs->cache_state.add(new BitVector(cache_line_length));
			}
			AbstractCacheState::AbstractCacheState *tmp=applyProcess(fw, ast, acs);
			cache_line_length = 0;
		}
	}
}

/**
 * @fn void ACSComputation::initialization(FrameWork *fw, AST *ast, AbstractCacheState *acs);
 * Initialize the abstract cache.
 * @param fw	Container framework.
 * @param ast	AST to process.
 * @param acs	AbstractCacheState of preceding nodes.
 */
void ACSComputation::initialization(FrameWork *fw, AST *ast, AbstractCacheState *acs) {
	if (ast->get<int>(ETS::ID_HITS, -1) == -1){
		ast->set<int>(ETS::ID_HITS,0);
		ast->set<int>(ETS::ID_MISSES,0);
		ast->set<int>(ETS::ID_FIRST_MISSES,0);
		ast->set<int>(ETS::ID_CONFLICTS,0);
	}
	
	switch (ast->kind()){
		case AST_Call:	{	
			ASTInfo *ast_info = fw->getASTInfo();
			Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				initialization(fw, fun_ast, acs);
			}
		}
		case AST_Block: {
			address_t last = ast->toBlock()->block()->address() + ast->toBlock()->size();
			int same_lblock = 0;
			for(Inst *inst = ast->toBlock()->block(); inst->address() < last; inst = inst->next()){
				switch (fw->caches().count()){
					case 1 : 
						//L1
						int which_line = fw->caches().get(0)->line(inst->address());
						if ((which_line == acs->cache_line)&&(!acs->htable.exists(inst->address()))){
							if (same_lblock == 0 ){
								AC_OUT(cout <<"|| "<< inst->address() <<" ~ "<< which_line <<" indice : "<<cache_line_length<<'\n');
								acs->htable.put(inst->address(), cache_line_length);
								cache_line_length++;
							}
							if (same_lblock < (fw->caches().get(0)->blockSize()/inst->size())-1)
								same_lblock++;
							else 
								same_lblock = 0;
						}
						break;
					default :
						;//L2, L3 ...
				}
			}
			break;
		}
		case AST_Seq: 	
			initialization(fw, ast->toSeq()->child1(), acs);
			initialization(fw, ast->toSeq()->child2(), acs);
			break;
		case AST_If:
			initialization(fw, ast->toIf()->condition(), acs);
			initialization(fw, ast->toIf()->thenPart(), acs);
			initialization(fw, ast->toIf()->elsePart(), acs);
			break;
		case AST_While:
			initialization(fw, ast->toWhile()->condition(), acs);
			initialization(fw, ast->toWhile()->body(), acs);
			break;
		case AST_For:		
			initialization(fw, ast->toFor()->initialization(), acs);
			initialization(fw, ast->toFor()->condition(), acs);
			initialization(fw, ast->toFor()->body(), acs);
			initialization(fw, ast->toFor()->incrementation(), acs);
			break;
		case AST_DoWhile:	
			initialization(fw, ast->toDoWhile()->body(), acs);
			initialization(fw, ast->toDoWhile()->condition(), acs);
			break;
		default :
			;
	}
}

/**
 * @fn AbstractCacheState * ACSComputation::applyProcess(FrameWork *fw, AST *ast, AbstractCacheState *state);
 * Simulate the cache and put of each AST node its input AbstractCacheState.
 * @param fw	Container framework.
 * @param ast	AST to process.
 * @param state	AbstractCacheState of preceding nodes.
 * @return AbstractCacheState of the current AST.
 * @remarks I use in this method the C. Ferdinand's algorithme (Update - Must) to load one l-block in the ACS. 
 */
AbstractCacheState * ACSComputation::applyProcess(FrameWork *fw, AST *ast, AbstractCacheState *state){
	switch (ast->kind()){
		case AST_Call:{	
			AC_OUT(cout << ".:Call : "<< " :.\n");
			ASTInfo *ast_info = fw->getASTInfo();
			Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
			if (fun_res){
				AST *fun_ast = (*fun_res)->ast();
				AbstractCacheState::AbstractCacheState *acs = new AbstractCacheState::AbstractCacheState(state);
				ast->toCall()->set<AbstractCacheState *>(ETS::ID_ACS,acs);
				
				//Algorithme to calculate cache state for Call.
				acs->assignment(applyProcess (fw, fun_ast, state));
				
				return acs;
			}
			else
				return state;
		}
		case AST_Block: {
			AC_OUT(cout << ".:Block : "<< ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ")<<" :.\n");
			AbstractCacheState::AbstractCacheState *acs = new AbstractCacheState::AbstractCacheState(state);
			ast->toBlock()->set<AbstractCacheState *>(ETS::ID_ACS,acs);
			address_t last = ast->toBlock()->block()->address() + ast->toBlock()->size();
			for(Inst *inst = ast->toBlock()->block(); inst->address() < last; inst = inst->next()){
				switch (fw->caches().count()){
					case 1 : {
						//L1
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
										ast->toBlock()->set<int>(ETS::ID_HITS, ast->toBlock()->use<int>(ETS::ID_HITS)+1);
										AC_OUT(	cout << "|| hit avec "<<inst->address()<<"\n";
												cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de hit : " << ast->toBlock()->get<int>(ETS::ID_HITS, -2)<< '\n');
									}
									else{
										if((acs->hcat.get(inst->address(), -1) == AbstractCacheState::ALWAYS_MISS)&&(ast->toBlock()->use<int>(ETS::ID_MISSES) > 0)){
											//FIRST_MISS.
											ast->toBlock()->set<int>(ETS::ID_MISSES, ast->toBlock()->use<int>(ETS::ID_MISSES)-1);
											acs->hcat.remove(inst->address());
											acs->hcat.put(inst->address(), AbstractCacheState::FIRST_MISS);
											ast->toBlock()->set<int>(ETS::ID_FIRST_MISSES, ast->toBlock()->use<int>(ETS::ID_FIRST_MISSES)+1);
											AC_OUT(	cout << "|| first_miss avec "<<inst->address()<<"\n";
													cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"unknown ") << " a pour nb de first miss : " << ast->toBlock()->get<int>(ETS::ID_FIRST_MISSES, -2)<< '\n');
										}	
									}
								}
								else{
									//CONFLICT.
									acs->hcat.put(inst->address(), AbstractCacheState::CONFLICT);
									ast->toBlock()->set<int>(ETS::ID_CONFLICTS, ast->toBlock()->use<int>(ETS::ID_CONFLICTS)+1);
									AC_OUT(	cout << "|| conflict avec "<<inst->address()<<"\n");
								}
							}
							else{
								//ALWAYS_MISS
								if(!acs->hcat.exists(inst->address())){
									acs->hcat.put(inst->address(), AbstractCacheState::ALWAYS_MISS);
									
									ast->toBlock()->set<int>(ETS::ID_MISSES, ast->toBlock()->use<int>(ETS::ID_MISSES)+1);
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
						break;
					}
					default :
						;//L2, L3 ...
				}
			}
			return acs;
			break;
		}
		case AST_Seq: {	
			AC_OUT(cout << ".:Seq : "<< " :.\n");
			AbstractCacheState::AbstractCacheState *acs = new AbstractCacheState::AbstractCacheState(state);
			cout <<"test1\n";
			ast->toSeq()->set<AbstractCacheState *>(ETS::ID_ACS,acs);
			cout <<"test2\n";
			
			//Algorithme to calculate cache state for Seq.
			acs->assignment(applyProcess (	fw, 
											ast->toSeq()->child2(), 
											applyProcess (	fw, 
															ast->toSeq()->child1(), 
															state)));
			
			return acs;
			break;
		}
		case AST_If: {
			AC_OUT(cout << ".:If : "<< " :.\n");
			AbstractCacheState::AbstractCacheState *acs = new AbstractCacheState::AbstractCacheState(state);
			ast->toIf()->set<AbstractCacheState *>(ETS::ID_ACS,acs);
			
			//Algorithme to calculate cache state for If.
			AbstractCacheState::AbstractCacheState *tmp = new AbstractCacheState::AbstractCacheState(applyProcess (fw, ast->toIf()->condition(), state));
			if(ast->toIf()->elsePart()->kind() != AST_Nop)
				acs->join(	applyProcess (	fw, 
											ast->toIf()->thenPart(), 
											tmp),
							applyProcess (	fw, 
				 							ast->toIf()->elsePart(), 
				 							tmp));
			else
				acs->assignment(	applyProcess (	fw, 
													ast->toIf()->thenPart(), 
													tmp));
			
			return acs;
			break;
		}
		case AST_While:	{
			AC_OUT(cout << ".:While : "<< " :.\n");
			int N = ast->toWhile()->use<int>(ETS::ID_LOOP_COUNT); 
			int i = 0;
			bool is_start = true;
			AbstractCacheState::AbstractCacheState *acs = new AbstractCacheState::AbstractCacheState(state);
			ast->toWhile()->set<AbstractCacheState *>(ETS::ID_ACS,acs);
			
			//Algorithme to calculate cache state for While.
			acs->assignment(applyProcess (fw, ast->toWhile()->condition(), state));
			AbstractCacheState::AbstractCacheState *tmp = new AbstractCacheState::AbstractCacheState(state);
			while (((acs->areDifferent(tmp))||(is_start))&&(i < N)){
				tmp->assignment(acs);
				acs->assignment(applyProcess (	fw, 
												ast->toWhile()->condition(), 
												applyProcess (fw, ast->toWhile()->body(), acs)));
				is_start = false;
				i++;
			}
			
			return acs;
			break;
		}
		case AST_For:	{	
			AC_OUT(cout << ".:For : "<< " :.\n");
			int N = ast->toFor()->use<int>(ETS::ID_LOOP_COUNT); 
			int i = 0;
			bool is_start = true;
			AbstractCacheState::AbstractCacheState *acs = new AbstractCacheState::AbstractCacheState(state);
			ast->toFor()->set<AbstractCacheState *>(ETS::ID_ACS,acs);
			
			//Algorithme to calculate cache state for For.
			acs->assignment(applyProcess (	fw, 
											ast->toFor()->condition(),
											applyProcess (fw, ast->toFor()->initialization(), state)));
			AbstractCacheState::AbstractCacheState *tmp = new AbstractCacheState::AbstractCacheState(state);
			while (((acs->areDifferent(tmp))||(is_start))&&(i < N)){
				tmp->assignment(acs);
				acs->assignment(applyProcess (	fw, 
												ast->toFor()->condition(),
												applyProcess (	fw, 
																ast->toFor()->incrementation(), 
																applyProcess (fw, ast->toFor()->body(), acs))));
				is_start = false;
				i++;
			}
			
			return acs;
			break;
		}
		case AST_DoWhile:	{
			AC_OUT(cout << ".:DoWhile : "<< " :.\n");
			bool is_start = true;
			int N = ast->toDoWhile()->use<int>(ETS::ID_LOOP_COUNT); 
			int i = 0;
			AbstractCacheState::AbstractCacheState *acs = new AbstractCacheState::AbstractCacheState(state);
			ast->toDoWhile()->set<AbstractCacheState *>(ETS::ID_ACS,acs);
			
			//Algorithme to calculate cache state for DoWhile.
			acs->assignment(applyProcess (fw, ast->toDoWhile()->body(), state));
			AbstractCacheState::AbstractCacheState *tmp = new AbstractCacheState::AbstractCacheState(state);
			while (((acs->areDifferent(tmp))||(is_start))&&(i < N)){
				tmp->assignment(acs);
				acs->assignment(applyProcess (	fw, 
												ast->toDoWhile()->body(), 
												applyProcess (fw, ast->toDoWhile()->condition(), acs)));
				is_start = false;
				i++;
			}
			
			return acs;
			break;
		}
		default :{
			AC_OUT(cout << ".:Default : "<< " :.\n");
			AbstractCacheState::AbstractCacheState *tmp = new AbstractCacheState::AbstractCacheState(state->cache_line);
			tmp->cache_state[0] = new BitVector(cache_line_length);
			return tmp;
		}
	}
}

} }// otawa::ets
