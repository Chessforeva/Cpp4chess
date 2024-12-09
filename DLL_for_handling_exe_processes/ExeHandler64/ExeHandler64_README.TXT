***********************
*
*   ExeHandler64.dll - a Win64 DLL library component to start sub-process,
*                      control stdin/stdout streams continuously(!).
*                      Compiled by Chessforeva, 2024.
*                      Updated dec., 2024.
*
***********************
*
* Intended for programming calculations of chess engines.
*


Functions:

0.  char_100kb_buffer <chunk (int)>

    chunk = [0-200];

    returns (char *) pointer to char[102400] chunk of dll-memory, for convenience or possibility to work with strings.
               Otherwise should manage allocation of buffers somewhere and how to access them. See flutter sample.
    Access memory, write put_stdin_i, read back get_stdout_i.
    This is more workaround if can't just pass memory variables. Sometimes variable type conversions lose data.
    Garbage collectors ignore low level programming needs.

  0.1  assign_memory_i_n <chunk (int), n (int), c (int)>

      returns 1.
      It is for writing into DLL-memory bytes. buffer[chunk][n]=c
      c=[0-255] ascii code
      Write string to internal memory ending with \0, to use after put_stdin_i. Reading back from stdout to memory chunk
      by get_stdout_i. Then access memory by pointer given by char_100kb_buffer.
      Or read by bytes:
  0.2  readfrom_memory_i_n <chunk (int), n (int)>

      returns (int) character ascii byte [0-255]
      Then read next char, next till \0

    This all is workaround for languages that just don't like strings and memory management.


1.  add_process <[path\]executable file name (char*)>, <arguments (char*)>,

    returns id (int) of process / -1 if error

 This starts a hidden process (view in the Task Manager) with parameters.
 Stdin/stdout streams are prepared for redirected input/output.
 There can be 20 processes simultaneously.

     1.1  add_process_i < chunk_prog (int), chunk_args (int) >

     The same as add_process, just takes program file name and arguments from 100K_buffer memory.


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

 3.1    put_stdin_i <id (int)>, <chunk (int)>

        The same as put_stdin, just reads char* from 100K_buffer memory[chunk].


4.  get_stdout <id (int)>, <buffer (char*)>

    returns bytes(int) in buffer of redirected stdout stream / -1 if no data anymore

  Reads data from stream that are buffered by the system.
  Copies to the buffer provided in parameter. Space of buffer should be enaugh!

  4.1   get_stdout_i <id (int)>, <chunk (int)>

        The same as get_stdout, just writes char* to 100K_buffer memory[chunk].


5.  kill_process <id (int)>

    returns count (1 or 0) of closed processes

 This closes a process and clears all handlers. Use after get_stdout.
 For example, start a process, wait till it ends, read stdout buffer and close.

6.  release_all

    returns count(int) of closed processes

 This closes all processes and clears all memory. It's called also when DLL unloads.
