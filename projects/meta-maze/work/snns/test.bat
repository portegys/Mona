# SNNS batch file.
loadNet("network.net")
loadPattern("pattern.pat")
testNet()
saveResult("results.res", 1, PAT, TRUE, TRUE, "append")

