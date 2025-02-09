/* *************************************************************************
                          format.g  -  parser for GDL format strings
                             -------------------
    begin                : July 22 2002
    copyright            : (C) 2002 by Marc Schellens
    email                : m_schellens@hotmail.com
 ***************************************************************************/

/* *************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

header "pre_include_cpp" {
#include "includefirst.hpp"
}

header {

#include <fstream>
#include <sstream>

#include "fmtnode.hpp"

#include "CFMTLexer.hpp"

#include <antlr/TokenStreamSelector.hpp>

//using namespace antlr;
}

options {
	language="Cpp";
	genHashLines = false;
	namespaceStd="std";         // cosmetic option to get rid of long defines
	namespaceAntlr="antlr";     // cosmetic option to get rid of long defines
}	

// the format Parser *********************************************
class FMTParser extends Parser;

options {
	importVocab = CFMT;	// use vocab generated by clexer
	exportVocab = FMT;	// use vocab generated by lexer
	buildAST = true;
  	ASTLabelType = "RefFMTNode";
	k=1;
//    defaultErrorHandler = true;
    defaultErrorHandler = false;
}

tokens {
    FORMAT;
}

{
// class extensions
}
// original work by Marc was missing things as print,"a","b","c",format='(/a/,a,/,a/)'
//based on https://www.antlr3.org/grammar/1183077163756/f77-antlr2.g GD changed to this:
format  [ int repeat] // mark last format for format reversion
    : LBRACE! (f | q (f)?)  ( q (f)? | 	COMMA! (f | q (f)?) )* RBRACE!
        {
            #format = #( [FORMAT,"FORMAT"], #format);
            #format->setRep( repeat);
        } ;

q!
{
    int n1 = 0;
}
    : (SLASH { n1++;})
        {
            if( n1 > 0) 
            {
                #q = #([SLASH,"/"]);
                #q->setRep( n1);
            }           
        }
    ;

f_csubcode // note: IDL doesn't allow hollerith strings inside C()
{
    int n1;
}
    : STRING // just write it out (H strings are handled in the scanner)
    | cstring
    ;

cstring
    : (s:CSTR { #s->setType( STRING);} | cformat)+
//    : (CSTR | cformat)+
    ;

cformat
{
    int w = 0;
    int infos[4]={0,0,0,0};
    int d = -1;
}
    :   ( (PM!|MP!) {infos[1]=1;infos[2]=1;} | (PLUS! {infos[1]=1;}) | (MOINS! {infos[2]=1;}) )? (cnnf [infos] { w=infos[0]; } (CDOT! d=cnn)?)?
        (
           c:CD   {  if (infos[1])#c->setShowSign();  if (infos[2])#c->setALignLeft();  if (infos[3])#c->setPadding(); #c->setW( w);  #c->setD( d);  #c->setType( I); }
        |  se:CSE {  if (infos[1])#se->setShowSign(); if (infos[2])#se->setALignLeft(); if (infos[3])#se->setPadding(); #se->setW( w);  #se->setD( d);  #se->setType( SE); }
        |  e:CE   {  if (infos[1])#e->setShowSign();  if (infos[2])#e->setALignLeft();  if (infos[3])#e->setPadding(); #e->setW( w);  #e->setD( d);  #e->setType( E); }
        |  i:CI   {  if (infos[1])#i->setShowSign();  if (infos[2])#i->setALignLeft();  if (infos[3])#i->setPadding(); #i->setW( w);  #i->setD( d);  #i->setType( I); }
        | ff:CF   {  if (infos[1])#ff->setShowSign(); if (infos[2])#ff->setALignLeft(); if (infos[3])#ff->setPadding(); #ff->setW( w); #ff->setD( d); #ff->setType( F); }
        |  sg:CSG {  if (infos[1])#sg->setShowSign(); if (infos[2])#sg->setALignLeft(); if (infos[3])#sg->setPadding(); #sg->setW( w);  #sg->setD( d);  #sg->setType( SG);}
        |  g:CG   {  if (infos[1])#g->setShowSign();  if (infos[2])#g->setALignLeft();  if (infos[3])#g->setPadding(); #g->setW( w);  #g->setD( d);  #g->setType( G); }
        |  o:CO   {  if (infos[1])#o->setShowSign();  if (infos[2])#o->setALignLeft();  if (infos[3])#o->setPadding(); #o->setW( w);  #o->setD( d);  #o->setType( O); }
        |  b:CB   {  if (infos[1])#b->setShowSign();  if (infos[2])#b->setALignLeft();  if (infos[3])#b->setPadding(); #b->setW( w);  #b->setD( d);  #b->setType( B); }
        |  x:CX   {  if (infos[1])#x->setShowSign();  if (infos[2])#x->setALignLeft();  if (infos[3])#x->setPadding(); #x->setW( w);  #x->setD( d);  #x->setType( Z); }
        |  z:CZ   {  if (infos[1])#z->setShowSign();  if (infos[2])#z->setALignLeft();  if (infos[3])#z->setPadding(); #z->setW( w);  #z->setD( d);  #z->setType( Z); }
        |  s:CS   {  if (infos[1])#s->setShowSign();  if (infos[2])#s->setALignLeft();  if (infos[3])#s->setPadding(); #s->setW( w);  #s->setType( A);}
        )
    ;

// no nodes for cnumbers
cnn! returns[ int n]
    : num:CNUMBER 
        { 
            std::istringstream s(#num->getText());
            s >> n;
        }
    ;

// no nodes for cnumbers with zero padding
cnnf! [int *infos]  //returns[ int *infos]
    : num:CNUMBER 
         { 
            char c;
            std::istringstream s(#num->getText());
            c=s.get();
            s.putback(c);
            s >> infos[0];
            if (c == '0') infos[3]=-1;;
         }
    ;

f
{
    int n1;
}
    : TERM
    | NONL
    | Q
    | CSTRING
    | tl:TL n1=nn { #tl->setW( n1);}
    | tr:TR n1=nn { #tr->setW( n1);}
    | t:T n1=nn { #t->setW( n1);}
    | f_csubcode
    | rep_fmt[ 1]
    | n1=nn (rep_fmt[ n1] | x:X { #x->setW( n1);})
    | xx:X { #xx->setW( 1);}
    ;

rep_fmt [ int repeat]
{
    int n1;
}
    : format[ repeat] 
    | a:A (n1=nnf [#a] { #a->setW( n1);})? { #a->setRep( repeat);}
    | ff:F w_d  [ #ff] {#ff->setRep( repeat);} // F and D are the same -> D->F
    | d:D w_d  [ #d] { #d->setRep( repeat); #d->setText("f"); #d->setType(F);}
    | e:E w_d_e[ #e] { #e->setRep( repeat);}
    | se:SE w_d_e[ #se] { #se->setRep( repeat);}
    | g:G w_d_e[ #g] { #g->setRep( repeat);}
    | sg:SG w_d_e[ #sg] { #sg->setRep( repeat);}
    | i:I w_d  [ #i] { #i->setRep( repeat);}
    | o:O w_d  [ #o] { #o->setRep( repeat);}
    | b:B w_d  [ #b] { #b->setRep( repeat);}
    | z:Z w_d  [ #z] { #z->setRep( repeat);}
    | zz:ZZ w_d  [ #zz] { #zz->setRep( repeat);}
    | c:C^ LBRACE! calendar_string RBRACE! { #c->setRep( repeat);}
    ;   

calendar_string 
    : (
      (calendar_code (COMMA! calendar_code)*)
      |
      )
    ;

calendar_code
{
    int n1;
}
    : c1:CMOA (n1=nn { #c1->setW( n1);})?
    | c2:CMoA (n1=nn { #c2->setW( n1);})? 
    | c3:CmoA (n1=nn { #c3->setW( n1);})?
    | c4:CHI w_d[ #c4]
    | c5:ChI w_d[ #c5]
    | c6:CDWA (n1=nn { #c6->setW( n1);})?
    | c7:CDwA (n1=nn { #c7->setW( n1);})?
    | c8:CdwA (n1=nn { #c8->setW( n1);})?
    | c9:CAPA (n1=nn { #c9->setW( n1);})?
    | c10:CApA (n1=nn { #c10->setW( n1);})?
    | c11:CapA (n1=nn { #c11->setW( n1);})?
    | c12:CMOI w_d[ #c12]
    | c13:CDI w_d[ #c13]
    | c14:CYI w_d[ #c14]
    | c15:CMI w_d[ #c15]
    | c16:CSI w_d[ #c16]
    | c17:CSF w_d[ #c17]
    | n1=nn (rep_fmt[ n1] | x:X { #x->setW( n1);})
    | xx:X { #xx->setW( 1);}
    | STRING
    ;

// no nodes for numbers
nn! returns[ int n]
{ 
  int sgn=1;
}
    : num:NUMBER 
        { 
            std::istringstream s(#num->getText());
            s >> n;
        }
    ;

// no nodes for numbers with zero padding
nnf! [ RefFMTNode fNode] returns[ int n=-1]
    : ( 
        ( 
          (PM|MP) {
                  fNode->setShowSign();
                  fNode->setALignLeft();
                 } 
          | PLUS {fNode->setShowSign();} 
          | MOINS {fNode->setALignLeft();} 
        )
     )?  
      (
        num:NUMBER  //as we are left-aligned, 0 leading (padding) can be read as number since there will be no 0 padding. 
        { 
            std::istringstream s(#num->getText());
            char c = s.get();
            char next = s.peek();
            s.putback(c);
            s >> n;
            if (c == '0') fNode->setPadding();
        }
      )?
    ;

w_d! [ RefFMTNode fNode]
{
    int n1=-1, n2=-1;
    fNode->setW( -1);
    fNode->setD( -1);
}
    : (n1=nnf[ fNode] { fNode->setW( n1);} (DOT n2=nn { fNode->setD( n2);} )?)?
    ;

w_d_e! [ RefFMTNode fNode]
    : (options { greedy=true;}: w_d[ fNode] ((E|SE) ignored:NUMBER {
            int n=0;
            std::istringstream s(#ignored->getText());
            s >> n;
//does not work (loops probably at format_reversion)            if (n < 0 || n > 255) throw GDLException("Value is out of allowed range (0 - 255).");
            }
           )?)? 
    ;

// the Format Lexer *********************************************
class FMTLexer extends Lexer;

options {
	charVocabulary = '\3'..'\377';
	caseSensitive=true ;
	testLiterals =true;
	caseSensitiveLiterals=false;
//	importVocab = CFMT;	// use vocab generated by clexer
	exportVocab = FMT;
	k=4;
    defaultErrorHandler = false;
//    defaultErrorHandler = true;
// 	analyzerDebug=true;
}

{
    private:
    antlr::TokenStreamSelector*  selector; 
    CFMTLexer*            cLexer;

    public:
    void SetSelector( antlr::TokenStreamSelector& s)
    {
        selector = &s;
    }
    void SetCLexer( CFMTLexer& l)
    {
        cLexer = &l;
    }
}

STRING
// 	: '\"'! (~('\"'|'\r'|'\n')| '\"' '\"'! )* '\"'!
// 	| '\''! (~('\''|'\r'|'\n')| '\'' '\''! )* '\''!
	: '\"'! (~('\"')| '\"' '\"'! )* '\"'!
	| '\''! (~('\'')| '\'' '\''! )* '\''!
	;	

CSTYLE_STRING!
	: '%' '\"' 
        { cLexer->DoubleQuotes( true); selector->push( cLexer); selector->retry();}
    | '%' '\'' 
        { cLexer->DoubleQuotes( false); selector->push( cLexer); selector->retry();}
	;	


LBRACE: '(';
RBRACE: ')';

SLASH: '/';

COMMA: ',';

A:('A'|'a');
TERM:':';
NONL:'$';
F:('f'|'F');
D:('d'|'D');
E:('E');
SE:('e');
G:('G');
SG:('g');

I:('i'|'I');
O:('o'|'O');
B:('b'|'B');
Z:('Z');
ZZ:('z'); // lower case output

Q:('q'|'Q');

H:('h'|'H');

T:('t'|'T');
TR:('t' 'r'|'T' 'R');
TL:('t' 'l'|'T' 'L');

L:('l'|'L');
R:('r'|'R');

X:('x'|'X');

C:('c'|'C');

CMOA: ( 'C' 'M' 'O' 'A');
CMoA: ( 'C' 'M' 'o' 'A');
CmoA: ( 'C' 'm' 'o' 'A');
CMOI: ( 'C' 'M' 'O' 'I');

CDI: ( 'C' 'D' 'I');
CMI: ( 'C' 'M' 'I');
CYI: ( 'C' 'Y' 'I');
CSI: ( 'C' 'S' 'I');
CSF: ( 'C' 'S' 'F');
CHI: ( 'C' 'H' 'I');
ChI: ( 'C' 'h' 'I');

CDWA: ( 'C' 'D' 'W' 'A');
CDwA: ( 'C' 'D' 'w' 'A');
CdwA: ( 'C' 'd' 'w' 'A');

CAPA: ( 'C' 'A' 'P' 'A');
CApA: ( 'C' 'A' 'p' 'A');
CapA: ( 'C' 'a' 'p' 'A');

PERCENT:'%';

DOT:'.';
PM: ('+' '-');
MP: ('-' '+');
PLUS: '+';
MOINS: '-';

protected     
W
//	: ( '\003'..'\010' | '\t' | '\r' | '\013' | '\f' | '\016'.. '\037' | ' ' )
	: (' '| '\t') 
	;

WHITESPACE
    : (W)+
        { _ttype=antlr::Token::SKIP; }
    ;

protected
DIGITS
	: ('0'..'9')+
	;

protected
CHAR: ('\003'..'\377');

NUMBER // handles hollerith strings also
{ 
    SizeT n = -1;
    SizeT i = 0;
} 
    :  
        (  num:DIGITS
            (
                { 
                    $setType(STRING); 
                    std::istringstream s(num->getText());
                    s >> n;
                    $setText(""); // clear string (remove number)
                }
                H! 
                (
                    { // init action gets executed even in guessing mode
                        if( i == n )
                        break;
                        i++; // count chars here so that guessing mode works
                    }: // ":" makes it an init action
                    CHAR
                )+
            )?
        )?
    ;
