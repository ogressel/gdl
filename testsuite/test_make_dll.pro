;
; Alain Coulais, 25 Janvier 2018, under GNU GPL v3 or later
;
; ---------------------------------------
; Modifications history :
;
; * 2018-JAN-29 : AC
;  * using the third param field in MAKE_DLL to have the good name
;  for the DLL lib.
; 
; ---------------------------------------
;
pro TEST_MAKE_DLL, no_exit=no_exit, help=help, test=test, verbose=verbose
;
; Looking for the path of the C file "test_make_dll.c",
; which is also the path where is store "test_make_dll.pro" (testsuite/)
;
radical='test_make_dll'
;
info=ROUTINE_INFO(radical, /source)
in_dir=FILE_DIRNAME(info.path)+PATH_SEP()
;
; do we have the "c_file" around ?
;
c_file=radical+'.c'
if ~FILE_TEST(in_dir+PATH_SEP()+c_file) then MESSAGE, 'missing input C file !'
;
; which name for the DLL (.so) file ?
;
so_file=GDL_IDL_FL(/upper)+'_'+radical+'.so'
idl_so_file=radical+'.so'
tmp=FILE_BASENAME(so_file, '.so')
;
; working local ! (eg. in /tmp)
;
out_dir=GETENV('PWD')+PATH_SEP()
print, 'Working here : ', out_dir
;
; calling the code !
;
MAKE_DLL, radical, tmp, '',$
          input_directory=in_dir, $
          output_directory=out_dir, $
          /SHOW_ALL_OUTPUT, /verbose
;
; Must add some test here whether the compilation + linking
; were ok or not ...
;
result=CALL_EXTERNAL(out_dir+so_file, 'add', 5, 6)
;
if (result NE 11) then errors=1 else errors=0
;
; not working for IDL < 8.4 :(
BANNER_FOR_TESTSUITE, 'TEST_MAKE_DLL', errors
;
if (errors GT 0) AND ~KEYWORD_SET(no_exit) then EXIT, status=1
;
if KEYWORD_SET(test) then STOP
;
end
