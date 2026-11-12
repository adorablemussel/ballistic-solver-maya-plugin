import sys
import time
import telnetlib

port = 260
if len(sys.argv) > 1:
    port = sys.argv[1]

try:
    tn = telnetlib.Telnet("localhost", port, timeout=3)

    cleanup_cmd = (
        'play -state off; '
        'select -clear; '
        'file -newFile -force; '
        'flushUndo; '
        'ogs -reset; '
        'refresh -force;\n'
    )
    tn.write(cleanup_cmd.encode("ascii"))
    tn.read_until(b"\x00", timeout=3)

    time.sleep(0.4)

    unload_cmd = (
        'if (`pluginInfo -q -loaded "BallisticSolver"`) { '
        '    catchQuiet(`unloadPlugin "BallisticSolver"`); '
        '} '
        'print("BallisticSolver plugin unloaded successfully.\\n");\n'
    )
    tn.write(unload_cmd.encode("ascii"))
    tn.read_until(b"\x00", timeout=3)
    tn.close()

    time.sleep(0.2)
except:
    pass