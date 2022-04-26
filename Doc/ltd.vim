" Vim syntax file
" Language: leitner data file
" Maintainer: Yihe Chen
" Latest Revision: 29 March 2017

if exists("b:current_syntax")
  finish
endif

syn match ltdID			"%"
syn match meaningPrefix 	"[.|:|;]"
syn match ltdComment 		"#.*$"
syn match ltdIdentifier		"\<[a-zA-Z_][a-zA-Z0-9_]*\>"

syn keyword ltdRegionType1	ENUM
syn keyword ltdRegionType2	RFX VOC VGRP GRAM VS	nextgroup=ltdID
syn keyword ltdRegionAtti	ITEM
syn keyword ltdReference	RFREF VREF VGREF GREF

syn region meaningDesc start='"' end='"'
syn region referBlock start='\[' end='\]' contains=ltdIdentifier
syn region ltdRegionBlock start="{" end="}" fold transparent contains=ltdRegionAtti,ltdReference,meaningDesc,meaningPrefix,referBlock,ltdIdentifier

let b:current_syntax = "ltd"

" hi def link celTodo        Todo
" hi def link ltdComment     Comment
" hi def link ltdRegionAtti    Statement
" hi def link celHip         Type
" k meaningDesc      Constant
" hi def link celDesc        PreProc
" hi def link celNumber      Constant

hi link ltdIdentifier		Identifier
hi meaningDesc 		ctermfg=LightBlue 	guifg=LightBlue 	gui=NONE
hi ltdComment 		ctermfg=DarkGreen 	guifg=#00A0A0 		gui=italic
hi ltdRegionAtti 		ctermfg=LightGreen 	guifg=Green 		gui=NONE
hi ltdReference 		ctermfg=DarkYellow 		guifg=Green 		gui=NONE
hi referBlock 			ctermfg=DarkYellow 	guifg=Green 		gui=NONE
hi ltdRegionType1 		ctermfg=LightRed	guifg=Orange 		gui=NONE
hi ltdRegionType2 		ctermfg=LightRed	guifg=Orange 		gui=NONE
hi meaningPrefix		ctermfg=LightGreen 		guifg=Green 		gui=NONE
