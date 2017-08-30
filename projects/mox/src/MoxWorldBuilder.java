/*
 * Build a Game of Life mox world using given parameters.
 */

import java.util.*;
import java.awt.*;

public class MoxWorldBuilder
{
   // Usage.
   public static final String Usage =
      "Usage:\n" +
      "    java MoxWorldBuilder\n" +
      "      -steps <steps>\n" +
      "      -gridSize <width> <height>\n" +
      "      -liveCellProbability <0.0-1.0>\n" +
      "     [-randomSeed <random number seed>]\n" +
      "     [-minBlueCells <quantity>]\n" +
      "     [-maxBlueCells <quantity>]\n" +
      "     [-minGreenCells <quantity>]\n" +
      "     [-maxGreenCells <quantity>]\n" +
      "     [-maxBuildAttempts <quantity>]\n" +
      "      -save <file name>\n" +
      "     [-dashboard]";

   // Default random seed.
   public static final int DEFAULT_RANDOM_SEED = 4517;

   // Default maximum build attempts.
   public static final int DEFAULT_MAX_BUILD_ATTEMPTS = 1000;

   // Game of Life mox world.
   public GameOfLife gameOfLife;

   // Random numbers.
   Random randomizer;

   // Dashboard display.
   MoxWorldDashboard dashboard;

   // Constructor.
   public MoxWorldBuilder(int width, int height)
   {
      // Create Game of Life.
      gameOfLife = new GameOfLife(new Dimension(width, height));
   }


   // Build.
   public boolean build(int steps, float liveCellProb, int randomSeed,
                        int minBlueCells, int maxBlueCells,
                        int minGreenCells, int maxGreenCells,
                        int maxBuildAttempts)
   {
      // Random numbers.
      randomizer = new Random(randomSeed);

      // Try to build world.
      for (int i = 0; i < maxBuildAttempts; i++)
      {
         // Initialize.
         init(liveCellProb);

         // Run steps.
         run(steps);

         // Check properties.
         if (check(minBlueCells, maxBlueCells, minGreenCells, maxGreenCells))
         {
            return(true);
         }
      }
      return(false);
   }


   // Initialize.
   public void init(float liveCellProb)
   {
      int x, y;
      int width  = gameOfLife.getWidth();
      int height = gameOfLife.getHeight();

      synchronized (gameOfLife.lock)
      {
         for (x = 0; x < width; x++)
         {
            for (y = 0; y < height; y++)
            {
               if (randomizer.nextFloat() < liveCellProb)
               {
                  gameOfLife.cells[x][y] = 1;
               }
            }
         }
         gameOfLife.step();
         gameOfLife.checkpoint();
      }
   }


   // Run steps.
   public void run(int steps)
   {
      for (int i = 0; i < steps; i++)
      {
         // Update dashboard.
         updateDashboard(i + 1, steps);

         // Step Game of Life.
         stepGameOfLife();
      }
      synchronized (gameOfLife.lock)
      {
         gameOfLife.checkpoint();
      }
   }


   // Step Game Of Life.
   public void stepGameOfLife()
   {
      synchronized (gameOfLife.lock)
      {
         gameOfLife.step();
      }
   }


   // Check.
   public boolean check(int minBlueCells, int maxBlueCells,
                        int minGreenCells, int maxGreenCells)
   {
      int x, y;
      int width      = gameOfLife.getWidth();
      int height     = gameOfLife.getHeight();
      int blueCount  = 0;
      int greenCount = 0;

      for (x = 0; x < width; x++)
      {
         for (y = 0; y < height; y++)
         {
            if (gameOfLife.cells[x][y] == GameOfLife.BLUE_CELL_COLOR_VALUE)
            {
               blueCount++;
            }
            if (gameOfLife.cells[x][y] == GameOfLife.GREEN_CELL_COLOR_VALUE)
            {
               greenCount++;
            }
         }
      }

      if ((blueCount >= minBlueCells) && (blueCount <= maxBlueCells) &&
          (greenCount >= minGreenCells) && (greenCount <= maxGreenCells))
      {
         return(true);
      }
      else
      {
         return(false);
      }
   }


   // Create dashboard.
   public void createDashboard()
   {
      if (dashboard == null)
      {
         dashboard = new MoxWorldDashboard(gameOfLife);
      }
   }


   // Destroy dashboard.
   public void destroyDashboard()
   {
      if (dashboard != null)
      {
         dashboard.reset();
         dashboard.setVisible(false);
         dashboard = null;
      }
   }


   // Update dashboard.
   // Return true if dashboard operational.
   public boolean updateDashboard(int step, int steps, String message)
   {
      if (dashboard != null)
      {
         dashboard.setMessage(message);
         dashboard.update(step, steps);
         if (dashboard.quit)
         {
            dashboard = null;
            return(false);
         }
         else
         {
            return(true);
         }
      }
      else
      {
         return(false);
      }
   }


   public boolean updateDashboard(int step, int steps)
   {
      return(updateDashboard(step, steps, ""));
   }


   public boolean updateDashboard()
   {
      if (dashboard != null)
      {
         dashboard.update();
         if (dashboard.quit)
         {
            dashboard = null;
            return(false);
         }
         else
         {
            return(true);
         }
      }
      else
      {
         return(false);
      }
   }


   // Main.
   public static void main(String[] args)
   {
      // Get options.
      int     steps            = -1;
      int     width            = -1;
      int     height           = -1;
      float   liveCellProb     = -1.0f;
      int     randomSeed       = DEFAULT_RANDOM_SEED;
      int     minBlueCells     = 0;
      int     maxBlueCells     = -1;
      int     minGreenCells    = 0;
      int     maxGreenCells    = -1;
      int     maxBuildAttempts = DEFAULT_MAX_BUILD_ATTEMPTS;
      String  savefile         = null;
      boolean dashboard        = false;

      for (int i = 0; i < args.length; i++)
      {
         if (args[i].equals("-steps"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid steps option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               steps = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid steps option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if (steps < 0)
            {
               System.err.println("Invalid steps option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-gridSize"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid gridSize option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               width = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid gridSize width option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if (width < 2)
            {
               System.err.println("Invalid gridSize width option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid gridSize option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               height = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid gridSize height option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if (height < 2)
            {
               System.err.println("Invalid gridSize height option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-liveCellProbability"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid liveCellProbability option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               liveCellProb = Float.parseFloat(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid liveCellProbability option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if ((liveCellProb < 0.0f) || (liveCellProb > 1.0f))
            {
               System.err.println("Invalid liveCellProbability option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-randomSeed"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid randomSeed option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               randomSeed = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid randomSeed option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-minBlueCells"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid minBlueCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               minBlueCells = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid minBlueCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if (minBlueCells < 0)
            {
               System.err.println("Invalid minBlueCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-maxBlueCells"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid maxBlueCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               maxBlueCells = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid maxBlueCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if (maxBlueCells < 0)
            {
               System.err.println("Invalid maxBlueCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-minGreenCells"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid minGreenCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               minGreenCells = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid minGreenCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if (minGreenCells < 0)
            {
               System.err.println("Invalid minGreenCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-maxGreenCells"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid maxGreenCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               maxGreenCells = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid maxGreenCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if (maxGreenCells < 0)
            {
               System.err.println("Invalid maxGreenCells option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-maxBuildAttempts"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid maxBuildAttempts option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            try
            {
               maxBuildAttempts = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid maxBuildAttempts option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if (maxBuildAttempts < 1)
            {
               System.err.println("Invalid maxBuildAttempts option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-save"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid save option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            if (savefile == null)
            {
               savefile = args[i];
            }
            else
            {
               System.err.println("Duplicate save option");
               System.err.println(MoxWorldBuilder.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-dashboard"))
         {
            dashboard = true;
            continue;
         }
         System.err.println(MoxWorldBuilder.Usage);
         System.exit(1);
      }

      // Check options.
      if ((steps < 0) || (width == -1) ||
          (height == -1) || (liveCellProb < 0.0f) ||
          (savefile == null))
      {
         System.err.println(MoxWorldBuilder.Usage);
         System.exit(1);
      }
      if (maxBlueCells == -1)
      {
         maxBlueCells = width * height;
      }
      if (maxGreenCells == -1)
      {
         maxGreenCells = width * height;
      }
      if (minBlueCells > maxBlueCells)
      {
         System.err.println(MoxWorldBuilder.Usage);
         System.exit(1);
      }
      if (minGreenCells > maxGreenCells)
      {
         System.err.println(MoxWorldBuilder.Usage);
         System.exit(1);
      }
      if ((minBlueCells + minGreenCells) > (width * height))
      {
         System.err.println(MoxWorldBuilder.Usage);
         System.exit(1);
      }

      // Create.
      MoxWorldBuilder moxWorldBuilder = new MoxWorldBuilder(width, height);

      // Create dashboard?
      if (dashboard)
      {
         moxWorldBuilder.createDashboard();
      }

      // Build mox world.
      if (!moxWorldBuilder.build(steps, liveCellProb, randomSeed,
                                 minBlueCells, maxBlueCells,
                                 minGreenCells, maxGreenCells,
                                 maxBuildAttempts))
      {
         System.err.println("Cannot build mox world");
         System.exit(1);
      }

      // Save.
      try
      {
         moxWorldBuilder.gameOfLife.save(savefile);
      }
      catch (Exception e)
      {
         System.err.println("Cannot save to file " + savefile + ": " + e.getMessage());
         System.exit(1);
      }
      System.exit(0);
   }
}
