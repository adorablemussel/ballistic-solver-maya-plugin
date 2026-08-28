import maya.cmds as cmds

# closes all ports
for port in cmds.commandPort(query = True, listPorts = True):
    cmds.commandPort(name = port, close = True)

# # 260, 261: 
#     # "26_" - Maya version
#     # "__0" - mel
#     # "__1" - python 
# cmds.commandPort(name = ":260", close = True)
# cmds.commandPort(name = ":261", close = True)

print(cmds.commandPort(query = True, listPorts = True))