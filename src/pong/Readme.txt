The Pong game environment.

Build pong_world and pong_lens using the makefile or VS solution.

To train and test Mona:
pong_world -trainingRuns 50 -testingRuns 50 -verbose -ballSpeed .1 -trainingRandomSeed 38 -testingRandomSeed 93

To create an input train or test set for Elman using Lens:
pong_world -trainingRuns 50 -createLensInput pong_lens_input.ex -verbose -ballSpeed .1 -trainingRandomSeed 38

To train and test Elman:
pong_lens -trainingExamples pong_lens_training.ex -trainingEpochs 5000 -testingExamples pong_lens_testing.ex

To create an input train or test set for NuPIC:
pong_world -trainingRuns 50 -createNuPICinput pong_nupic_input.txt -verbose -ballSpeed .1 -trainingRandomSeed 38

To train and test NuPIC, use the Hot Gym sample for an example: https://github.com/numenta/nupic/tree/master/examples/opf/clients/hotgym

To create an input train or test set for MoxWorx:
pong_world -trainingRuns 50 -createMoxWorxInput pong_moxworx_input.txt -verbose -ballSpeed .1 -trainingRandomSeed 38

To train and test MoxWorx, refer to https://github.com/portegys/MoxWorx.
