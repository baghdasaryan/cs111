/**
 * @version
 * 1.0.0 (November 6, 2013)
 * 
 * @copyright
 * Copyright (C) 2013 Georgi Baghdasaryan.
 *
 * @license
 * No license for this file.
 */
;(function()
{
	// CommonJS
	typeof(require) != 'undefined' ? SyntaxHighlighter = require('shCore').SyntaxHighlighter : null;

	function Brush()
	{
		var keywords = 'eax ebx ecx edx ax bx cx dx ah al bh bl ch cl dh dl cs ds ' + 
					   'es fs gs ss esi edi ebp eip esp';
		
		var functions =	'movl movsbl addl subl imull sall sarl shrl xorl andl orl ' +
						'incl decl negl notl leal cmpl testl jmp je jne js jns jg ' +
						'jge jl jle ja jb push pushl pop popl call ret';

		this.regexList = [
			{ regex: /[;#@!|].*$/gm,			 						css: 'comments' },			// one line comments
			{ regex: SyntaxHighlighter.regexLib.multiLineCComments,		css: 'comments' },			// multiline comments
			{ regex: /^ *#.*/gm,										css: 'preprocessor' },
			{ regex: /\%/gm, 											css: 'keyword' },
			{ regex: new RegExp(this.getKeywords(keywords), 'gmi'),		css: 'keyword' },
			{ regex: new RegExp(this.getKeywords(functions), 'gm'),		css: 'functions bold' }
			];
	};

	Brush.prototype	= new SyntaxHighlighter.Highlighter();
	Brush.aliases	= ['asm', 'assembly'];

	SyntaxHighlighter.brushes.Asm = Brush;

	// CommonJS
	typeof(exports) != 'undefined' ? exports.Brush = Brush : null;
})();
