import tkinter as tk
from tkinter import filedialog
import pathlib
import os.path
import os



# set shader compiler path
shaderCompiler = '/Users/pine/lib/vulkansdk-macos-1.1.130.0/macOS/bin/glslc'

def BuildShaderFile(shaderBuilder, sourceFilePath, outputFileName):
	outputPath = os.path.join(str(pathlib.Path(sourceFilePath).parent), outputFileName);
	result = os.system(shaderBuilder + " " + sourceFilePath + " -o " + outputPath)
	if not (int(result) == 0) :
		print("Shader compile Failed! : " + sourceFilePath)
		return False

	# print(sourceFilePath + ". output " + outputPath)
	return True



def browse_button():
    # Allow user to select a directory and store it in global var
    filename = tk.filedialog.askdirectory()
    return filename



def BuildShaderRecursive(sourcePath):
	shaderExtension = ['.vert', '.frag', '.comp']
	shaderfiles = []

	files = os.listdir(sourcePath)	
	for f in files:
		# print(f)
		fullPath = os.path.join(sourcePath,f)
		if os.path.isfile(fullPath) :
			extension = pathlib.Path(f).suffix
			if extension in shaderExtension:
				shaderfiles.append(fullPath)

	compileDoneNum = 0
	compileFailNum = 0
	for shaderFile in shaderfiles :
		outputFileName = pathlib.Path(shaderFile).name + '.spv'
		result = BuildShaderFile(shaderCompiler, shaderFile, outputFileName)
		if result :
			compileDoneNum = compileDoneNum + 1
		else:
			compileFailNum = compileFailNum + 1


	print("")
	print("")
	print(str(compileDoneNum) + " shader compiled")
	print(str(compileFailNum) + " shader compile failed!")







window = tk.Tk()
top_frame = tk.Frame(window)
folder_path = tk.StringVar()

lbl1 = tk.Label(master=window,textvariable=folder_path,width=50)
lbl1.grid(row=0, column=1)
selectBuildRootButton = tk.Button(text="Browse", command=lambda : folder_path.set(browse_button()))
selectBuildRootButton.grid(row=0, column=3)

selectBuildRootButton = tk.Button(text="Build", command=lambda : BuildShaderRecursive(folder_path.get()))
selectBuildRootButton.grid(row=0, column=4)






tk.mainloop()