#
# See py_output.txt
#   console output
#
#-----------------------------------------------------
# load dll and test

# provide DLL file exact filename with path

import ctypes
chesslib = ctypes.CDLL("C:\\u64chesslib\\u64chesslib.dll")

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

chesslib.whitecount.argtypes = []
chesslib.whitecount.restype = ctypes.c_int

chesslib.blackcount.argtypes = []
chesslib.blackcount.restype = ctypes.c_int

chesslib.materialdiff.argtypes = []
chesslib.materialdiff.restype = ctypes.c_int

chesslib.sidetomove.argtypes = []
chesslib.sidetomove.restype = ctypes.c_int

chesslib.swaptomove.argtypes = []
chesslib.swaptomove.restype = ctypes.c_int

chesslib.seemslegitpos.argtypes = []
chesslib.seemslegitpos.restype = ctypes.c_bool


#-----------------------------------------------------
#samples of usage

print("1.Starting position....")
chesslib.setstartpos()

#display chess board to console
print( chesslib.sboard().decode() )
print( chesslib.uniq().decode() )
print( chesslib.getfen().decode() )

print( hex( chesslib.polyglotkey() ) )
print( chesslib.spolyglotkey().decode() )
print( chesslib.piecescount() )
print( chesslib.whitecount() )
print( chesslib.blackcount() )

# this is kinda indicator of material, nothing precise
print( chesslib.materialdiff() )

now_tomove = chesslib.swaptomove()
# now opponent should move from same position (nullmove)
# swap back
chesslib.swaptomove()

if( chesslib.sidetomove() == 0):
  print("white to move")
else:
  print("black to move")

chesslib.movegen()
print( chesslib.legalmoves().decode() )

print("2.Moving e4 ......")
print( chesslib.ucimove("e2e4".encode()))
print( chesslib.sboard().decode() )
print( chesslib.getfen().decode() )
# get 70 bytes unique string as a key for this position
savepos = chesslib.uniq().decode()

print("3.Moving back....")
chesslib.undomove()
print( chesslib.getfen().decode() )

chesslib.setasuniq(savepos.encode())
print( chesslib.sboard().decode() )

print( chesslib.parseucimoves("e7e5 g1f3 d7d6".encode()))
print( chesslib.sboard().decode() )

# Try to set strange chess position
# Validator when generating chess positions
print( chesslib.seemslegitpos() )
chesslib.setfen("PPPPPPPP w".encode())
print( chesslib.seemslegitpos() )


#-----------------------------------------------------
print("4.Iterations. Solve puzzle....")
# Functions i_movegen, i_domove, undomove are iterable in depth.
# ucimove, movegen, legalmoves are slower. Do not use them in for-loops.
# Anyway, write it in C if really need performance.

print("Solve checkmate in 2 moves, the right move is 1.Rd8 Kd3 2.Nc5#")

chesslib.setfen("7R/1B1N4/8/3r4/1K2k3/8/5Q2/8 w".encode())
print( chesslib.piecescount() )
print( chesslib.materialdiff() )

print( chesslib.sboard().decode() )

# depth 0 white 1. move
for i0 in range(chesslib.i_movegen(0)):
  chesslib.i_domove(0)

  can_escape = True;

  #depth 1 black 1... move
  for i1 in range(chesslib.i_movegen(1)):
    chesslib.i_domove(1)

    # depth 2 white 2. move
    yee = False
    #if no moves then stalemate
    for i2 in range(chesslib.i_movegen(2)):
      chesslib.i_domove(2)

      if chesslib.ischeckmate():
        # i0,i1,i2 represent moves
        yee = True

      chesslib.undomove()
      if(yee):
        break

    chesslib.undomove()

    can_escape = not yee
    if can_escape:
      break

  chesslib.undomove()

  if(not can_escape):
    chesslib.movegen()
    mv = chesslib.legalmoves().decode().split(" ")
    print("Python says")
    print(mv[i0]);
    break

#-----------------------------------------------------
print("5.Move info and skips....")

# Looking for move that checkmates
# goes this move and prints position

chesslib.setstartpos()
chesslib.parsepgn("1.g4 e5 2.f3".encode())
# depth 0 white 1. move
for i0 in range(chesslib.i_movegen(0)):
  m = chesslib.i_moveinfo(0).decode()

  #
  # m[0]=piece
  # m[1],m[2]=fromsquare "e2"
  # m[3]=goes '-' or captures 'x'
  # m[4],m[5]=tosquare "e4"
  # m[6]=promoted piece
  # m[7]=check flag '+'
  # m[8]=checkmate flag '#'
  # m[9]=piece captured
  # m[10]=castling identifier
  #

  if(m[8]=='#'):
    print(m)
    chesslib.i_domove(0)
    print(chesslib.sboard().decode())
    chesslib.undomove()

  else:
    chesslib.i_skipmove(0)

print("Ok")
