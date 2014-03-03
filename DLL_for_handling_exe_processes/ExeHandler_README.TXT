***********************
*
*   ExeHandler.dll - a Win32 DLL library component to start sub-process, put stdin data once
*                    and periodically get stdout data.
*
***********************
*
* Intended for programming calculations of chess engines.
*


Functions:

1.  add_process <[path\]executable file name (char*)>,
              <arguments (char*)>,
              <stdin data (char*)>,
              <milliseconds (int)>
    returns id (int) of process / -1 if error

 This starts a hidden process (view in the Task Manager) with parameters.
 Redirects stdin and processes all stdin data. Input is closed when dll returns control
 to upper level proocedures out of dll. Unfortunately, dll can't manage stdin continuously.
 Anyway, we can set the task to sub-process and let it do the processing work.
 Milliseconds values:
   -1, if just start process (gets back from dll procedure)
   0, wait until process finishes (do all the job, but may freeze dll)
   number in milliseconds (1000 is a second) to wait for process to start normally (kinda pause)

 There can be 20 processes simultaneously.

2.  get_status <id (int)>

    returns status (int) of process
    Bits:
     0 - there is such id registered, data in buffers (check by &1)
     1 - the .Exited event happened for this process, not reachable, just keeps data buffers (check by &2)
     2 - there is process running in memory (check by &4)
     3 - if the process in memory has .HasExited flag (check by &8)


  Use this to identify current status of the process.
  For example, we may read results from output buffer when the program (sub-process) has finished.

3.  get_stdout <id (int)>,
               <buffer (char*)>

    returns bytes(int) in buffer / -1 if no data anymore

 All stdout data are buffered in dll. This returns data in predefined(!) buffer.


4.  get_errors <id (int)>,
               <buffer (char*)>

    returns bytes(int) in buffer/ -1 if no data anymore

 All exceptions and internal error messages are buffered in dll. Optional.

5.  kill_process <id (int)>

    returns count (1 or 0) of closed processes

 This closes a process and clears all data buffers for this id. Use after get_stdout.
 For example, start a process, wait few seconds, read stdout buffer and close.

6.  release_all

    returns count(int) of closed processes

 This closes all processes and clears all memory. Use this when exiting.
 

Samples:

Declare functions:

FoxPro sample:
  DECLARE INTEGER add_process IN ExeHandler.DLL STRING eName, STRING eArgs, STRING sStdin, INTEGER ms
  DECLARE INTEGER get_status IN ExeHandler.DLL INTEGER eID
  DECLARE INTEGER get_stdout IN ExeHandler.DLL INTEGER eID, STRING buff
  DECLARE INTEGER get_errors IN ExeHandler.DLL INTEGER eID, STRING buff
  DECLARE INTEGER kill_process IN ExeHandler.DLL INTEGER eID
  DECLARE INTEGER release_all IN ExeHandler.DLL

  eID=add_process("NOTEPAD.EXE","myfile.txt","",-1) && Start notepad as process, no data to stdin
  s = SPACE(64*1024)       && prepare 64Kb buffer
  l = get_stdout(eID,@s)   && read data from stdout
  PRINT LEFT(s,l)          && print data
  b = get_status(eID)      && get status, check by bitand(...)
  PRINT release_all();     && close all and print processes closed  
  

QB64 sample:
 DECLARE DYNAMIC LIBRARY "ExeHandler"
    FUNCTION add_process& (eName AS STRING, eArgs AS STRING, sStdin AS STRING, BYVAL ms AS INTEGER)
    FUNCTION get_status& (BYVAL eID AS INTEGER)
    FUNCTION get_stdout& (BYVAL eID AS INTEGER, buff AS STRING)
    FUNCTION get_errors& (BYVAL eID AS INTEGER, buff AS STRING)
    FUNCTION kill_process& (BYVAL eID AS INTEGER)
    FUNCTION release_all& ()
 END DECLARE
 eID = add_process("myfolder\my.exe", "", "dojob" + CHR$(10), -1)
 PRINT eID
 buffer$ = SPACE$(64 * 1024)
 DO
    l = get_stdout(eID, buffer$)
    IF l > 0 THEN
        PRINT LEFT$(buffer$, l)  'output stdout to screen...
    END IF
    SLEEP 1
 LOOP UNTIL INKEY$ = CHR$(32) OR (l = 0 AND ((get_status(eID) AND 2) > 0))
    'allow to press space key
    'or loop till the process is over or forced to end in ctrl+alt+del
 
 PRINT kill_process(eID)
