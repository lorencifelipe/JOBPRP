import os

def runGA(dir, folder, lytFileName, ordFileName, seed):
    if not os.path.exists(dir+"/ga_results_iraceParameters_newHDistance"):
        os.makedirs("ga_results_iraceParameters_newHDistance")
        print("Created folder /ga_results_iraceParameters_newHDistance \n")
    if not os.path.exists(dir+"/ga_results_iraceParameters_newHDistance/henn"):
        os.chdir(dir+"/ga_results_iraceParameters_newHDistance")
        os.makedirs("henn")
        print("Created folder /ga_results_iraceParameters_newHDistance/henn \n")
    os.chdir(dir+"/ga_results_iraceParameters_newHDistance/henn")
    if not os.path.exists(dir+"/ga_results_iraceParameters_newHDistance/henn/"+folder):
        os.makedirs(folder)
        print("Created folder /ga_results_iraceParameters_newHDistance/henn/"+folder)
    os.chdir(dir+"/ga_results_iraceParameters_newHDistance/henn/"+folder)
    newFile = folder+"_"+ordFileName[:-4]+"_"+"seed"+"_"+str(seed)+".out"
    if not os.path.isfile(dir+"/ga_results_iraceParameters_newHDistance/henn/"+folder+"/"+newFile):
        print("Working at " + newFile + " file... \n")
        os.system(dir + "/sources/jobprp_ga_v1 " + dir + "/reduced_instances/henn/" + folder + "/" + lytFileName +
                  " " + dir + "/reduced_instances/henn/" + folder + "/" + ordFileName + " " + str(seed) + " 340 120 1 500 > " + newFile)
        print("Created file " + newFile + "\n")
    else:
        print("Sorry. This file is already created. \n")
    os.chdir(dir)

def selectHennFiles():
    dir = os.getcwd()
    folders = os.listdir(dir+"/reduced_instances/henn")
    for folder in folders:
        files = os.listdir(dir+"/reduced_instances/henn/"+folder)
        files.sort()
        i = 0
        while(i < len(files)):
            seeds = range(1, 31)
            for seed in seeds:
                runGA(dir, folder, files[i], files[i+1], seed)
            i += 2

selectHennFiles()
