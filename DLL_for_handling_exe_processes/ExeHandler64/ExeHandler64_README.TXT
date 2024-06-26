***********************
*
*   ExeHandler64.dll - a Win64 DLL library component to start sub-process,
*                      control stdin/stdout streams continuously(!).
*                      Compiled by Chessforeva, 2024.
*
***********************
*
* Intended for programming calculations of chess engines.
*


Functions:

0.  char_1kb_buffer( nr (int) )

    nr = [0-9];

    returns (char *) pointer to char[1024] chunk of dll-memory, for convenience to work with strings.
               Otherwise should manage allocation of buffers somewhere and how to access them. See flutter sample. 


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
