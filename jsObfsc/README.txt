
 jsObfsc v1
 
A simple win32 tool for javascript code:
 a) minifier - reduces spaces, tabs and linebrakes
 b) obfuscator - renames variables and functions, tool to do that.


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


Compile.bat
c:\tcc\tcc.exe jsObfsc.c -IINCLUDE -lcomdlg32 -lgdi32 -luser32 -lkernel32 -lcomctl32 -mwindows
