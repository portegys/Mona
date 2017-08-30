/*
 * Evolve a system of mox foragers and predators
 * by mutating and recombining parameters.
 */

import java.util.*;
import java.io.*;
import mona.NativeFileDescriptor;

public class EvolveMoxenSystem
{
   // Usage.
   public static final String Usage =
      "Usage:\n" +
      "  New run:\n" +
      "    java EvolveMoxenSystem\n" +
      "      -generations <evolution generations>\n" +
      "      -steps <moxen steps>\n" +
      "     [-stepGameOfLife]\n" +
      "      -moxPopulations \"" + EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.getName() +
      "\" | \"" + EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.getName() + "\"\n" +
      "      -loadCells <file name>\n" +
      "      -output <evolution output file name>\n" +
      "     [-mutationRate <mutation rate>]\n" +
      "     [-randomMutationRate <random mutation rate>]\n" +
      "     [-maxSensorRange <maximum sensor range>]\n" +
      "     [-randomSeed <random seed> (for new run)]\n" +
      "     [-logfile <log file name>]\n" +
      "     [-dashboard (run with mox world dashboard)]\n" +
      "  Resume run:\n" +
      "    java EvolveMoxenSystem\n" +
      "      -generations <evolution generations>\n" +
      "      -steps <moxen steps>\n" +
      "     [-stepGameOfLife]\n" +
      "      -input <evolution input file name>\n" +
      "      -output <evolution output file name>\n" +
      "     [-mutationRate <mutation rate>]\n" +
      "     [-randomMutationRate <random mutation rate>]\n" +
      "     [-maxSensorRange <maximum sensor range>]\n" +
      "     [-randomSeed <random seed> (for new run)]\n" +
      "     [-logfile <log file name>]\n" +
      "     [-dashboard (run with mox world dashboard)]\n" +
      "  Extract into mox_world_forager|predator_<mox id>.mw files:\n" +
      "    java EvolveMoxenSystem\n" +
      "      -extract\n" +
      "      -input <evolution input file name>\n" +
      "  Print evolved property values:\n" +
      "    java EvolveMoxenSystem\n" +
      "      -print\n" +
      "      -input <evolution input file name>";

   // Generations.
   int Generation;
   int Generations;

   // Steps.
   int Steps;

   // Step Game of Life?
   boolean StepGameOfLife;

   // Mox populations.
   EvolveCommon.MOX_POPULATIONS MoxPopulations;

   // File names.
   String      CellsFileName;
   String      InputFileName;
   String      OutputFileName;
   String      LogFileName;
   PrintWriter LogWriter;

   // Random numbers.
   Random Randomizer;

   // Run with dashboard.
   boolean Dashboard;

   // Extract moxen files.
   boolean Extract;

   // Print moxen property values.
   boolean PrintProperties;

   // Maximum mox cycle time.
   long MaxMoxCycleTime;

   // Mox world.
   MoxWorld moxWorld;

   // Populations.
   EvolveCommon.Member[] ForagerPopulation;
   EvolveCommon.Member[] PredatorPopulation;

   // Constructor.
   public EvolveMoxenSystem(String[] args)
   {
      int i;

      // Get options.
      Generation     = 0;
      Generations    = -1;
      Steps          = -1;
      StepGameOfLife = false;
      boolean gotStepGameOfLife = false;
      MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
      boolean gotMoxPopulations = false;
      CellsFileName = null;
      InputFileName = OutputFileName = LogFileName = null;
      LogWriter     = null;
      boolean gotMutationRate       = false;
      boolean gotRandomMutationRate = false;
      boolean gotMaxSensorRange     = false;
      boolean gotRandomSeed         = false;
      Extract         = false;
      PrintProperties = false;
      Dashboard       = false;
      for (i = 0; i < args.length; i++)
      {
         if (args[i].equals("-generations"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            Generations = Integer.parseInt(args[i]);
            if (Generations < 0)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            continue;
         }

         if (args[i].equals("-steps"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            Steps = Integer.parseInt(args[i]);
            if (Steps < 0)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            continue;
         }

         if (args[i].equals("-stepGameOfLife"))
         {
            StepGameOfLife    = true;
            gotStepGameOfLife = true;
            continue;
         }

         if (args[i].equals("-moxPopulations"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            if (args[i].equals(EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.getName()))
            {
               MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
            }
            else if (args[i].equals(EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.getName()))
            {
               MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS;
            }
            else
            {
               System.err.println(Usage);
               System.exit(1);
            }
            gotMoxPopulations = true;
            continue;
         }

         if (args[i].equals("-loadCells"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            if (CellsFileName == null)
            {
               CellsFileName = new String(args[i]);
            }
            else
            {
               System.err.println(Usage);
               System.exit(1);
            }
            continue;
         }

         if (args[i].equals("-input"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            InputFileName = new String(args[i]);
            continue;
         }

         if (args[i].equals("-output"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            OutputFileName = new String(args[i]);
            continue;
         }

         if (args[i].equals("-mutationRate"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            EvolveCommon.MutationRate = Double.parseDouble(args[i]);
            if ((EvolveCommon.MutationRate < 0.0) || (EvolveCommon.MutationRate > 1.0))
            {
               System.err.println(Usage);
               System.exit(1);
            }
            gotMutationRate = true;
            continue;
         }

         if (args[i].equals("-randomMutationRate"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            EvolveCommon.RandomMutationRate = Double.parseDouble(args[i]);
            if ((EvolveCommon.RandomMutationRate < 0.0) || (EvolveCommon.RandomMutationRate > 1.0))
            {
               System.err.println(Usage);
               System.exit(1);
            }
            gotRandomMutationRate = true;
            continue;
         }

         if (args[i].equals("-maxSensorRange"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            EvolveCommon.MaxSensorRange = Float.parseFloat(args[i]);
            if (EvolveCommon.MaxSensorRange < 0.0f)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            gotMaxSensorRange = true;
            continue;
         }

         if (args[i].equals("-randomSeed"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            EvolveCommon.RandomSeed = Integer.parseInt(args[i]);
            gotRandomSeed           = true;
            continue;
         }

         if (args[i].equals("-logfile"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println(Usage);
               System.exit(1);
            }
            LogFileName = new String(args[i]);
            continue;
         }

         if (args[i].equals("-dashboard"))
         {
            Dashboard = true;
            continue;
         }

         if (args[i].equals("-extract"))
         {
            Extract = true;
            continue;
         }

         if (args[i].equals("-print"))
         {
            PrintProperties = true;
            continue;
         }

         System.err.println(Usage);
         System.exit(1);
      }

      // Extract moxen files or print properties?
      if (Extract || PrintProperties)
      {
         if ((Generations != -1) || (Steps != -1) || gotStepGameOfLife ||
             gotMoxPopulations || (InputFileName == null) ||
             (OutputFileName != null) || (CellsFileName != null) ||
             (LogFileName != null) || gotMutationRate || gotRandomMutationRate ||
             gotMaxSensorRange || gotRandomSeed || Dashboard)
         {
            System.err.println(Usage);
            System.exit(1);
         }
         if (Extract && PrintProperties)
         {
            System.err.println(Usage);
            System.exit(1);
         }
      }
      else
      {
         if (Generations == -1)
         {
            System.err.println("Generations option required");
            System.err.println(Usage);
            System.exit(1);
         }

         if (Steps == -1)
         {
            System.err.println("Steps option required");
            System.err.println(Usage);
            System.exit(1);
         }

         if (OutputFileName == null)
         {
            System.err.println("Output file required");
            System.err.println(Usage);
            System.exit(1);
         }

         if (InputFileName != null)
         {
            if (gotMoxPopulations || (CellsFileName != null))
            {
               System.err.println(Usage);
               System.exit(1);
            }
         }
         else
         {
            if (!gotMoxPopulations || (CellsFileName == null))
            {
               System.err.println(Usage);
               System.exit(1);
            }
         }
      }

      // Set maximum sensor range.
      ForagerMox.MAX_SENSOR_RANGE  = EvolveCommon.MaxSensorRange;
      PredatorMox.MAX_SENSOR_RANGE = EvolveCommon.MaxSensorRange;

      // Seed random numbers.
      Randomizer = new Random(EvolveCommon.RandomSeed);

      // Open log file?
      if (LogFileName != null)
      {
         try
         {
            LogWriter = new PrintWriter(new FileOutputStream(new File(LogFileName)));
         }
         catch (Exception e) {
            System.err.println("Cannot open log file " + LogFileName +
                               ":" + e.getMessage());
            System.exit(1);
         }
      }
   }


   // Start evolve.
   public void start()
   {
      // Initialize populations?
      if (InputFileName == null)
      {
         init();
      }
      else
      {
         // Load populations.
         load();
      }

      // Log run.
      log("Initializing evolve:");
      log("  Options:");
      log("    generations=" + Generations);
      log("    steps=" + Steps);
      if (StepGameOfLife)
      {
         log("    stepGameOfLife=true");
      }
      else
      {
         log("    stepGameOfLife=false");
      }
      if (InputFileName == null)
      {
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
         {
            log("    MoxPopulations=" + EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.getName());
         }
         else
         {
            log("    MoxPopulations=" + EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.getName());
         }
         log("    loadCells=" + CellsFileName);
      }
      else
      {
         log("    input=" + InputFileName);
      }
      log("    output=" + OutputFileName);
      log("    MutationRate=" + EvolveCommon.MutationRate);
      log("    RandomMutationRate=" + EvolveCommon.RandomMutationRate);
      log("    MaxSensorRange=" + EvolveCommon.MaxSensorRange);
      log("    RandomSeed=" + EvolveCommon.RandomSeed);
      log("  Parameters:");
      log("    FORAGER_FIT_POPULATION_SIZE=" + EvolveCommon.FORAGER_FIT_POPULATION_SIZE);
      log("    FORAGER_NUM_MUTANTS=" + EvolveCommon.FORAGER_NUM_MUTANTS);
      log("    FORAGER_NUM_OFFSPRING=" + EvolveCommon.FORAGER_NUM_OFFSPRING);
      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         log("    PREDATOR_FIT_POPULATION_SIZE=" + EvolveCommon.PREDATOR_FIT_POPULATION_SIZE);
         log("    PREDATOR_NUM_MUTANTS=" + EvolveCommon.PREDATOR_NUM_MUTANTS);
         log("    PREDATOR_NUM_OFFSPRING=" + EvolveCommon.PREDATOR_NUM_OFFSPRING);
      }

      // Extract moxen files?
      if (Extract)
      {
         extract();
         return;
      }

      // Print moxen properties?
      if (PrintProperties)
      {
         printProperties();
         return;
      }

      // Set maximum mox cycle time according to current running conditions.
      MaxMoxCycleTime = Mox.getMaxCycleTime();

      // Evolution loop.
      log("Begin evolve:");
      for (Generations += Generation; Generation < Generations; Generation++)
      {
         log("Generation=" + Generation);

         evolve(Generation);

         // Save populations?
         if ((Generation % EvolveCommon.SAVE_FREQUENCY) == 0)
         {
            save(Generation);
         }
      }

      // Save populations.
      save(Generation - 1);

      log("End evolve");
   }


   // Initialize evolution.
   void init()
   {
      int i;

      try
      {
         moxWorld = new MoxWorld();
         moxWorld.loadCells(CellsFileName);
      }
      catch (Exception e) {
         System.err.println("Cannot load cells file " +
                            CellsFileName +
                            ":" + e.getMessage());
         System.exit(1);
      }

      ForagerPopulation = new EvolveCommon.Member[EvolveCommon.FORAGER_POPULATION_SIZE];
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         if (i == 0)
         {
            ForagerPopulation[i] =
               new EvolveCommon.Member(Mox.SPECIES.FORAGER.getValue(), 0,
                                       0, 0, Mox.DIRECTION.NORTH.getValue(), Randomizer);
         }
         else
         {
            // Mutate parameters.
            ForagerPopulation[i] =
               new EvolveCommon.Member(ForagerPopulation[0], 0, Randomizer);
         }
      }
      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         PredatorPopulation = new EvolveCommon.Member[EvolveCommon.PREDATOR_POPULATION_SIZE];
         for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            if (i == 0)
            {
               PredatorPopulation[i] =
                  new EvolveCommon.Member(Mox.SPECIES.PREDATOR.getValue(), 0,
                                          moxWorld.getWidth() - 1,
                                          moxWorld.getHeight() - 1,
                                          Mox.DIRECTION.SOUTH.getValue(), Randomizer);
            }
            else
            {
               // Mutate parameters.
               PredatorPopulation[i] =
                  new EvolveCommon.Member(PredatorPopulation[0], 0, Randomizer);
            }
         }
      }
   }


   // Load evolution.
   void load()
   {
      int                  i;
      FileInputStream      input  = null;
      NativeFileDescriptor fd     = null;
      DataInputStream      reader = null;

      // Open the file.
      try
      {
         input  = new FileInputStream(new File(InputFileName));
         reader = new DataInputStream(input);
         fd     = new NativeFileDescriptor(InputFileName, "r");
         fd.open();
      }
      catch (Exception e) {
         System.err.println("Cannot open input file " + InputFileName +
                            ":" + e.getMessage());
      }

      try
      {
         Generation = Utility.loadInt(reader);
         Generation++;
      }
      catch (Exception e) {
         System.err.println("Cannot load from file " + InputFileName +
                            ":" + e.getMessage());
         System.exit(1);
      }

      // Load mox world.
      try
      {
         moxWorld = new MoxWorld();
         moxWorld.load(input, fd);
      }
      catch (Exception e) {
         System.err.println("Cannot load mox world from file " + InputFileName +
                            ":" + e.getMessage());
         System.exit(1);
      }

      // Load populations.
      try
      {
         if (Utility.loadInt(reader) == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.getValue())
         {
            MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
         }
         else
         {
            MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS;
         }
         ForagerPopulation = new EvolveCommon.Member[EvolveCommon.FORAGER_POPULATION_SIZE];
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            ForagerPopulation[i] =
               new EvolveCommon.Member(Mox.SPECIES.FORAGER.getValue(), 0, 0, 0, 0, Randomizer);
            ForagerPopulation[i].load(input, fd);
         }
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            PredatorPopulation = new EvolveCommon.Member[EvolveCommon.PREDATOR_POPULATION_SIZE];
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               PredatorPopulation[i] =
                  new EvolveCommon.Member(Mox.SPECIES.PREDATOR.getValue(), 0, 0, 0, 0, Randomizer);
               PredatorPopulation[i].load(input, fd);
            }
         }

         input.close();
         fd.close();
      }
      catch (Exception e) {
         System.err.println("Cannot load populations from file " + InputFileName +
                            ":" + e.getMessage());
         System.exit(1);
      }
   }


   // Save evolution.
   void save(int generation)
   {
      int                  i;
      FileOutputStream     output = null;
      NativeFileDescriptor fd     = null;
      PrintWriter          writer = null;

      try
      {
         output = new FileOutputStream(new File(OutputFileName));
         writer = new PrintWriter(output);
         fd     = new NativeFileDescriptor(OutputFileName, "w");
         fd.open();
      }
      catch (Exception e) {
         System.err.println("Cannot open output file " + OutputFileName +
                            ":" + e.getMessage());
         System.exit(1);
      }

      try
      {
         Utility.saveInt(writer, generation);
         writer.flush();
      }
      catch (Exception e) {
         System.err.println("Cannot save to file " + OutputFileName +
                            ":" + e.getMessage());
         System.exit(1);
      }

      // Save mox world.
      try
      {
         moxWorld.save(output, fd);
      }
      catch (Exception e) {
         System.err.println("Cannot save mox world to file " + OutputFileName +
                            ":" + e.getMessage());
         System.exit(1);
      }

      // Save populations.
      try
      {
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
         {
            Utility.saveInt(writer, EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.getValue());
         }
         else
         {
            Utility.saveInt(writer, EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.getValue());
         }
         writer.flush();
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            ForagerPopulation[i].save(output, fd);
         }
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               PredatorPopulation[i].save(output, fd);
            }
         }

         output.close();
         fd.close();
      }
      catch (Exception e) {
         System.err.println("Cannot save populations to file " + OutputFileName +
                            ":" + e.getMessage());
         System.exit(1);
      }
   }


   // Evolution generation.
   void evolve(int generation)
   {
      // Evaluate member fitness.
      evaluate(generation);

      // Prune unfit members.
      prune();

      // Create new members by mutation.
      mutate();

      // Create new members by mating.
      mate();
   }


   // Evaluate member fitnesses.
   void evaluate(int generation)
   {
      int i, step;
      int blueFoodNeedIdx, moxFoodNeedIdx;
      Mox mox;

      ArrayList<Mox> moxen;

      log("Evaluate:");

      // Prepare for evaluation.
      prepareEvaluation();

      // Set up mox world.
      moxen = new ArrayList<Mox>();
      populateMoxen(moxen);
      moxWorld.setMoxen(moxen);
      moxWorld.reset();

      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         ForagerPopulation[i].fitness = 0.0;
      }
      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            PredatorPopulation[i].fitness = 0.0;
         }
      }
      blueFoodNeedIdx = ForagerMox.NEED_TYPE.BLUE_FOOD.getValue();
      moxFoodNeedIdx  = PredatorMox.NEED_TYPE.MOX_FOOD.getValue();

      // Step world.
      if (Dashboard)
      {
         moxWorld.createDashboard();
         Dashboard = moxWorld.updateDashboard(0, Steps);
      }
      for (step = 0; step < Steps; step++)
      {
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            mox = ForagerPopulation[i].mox;
            mox.startCycleTimeAccumulation();
         }
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               mox = PredatorPopulation[i].mox;
               mox.startCycleTimeAccumulation();
            }
         }

         // Step the moxen.
         moxWorld.stepMoxen();

         // Update forager fitness.
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            mox = ForagerPopulation[i].mox;
            if (mox.isAlive)
            {
               // Increase fitness when food found.
               if (mox.getNeed(blueFoodNeedIdx) == 0.0)
               {
                  ForagerPopulation[i].fitness += 1.0;
                  mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
               }

               // Kill sluggish mox?
               if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
               {
                  mox.isAlive = false;
               }
            }
         }

         // Update predator fitness.
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               mox = PredatorPopulation[i].mox;
               if (mox.isAlive)
               {
                  // Increase fitness when food found.
                  if (mox.getNeed(moxFoodNeedIdx) == 0.0)
                  {
                     PredatorPopulation[i].fitness += 1.0;
                     mox.setNeed(moxFoodNeedIdx, PredatorMox.MOX_FOOD_NEED_VALUE);
                  }

                  // Kill sluggish mox?
                  if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
                  {
                     mox.isAlive = false;
                  }
               }
            }
         }

         // Step the Game of Life.
         if (StepGameOfLife)
         {
            moxWorld.stepGameOfLife();
         }

         // Update dashboard?
         if (Dashboard)
         {
            Dashboard = moxWorld.updateDashboard(step + 1, Steps);
         }
      }
      moxWorld.destroyDashboard();
      moxWorld.setMoxen(new ArrayList<Mox>());
      moxWorld.reset();

      log("  Foragers:");
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         log("    member=" + i + ", " + ForagerPopulation[i].getInfo());
      }
      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         log("  Predators:");
         for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            log("    member=" + i + ", " + PredatorPopulation[i].getInfo());
         }
      }
   }


   // Populate moxen.
   void populateMoxen(ArrayList<Mox> moxen)
   {
      int i, x, y, direction;
      int blueFoodNeedIdx, moxFoodNeedIdx;
      Mox mox;

      blueFoodNeedIdx = ForagerMox.NEED_TYPE.BLUE_FOOD.getValue();
      moxFoodNeedIdx  = PredatorMox.NEED_TYPE.MOX_FOOD.getValue();

      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         mox = ForagerPopulation[i].mox;
         mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
         x         = Randomizer.nextInt(moxWorld.getWidth());
         y         = Randomizer.nextInt(moxWorld.getHeight());
         direction = Randomizer.nextInt(4);
         mox.setSpacialProperties(x, y, direction);
         moxen.add(mox);
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            mox = PredatorPopulation[i].mox;
            mox.setNeed(moxFoodNeedIdx, PredatorMox.MOX_FOOD_NEED_VALUE);
            x         = Randomizer.nextInt(moxWorld.getWidth());
            y         = Randomizer.nextInt(moxWorld.getHeight());
            direction = Randomizer.nextInt(4);
            mox.setSpacialProperties(x, y, direction);
            moxen.add(mox);
         }
      }
   }


   // Prepare new moxen for evaluation by giving them
   // some experience before competing with existing moxen.
   void prepareEvaluation()
   {
      int i, step;
      int blueFoodNeedIdx, moxFoodNeedIdx;
      Mox mox;

      ArrayList<Mox> moxen;

      log("  Preparing new moxen");

      // Set up mox world.
      moxen = new ArrayList<Mox>();
      populateMoxen(moxen);
      moxWorld.setMoxen(moxen);
      moxWorld.reset();

      blueFoodNeedIdx = ForagerMox.NEED_TYPE.BLUE_FOOD.getValue();
      moxFoodNeedIdx  = PredatorMox.NEED_TYPE.MOX_FOOD.getValue();

      // Step world.
      for (step = 0; step < Steps; step++)
      {
         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            mox = ForagerPopulation[i].mox;
            mox.startCycleTimeAccumulation();
         }
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               mox = PredatorPopulation[i].mox;
               mox.startCycleTimeAccumulation();
            }
         }

         // Step the moxen.
         moxWorld.stepMoxen();

         for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            mox = ForagerPopulation[i].mox;
            if (mox.isAlive)
            {
               if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
               {
                  mox.isAlive = false;
               }
               else
               {
                  if (mox.getNeed(blueFoodNeedIdx) == 0.0)
                  {
                     mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
                  }
               }
            }
         }

         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               mox = PredatorPopulation[i].mox;
               if (mox.isAlive)
               {
                  if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
                  {
                     mox.isAlive = false;
                  }
                  else
                  {
                     if (mox.getNeed(moxFoodNeedIdx) == 0.0)
                     {
                        mox.setNeed(moxFoodNeedIdx, PredatorMox.MOX_FOOD_NEED_VALUE);
                     }
                  }
               }
            }
         }

         // Step the Game of Life.
         if (StepGameOfLife)
         {
            moxWorld.stepGameOfLife();
         }
      }
      moxWorld.setMoxen(new ArrayList<Mox>());
      moxWorld.reset();
      log("  Preparation completed");
   }


   // Prune unfit members.
   void prune()
   {
      double max;
      int    i, j, m;

      EvolveCommon.Member member;

      log("Select:");
      log("  Foragers:");
      EvolveCommon.Member[] fitPopulation =
         new EvolveCommon.Member[EvolveCommon.FORAGER_FIT_POPULATION_SIZE];
      max = 0.0;
      for (i = 0; i < EvolveCommon.FORAGER_FIT_POPULATION_SIZE; i++)
      {
         m = -1;
         for (j = 0; j < EvolveCommon.FORAGER_POPULATION_SIZE; j++)
         {
            member = ForagerPopulation[j];
            if (member == null)
            {
               continue;
            }
            if ((m == -1) || (member.fitness > max))
            {
               m   = j;
               max = member.fitness;
            }
         }
         member = ForagerPopulation[m];
         ForagerPopulation[m] = null;
         fitPopulation[i]     = member;
         log("    " + member.getInfo());
      }
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         if (ForagerPopulation[i] != null)
         {
            ForagerPopulation[i].clear();
            ForagerPopulation[i] = null;
         }
      }
      for (i = 0; i < EvolveCommon.FORAGER_FIT_POPULATION_SIZE; i++)
      {
         ForagerPopulation[i] = fitPopulation[i];
         fitPopulation[i]     = null;
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      log("  Predators:");
      fitPopulation =
         new EvolveCommon.Member[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE];
      for (i = 0; i < EvolveCommon.PREDATOR_FIT_POPULATION_SIZE; i++)
      {
         m = -1;
         for (j = 0; j < EvolveCommon.PREDATOR_POPULATION_SIZE; j++)
         {
            member = PredatorPopulation[j];
            if (member == null)
            {
               continue;
            }
            if ((m == -1) || (member.fitness > max))
            {
               m   = j;
               max = member.fitness;
            }
         }
         member = PredatorPopulation[m];
         PredatorPopulation[m] = null;
         fitPopulation[i]      = member;
         log("    " + member.getInfo());
      }
      for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
      {
         if (PredatorPopulation[i] != null)
         {
            PredatorPopulation[i] = null;
         }
      }
      for (i = 0; i < EvolveCommon.PREDATOR_FIT_POPULATION_SIZE; i++)
      {
         PredatorPopulation[i] = fitPopulation[i];
      }
   }


   // Mutate members.
   void mutate()
   {
      int i, j;

      EvolveCommon.Member member, mutant;

      log("Mutate:");
      log("  Foragers:");
      for (i = 0; i < EvolveCommon.FORAGER_NUM_MUTANTS; i++)
      {
         // Select a fit member to mutate.
         j      = Randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE);
         member = ForagerPopulation[j];

         // Create mutant member.
         mutant = new EvolveCommon.Member(member, member.generation + 1, Randomizer);
         ForagerPopulation[EvolveCommon.FORAGER_FIT_POPULATION_SIZE + i] = mutant;
         log("    member=" + j + ", " + member.getInfo() +
             " -> member=" + (EvolveCommon.FORAGER_FIT_POPULATION_SIZE + i) +
             ", " + mutant.getInfo());
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      log("  Predators:");
      for (i = 0; i < EvolveCommon.PREDATOR_NUM_MUTANTS; i++)
      {
         // Select a fit member to mutate.
         j      = Randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE);
         member = PredatorPopulation[j];

         // Create mutant member.
         mutant = new EvolveCommon.Member(member, member.generation + 1, Randomizer);
         PredatorPopulation[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE + i] = mutant;
         log("    member=" + j + ", " + member.getInfo() +
             " -> member=" + (EvolveCommon.PREDATOR_FIT_POPULATION_SIZE + i) +
             ", " + mutant.getInfo());
      }
   }


   // Produce offspring by melding parent parameters.
   void mate()
   {
      int i, j, k;

      EvolveCommon.Member member1, member2, offspring;

      log("Mate:");
      if (EvolveCommon.FORAGER_FIT_POPULATION_SIZE > 1)
      {
         log("  Foragers:");
         for (i = 0; i < EvolveCommon.FORAGER_NUM_OFFSPRING; i++)
         {
            // Select a pair of fit members to mate.
            j       = Randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE);
            member1 = ForagerPopulation[j];
            while ((k = Randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE)) == j) {}
            member2 = ForagerPopulation[k];

            // Create offspring.
            offspring = new EvolveCommon.Member(member1, member2,
                                                (member1.generation > member2.generation ?
                                                 member1.generation : member2.generation) + 1, Randomizer);
            ForagerPopulation[EvolveCommon.FORAGER_FIT_POPULATION_SIZE +
                              EvolveCommon.FORAGER_NUM_MUTANTS + i] = offspring;
            log("    member=" + j + ", " + member1.getInfo() + " + member=" +
                k + ", " + member2.getInfo() +
                " -> member=" + (EvolveCommon.FORAGER_FIT_POPULATION_SIZE +
                                 EvolveCommon.FORAGER_NUM_MUTANTS + i) +
                ", " + offspring.getInfo());
         }
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      if (EvolveCommon.PREDATOR_FIT_POPULATION_SIZE > 1)
      {
         log("  Predators:");
         for (i = 0; i < EvolveCommon.PREDATOR_NUM_OFFSPRING; i++)
         {
            // Select a pair of fit members to mate.
            j       = Randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE);
            member1 = PredatorPopulation[j];
            while ((k = Randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE)) == j) {}
            member2 = PredatorPopulation[k];

            // Create offspring.
            offspring = new EvolveCommon.Member(member1, member2,
                                                (member1.generation > member2.generation ?
                                                 member1.generation : member2.generation) + 1, Randomizer);
            PredatorPopulation[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE +
                               EvolveCommon.PREDATOR_NUM_MUTANTS + i] = offspring;
            log("    member=" + j + ", " + member1.getInfo() + " + member=" +
                k + ", " + member2.getInfo() +
                " -> member=" + (EvolveCommon.PREDATOR_FIT_POPULATION_SIZE +
                                 EvolveCommon.PREDATOR_NUM_MUTANTS + i) +
                ", " + offspring.getInfo());
         }
      }
   }


   // Extract moxen files.
   void extract()
   {
      int    i;
      int    blueFoodNeedIdx, moxFoodNeedIdx;
      Mox    mox;
      String filename;

      // Extract foragers.
      blueFoodNeedIdx = ForagerMox.NEED_TYPE.BLUE_FOOD.getValue();
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         // Set up mox world.
         mox = ForagerPopulation[i].mox;
         mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
         ArrayList<Mox> moxen = new ArrayList<Mox>(1);
         moxen.add(0, mox);

         // Save mox world.
         filename = "mox_world_forager_" + mox.id + ".mw";
         moxWorld.setMoxen(moxen);
         moxWorld.reset();
         try
         {
            moxWorld.save(filename);
         }
         catch (Exception e) {
            System.err.println("Cannot save mox world to file " + filename +
                               ":" + e.getMessage());
            System.exit(1);
         }
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      // Extract predators.
      moxFoodNeedIdx = PredatorMox.NEED_TYPE.MOX_FOOD.getValue();
      for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
      {
         // Set up mox world.
         mox = PredatorPopulation[i].mox;
         mox.setNeed(moxFoodNeedIdx, PredatorMox.MOX_FOOD_NEED_VALUE);
         ArrayList<Mox> moxen = new ArrayList<Mox>(1);
         moxen.add(0, mox);

         // Save mox world.
         filename = "mox_world_predator_" + mox.id + ".mw";
         moxWorld.setMoxen(moxen);
         moxWorld.reset();
         try
         {
            moxWorld.save(filename);
         }
         catch (Exception e) {
            System.err.println("Cannot save mox world to file " + filename +
                               ":" + e.getMessage());
            System.exit(1);
         }
      }
   }


   // Print moxen properties.
   void printProperties()
   {
      int i;

      System.out.println("Moxen properties:");

      // Print foragers.
      System.out.println("=============================");
      System.out.println("Foragers:");
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         System.out.println("-----------------------------");
         ForagerPopulation[i].printProperties();
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      // Print predators.
      System.out.println("=============================");
      System.out.println("Predators:");
      for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
      {
         System.out.println("-----------------------------");
         PredatorPopulation[i].printProperties();
      }
   }


   // Logging.
   void log(String message)
   {
      if (LogWriter != null)
      {
         LogWriter.println(message);
         LogWriter.flush();
      }
   }


   // Main.
   public static void main(String[] args)
   {
      EvolveMoxenSystem evolveMoxenSystem = new EvolveMoxenSystem(args);

      evolveMoxenSystem.start();
      System.exit(0);
   }
}
