import maya.cmds as cmds
import os

folder_path = "C:/Users/Szymon/Desktop/PRACA_INZYNIERSKA/BallisticSolver/dev/IFV-Borsuk"

for file_name in os.listdir(folder_path):
    if file_name.lower().endswith(".stl"):
        full_path = os.path.join(folder_path, file_name)
        safe_name = file_name.replace(" ", "_").replace(".stl", "")
        
        print(f"Loading: {file_name}...")
        try:
            cmds.file(full_path, i=True, type="STLImport", ignoreVersion=True, renameAll=True, namespace=safe_name)
        except Exception as e:
            print(f"Cannot load {file_name}. Error: {e}")

print("IFV-Borsuk files has been loaded!")