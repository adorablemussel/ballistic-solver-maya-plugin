import sys
import telnetlib

port = 260
if len(sys.argv) > 1:
    port = sys.argv[1]

try:
    tn = telnetlib.Telnet("localhost", port, timeout=3)
    mel_cmd = (
        'evalDeferred("if (!`pluginInfo -q -loaded \\"BallisticSolver\\"`) { '
        'catchQuiet(`loadPlugin \\"BallisticSolver\\"`); '
        'print(\\"BallisticSolver plugin loaded successfully.\\\\n\\"); }");\n'
    )
    tn.write(mel_cmd.encode("ascii"))
    tn.read_until(b"\x00", timeout=3)
    tn.close()
except:
    pass