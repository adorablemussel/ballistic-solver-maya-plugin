import maya.cmds as cmds

# 260, 261: 
    # "26_" - Maya version
    # "__0" - mel
    # "__1" - python 
cmds.commandPort(name = ":260", sourceType = "mel")
cmds.commandPort(name = ":261", sourceType = "python")

print(cmds.commandPort(q = True, listPorts = True))