" Vim syntax file
" Language: Hexabus Compiler Language
" Maintainer: Hexabus Dev Team
" Latest Revision: nn. July 2012

" TODO remove this line - it's only for testing!
syntax clear

if exists("b:current_syntax")
  finish
endif

syn match hbcComment "#.*$" contains=hbcTodo
syn keyword hbcTodo contained TODO FIXME XXX NOTE
syn match hbcNumber contained "\d\+"
syn region hbcBlock start="{" end="}" fold transparent contains=hbcIPAddr,hbcAliasCommand,hbcList

" Alias definitions
syn match hbcIPAddr "\(\x\{1,4}:\)\{7}\x\{1,4}" "fullsize IP Addr
syn match hbcIPAddr "\(:\|\(\x\{1,4}:\)\{1,6}\):\(\(\x\{1,4}:\)\{0,6}\x\{1,4}\)\?" "shortened
syn keyword hbcAliasCommand contained ip nextgroup=hbcIPAddr
syn match hbcList "{[ ]*\d\+\(,[ ]*\d\+\)*[ ]*}" contains=hbcNumber
syn keyword hbcAliasCommand eids nextgroup=hbcList
syn keyword hbcAliasCommand contained endpoint nextgroup=hbcLoaclEPDef
" syn match hbcLocalEPDef TODO
" TODO distinguish between AliasNames, Identifiers, ... OR call them all identifier
syn match hbcAliasName '\h\w*' nextgroup=hbcAliasDef
syn keyword hbcAlias alias nextgroup=hbcAliasName

" State machine definition
syn keyword hbcIf if else in event nextgroup=hbcCondition
"TODO event will be dropped from the syntax -- remove it here
syn region hbcCondition start="(" end=")" transparent contains=hbcCondOp
syn keyword hbcCondOp contained ==

" 'preprocessor' stuff like include
syn keyword hbcPreproc include nextgroup=hbcFileName
" TODO make this more... useful
syn match hbcFileName '\h\h*\.hb.'


let b:current_syntax = "hbc"
hi def link hbcPreproc        PreProc
hi def link hbcFileName       Constant
hi def link hbcTodo           Todo
hi def link hbcComment        Comment
hi def link hbcAlias          Statement
hi def link hbcAliasCommand   Statement
hi def link hbcAliasName      Type
hi def link hbcIPAddr         Constant
hi def link hbcList           Number
hi def link hbcNumber         Number
hi def link hbcIf             Conditional
hi def link hbcElse           Conditional
hi def link hbcIn             Conditional
hi def link hbcCondOp         Operator

" For reference:
" hi Keyword
" hi Statement
" hi Constant
" hi Number
" hi PreProc
" hi Function
" hi Identifier
" hi Type
" hi Special
" hi String
" hi Comment
" hi Todo

" even more comprehensively:
" 
" *Comment	any comment
" 
" 	*Constant	any constant
" 	 String		a string constant: "this is a string"
" 	 Character	a character constant: 'c', '\n'
" 	 Number		a number constant: 234, 0xff
" 	 Boolean	a boolean constant: TRUE, false
" 	 Float		a floating point constant: 2.3e10
" 
" 	*Identifier	any variable name
" 	 Function	function name (also: methods for classes)
" 
" 	*Statement	any statement
" 	 Conditional	if, then, else, endif, switch, etc.
" 	 Repeat		for, do, while, etc.
" 	 Label		case, default, etc.
" 	 Operator	"sizeof", "+", "*", etc.
" 	 Keyword	any other keyword
" 	 Exception	try, catch, throw
" 
" 	*PreProc	generic Preprocessor
" 	 Include	preprocessor #include
" 	 Define		preprocessor #define
" 	 Macro		same as Define
" 	 PreCondit	preprocessor #if, #else, #endif, etc.
" 
" 	*Type		int, long, char, etc.
" 	 StorageClass	static, register, volatile, etc.
" 	 Structure	struct, union, enum, etc.
" 	 Typedef	A typedef
" 
" 	*Special	any special symbol
" 	 SpecialChar	special character in a constant
" 	 Tag		you can use CTRL-] on this
" 	 Delimiter	character that needs attention
" 	 SpecialComment	special things inside a comment
" 	 Debug		debugging statements
" 
" 	*Underlined	text that stands out, HTML links
" 
" 	*Ignore		left blank, hidden  |hl-Ignore|
" 
" 	*Error		any erroneous construct
" 
" 	*Todo		anything that needs extra attention; mostly the
" 			keywords TODO FIXME and XXX
