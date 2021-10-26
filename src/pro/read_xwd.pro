function READ_XWD, filename, red, green, blue
;+
;
; NAME: READ_XWD
;
;
; PURPOSE: Reads a X Windows Dump file into memory
;
; CATEGORY: Images (IO)
;
; CALLING SEQUENCE: image=read_xwd(filename,red,green,blue)
;
; OUTPUTS: 
;    IMAGE: A 2d file containing the vector indices
;
; OPTIONAL OUTPUTS:
;        red  : the Red colormap vector (for PseudoColor images)
;        green: the Green colormap vector (for PseudoColor images)
;        blue : the Blue colormap vector (for PseudoColor images)
;
;
; RESTRICTIONS:
;         Requires ImageMagick
;
; PROCEDURE:
;         Use ImageMagick to read the data as requested
;
; EXAMPLE:
;         
; MODIFICATION HISTORY:
; 	Written by: Christopher Lee 2004-05-17
;       2011-Nov-15, Alain Coulais : do we try to read a real file ?!
;
;-
; LICENCE:
; Copyright (C) 2004, 2011
; This program is free software; you can redistribute it and/or modify  
; it under the terms of the GNU General Public License as published by  
; the Free Software Foundation; either version 2 of the License, or     
; (at your option) any later version.                                   
;
;-
;

compile_opt hidden, idl2

ON_ERROR, 2
;
; this line allows to compile also in IDL ...
FORWARD_FUNCTION MAGICK_EXISTS
;
if KEYWORD_SET(help) then begin
    print, 'function READ_XWD, filename, red, green, blue, $'
    print, '                   verbose=verbose, help=help, test=test'
    return, -1
endif
;
; Do we have access to ImageMagick functionnalities ??
;
if (MAGICK_EXISTS() EQ 0) then begin
    MESSAGE, /continue, "GDL was compiled without ImageMagick support."
    MESSAGE, "You must have ImageMagick support to use this functionaly."
endif
;
if (N_PARAMS() EQ 0) then MESSAGE, "Incorrect number of arguments."
if (N_ELEMENTS(filename) GT 1) then MESSAGE, "Only one file at once !"
if (STRLEN(filename) EQ 0) then MESSAGE, "Null filename not allowed."
if ((FILE_INFO(filename)).exists EQ 0) then MESSAGE, "Error opening file. File: "+filename
if (FILE_TEST(filename, /regular) EQ 0) then MESSAGE, "Not a regular File: "+filename
READ_ANYGRAPHICSFILEWITHMAGICK, filename, image, colortable
if ( n_elements(colortable) gt 0 ) then begin
   red=colortable[*,0]
   green=colortable[*,1]
   blue=colortable[*,2]
endif
return, image
;
end



