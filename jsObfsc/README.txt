
 jsObfsc v1.2
 
A simple win32 tool for javascript code:
 a) minifier - reduces spaces, tabs and linebrakes, reduces ";"
 b) obfuscator - renames variables and functions,
 a helping tool to do this.


Example:
--------

Converts this document:


    <html>
    <script>
    function factorial(arg_n)
     {
      return (arg_n<2 ? 1 : factorial(arg_n-1)*arg_n );
     }

    document.write('Fact(10)='+factorial(10));
    </script>
    </html>


to this:

    <html>
    <script>function b(a){return (a<2?1:b(a-1)*a)}document.write('Fact(10)='+b(10));</script>
    </html>

	
	
About project:
--------------

TinyC (http://hwww.tinycc.org)
Uses win32 dlls, no other GUI

3 files: jsObfsc.c, minfy.c, obfsc.c

Also an icon and resource file, if need


Compiling (Compile.bat)
-----------------------
1.Create jsObfsc.exe by

c:\tcc\tcc.exe jsObfsc.c -IINCLUDE -lcomdlg32 -lgdi32 -luser32 -lkernel32 -lcomctl32 -mwindows


(optional: icon build-in)
2.Compile icon in resource to rsrc.RES file

c:\\masm32\bin\rc rsrc.rc

3.Add resource to exe, using tool
LinkRes2Exe.exe rsrc.RES jsObfsc.exe





New:
02.2017 - added keywords save/open feature for larger project
03.2017 - added icon resource by masm32 resource builder and LinkRes2Exe tool (Much thanks).
04.2017 - added rare "const" keyword case
