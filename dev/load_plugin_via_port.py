import socket
import sys
import time


def wait_for_reply(sock, timeout=3):
    # Same timing as the old telnetlib read_until(b"\x00", timeout=3).
    # telnetlib drops NUL bytes from incoming data, so that call never
    # matched and always read until the port closed or the timeout ran out.
    deadline = time.monotonic() + timeout
    while True:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            return
        sock.settimeout(remaining)
        try:
            if not sock.recv(4096):
                return
        except socket.timeout:
            return

port = 260
if len(sys.argv) > 1:
    port = sys.argv[1]

try:
    tn = socket.create_connection(("localhost", port), timeout=3)
    mel_cmd = (
        'evalDeferred("if (!`pluginInfo -q -loaded \\"BallisticSolver\\"`) { '
        'catchQuiet(`loadPlugin \\"BallisticSolver\\"`); '
        'print(\\"BallisticSolver plugin loaded successfully.\\\\n\\"); }");\n'
    )
    tn.sendall(mel_cmd.encode("ascii"))
    wait_for_reply(tn, timeout=3)
    tn.close()
except:
    pass