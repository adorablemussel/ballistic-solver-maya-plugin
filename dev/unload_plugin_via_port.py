import sys
import telnetlib

port = 260

if len(sys.argv) > 1:
    port = sys.argv[1]
    
try:
    tn = telnetlib.Telnet("localhost", port)
    tn.write('file -newFile -force;'.encode())
    tn.write('catchQuiet(`unloadPlugin "BallisticSolver"`);'.encode())
    tn.close()
except:
    pass
