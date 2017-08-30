Robot: Lego Mindstorms NXT 2.0
Neural network: Mona 5.2
Middleware: LeJOS NXJ 0.9.0 beta

1. See robot.jpg for construction guide. Range sensor plugs into port S3.
   Color sensor plugs into port S2.
2. Obtain LeJOS from lejos.sourceforge.net.
2. Obtain Mona from mona.codeplex.com. After building, copy mona.jar and
   mona_jni.dll from Mona installation to lib folder.
3. Build with bin/build.bat.
4. Turn on robot and run with bin/run.bat.
5. Use dashboard:
   a. Observe sensory input and response output.
   b. Observe and modify need values:
      - Success: need to accomplish task (you need to code this).
      - Reward: need to acquire treat.
   c. Control responses:
      - Atani: robot generates responses.
      - Override: robot learns from forced responses.
      - Hijack: robot is unaware of forced responses.
   d. Manage rewards:
      - create reward goals from current sensory state.
      - apply treat rewards.
