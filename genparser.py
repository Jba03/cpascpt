import os
import shutil

if os.path.exists("parser"):
    shutil.rmtree("parser")
    
os.mkdir("parser")
os.system("antlr4 -Dlanguage=Cpp Generic.g4 -o parser")
