# 260, 261: 
    # "26_" - Maya version
    # "__0" - mel
    # "__1" - python 
import os
import maya.cmds as cmds
import maya.OpenMaya as om

plugin_dir = r"C:\Users\Szymon\Documents\maya\2026\plug-ins"
if plugin_dir.lower() not in os.environ.get("PATH", "").lower():
    os.environ["PATH"] = plugin_dir + ";" + os.environ.get("PATH", "")

cmds.commandPort(name=":260", sourceType="mel")
cmds.commandPort(name=":261", sourceType="python")

print(cmds.commandPort(q=True, listPorts=True))
om.MGlobal.displayInfo("Development Ports and PATH are now configured.")