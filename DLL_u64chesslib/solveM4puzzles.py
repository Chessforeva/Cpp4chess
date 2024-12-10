#-----------------------------------------------------
#
#
#  This sample does python chess processing.
#  Reads chess puzzles from M4_gen_363_puzzles.pgn
#  Processes all puzzles on 10 Stockfish instances like threads for better CPU usage.
#  Verifies bestmove results and writes to the log file M4log.txt
#
#


#-----------------------------------------------------
# u64chesslib.dll
#
# load dll and test

# provide DLL file exact filename with path

import ctypes
chesslib = ctypes.CDLL("C:\\chesslib\\u64chesslib.dll")

#-----------------------------------------------------
#declaration of paramaters and return values of DLL function methods

chesslib.setstartpos.argtypes = []
chesslib.setstartpos.restype = None

chesslib.sboard.argtypes = []
chesslib.sboard.restype = ctypes.c_char_p

chesslib.uniq.argtypes = []
chesslib.uniq.restype = ctypes.c_char_p

chesslib.setasuniq.argtypes = [ ctypes.c_char_p ]
chesslib.setasuniq.restype = None

chesslib.getfen.argtypes = []
chesslib.getfen.restype = ctypes.c_char_p

chesslib.setfen.argtypes = [ ctypes.c_char_p ]
chesslib.setfen.restype = None

chesslib.movegen.argtypes = []
chesslib.movegen.restype = ctypes.c_int

chesslib.legalmoves.argtypes = []
chesslib.legalmoves.restype = ctypes.c_char_p

chesslib.ucimove.argtypes = [ ctypes.c_char_p ]
chesslib.ucimove.restype = ctypes.c_int

chesslib.parseucimoves.argtypes = [ ctypes.c_char_p ]
chesslib.parseucimoves.restype = ctypes.c_int

chesslib.undomove.argtypes = []
chesslib.undomove.restype = None

chesslib.parsepgn.argtypes = [ ctypes.c_char_p ]
chesslib.parsepgn.restype = ctypes.c_char_p

chesslib.ischeck.argtypes = []
chesslib.ischeck.restype = ctypes.c_bool

chesslib.ischeck.argtypes = []
chesslib.ischeckmate.restype = ctypes.c_bool

chesslib.polyglotkey.argtypes = []
chesslib.polyglotkey.restype = ctypes.c_ulonglong

chesslib.spolyglotkey.argtypes = []
chesslib.spolyglotkey.restype = ctypes.c_char_p

chesslib.i_movegen.argtypes = [ ctypes.c_int ]
chesslib.i_movegen.restype = ctypes.c_int

chesslib.i_domove.argtypes = [ ctypes.c_int ]
chesslib.i_domove.restype = ctypes.c_int

chesslib.i_moveinfo.argtypes = [ ctypes.c_int ]
chesslib.i_moveinfo.restype = ctypes.c_char_p

chesslib.i_skipmove.argtypes = [ ctypes.c_int ]
chesslib.i_skipmove.restype = None

chesslib.piecescount.argtypes = []
chesslib.piecescount.restype = ctypes.c_int

chesslib.materialdiff.argtypes = []
chesslib.materialdiff.restype = ctypes.c_int

#-----------------------------------------------------
# ExeHandler64.dll

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

#
#not needed, can use python memory directly from memory variables
#intended if memory should be accessed in DLL inside
#
eh_dll.char_100kb_buffer.argtypes = [ ctypes.c_int ]
eh_dll.char_100kb_buffer.restype = ctypes.c_char_p

eh_dll.assign_memory_i_n.argtypes = [ ctypes.c_int, ctypes.c_int, ctypes.c_int ]
eh_dll.assign_memory_i_n.restype = ctypes.c_int

eh_dll.getfrom_memory_i_n.argtypes = [ ctypes.c_int, ctypes.c_int ]
eh_dll.getfrom_memory_i_n.restype = ctypes.c_int

eh_dll.add_process_i.argtypes = [ ctypes.c_int, ctypes.c_int ]
eh_dll.add_process_i.restype = ctypes.c_int

eh_dll.put_stdin_i.argtypes = [ ctypes.c_int,  ctypes.c_int ]
eh_dll.put_stdin_i.restype = ctypes.c_int

eh_dll.get_stdout_i.argtypes = [ ctypes.c_int,  ctypes.c_int ]
eh_dll.get_stdout_i.restype = ctypes.c_int

#-----------------------------------
#
# Run 10 Stockfish instances and process positions of M4_gen_363_puzzles.pgn
#
Threads = 10

# read fen positions, store into array

file = open("M4_gen_363_puzzles.pgn", "r")
content = file.read()
posspl = content.split("[FEN ")
file.close()

FENs = []
NotedSolution = []
PuzzleStatus = []
StockFishSolves = []
Solutions = []

N = 0
for pp in posspl:
  a = pp.rfind("]")
  if(a>0):

    # chess position
    FENs.append( pp[1:a-1] )
    NotedSolution.append( pp[a+3:].strip() )

    # current status of solving 0=not started, 1=solving, 2=solved
    PuzzleStatus.append( 0 )

    # which thread works on it
    StockFishSolves.append( -1 )

    Solutions.append( "" )

    N = N + 1

#use Stockfish as a chess engine
Prog = "C:\\stockfish\\stockfish-windows-x86-64.exe"
Args = ""

# Stockfish threads as processes, stdin, stdout streams
Stockf = []
StockfStatus = []
Stdout = []
Stdin = []

# Prepare
for I in range(0,Threads):
  Stockf.append( eh_dll.add_process(Prog.encode('utf-8'), Args.encode('utf-8')) )

  # What this Stockfish is doing currently: 0=idle, 1=working
  StockfStatus.append( 0 )

  Stdout.append( "" )
  Stdin.append( "" )

# The main loop till everything solved
Solved = False
while (not Solved):

  Solved = True
  for n in range(0,N):
    if PuzzleStatus[n] == 0:
      Solved = False
      for I in range(0,Threads):
        if StockfStatus[I] == 0:
          #it is free Stockfish
          StockFishSolves[n] = I
          PuzzleStatus[n] = 1
          StockfStatus[I] = 1
          Stdin[I] = "position fen " + FENs[n] + chr(10) + "go mate 4" + chr(10)
          eh_dll.put_stdin(Stockf[I], Stdin[I].encode('utf-8'))
          Stdout[I] = ""

    if PuzzleStatus[n] == 1:
      Solved = False
      I = StockFishSolves[n]

      # 10K buffer for data from Stockfish....
      readstr = (" "*10000).encode('utf-8')

      read = eh_dll.get_stdout(Stockf[I],readstr)
      if (read > 0):
        # can analyse Stockfish results...
        Stdout[I] = Stdout[I] + readstr.decode('utf-8').strip()
        a = Stdout[I].rfind("bestmove ")
        if(a>0):
          Solutions[n] = Stdout[I][a+9:a+14]
          StockFishSolves[n] = -1
          PuzzleStatus[n] = 2
          StockfStatus[I] = 0
          print( "Solved puzzle " + str(n) + " bestmove " + Solutions[n] )

for I in range(0,Threads):
  eh_dll.kill_process(Stockf[I])

eh_dll.release_all(0)



#write solutions to the file
file = open("M4log.txt", "w")
file.write("chesslib.uniq()" + (" "*55) + "Stockfish \n")
for n in range(0,N):

  #verify and write to file
  chesslib.setstartpos()
  chesslib.setfen(FENs[n].encode('utf-8'))
  uniq = chesslib.uniq().decode('utf-8')
  ucismade = chesslib.parsepgn( NotedSolution[n].encode('utf-8') ).decode('utf-8') + (" "*4)
  if ucismade[0:4] != Solutions[n][0:4]:
    print("Wtf! Solutions differ?!")
    isok = "??"
  else:
    isok = "ok"

  file.write( uniq + " " + Solutions[n] + " " + isok + "\n" )

file.close()

print("Ok")
