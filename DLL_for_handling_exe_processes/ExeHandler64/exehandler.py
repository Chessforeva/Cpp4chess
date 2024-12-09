##################################################
#
# Sample of ExeHandler usage in windows python
#
##################################################

# load dll and test
#-----------------------------------

# provide DLL file exact filename with path

import ctypes
eh_dll = ctypes.CDLL("C:\\ExeHandler64\\ExeHandler64.dll")


# declaration of dll functions
eh_dll.add_process.argtypes = [ ctypes.c_char_p, ctypes.c_char_p ]
eh_dll.add_process.restype = ctypes.c_int

eh_dll.put_stdin.argtypes = [ ctypes.c_int, ctypes.c_char_p ]
eh_dll.put_stdin.restype = ctypes.c_int

eh_dll.get_stdout.argtypes = [ ctypes.c_int, ctypes.c_char_p ]
eh_dll.get_stdout.restype = ctypes.c_int

eh_dll.get_status.argtypes = [ ctypes.c_int ]
eh_dll.get_status.restype = ctypes.c_int

eh_dll.kill_process.argtypes = [ ctypes.c_int ]
eh_dll.kill_process.restype = ctypes.c_int

#(void) should be parameter in this case, just put an integer 0 
eh_dll.release_all.argtypes = [ ctypes.c_int ]
eh_dll.release_all.restype = ctypes.c_int

#not needed, can use python memory directly from memory variables
#intended if memory should be accessed in DLL inside
eh_dll.char_100kb_buffer.argtypes = [ ctypes.c_int ]
eh_dll.char_100kb_buffer.restype = ctypes.c_char_p

eh_dll.put_stdin_i.argtypes = [ ctypes.c_int,  ctypes.c_int ]
eh_dll.put_stdin_i.restype = ctypes.c_int

eh_dll.get_stdout_i.argtypes = [ ctypes.c_int,  ctypes.c_int ]
eh_dll.get_stdout_i.restype = ctypes.c_int

# usage samples
#-----------------------------------

#use Stockfish as a chess engine
Prog = "C:\\stockfish\\stockfish-windows-x86-64.exe"
Args = ""
Id = eh_dll.add_process(Prog.encode('utf-8'), Args.encode('utf-8'))
print(Id)

# pause for 3 seconds
import time
time.sleep(3)

# 10K buffer for data from Stockfish....
Stdout = (" "*10000).encode('utf-8')

Stdin = "go depth 3" + chr(10)

#Go and search
wrote1 = eh_dll.put_stdin(Id, Stdin.encode('utf-8'))
print(wrote1)

#display results
for T in range(8):
 read1 = eh_dll.get_stdout(Id,Stdout)
 if (read1 > 0):
   # can analyse Stockfish results...
   print(read1)
   print( Stdout.decode('utf-8').strip() )
 time.sleep(1)

print(eh_dll.get_status(Id))

eh_dll.kill_process(Id)

print( eh_dll.get_status(Id) )

eh_dll.release_all(0)

print("Ok")


#---------------------------------------
# python output
#
#
#  10448
#  11
#  61
#  Stockfish 17 by the Stockfish developers (see AUTHORS file)
#  563
#  info string Available processors: 0-3
#  info string Using 1 thread
#  info string NNUE evaluation using nn-1111cefa1111.nnue (133MiB, (22528, 3072, 15, 32, 1))
#  info string NNUE evaluation using nn-37f18f62d772.nnue (6MiB, (22528, 128, 15, 32, 1))
#  info depth 1 seldepth 2 multipv 1 score cp 13 nodes 20 nps 6666 hashfull 0 tbhits 0 time 3 pv e2e4
#  info depth 2 seldepth 2 multipv 1 score cp 14 nodes 48 nps 16000 hashfull 0 tbhits 0 time 3 pv c2c3
#  info depth 3 seldepth 2 multipv 1 score cp 22 nodes 76 nps 25333 hashfull 0 tbhits 0 time 3 pv e2e4
#  bestmove e2e4
#  3
#  0
#  Ok
#
