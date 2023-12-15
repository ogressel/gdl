/* *************************************************************************
                          format.out.g  -  interpreter for formatted output
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

header "post_include_cpp" {
    // gets inserted after the antlr generated includes in the cpp file
}

header {

#include <fstream>
#include <cassert>

//***test
#include "print_tree.hpp"

#include "envt.hpp"

#include "fmtnode.hpp"
//#include "getfmtast.hpp"    // for FMTNodeFactory;    

#include "gdljournal.hpp"
}

options {
	language="Cpp";
//	genHashLines = true;
	genHashLines = false;
	namespaceStd="std";         // cosmetic option to get rid of long defines
	namespaceAntlr="antlr";     // cosmetic option to get rid of long defines
}	

// the format Parser *********************************************
class FMTOut extends TreeParser;

options {
	importVocab = FMT;	// use vocab generated by format lexer
	buildAST = false;
  	ASTLabelType = "RefFMTNode";
    defaultErrorHandler = false;
//    defaultErrorHandler = true;
//    codeGenBitsetTestThreshold=999;
//    codeGenMakeSwitchThreshold=1;
}

{
public:
    FMTOut( RefFMTNode fmt, std::ostream* os_, EnvT* e_, int parOffset)
    : antlr::TreeParser(), os(os_), e( e_), nextParIx( parOffset),
	valIx(0), termFlag(false), nonlFlag(false), nElements(0)
    {
        std::ostringstream* osLocal;
std::unique_ptr<std::ostream> osLocalGuard;
        //if( *os_ == std::cout) // SA: this did not work with win32
        if( os_->rdbuf() == std::cout.rdbuf())
            {
                // e.g. print, 1, f='(A)'
                osLocal = new std::ostringstream();
                osLocalGuard.reset( osLocal);
                os = osLocal;
            }
        else
            {
                // e.g. print, string(1, f='(A)')
                os = os_;
            }

        nParam = e->NParam();

        NextPar();
    
        GDLStream* j = lib::get_journal();

        if( j != NULL && j->OStream().rdbuf() == os->rdbuf()) 
            (*os) << lib::JOURNALCOMMENT;

        format( fmt);
        
        SizeT nextParIxComp = nextParIx;
        SizeT valIxComp = valIx;

        // format reversion
        while( actPar != NULL)
        {
           (*os) << '\n';
            
            if( j != NULL && j->OStream().rdbuf() == os->rdbuf()) 
                (*os) << lib::JOURNALCOMMENT;

            format_reversion( reversionAnker);            
 
           if( (nextParIx == nextParIxComp) && (valIx == valIxComp))   
                throw GDLException("Infinite format loop detected.");
         }
        
        os->seekp( 0, std::ios_base::end);

        if( !nonlFlag)
            {
                (*os) << '\n';
            }
        (*os) << std::flush;

        if( os_->rdbuf() == std::cout.rdbuf()) // SA: see note above
            {
                os = os_;
                (*os) << osLocal->str();
                (*os) << std::flush;
            }
    }
    
private:
    void NextPar()
    {
        valIx = 0;
        if( nextParIx < nParam)
        {
            actPar = e->GetPar( nextParIx);
            if( actPar != NULL)
            nElements = actPar->ToTransfer();
            else
            nElements = 0;
        } 
        else 
        {
            actPar = NULL;
            nElements = 0;
        }
        nextParIx++;
    }

    void NextVal( SizeT n=1)
    {
        valIx += n;
        if( valIx >= nElements)
            NextPar();
    }
    
    std::ostream* os;
    EnvT*    e;
    SizeT   nextParIx;
    SizeT   valIx;

    bool termFlag;
    bool nonlFlag;

    SizeT   nParam;
    BaseGDL* actPar;
    SizeT nElements;

    RefFMTNode reversionAnker;
}

format
    : #(fmt:FORMAT 
            { goto realCode; } // note: following is never executed in order to fool ANTLR
            q (f q)+ // this gets never executed
            {
                realCode:

                reversionAnker = #fmt;
                
                RefFMTNode blk = _t; // q (f q)+

                for( int r = #fmt->getRep(); r > 0; r--)
                {
                    q( blk);
                    _t = _retTree;

                    for (;;) 
                    {
                        if( _t == static_cast<RefFMTNode>(antlr::nullAST))
                            _t = ASTNULL;

                        switch ( _t->getType()) {
                        case FORMAT:
                        case TL:
                        case TR:
                        case TERM:
                        case NONL:
                        case Q: case T: case X: case A:
                        case F: case D: case E: case SE: case G: case SG:
                        case I: case O: case B: case Z: case ZZ: case C:
                            {
                                if( actPar == NULL && termFlag) goto endFMT;
                                // no break
                            }
                        case STRING:
                        case CSTYLE_STRING:
                            {
                                f(_t);
//                                if( actPar == NULL && termFlag) goto endFMT;
                                _t = _retTree;
                                q(_t);
                                _t = _retTree;
                                break; // out of switch
                            }
                        default:
                            goto endFMT;
                        }
                    }
                    
                    endFMT: // end of one repetition
                    if( actPar == NULL && termFlag) break;
                }
            }
        )
    ;

format_reversion
    : format 
        { goto realCode; } // note: following is never executed in order to fool ANTLR
        q (f q)*
        {
            realCode:
            
            q( _t);
            _t = _retTree;
            
            for (;;) 
            {
                if( _t == static_cast<RefFMTNode>(antlr::nullAST))
                _t = ASTNULL;
                
                switch ( _t->getType()) {
                case FORMAT:
                case STRING:
                case CSTYLE_STRING:
                case TL:
                case TR:
                case TERM:
                case NONL:
                case Q: case T: case X: case A:
                case F: case D: case E: case SE: case G: case SG:
                case I: case O: case B: case Z: case ZZ: case C:
                    {
                        f(_t);
                        if( actPar == NULL) goto endFMT;
                        _t = _retTree;
                        q(_t);
                        _t = _retTree;
                        break; // out of switch
                    }
                default:
                    goto endFMT;
                }
            }
            endFMT: // end of one repetition
        }
    ;

q
    : (s:SLASH 
            {
                // only one newline to journal file
                GDLStream* j = lib::get_journal();
                if( j != NULL && j->OStream().rdbuf() == os->rdbuf())
                    (*os) << '\n' << lib::JOURNALCOMMENT;
                else
                    for( int r=s->getRep(); r > 0; r--) (*os) << '\n';
            }
        )?
    ;

f_csubcode // note: IDL doesn't allow hollerith strings inside C()
    : s:STRING { (*os) << s->getText(); }
//    | CSTYLE_STRING // *** requires special handling
    | tl:TL 
        { 
            SizeT actP  = os->tellp(); 
            int    tlVal = tl->getW();
            if( tlVal > actP)
                os->seekp( 0);
            else
                os->seekp( actP - tlVal);
        }
    | tr:TR 
        { 
            int    tlVal = tl->getW();
            for( int i=tlVal; i>0; --i)
            (*os) << " ";
//            os->seekp( tlVal, std::ios_base::cur);
        }
    ;

f
    : TERM { termFlag = true; }
    | NONL { nonlFlag = true; }
    | Q // ignored on output
    | t:T
        { 
            int    tVal = t->getW();
            assert( tVal >= 1);
            os->seekp( tVal-1, std::ios_base::beg);
        }
    | f_csubcode
    | x
    | format // following are repeatable formats
    | a:A 
        {
            if( actPar == NULL) break;

            int r = a->getRep();
            int w = a->getW();
            int c = a->getCode();
            do {
                SizeT tCount = actPar->OFmtA( os, valIx, r, w, c);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
//  | d:D // D is transformed to F below:
    | ff:F
        {
            if( actPar == NULL) break;
            
            int r = ff->getRep();
            int w = ff->getW();
            int d = ff->getD();
            int c = ff->getCode();
            do {
                SizeT tCount = actPar->OFmtF( os, valIx, r, w, d, c, BaseGDL::FIXED);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | se:SE
        {
            if( actPar == NULL) break;
            
            int r = se->getRep();
            int w = se->getW();
            int d = se->getD();
            int c = se->getCode();
            do {
                SizeT tCount = actPar->OFmtF( os, valIx, r, w, d, c, BaseGDL::SCIENTIFIC);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | ee:E
        {
            if( actPar == NULL) break;
            
            int r = ee->getRep();
            int w = ee->getW();
            int d = ee->getD();
                    ee->setUpper();  //'E' in uppercase
            int c = ee->getCode();
            do {
                SizeT tCount = actPar->OFmtF( os, valIx, r, w, d, c, BaseGDL::SCIENTIFIC);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | sg:SG
        {
            if( actPar == NULL) break;
            
            int r = sg->getRep();
            int w = sg->getW();
            int d = sg->getD();
            int c = sg->getCode();
            do {
                SizeT tCount = actPar->OFmtF( os, valIx, r, w, d, c, BaseGDL::AUTO);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | g:G
        {
            if( actPar == NULL) break;
            
            int r = g->getRep();
            int w = g->getW();
            int d = g->getD();
                    g->setUpper();  //'E' in uppercase
            int c = g->getCode();
            do {
                SizeT tCount = actPar->OFmtF( os, valIx, r, w, d, c, BaseGDL::AUTO);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | i:I
        {
            if( actPar == NULL) break;
            
            int r = i->getRep();
            int w = i->getW();
            int d = i->getD();
            int c = i->getCode();
            do {
                SizeT tCount = actPar->OFmtI( os, valIx, r, w, d, c, BaseGDL::DEC);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | o:O
        {
            if( actPar == NULL) break;
            
            int r = o->getRep();
            int w = o->getW();
            int d = o->getD();
            int c = o->getCode();
            do {
                SizeT tCount = actPar->OFmtI( os, valIx, r, w, d, c, BaseGDL::OCT);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | b:B
        {
            if( actPar == NULL) break;
            
            int r = b->getRep();
            int w = b->getW();
            int d = b->getD();
            int c = b->getCode();
            do {
                SizeT tCount = actPar->OFmtI( os, valIx, r, w, d, c, BaseGDL::BIN);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | z:Z
        {
            if( actPar == NULL) break;
            
            int r = z->getRep();
            int w = z->getW();
            int d = z->getD();
            int c = z->getCode();
            do {
                SizeT tCount = actPar->OFmtI( os, valIx, r, w, d, c, BaseGDL::HEX);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | zz:ZZ
        {
            if( actPar == NULL) break;
            
            int r = zz->getRep();
            int w = zz->getW();
            int d = zz->getD();
            int c = zz->getCode();
            do {
                SizeT tCount = actPar->OFmtI( os, valIx, r, w, d, c, BaseGDL::HEXL);
                r -= tCount;
                NextVal( tCount);
                if( actPar == NULL) {termFlag=true; break;};
            } while( r>0);
        }
    | 
(
        #(
        c:C
        {
          int r = c->getRep(); if (r<1) r=1;
          if( actPar == NULL) break;
          SizeT nTrans = actPar->ToTransfer();
          if (r > nTrans) r=nTrans;
          actPar->OFmtCal( os, valIx, r, 0, 0, NULL, 0, BaseGDL::COMPUTE); //convert to hour, min, etc
        }


(        
(calendar_code[r])+ |
             {
                if( actPar == NULL) break;
                actPar->OFmtCal( os, valIx, r, 0, 0, NULL, 0, BaseGDL::DEFAULT);
             }
)

        {
           if( actPar == NULL) break;
           SizeT tCount = actPar->OFmtCal( os, valIx, r, 0, 0, NULL, 0, BaseGDL::WRITE); //Write the complete formatted string to os.
           NextVal( tCount);
           if( actPar == NULL) break;
        }

        ) 
//        exception
//        catch [ antlr::RecognitionException& e] {std::cerr<<e.toString();}
)



    ;  

calendar_code
[SizeT r]
    : c1:CMOA
        {
            if( actPar == NULL) break;
            int w = c1->getW();
            int d = c1->getD();
            int c = c1->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CMOA);
        }

    | c2:CMoA
        {
            if( actPar == NULL) break;
            
            int w = c2->getW();
            int d = c2->getD();
            int c = c2->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CMoA);
        }
    | c3:CmoA
        {
            if( actPar == NULL) break;
            
            int w = c3->getW();
            int d = c3->getD();
            int c = c3->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CmoA);
        }
    | c4:CHI
        {
            if( actPar == NULL) break;
            
            int w = c4->getW();
            int d = c4->getD();
            int c = c4->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CHI);
        }
    | c5:ChI
        {
            if( actPar == NULL) break;
            
            int w = c5->getW();
            int d = c5->getD();
            int c = c5->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::ChI);
        }
    | c6:CDWA
        {
            if( actPar == NULL) break;
            
            int w = c6->getW();
            int d = c6->getD();
            int c = c6->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CDWA);
        }
    | c7:CDwA
        {
            if( actPar == NULL) break;
            
            int w = c7->getW();
            int d = c7->getD();
            int c = c7->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CDwA);
        }
    | c8:CdwA
        {
            if( actPar == NULL) break;
            
            int w = c8->getW();
            int d = c8->getD();
            int c = c8->getCode();
                SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CdwA);
        }
    | c9:CAPA
        {
            if( actPar == NULL) break;
            
            int w = c9->getW();
            int d = c9->getD();
            int c = c9->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CAPA);
        }
    | c10:CApA
        {
            if( actPar == NULL) break;
            
            int w = c10->getW();
            int d = c10->getD();
            int c = c10->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CApA);
        }
    | c11:CapA
        {
            if( actPar == NULL) break;
            
            int w = c11->getW();
            int d = c11->getD();
            int c = c11->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CapA);
        }
    | c12:CMOI
        {
            if( actPar == NULL) break;
            
            int w = c12->getW();
            int d = c12->getD();
            int c = c12->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CMOI);
        }
    | c13:CDI 
        {
            if( actPar == NULL) break;
            
            int w = c13->getW();
            int d = c13->getD();
            int c = c13->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CDI);
        }
    | c14:CYI
        {
            if( actPar == NULL) break;
            
            int w = c14->getW();
            int d = c14->getD();
            int c = c14->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CYI);
        }
    | c15:CMI
        {
            if( actPar == NULL) break;
            
            int w = c15->getW();
            int d = c15->getD();
            int c = c15->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CMI);
        }
    | c16:CSI
        {
            if( actPar == NULL) break;
            
            int w = c16->getW();
            int d = c16->getD();
            int c = c16->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CSI);
        }
    | c17:CSF
        {
            if( actPar == NULL) break;
            int w = c17->getW();
            int d = c17->getD();
            int c = c17->getCode();
            SizeT tCount = actPar->OFmtCal( os, valIx, r, w, d, NULL, c, BaseGDL::CSF);
        }
    | c18:X
        {
		if( actPar == NULL) break;
		int    tlVal = c18->getW(); if (tlVal < 1) tlVal=1;
		std::string *s=new std::string(tlVal,' ');
		SizeT tCount = actPar->OFmtCal( os, valIx, r, 0, 0, (char*)s->c_str(), BaseGDL::STRING);
                delete s;
        }
    | c19:STRING
        {
		if( actPar == NULL) break;
		SizeT tCount = actPar->OFmtCal( os, valIx, r, 0, 0, (char*)c19->getText().c_str(), 0, BaseGDL::STRING);
        }
    ; //AND NOTHING ELSE PERMITTED!

x
    : tl:X 
        {
//            if( _t != static_cast<RefFMTNode>(antlr::nullAST))
//            {
//                int    tlVal = #tl->getW();
//                (*os) << " "; //for format "X" (no width)
//                for( int i=tlVal; i>1; --i)
//                (*os) << " "; //for format "nX"
////                os->seekp( tlVal, std::ios_base::cur);
//            }
             for( int r=tl->getW(); r > 0; r--) (*os) << ' ';
        }
    ;
