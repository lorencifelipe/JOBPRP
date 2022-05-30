import os

def runHeuristic(dir, folder, lytFileName, ordFileName):
    if not os.path.exists(dir+"/heuristic_results"):
        os.makedirs("heuristic_results")
        print("Created folder /heuristic_results")
    if not os.path.exists(dir+"/heuristic_results/henn"):
        os.chdir(dir+"/heuristic_results")
        os.makedirs("henn")
        print("Created folder /henn")
    os.chdir(dir+"/heuristic_results/henn")
    if not os.path.exists(dir+"/heuristic_results/henn/"+folder):
        os.makedirs(folder)
        print("Created folder /"+folder)
    os.chdir(dir+"/heuristic_results/henn/"+folder)
    newFile = folder+"_"+ordFileName[:-4]+".out"
    print("Working at " + newFile + " file... \n")
    os.system(dir + "/heuristic " + dir + "/reduced_instances/henn/" + folder + "/" + lytFileName + " " + dir + "/reduced_instances/henn/" + folder + "/" + ordFileName + " > " + newFile)
    print("Created file " + newFile + "\n")

def selectHennFiles():
    dir = os.getcwd()
    folders = os.listdir(dir+"/reduced_instances/henn")
    for folder in folders:
        files = os.listdir(dir+"/reduced_instances/henn/"+folder)
        files.sort()
        i = 0
        while(i < len(files)):
            runHeuristic(dir,folder,files[i],files[i+1])
            i+=2

selectHennFiles()