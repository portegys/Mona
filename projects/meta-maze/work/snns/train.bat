# SNNS batch file.
loadNet("network.net")
loadPattern("pattern.pat")
#setInitFunc("JE_Weights", 1, -1, 0, 1, 0)
#initNet()
trainNet()
saveResult("results.res", 1, PAT, TRUE, TRUE, "append")
saveNet("network.net")
