import sys
import telnetlib

port = 260

if len(sys.argv) > 1:
    port = sys.argv[1]
    
try:
    tn = telnetlib.Telnet("localhost", port)
    tn.write('catchQuiet(`loadPlugin "BallisticSolver"`);'.encode())
    tn.close()
except:
    pass
