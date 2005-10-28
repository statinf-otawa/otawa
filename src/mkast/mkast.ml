(* 
 *	$Id$
 *	Copyright (c) 2003, Hugues Cassé <hugues.casse@laposte.net>
 *
 *	Pretty printer of XML document.
 *)

open Frontc
open Cabs

type heptane_ast =
	  HEP_Vide
	| HEP_Code of string * string * string list
	| HEP_Seq of heptane_ast list
	| HEP_If of heptane_ast * heptane_ast * heptane_ast
	| HEP_While of heptane_ast * heptane_ast
	| HEP_DoWhile of heptane_ast * heptane_ast
	| HEP_For of heptane_ast * heptane_ast * heptane_ast * heptane_ast

exception UnsupportedExpression of expression

(* Options *)
let banner =
	"mkast V0.1 (04/15/05)\n" ^
	"Copyright (c) 2004, Hugues Cassé <casse@irit.fr>\n\n" ^
	"SYNTAX:\tmkast [options] files...\n" ^
	"\tmkast [options] --\n"
let args: parsing_arg list ref = ref []
let files: string list ref = ref []
let out_file = ref ""
let from_stdin = ref false


(* Options scanning *)
let opts = [
	("-o", Arg.Set_string out_file,
		"Output file for the AST.");
	("-pp", Arg.Unit (fun _ -> args := USE_CPP :: !args),
		"Preprocess the input files.");
	("-proc", Arg.String (fun cpp -> args := (PREPROC cpp) :: !args),
		"Use the given preprocessor");
	("-i", Arg.String (fun file -> args := (INCLUDE file) :: !args),
		"Include the given file.");
	("-I", Arg.String (fun dir -> args := (INCLUDE_DIR dir) :: !args),
		"Include retrieval directory");
	("-D", Arg.String (fun def -> args := (DEF def) :: !args),
		"Pass this definition to the preprocessor.");
	("-U", Arg.String (fun undef -> args := (UNDEF undef) :: !args),
		"Pass this undefinition to the preprocessor.");
	("--", Arg.Set from_stdin,
		"Takes input from standard input.");
]


let rec print_ast out ast =

	let print_label label =
		output_string out ("_" ^ label) in

	match ast with
	
	  HEP_Vide ->
	  	output_string out "Vide"
		
	| HEP_Code (start, stop, calls) ->
		output_string out "Code(";
		print_label start;
		List.iter 
			(fun call -> output_string out (", Appel{_" ^ call ^ "}"))
			calls;
		output_string out ", ";
		print_label stop;
		output_string out ")"
		
	| HEP_Seq asts ->
		output_string out "Seq [\n";
		let _ = List.fold_left (fun arg ast ->
				if arg then output_string out ";\n";
				print_ast out ast; true
			) false asts in
		output_string out "\n]\n"
		
	| HEP_If (cond, _then, _else) ->
		output_string out "If(";
		print_ast out cond;
		output_string out ", ";
		print_ast out _then;
		output_string out ", ";
		print_ast out _else;
		output_string out ")"
		
	| HEP_While (cond, body) ->
		output_string out "While([0, 0], ";
		print_ast out cond;
		output_string out ", ";
		print_ast out body;
		output_string out ")"
	
	| HEP_DoWhile (body, cond) ->
		output_string out "DoWhile([0, 0], ";
		print_ast out body;
		output_string out ", ";
		print_ast out cond;
		output_string out ")"
	
	| HEP_For (init, cond, inc, body) ->
		output_string out "For([0, 0], ";
		print_ast out init;
		output_string out ", ";
		print_ast out cond;
		output_string out ", ";
		print_ast out inc;
		output_string out ", ";
		print_ast out body;
		output_string out ")"


(**
 * Display a function in Heptane format.
 * @param out	Output channel.
 * @param funs	Functions to display.
 *)
let print_funs out funs =

	let print_fun (name, ast) =
		if name <> "" then
			begin
				output_string out ("_" ^ name ^ " = \n");
				print_ast out ast;
				output_string out "\n\n"
			end in

	List.iter print_fun funs

let rec scan_expression calls exp =
	match exp with
	  NOTHING
	| CONSTANT _
	| VARIABLE _
	| TYPE_SIZEOF _
	| EXPR_SIZEOF _ ->
	  	calls
	| UNARY (_, exp) ->
		scan_expression calls exp
	| BINARY (_, exp1, exp2) ->
		scan_expression (scan_expression calls exp1) exp2
	| QUESTION (exp1, exp2, exp3) ->
		scan_expression (scan_expression (scan_expression calls exp1) exp2) exp3
	| CAST (_, exp) ->
		scan_expression calls exp
	| CALL (VARIABLE id, args) ->
		id::(List.fold_left scan_expression calls args)
	| CALL (call, args) ->
		List.fold_left scan_expression calls (call::args)
	| COMMA exps ->
		List.fold_left scan_expression calls exps
	| INDEX (exp1, exp2) ->
		scan_expression (scan_expression calls exp1) exp2
	| MEMBEROF (exp, _) ->
		scan_expression calls exp
	| MEMBEROFPTR (exp, _) ->
		scan_expression calls exp
	| GNU_BODY _ ->
		raise (UnsupportedExpression exp)
		

(**
 * Rebuid the statement with ASM inserted.
 * @param fname	Function name.
 * @param stat	Statement ro process.
 * @param cnt	Counter to use.
 * @return		(modified FrontC tree, Heptane AST)
 *)
let rec process_statement fname stat cnt =

	let new_label _ =
		let res = "__mkast_" ^ (string_of_int !cnt) ^ ":" in
		cnt := !cnt + 1;
		res in
	
	let before lab ast =
		SEQUENCE (ASM lab, ast) in	
	
	let after ast lab =
		SEQUENCE (ast, ASM lab) in	
	
	let sequence stat1 stat2 =
		if stat1 = NOP then stat2 else
		if stat2 = NOP then stat1 else
		SEQUENCE(stat1, stat2) in

	let rec flatten stat =
		match stat with
		  SEQUENCE (stat1, stat2) ->
			List.append (flatten stat1) (flatten stat2)
		| BLOCK ([], stat) ->
			flatten stat
		| _ ->
			[stat] in

	let rec is_first_loop stat =
		match stat with
		  SEQUENCE (stat, _) -> is_first_loop stat
		| BLOCK ([], stat) -> is_first_loop stat
		| WHILE (_, _)
		| DOWHILE (_, _)
		| FOR (_, _, _, _) -> true
		| _ -> false in

	let rec currify cur =
		match cur with
		  [] -> NOP
		| NOP::stats -> currify stats
		| [stat] -> stat
		| stat::stats -> sequence stat (currify stats) in

	let hep_code start stop exp =
		HEP_Code (start, stop, List.rev (scan_expression [] exp)) in
	
	let hep_seq heps =
		match heps with
		  [] -> HEP_Vide
		| [hep] -> hep
		| _ -> HEP_Seq heps in

	let rec trans_seq start stop new_stat asts code calls stats =

		match stats with
		
		  [] ->
		  	let asts' =
				if not code then asts else
				(HEP_Code (start, stop, List.rev calls))::asts in
		  	(new_stat, hep_seq (List.rev asts'))
			
		| (COMPUTATION exp)::tl ->
			trans_seq start stop (sequence new_stat (List.hd stats))
				asts true (scan_expression calls exp) tl

		| NOP::tl ->
			trans_seq start stop (sequence new_stat (List.hd stats))
				asts true calls tl
		
		| [BREAK]
		| [CONTINUE]
		| [RETURN _] ->
			trans_seq start stop (sequence new_stat (List.hd stats)) asts
			true calls []
		
		| BREAK::_
		| CONTINUE::_
		| (RETURN _)::_ ->
			let lab = new_label () in
			trans_seq lab stop (sequence new_stat (after (List.hd stats) lab))
				((HEP_Code (start, lab, List.rev calls))::asts) false [] (List.tl stats)
		
		| [hd] ->
			let (new_stat', asts', start') =
				if not code then (new_stat, asts, start)
				else
					let lab = new_label () in
					(after new_stat lab,
					HEP_Code (start, lab, List.rev calls)::asts, lab) in
			let (hd_stat, hd_ast) = trans start' stop hd in
			(sequence new_stat' hd_stat, hep_seq (List.rev (hd_ast::asts')))
			
		| hd::tl ->
			let (new_stat', asts', start') =
				if not code then (new_stat, asts, start)
				else
					let lab = new_label () in
					(after new_stat lab,
					HEP_Code (start, lab, List.rev calls)::asts, lab) in
			let stop' = new_label () in
			let (hd_stat, hd_ast) = trans start' stop' hd in
			trans_seq stop' stop 
				(currify [new_stat'; hd_stat; (ASM stop')])
				(hd_ast::asts') false [] tl
		
	and trans start stop stat =
	
		match stat with
		
		  NOP ->
		  	(stat, HEP_Vide)
			
		| COMPUTATION exp ->
			(stat, hep_code start stop exp)
			
		| BLOCK (defs, stat) ->
			let (stat', ast) = trans start stop stat in
			(BLOCK (defs, stat'), ast) 
			
		| IF (cond, _then, _else) ->
			let then_label = new_label () in
			let else_label = new_label () in
			let (_then', then_ast) = trans then_label else_label _then in
			let (_else', else_ast) = trans else_label stop _else in
			(IF (cond, before then_label _then', before else_label _else'),
			HEP_If (hep_code start then_label cond, then_ast, else_ast))
		
		| WHILE (cond, body) ->
			let body_label = new_label () in
			let (body', ast) = trans body_label stop body in
			(WHILE(cond, before body_label body'),
			HEP_While (hep_code start body_label cond, ast))

		| DOWHILE (cond, body) ->
			let cond_label = new_label () in
			let (body', ast) = trans start cond_label body in
			(DOWHILE(cond, after body' cond_label),
			HEP_DoWhile (ast, hep_code cond_label stop cond)) 

		| FOR (init, cond, inc, body) ->
			let (ast_init, hep_init, cond_label) =
				if init = NOTHING then (NOP, HEP_Vide, start) else
				let cond_label = new_label () in
				(after (COMPUTATION init) cond_label,
				hep_code start cond_label init, cond_label) in
			let (ast_inc, hep_inc, inc_label) =
				if inc = NOTHING then (NOP, HEP_Vide, stop) else
				let inc_label = new_label () in
				(before inc_label (COMPUTATION inc),
				hep_code inc_label stop inc, inc_label) in
			let ast_cond =
				if cond = NOTHING
				then CONSTANT (CONST_INT "1")
				else cond in
			let body_label = new_label () in
			let (ast_body, hep_body) = trans body_label inc_label body in
			let hep_cond = hep_code cond_label body_label cond in
			let ast_loop = 
				FOR (NOTHING, ast_cond, NOTHING,
					sequence (before body_label ast_body) ast_inc) in
			let hep_loop = 
				HEP_For (hep_init, hep_cond, hep_body, hep_inc) in
			if ast_init = NOP then (ast_loop, hep_loop)
			else (sequence ast_init ast_loop, hep_loop)
			
		| SEQUENCE (_, _) ->
			trans_seq start stop NOP [] false [] (flatten stat)
		| _ ->
			(stat, HEP_Code (start, stop, [])) in

	let first_loop = is_first_loop stat in
	let start = if first_loop then new_label() else fname ^ ":" in
	let stop = new_label () in	
	let (stat, ast) = trans start stop stat in
	if first_loop
	then
		(currify [ASM start; stat; (ASM stop)],
		HEP_Seq [HEP_Code (fname ^ ":", start, []); ast])
	else
		(currify [stat; (ASM stop)], ast)


(**
 * Process the file, that is, transform functions and output them.
 *  @param file		File to process.
 *  @param output	Used output.
 *)
let process file opts output: unit =

	let cnt = ref 0 in
	let path =
		let len = String.length file in
		if len < 2 or (String.sub file (len - 2) 2) <> ".c"
		then file
		else String.sub file 0 (len - 2) in
	let cout = open_out (path ^ ".ast.c") in

	let trans def =
		match def with
		  FUNDEF (name,(defs, stat)) ->
			let (_, _, (string_name, _, _, _)) = name in
		  	let (stat, ast) = process_statement string_name stat cnt in
			(FUNDEF (name, (defs, stat)), (string_name, ast))
		| _ ->
			(def, ("", HEP_Vide)) in

	(match Frontc.parse ((FROM_FILE file)::opts) with
	  PARSING_ERROR -> 
	  	failwith "Stopped."
	| PARSING_OK file ->
		(*Cprint.print stdout file;*)
		let (defs, funs) = List.split (List.map trans
			(Frontc.trans_old_fun_defs file)) in
		Cprint.print cout defs;
		print_funs output funs);
	
	close_out cout



(* Main Program *)
let _ =

	(* Parse arguments *)
	Arg.parse opts (fun file -> files := file :: !files) banner;
	if(!files = []) then  Arg.usage opts banner else

	(* Get the output *)
	let (output, close) =
		if !out_file <> ""
		then ((open_out !out_file), true)
		else (stdout, false) in

	(* Process the inputs *)
	let _ =
		if !from_stdin || !files = []
		then process "" !args output
		else
			List.iter (fun file -> process file !args output) !files in

	(* Close the output if needed *)
	if close then close_out output

	
