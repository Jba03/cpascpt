import os
import shutil

if os.path.exists("parser"):
    shutil.rmtree("parser")
    
os.mkdir("parser")
if os.system("antlr4 -Dlanguage=Cpp Generic.g4 -o parser") != 0:
    # Try with "antlr" only
    os.system("antlr -Dlanguage=Cpp Generic.g4 -o parser")
