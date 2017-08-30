/*
 * Evolve individual mox foragers and predator-forager pairs
 * by mutating and recombining parameters.
 */

import java.util.*;
import java.io.*;
import mona.NativeFileDescriptor;

public class EvolveMoxen
{
   // Usage.
   public static final String Usage =
      "Usage:\n" +
      "  New run:\n" +
      "    java EvolveMoxen\n" +
      "      -generations <evolution generations>\n" +
      "      -steps <moxen steps>\n" +
      "     [-stepGameOfLife]\n" +
      "      -moxPopulations \"" + EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY.getName() +
      "\" | \"" + EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS.getName() + "\"\n" +
      "     [-trainForagers (train foragers before evaluating them)]\n" +
      "      -loadCells <file name> (repeatable option)\n" +
      "      -output <evolution output file name>\n" +
      "     [-mutationRate <mutation rate>]\n" +
      "     [-randomMutationRate <random mutation rate>]\n" +
      "     [-maxSensorRange <maximum sensor range>]\n" +
      "     [-randomSeed <random seed> (for new run)]\n" +
      "     [-logfile <log file name>]\n" +
      "     [-dashboard (run with mox world dashboard)]\n" +
      "  Resume run:\n" +
      "    java EvolveMoxen\n" +
      "      -generations <evolution generations>\n" +
      "      -steps <moxen steps>\n" +
      "     [-stepGameOfLife]\n" +
      "     [-trainForagers (train foragers before evaluating them)]\n" +
      "      -input <evolution input file name>\n" +
      "      -output <evolution output file name>\n" +
      "     [-mutationRate <mutation rate>]\n" +
      "     [-randomMutationRate <random mutation rate>]\n" +
      "     [-maxSensorRange <maximum sensor range>]\n" +
      "     [-randomSeed <random seed> (for new run)]\n" +
      "     [-logfile <log file name>]\n" +
      "     [-dashboard (run with mox world dashboard)]\n" +
      "  Extract into mox_world_forager|predator_<mox id>_world_<number>.mw files:\n" +
      "    java EvolveMoxen\n" +
      "      -extract\n" +
      "      -input <evolution input file name>\n" +
      "  Print population properties:\n" +
      "    java EvolveMoxen\n" +
      "      -properties\n" +
      "      -input <evolution input file name>\n" +
      "  Print evolution statistics:\n" +
      "    java EvolveMoxen\n" +
      "      -statistics\n" +
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

   // Train foragers?
   boolean TrainForagers;

   // File names.
   ArrayList<String> CellsFileNames;
   String            InputFileName;
   String            OutputFileName;
   String            LogFileName;
   PrintWriter       LogWriter;

   // Random numbers.
   Random Randomizer;

   // Run with dashboard.
   boolean Dashboard;

   // Extract moxen files.
   boolean Extract;

   // Print population properties.
   boolean PrintProperties;

   // Print evolution statistics.
   boolean PrintStatistics;

   // Evolution statistics.
   double[] ForagerFittest;
   double[] ForagerAverage;
   double[] PredatorFittest;
   double[] PredatorAverage;

   // Maximum mox cycle time.
   long MaxMoxCycleTime;

   // Mox worlds.
   ArrayList<MoxWorld> moxWorlds;

   // Populations.
   EvolveCommon.Member[] ForagerPopulation;
   EvolveCommon.Member[] PredatorPopulation;

   // Constructor.
   public EvolveMoxen(String[] args)
   {
      int i;

      // Get options.
      Generation     = 0;
      Generations    = -1;
      Steps          = -1;
      StepGameOfLife = false;
      boolean gotStepGameOfLife = false;
      MoxPopulations = EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY;
      TrainForagers  = false;
      boolean gotMoxPopulations = false;
      CellsFileNames = new ArrayList<String>();
      InputFileName  = OutputFileName = LogFileName = null;
      LogWriter      = null;
      boolean gotMutationRate       = false;
      boolean gotRandomMutationRate = false;
      boolean gotMaxSensorRange     = false;
      boolean gotRandomSeed         = false;
      Extract         = false;
      PrintProperties = false;
      PrintStatistics = false;
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

         if (args[i].equals("-trainForagers"))
         {
            TrainForagers = true;
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
            CellsFileNames.add(new String(args[i]));
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

         if (args[i].equals("-properties"))
         {
            PrintProperties = true;
            continue;
         }

         if (args[i].equals("-statistics"))
         {
            PrintStatistics = true;
            continue;
         }

         System.err.println(Usage);
         System.exit(1);
      }

      // Extract moxen files or print properties?
      if (Extract || PrintProperties || PrintStatistics)
      {
         if ((Generations != -1) || (Steps != -1) || gotStepGameOfLife ||
             gotMoxPopulations || TrainForagers || (InputFileName == null) ||
             (OutputFileName != null) || (CellsFileNames.size() > 0) ||
             (LogFileName != null) || gotMutationRate || gotRandomMutationRate ||
             gotMaxSensorRange || gotRandomSeed || Dashboard)
         {
            System.err.println(Usage);
            System.exit(1);
         }
         if (Extract && (PrintProperties || PrintStatistics))
         {
            System.err.println(Usage);
            System.exit(1);
         }
         if (PrintProperties && (Extract || PrintStatistics))
         {
            System.err.println(Usage);
            System.exit(1);
         }
         if (PrintStatistics && (Extract || PrintProperties))
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
            if (gotMoxPopulations || (CellsFileNames.size() > 0))
            {
               System.err.println(Usage);
               System.exit(1);
            }
         }
         else
         {
            if (!gotMoxPopulations || (CellsFileNames.size() == 0))
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
         for (int i = 0; i < CellsFileNames.size(); i++)
         {
            log("    loadCells=" + CellsFileNames.get(i));
         }
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

      // Print population properties?
      if (PrintProperties)
      {
         printProperties();
         return;
      }

      // Print evolution statistics?
      if (PrintStatistics)
      {
         printStatistics();
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
      int      i;
      MoxWorld moxWorld;

      moxWorlds = new ArrayList<MoxWorld>();
      for (i = 0; i < CellsFileNames.size(); i++)
      {
         try
         {
            moxWorld = new MoxWorld();
            moxWorld.loadCells(CellsFileNames.get(i));
            moxWorlds.add(moxWorld);
            if (i > 0)
            {
               if ((moxWorlds.get(0).getWidth() != moxWorld.getWidth()) ||
                   (moxWorlds.get(0).getHeight() != moxWorld.getHeight()))
               {
                  System.err.println("Dimensions in cells file " +
                                     CellsFileNames.get(i) +
                                     "must equal those in other cells files");
                  System.exit(1);
               }
            }
         }
         catch (Exception e) {
            System.err.println("Cannot load cells file " +
                               CellsFileNames.get(i) +
                               ":" + e.getMessage());
            System.exit(1);
         }
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
      ForagerFittest = new double[Generations + 1];
      ForagerAverage = new double[Generations + 1];
      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
      {
         PredatorPopulation = new EvolveCommon.Member[EvolveCommon.PREDATOR_POPULATION_SIZE];
         for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            if (i == 0)
            {
               PredatorPopulation[i] =
                  new EvolveCommon.Member(Mox.SPECIES.PREDATOR.getValue(), 0,
                                          moxWorlds.get(0).getWidth() - 1,
                                          moxWorlds.get(0).getHeight() - 1,
                                          Mox.DIRECTION.SOUTH.getValue(), Randomizer);
            }
            else
            {
               // Mutate parameters.
               PredatorPopulation[i] =
                  new EvolveCommon.Member(PredatorPopulation[0], 0, Randomizer);
            }
         }
         PredatorFittest = new double[Generations + 1];
         PredatorAverage = new double[Generations + 1];
      }
   }


   // Load evolution.
   void load()
   {
      int                  i, n;
      FileInputStream      input  = null;
      NativeFileDescriptor fd     = null;
      DataInputStream      reader = null;
      MoxWorld             moxWorld;

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
      moxWorlds = new ArrayList<MoxWorld>();
      try
      {
         n = Utility.loadInt(reader);
         for (i = 0; i < n; i++)
         {
            moxWorld = new MoxWorld();
            moxWorld.load(input, fd);
            moxWorlds.add(moxWorld);
         }
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
         ForagerFittest = new double[Generation + Generations + 1];
         ForagerAverage = new double[Generation + Generations + 1];
         for (i = 0; i < Generation; i++)
         {
            ForagerFittest[i] = Utility.loadDouble(reader);
            ForagerAverage[i] = Utility.loadDouble(reader);
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
            PredatorFittest = new double[Generation + Generations + 1];
            PredatorAverage = new double[Generation + Generations + 1];
            for (i = 0; i < Generation; i++)
            {
               PredatorFittest[i] = Utility.loadDouble(reader);
               PredatorAverage[i] = Utility.loadDouble(reader);
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
      int                  i, n;
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
         n = moxWorlds.size();
         Utility.saveInt(writer, n);
         writer.flush();
         for (i = 0; i < n; i++)
         {
            moxWorlds.get(i).save(output, fd);
         }
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
         writer.flush();
         for (i = 0, n = generation + 1; i < n; i++)
         {
            Utility.saveDouble(writer, ForagerFittest[i]);
            Utility.saveDouble(writer, ForagerAverage[i]);
         }
         writer.flush();
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_AND_PREDATORS)
         {
            for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
            {
               PredatorPopulation[i].save(output, fd);
            }
            writer.flush();
            for (i = 0, n = generation + 1; i < n; i++)
            {
               Utility.saveDouble(writer, PredatorFittest[i]);
               Utility.saveDouble(writer, PredatorAverage[i]);
            }
         }
         writer.flush();
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
      // Train foragers?
      if (TrainForagers)
      {
         trainForagers();
      }

      // Evaluate member fitness.
      evaluate(generation);

      // Prune unfit members.
      prune();

      // Create new members by mutation.
      mutate();

      // Create new members by mating.
      mate();
   }


   // Train foragers.
   void trainForagers()
   {
      int i, j, step;
      int blueFoodNeedIdx;
      Mox mox;

      ArrayList<Mox> moxen;
      MoxWorld       moxWorld;

      blueFoodNeedIdx = ForagerMox.NEED_TYPE.BLUE_FOOD.getValue();
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         mox = ForagerPopulation[i].mox;
         mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
         moxen = new ArrayList<Mox>(1);
         moxen.add(0, mox);

         // Train mox in all mox worlds.
         ForagerPopulation[i].fitness = 0.0;
         for (j = 0; j < moxWorlds.size(); j++)
         {
            moxWorld = moxWorlds.get(j);
            moxWorld.setMoxen(moxen);
            moxWorld.reset();

            // Step world.
            if (Dashboard)
            {
               moxWorld.createDashboard();
               Dashboard = moxWorld.updateDashboard(0, Steps,
                                                    "forager training" +
                                                    ", member=" + i + ", mox=" + mox.id +
                                                    ", world=" + j +
                                                    ", blue food need=" +
                                                    mox.getNeed(blueFoodNeedIdx));
            }
            for (step = 0; step < Steps; step++)
            {
               if (mox.getNeed(blueFoodNeedIdx) == 0.0)
               {
                  break;
               }
               if (!mox.isAlive)
               {
                  break;
               }
               setTrainingResponse(mox, moxWorld);
               moxWorld.stepMoxen();
               if (StepGameOfLife)
               {
                  moxWorld.stepGameOfLife();
               }
               if (Dashboard)
               {
                  Dashboard = moxWorld.updateDashboard(step + 1, Steps,
                                                       "forager training" +
                                                       ", member=" + i + ", mox=" + mox.id +
                                                       ", world=" + j +
                                                       ", blue food need=" +
                                                       mox.getNeed(blueFoodNeedIdx));
               }
            }
            moxWorld.destroyDashboard();
            moxWorld.setMoxen(new ArrayList<Mox>());
            moxWorld.reset();
         }
      }
   }


   // Set mox training response.
   void setTrainingResponse(Mox mox, MoxWorld moxWorld)
   {
      int     x, y, nx, ny, ex, ey, sx, sy, wx, wy, w, h;
      int     d, dn, de, ds, dw;
      int     blueFoodNeedIdx;
      boolean needBlueFood;

      // Locate adjacent cells.
      w  = moxWorld.getWidth();
      h  = moxWorld.getHeight();
      nx = mox.x;
      ny = ((mox.y + 1) % h);
      ex = (mox.x + 1) % w;
      ey = mox.y;
      sx = mox.x;
      sy = mox.y - 1;
      if (sy < 0) { sy += h; }
      wx = mox.x - 1;
      if (wx < 0) { wx += w; }
      wy = mox.y;

      // Get distance from goal to nearest adjacent cell.
      blueFoodNeedIdx = ForagerMox.NEED_TYPE.BLUE_FOOD.getValue();
      needBlueFood    = false;
      if (mox.getNeed(blueFoodNeedIdx) > 0.0)
      {
         needBlueFood = true;
      }
      dn = de = ds = dw = -1;
      for (x = 0; x < w; x++)
      {
         for (y = 0; y < h; y++)
         {
            if ((moxWorld.gameOfLife.cells[x][y] == GameOfLife.BLUE_CELL_COLOR_VALUE) &&
                needBlueFood)
            {
               d = dist(x, y, nx, ny, w, h);
               if ((dn == -1) || (d < dn))
               {
                  dn = d;
               }
               d = dist(x, y, ex, ey, w, h);
               if ((de == -1) || (d < de))
               {
                  de = d;
               }
               d = dist(x, y, sx, sy, w, h);
               if ((ds == -1) || (d < ds))
               {
                  ds = d;
               }
               d = dist(x, y, wx, wy, w, h);
               if ((dw == -1) || (d < dw))
               {
                  dw = d;
               }
            }
         }
      }
      if (dn == -1)
      {
         return;
      }

      if (mox.direction == Mox.DIRECTION.NORTH.getValue())
      {
         if ((dn <= de) && (dn <= dw) && (dn <= ds))
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.FORWARD.getValue());
         }
         else if ((de <= dw) && (de <= ds))
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.RIGHT.getValue());
         }
         else
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.LEFT.getValue());
         }
         return;
      }
      else if (mox.direction == Mox.DIRECTION.EAST.getValue())
      {
         if ((de <= dn) && (de <= ds) && (de <= dw))
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.FORWARD.getValue());
         }
         else if ((ds <= dn) && (ds <= dw))
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.RIGHT.getValue());
         }
         else
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.LEFT.getValue());
         }
         return;
      }
      else if (mox.direction == Mox.DIRECTION.SOUTH.getValue())
      {
         if ((ds <= de) && (ds <= dw) && (ds <= dn))
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.FORWARD.getValue());
         }
         else if ((dw <= de) && (dw <= dn))
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.RIGHT.getValue());
         }
         else
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.LEFT.getValue());
         }
         return;
      }
      else
      {
         if ((dw <= dn) && (dw <= ds) && (dw <= de))
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.FORWARD.getValue());
         }
         else if ((dn <= ds) && (dn <= de))
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.RIGHT.getValue());
         }
         else
         {
            mox.overrideResponse(Mox.RESPONSE_TYPE.LEFT.getValue());
         }
         return;
      }
   }


   // City-block distance.
   int dist(int x1, int y1, int x2, int y2, int w, int h)
   {
      int d, dx, dy;

      dx = x2 - x1;
      if (dx < 0) { dx = -dx; }
      d = w - dx;
      if (d < dx) { dx = d; }
      dy = y2 - y1;
      if (dy < 0) { dy = -dy; }
      d = h - dy;
      if (d < dy) { dy = d; }
      d = dx + dy;
      return(d);
   }


   // Evaluate member fitnesses.
   void evaluate(int generation)
   {
      int    i, j, step;
      int    blueFoodNeedIdx, moxFoodNeedIdx;
      int    moxFoodStep;
      Mox    mox;
      long   excessCycleTime;
      String logEntry;

      ArrayList<Mox> moxen;
      MoxWorld       moxWorld;

      log("Evaluate:");

      // Prepare for evaluation.
      prepareEvaluation(generation);

      // Evaluate foragers.
      log("  Foragers:");
      blueFoodNeedIdx = ForagerMox.NEED_TYPE.BLUE_FOOD.getValue();
      moxFoodNeedIdx  = PredatorMox.NEED_TYPE.MOX_FOOD.getValue();
      for (i = 0; i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
      {
         // Set up mox world.
         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
         {
            mox = ForagerPopulation[i].mox;
            mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
            moxen = new ArrayList<Mox>(1);
            moxen.add(0, mox);
         }
         else
         {
            // Set up mox world with random fit predator.
            mox = PredatorPopulation[Randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE)].mox;
            mox.setNeed(moxFoodNeedIdx, PredatorMox.MOX_FOOD_NEED_VALUE);
            moxen = new ArrayList<Mox>(2);
            moxen.add(0, mox);
            mox = ForagerPopulation[i].mox;
            mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
            moxen.add(1, mox);
         }

         // Evaluate mox in all mox worlds.
         excessCycleTime = 0;
         ForagerPopulation[i].fitness = 0.0;
         for (j = 0; j < moxWorlds.size(); j++)
         {
            moxWorld = moxWorlds.get(j);
            moxWorld.setMoxen(moxen);
            moxWorld.reset();

            // Step world.
            if (Dashboard)
            {
               moxWorld.createDashboard();
               Dashboard = moxWorld.updateDashboard(0, Steps,
                                                    "generation=" + generation +
                                                    ", member=" + i + ", mox=" + mox.id +
                                                    ", world=" + j +
                                                    ", blue food need=" +
                                                    mox.getNeed(blueFoodNeedIdx));
            }
            for (step = 0; step < Steps; step++)
            {
               mox.startCycleTimeAccumulation();
               moxWorld.stepMoxen();
               if (!mox.isAlive)
               {
                  break;
               }
               if (mox.getNeed(blueFoodNeedIdx) == 0.0)
               {
                  ForagerPopulation[i].fitness += 1.0 + (1.0 / (double)(step + 1));
                  mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
               }
               if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
               {
                  excessCycleTime = mox.getCycleTimeAccumulator();
                  break;
               }
               if (StepGameOfLife)
               {
                  moxWorld.stepGameOfLife();
               }
               if (Dashboard)
               {
                  Dashboard = moxWorld.updateDashboard(step + 1, Steps,
                                                       "generation=" + generation +
                                                       ", member=" + i + ", mox=" + mox.id +
                                                       ", world=" + j +
                                                       ", blue food need=" +
                                                       mox.getNeed(blueFoodNeedIdx));
               }
            }
            moxWorld.destroyDashboard();
            moxWorld.setMoxen(new ArrayList<Mox>());
            moxWorld.reset();
         }
         logEntry = "    member=" + i + ", " + ForagerPopulation[i].getInfo();
         if (!mox.isAlive)
         {
            logEntry = logEntry + ", eaten";
         }
         if (excessCycleTime > 0)
         {
            logEntry = logEntry + ", excess cycle time=" + excessCycleTime;
         }
         log(logEntry);
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      // Evaluate predators.
      log("  Predators:");
      for (i = 0; i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
      {
         // Set up mox world with random fit prey.
         mox = ForagerPopulation[Randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE)].mox;
         mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
         moxen = new ArrayList<Mox>(2);
         moxen.add(0, mox);
         mox = PredatorPopulation[i].mox;
         mox.setNeed(moxFoodNeedIdx, PredatorMox.MOX_FOOD_NEED_VALUE);
         moxen.add(1, mox);

         excessCycleTime = 0;
         PredatorPopulation[i].fitness = 0.0;
         for (j = 0; j < moxWorlds.size(); j++)
         {
            moxWorld = moxWorlds.get(j);
            moxWorld.setMoxen(moxen);
            moxWorld.reset();

            // Step world.
            if (Dashboard)
            {
               moxWorld.createDashboard();
               Dashboard = moxWorld.updateDashboard(0, Steps,
                                                    "generation=" + generation +
                                                    ", member=" + i + ", mox=" + mox.id +
                                                    ", world=" + j +
                                                    ", mox food need=" +
                                                    mox.getNeed(moxFoodNeedIdx));
            }
            moxFoodStep = Steps;
            for (step = 0; step < Steps; step++)
            {
               mox.startCycleTimeAccumulation();
               moxWorld.stepMoxen();
               if (mox.getNeed(moxFoodNeedIdx) == 0.0)
               {
                  moxFoodStep = step;
                  break;
               }
               if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
               {
                  excessCycleTime = mox.getCycleTimeAccumulator();
                  break;
               }
               if (StepGameOfLife)
               {
                  moxWorld.stepGameOfLife();
               }
               if (Dashboard)
               {
                  Dashboard = moxWorld.updateDashboard(step + 1, Steps,
                                                       "generation=" + generation +
                                                       ", member=" + i + ", mox=" + mox.id +
                                                       ", world=" + j +
                                                       ", mox food need=" +
                                                       mox.getNeed(moxFoodNeedIdx));
               }
            }
            PredatorPopulation[i].fitness += (double)moxFoodStep;
            moxWorld.destroyDashboard();
            moxWorld.setMoxen(new ArrayList<Mox>());
            moxWorld.reset();
         }
         logEntry = "    member=" + i + ", " + PredatorPopulation[i].getInfo();
         if (excessCycleTime > 0)
         {
            logEntry = logEntry + ", excess cycle time=" + excessCycleTime;
         }
         log(logEntry);
      }
   }


   // Prepare new moxen for evaluation by giving them
   // experience equivalent to existing moxen.
   void prepareEvaluation(int generation)
   {
      int i, j, n, runs, step;
      int blueFoodNeedIdx, moxFoodNeedIdx;
      Mox mox;

      ArrayList<Mox> moxen;
      MoxWorld       moxWorld;
      blueFoodNeedIdx = ForagerMox.NEED_TYPE.BLUE_FOOD.getValue();
      moxFoodNeedIdx  = PredatorMox.NEED_TYPE.MOX_FOOD.getValue();

      log("  Preparing new moxen:");

      // Catch up to current generation.
      if (generation < EvolveCommon.MAX_PREPARATION_TRIALS)
      {
         runs = generation;
      }
      else
      {
         runs = EvolveCommon.MAX_PREPARATION_TRIALS;
      }
      for (n = 0; n < runs; n++)
      {
         log("    Run " + (n + 1) + " of " + runs);

         // Run new foragers.
         for (i = EvolveCommon.FORAGER_FIT_POPULATION_SIZE;
              i < EvolveCommon.FORAGER_POPULATION_SIZE; i++)
         {
            // Set up mox world.
            if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
            {
               mox = ForagerPopulation[i].mox;
               mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
               moxen = new ArrayList<Mox>(1);
               moxen.add(0, mox);
            }
            else
            {
               // Set up mox world with random fit predator.
               mox = PredatorPopulation[Randomizer.nextInt(EvolveCommon.PREDATOR_FIT_POPULATION_SIZE)].mox;
               mox.setNeed(moxFoodNeedIdx, PredatorMox.MOX_FOOD_NEED_VALUE);
               moxen = new ArrayList<Mox>(2);
               moxen.add(0, mox);
               mox = ForagerPopulation[i].mox;
               mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
               moxen.add(1, mox);
            }

            // Run mox in all mox worlds.
            ForagerPopulation[i].fitness = 0.0;
            for (j = 0; j < moxWorlds.size(); j++)
            {
               moxWorld = moxWorlds.get(j);
               moxWorld.setMoxen(moxen);
               moxWorld.reset();

               // Step world.
               for (step = 0; step < Steps; step++)
               {
                  mox.startCycleTimeAccumulation();
                  moxWorld.stepMoxen();
                  if (!mox.isAlive)
                  {
                     break;
                  }
                  if (mox.getNeed(blueFoodNeedIdx) == 0.0)
                  {
                     mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
                  }
                  if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
                  {
                     break;
                  }
                  if (StepGameOfLife)
                  {
                     moxWorld.stepGameOfLife();
                  }
               }
               moxWorld.destroyDashboard();
               moxWorld.setMoxen(new ArrayList<Mox>());
               moxWorld.reset();
            }
         }

         if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
         {
            continue;
         }

         // Run new predators.
         for (i = EvolveCommon.PREDATOR_FIT_POPULATION_SIZE;
              i < EvolveCommon.PREDATOR_POPULATION_SIZE; i++)
         {
            // Set up mox world with random fit prey.
            mox = ForagerPopulation[Randomizer.nextInt(EvolveCommon.FORAGER_FIT_POPULATION_SIZE)].mox;
            mox.setNeed(blueFoodNeedIdx, ForagerMox.BLUE_FOOD_NEED_VALUE);
            moxen = new ArrayList<Mox>(2);
            moxen.add(0, mox);
            mox = PredatorPopulation[i].mox;
            mox.setNeed(moxFoodNeedIdx, PredatorMox.MOX_FOOD_NEED_VALUE);
            moxen.add(1, mox);

            PredatorPopulation[i].fitness = 0.0;
            for (j = 0; j < moxWorlds.size(); j++)
            {
               moxWorld = moxWorlds.get(j);
               moxWorld.setMoxen(moxen);
               moxWorld.reset();

               // Step world.
               for (step = 0; step < Steps; step++)
               {
                  mox.startCycleTimeAccumulation();
                  moxWorld.stepMoxen();
                  if (mox.getNeed(moxFoodNeedIdx) == 0.0)
                  {
                     break;
                  }
                  if (mox.getCycleTimeAccumulator() > MaxMoxCycleTime)
                  {
                     break;
                  }
                  if (StepGameOfLife)
                  {
                     moxWorld.stepGameOfLife();
                  }
               }
               moxWorld.destroyDashboard();
               moxWorld.setMoxen(new ArrayList<Mox>());
               moxWorld.reset();
            }
         }
      }
      log("  Preparation completed");
   }


   // Prune unfit members.
   void prune()
   {
      double min, max, d;
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
      d = 0.0;
      for (i = 0; i < EvolveCommon.FORAGER_FIT_POPULATION_SIZE; i++)
      {
         ForagerPopulation[i] = fitPopulation[i];
         fitPopulation[i]     = null;
         d += ForagerPopulation[i].fitness;
      }
      ForagerFittest[Generation] = ForagerPopulation[0].fitness;
      ForagerAverage[Generation] = d / (double)EvolveCommon.FORAGER_FIT_POPULATION_SIZE;

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      log("  Predators:");
      fitPopulation =
         new EvolveCommon.Member[EvolveCommon.PREDATOR_FIT_POPULATION_SIZE];
      min = 0.0;
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
            if ((m == -1) || (member.fitness < min))
            {
               m   = j;
               min = member.fitness;
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
      d = 0.0;
      for (i = 0; i < EvolveCommon.PREDATOR_FIT_POPULATION_SIZE; i++)
      {
         PredatorPopulation[i] = fitPopulation[i];
         d += PredatorPopulation[i].fitness;
      }
      PredatorFittest[Generation] = PredatorPopulation[0].fitness;
      PredatorAverage[Generation] = d / (double)EvolveCommon.PREDATOR_FIT_POPULATION_SIZE;
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
      int      i, j;
      int      blueFoodNeedIdx, moxFoodNeedIdx;
      Mox      mox;
      String   filename;
      MoxWorld moxWorld;

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
         for (j = 0; j < moxWorlds.size(); j++)
         {
            filename = "mox_world_forager_" + mox.id + "_world_" + j + ".mw";
            moxWorld = moxWorlds.get(j);
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
         for (j = 0; j < moxWorlds.size(); j++)
         {
            filename = "mox_world_predator_" + mox.id + "_world_" + j + ".mw";
            moxWorld = moxWorlds.get(j);
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
   }


   // Print population properties.
   void printProperties()
   {
      int i;

      System.out.println("Population properties:");

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


   // Print evolution statistics.
   void printStatistics()
   {
      int i;

      System.out.println("Evolution statistics:");

      // Print forager statistics.
      System.out.println("Foragers:");
      System.out.println("Generation\tFittest");
      for (i = 0; i < Generation; i++)
      {
         System.out.println(i + "\t\t" + ForagerFittest[i]);
      }
      System.out.println("Generation\tAverage");
      for (i = 0; i < Generation; i++)
      {
         System.out.println(i + "\t\t" + ForagerAverage[i]);
      }

      if (MoxPopulations == EvolveCommon.MOX_POPULATIONS.FORAGERS_ONLY)
      {
         return;
      }

      // Print predator statistics.
      System.out.println("Predators:");
      System.out.println("Generation\tFittest");
      for (i = 0; i < Generation; i++)
      {
         System.out.println(i + "\t\t" + PredatorFittest[i]);
      }
      System.out.println("Generation\tAverage");
      for (i = 0; i < Generation; i++)
      {
         System.out.println(i + "\t\t" + PredatorAverage[i]);
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
      EvolveMoxen evolveMoxen = new EvolveMoxen(args);

      evolveMoxen.start();
      System.exit(0);
   }
}
