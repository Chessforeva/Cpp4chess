***********************
*
*   ExeHandler.dll - a Win32 DLL library component to start sub-process,
*                    control stdin/stdout streams continuously(!).
*                    Developed for gcc compiler.
*                    +More advanced and system usage at lower level
*                     than in the previous MS Studio version.
*
***********************
*
* Intended for programming calculations of chess engines.
*


Functions:

1.  add_process <[path\]executable file name (char*)>, <arguments (char*)>,

    returns id (int) of process / -1 if error

 This starts a hidden process (view in the Task Manager) with parameters.
 Stdin/stdout streams are prepared for redirected input/output.
 There can be 20 processes simultaneously.

2.  get_status <id (int)>

    returns status (int) of process
    Bits:
     0 - there is such id registered, data in buffers (check by &1)
     1 - the process is running in memory (check by &2)
     2 - the process has exited, or closed by force (check by &4) ctrl+alt+del

  Use this to identify current status of the process.
  For example, we may read results from output buffer when the program (sub-process) has finished.

3.  put_stdin <id (int)>, <string (char*)>

    returns bytes(int) that were sent to redirected stdin stream/ -1 if error

  Sends directly to the stream without buffering.


4.  get_stdout <id (int)>,
               <buffer (char*)>

    returns bytes(int) in buffer of redirected stdout stream / -1 if no data anymore

  Reads data from stream that are buffered by the system.
  Copies to the buffer provided in parameter. Space of buffer should be enaugh!

5.  kill_process <id (int)>

    returns count (1 or 0) of closed processes

 This closes a process and clears all handlers. Use after get_stdout.
 For example, start a process, wait till it ends, read stdout buffer and close.

6.  release_all

    returns count(int) of closed processes

 This closes all processes and clears all memory. It's called also when DLL unloads.


Samples:

Declare functions:

FoxPro sample:
  DECLARE INTEGER add_process IN ExeHandler.DLL STRING eName, STRING eArgs
  DECLARE INTEGER get_status IN ExeHandler.DLL INTEGER eID
  DECLARE INTEGER put_stdin IN ExeHandler.DLL INTEGER eID, STRING buff
  DECLARE INTEGER get_stdout IN ExeHandler.DLL INTEGER eID, STRING buff
  DECLARE INTEGER kill_process IN ExeHandler.DLL INTEGER eID
  DECLARE INTEGER release_all IN ExeHandler.DLL

  eID=add_process("NOTEPAD.EXE","myfile.txt") && Start notepad as process

  put_stdin(eID,"Hi!"+CHR(10))  && put data to stdin

  s = SPACE(64*1024)       && prepare 64Kb buffer
  l = get_stdout(eID,@s)   && read data from stdout
  PRINT LEFT(s,l)          && print data

  b = get_status(eID)      && get status, check by bitand(...)
  PRINT release_all();     && close all and print processes closed  
  

QB64 sample:
 DECLARE DYNAMIC LIBRARY "ExeHandler"
    FUNCTION add_process& (eName AS STRING, eArgs AS STRING)
    FUNCTION get_status& (BYVAL eID AS INTEGER)
    FUNCTION put_stdin& (BYVAL eID AS INTEGER, buff AS STRING)
    FUNCTION get_stdout& (BYVAL eID AS INTEGER, buff AS STRING)
    FUNCTION kill_process& (BYVAL eID AS INTEGER)
    FUNCTION release_all& ()
 END DECLARE
 eID = add_process("myfolder\my.exe", "")
 PRINT eID
 l = put_stdin(eID,"dojob" + CHR$(10))
 buffer$ = SPACE$(64 * 1024)
 DO
    l = get_stdout(eID, buffer$)
    IF l > 0 THEN
        PRINT LEFT$(buffer$, l)  'output stdout to screen...
    END IF
    SLEEP 1
 LOOP UNTIL INKEY$ = CHR$(32) OR (l = 0 AND ((get_status(eID) AND 4) > 0))
    'allow to press space key
    'or loop till the process is over or forced to end in ctrl+alt+del
 
 PRINT kill_process(eID)
