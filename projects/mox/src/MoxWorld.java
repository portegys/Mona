/*
 * Mox world: mona learning automata in a Game of Life world.
 */

import java.util.*;
import java.io.*;
import java.awt.*;
import javax.swing.*;
import mona.NativeFileDescriptor;

// Mox world.
public class MoxWorld
{
   // Usage.
   public static final String Usage =
      "Usage:\n" +
      "  New run:\n" +
      "    java MoxWorld\n" +
      "      -steps <steps>\n" +
      "     [-stepGameOfLife]\n" +
      "        -gridSize <width> <height>\n" +
      "        -liveCellProbability <0.0-1.0>\n" +
      "      |\n" +
      "        -loadCells <file name>\n" +
      "     [-foragerMoxen <quantity>]\n" +
      "     [-predatorMoxen <quantity>]\n" +
      "     [-randomSeed <random number seed>]\n" +
      "     [-save <file name>]\n" +
      "     [-dashboard]\n" +
      "  Resume run:\n" +
      "    java MoxWorld\n" +
      "      -steps <steps>\n" +
      "     [-stepGameOfLife]\n" +
      "      -load <file name>\n" +
      "     [-save <file name>]\n" +
      "     [-dashboard]";

   // Default random seed.
   public static final int DEFAULT_RANDOM_SEED = 4517;

   // Moxen.
   ArrayList<Mox> moxen;

   // Game of Life.
   GameOfLife gameOfLife;

   // Dashboard display.
   MoxWorldDashboard dashboard;

   // Constructor.
   public MoxWorld()
   {
   }


   // Initialize cells.
   public void initCells(int width, int height, float liveCellProb, int randomSeed)
   {
      int x, y;

      // Random numbers.
      Random random = new Random(randomSeed);

      // Create Game of Life.
      gameOfLife = new GameOfLife(new Dimension(width, height));
      for (x = 0; x < width; x++)
      {
         for (y = 0; y < height; y++)
         {
            if (random.nextFloat() < liveCellProb)
            {
               gameOfLife.cells[x][y] = 1;
            }
         }
      }
      gameOfLife.step();
      gameOfLife.checkpoint();
   }


   // Get width.
   int getWidth()
   {
      if (gameOfLife != null)
      {
         return(gameOfLife.size.width);
      }
      else
      {
         return(0);
      }
   }


   // Get height.
   int getHeight()
   {
      if (gameOfLife != null)
      {
         return(gameOfLife.size.height);
      }
      else
      {
         return(0);
      }
   }


   // Load cells from file.
   public void loadCells(String cellsFilename) throws IOException
   {
      gameOfLife = new GameOfLife();
      try {
         gameOfLife.load(cellsFilename);
      }
      catch (Exception e) {
         throw new IOException("Cannot open cells file " + cellsFilename +
                               ":" + e.getMessage());
      }
   }


   // Create moxen.
   public void createMoxen(int numForagers, int numPredators, int randomSeed)
   {
      int i, x, y, w, h;

      // Random numbers.
      Random random = new Random(randomSeed);

      // Create moxen.
      w     = gameOfLife.size.width;
      h     = gameOfLife.size.height;
      moxen = new ArrayList<Mox>(numForagers + numPredators);
      for (i = 0; i < numForagers; i++)
      {
         x = random.nextInt(w);
         y = random.nextInt(h);
         moxen.add(i, new ForagerMox(i, x, y,
                                     random.nextInt(Mox.DIRECTION.NUM_DIRECTIONS.getValue()),
                                     random.nextInt()));
      }
      for (i = 0; i < numPredators; i++)
      {
         x = random.nextInt(w);
         y = random.nextInt(h);
         moxen.add(numForagers + i, new PredatorMox(numForagers + i, x, y,
                                                    random.nextInt(Mox.DIRECTION.NUM_DIRECTIONS.getValue()),
                                                    random.nextInt()));
      }
   }


   // Set moxen.
   public void setMoxen(ArrayList<Mox> moxen)
   {
      this.moxen = moxen;
      if (dashboard != null)
      {
         dashboard.setMoxen(moxen);
      }
   }


   // Reset.
   public void reset()
   {
      if (gameOfLife != null)
      {
         gameOfLife.restore();
      }
      if (moxen != null)
      {
         int numMox = moxen.size();
         for (int i = 0; i < numMox; i++)
         {
            moxen.get(i).reset();
         }
      }
      if (dashboard != null)
      {
         dashboard.reset();
      }
   }


   // Clear.
   public void clear()
   {
      if (dashboard != null)
      {
         dashboard.setVisible(false);
         dashboard = null;
      }
      gameOfLife = null;
      if (moxen != null)
      {
         int numMox = moxen.size();
         for (int i = 0; i < numMox; i++)
         {
            moxen.get(i).clear();
         }
         moxen = null;
      }
   }


   // Load from file.
   public void load(String filename) throws IOException
   {
      FileInputStream      input;
      NativeFileDescriptor fd;

      // Open the file.
      try {
         input = new FileInputStream(new File(filename));
         fd    = new NativeFileDescriptor(filename, "r");
         fd.open();
      }
      catch (Exception e) {
         throw new IOException("Cannot open input file " + filename +
                               ":" + e.getMessage());
      }

      // Load world.
      load(input, fd);

      input.close();
      fd.close();
   }


   // Load.
   public void load(FileInputStream input, NativeFileDescriptor fd) throws IOException
   {
      // DataInputStream is for unbuffered input.
      DataInputStream reader = new DataInputStream(input);

      // Load Game of Life.
      gameOfLife = new GameOfLife();
      gameOfLife.load(reader);

      // Load moxen.
      int numMox = Utility.loadInt(reader);
      moxen = new ArrayList<Mox>(numMox);
      ForagerMox  forager;
      PredatorMox predator;
      for (int i = 0; i < numMox; i++)
      {
         if (Utility.loadInt(reader) == Mox.SPECIES.FORAGER.getValue())
         {
            forager = new ForagerMox();
            forager.load(input, fd);
            moxen.add(i, forager);
         }
         else
         {
            predator = new PredatorMox();
            predator.load(input, fd);
            moxen.add(i, predator);
         }
      }
   }


   // Save to file.
   public void save(String filename) throws IOException
   {
      FileOutputStream     output;
      NativeFileDescriptor fd;

      try
      {
         output = new FileOutputStream(new File(filename));
         fd     = new NativeFileDescriptor(filename, "w");
         fd.open();
      }
      catch (Exception e) {
         throw new IOException("Cannot open output file " + filename +
                               ":" + e.getMessage());
      }

      // Save world.
      save(output, fd);

      output.close();
      fd.close();
   }


   // Save.
   public void save(FileOutputStream output, NativeFileDescriptor fd) throws IOException
   {
      PrintWriter writer = new PrintWriter(output);

      // Save Game of Life.
      gameOfLife.save(writer);

      // Save moxen.
      int numMox = moxen.size();
      Utility.saveInt(writer, numMox);
      writer.flush();
      Mox mox;
      for (int i = 0; i < numMox; i++)
      {
         mox = moxen.get(i);
         Utility.saveInt(writer, mox.species);
         writer.flush();
         mox.save(output, fd);
      }
   }


   // Run.
   public void run(int steps, boolean stepGameOfLife)
   {
      for (int i = 0; i < steps; i++)
      {
         // Update dashboard.
         updateDashboard(i + 1, steps);

         // Step moxen.
         stepMoxen();

         // Step Game of Life.
         if (stepGameOfLife)
         {
            stepGameOfLife();
         }
      }
   }


   // Step moxen.
   public void stepMoxen()
   {
      int i, x, y, numMox;
      Mox mox;

      int width  = gameOfLife.size.width;
      int height = gameOfLife.size.height;

      int[][] moveTo      = new int[width][height];
      boolean[][] eatCell = new boolean[width][height];

      numMox = moxen.size();

      synchronized (gameOfLife.lock)
      {
         // Load moxen into cells.
         for (i = 0; i < numMox; i++)
         {
            mox = moxen.get(i);
            if (mox.isAlive)
            {
               if (mox.species == Mox.SPECIES.FORAGER.getValue())
               {
                  gameOfLife.cells[mox.x][mox.y] = ForagerMox.FORAGER_COLOR_VALUE;
               }
               else
               {
                  gameOfLife.cells[mox.x][mox.y] = PredatorMox.PREDATOR_COLOR_VALUE;
               }
            }
         }

         // Clear action maps.
         for (x = 0; x < width; x++)
         {
            for (y = 0; y < height; y++)
            {
               moveTo[x][y]  = -1;
               eatCell[x][y] = false;
            }
         }

         // Step forager moxen.
         for (i = 0; i < numMox; i++)
         {
            mox = moxen.get(i);
            if (mox.isAlive && (mox.species == Mox.SPECIES.FORAGER.getValue()))
            {
               stepMox(i, moveTo, eatCell);
            }
         }

         // Step predator moxen.
         // This is done after foragers since predators may eat foragers.
         for (i = 0; i < numMox; i++)
         {
            mox = moxen.get(i);
            if (mox.isAlive && (mox.species == Mox.SPECIES.PREDATOR.getValue()))
            {
               stepMox(i, moveTo, eatCell);
            }
         }

         // Perform actions.
         for (x = 0; x < width; x++)
         {
            for (y = 0; y < height; y++)
            {
               if (moveTo[x][y] >= 0)
               {
                  mox = moxen.get(moveTo[x][y]);
                  gameOfLife.cells[mox.x][mox.y] = 0;
                  mox.x = x;
                  mox.y = y;
                  if (mox.isAlive)
                  {
                     if (mox.species == Mox.SPECIES.FORAGER.getValue())
                     {
                        gameOfLife.cells[mox.x][mox.y] = ForagerMox.FORAGER_COLOR_VALUE;
                     }
                     else
                     {
                        gameOfLife.cells[mox.x][mox.y] = PredatorMox.PREDATOR_COLOR_VALUE;
                     }
                  }
               }
               if (eatCell[x][y] == true)
               {
                  gameOfLife.cells[x][y] = 0;
               }
            }
         }
      }
   }


   // Step mox.
   void stepMox(int moxIndex, int[][] moveTo, boolean[][] eatCell)
   {
      int     x, y, mx, my, x1, y1, x2, y2, x3, y3;
      int     range, rangeIndex, colorHueIndex, colorIntensityIndex, response, numMox;
      float   distance;
      Mox     mox, preyMox;
      boolean gotGoal;

      float[] sensors     = new float[Mox.SENSOR_CONFIG.NUM_SENSORS.getValue()];
      rangeIndex          = Mox.SENSOR_CONFIG.RANGE_SENSOR_INDEX.getValue();
      colorHueIndex       = Mox.SENSOR_CONFIG.COLOR_HUE_SENSOR_INDEX.getValue();
      colorIntensityIndex = Mox.SENSOR_CONFIG.COLOR_INTENSITY_SENSOR_INDEX.getValue();

      // Detect color ahead.
      int width  = gameOfLife.size.width;
      int height = gameOfLife.size.height;

      mox = moxen.get(moxIndex);
      mx  = mox.x;
      my  = mox.y;
      if (mox.direction == Mox.DIRECTION.NORTH.getValue())
      {
         for (range = 0, x = mox.x, y = ((mox.y + 1) % height);
              range < height - 2; range++, y = ((y + 1) % height))
         {
            if (range == 0)
            {
               mx = x;
               my = y;
            }
            if (gameOfLife.cells[x][y] > 0) { break; }
         }
      }
      else if (mox.direction == Mox.DIRECTION.EAST.getValue())
      {
         for (range = 0, x = (mox.x + 1) % width, y = mox.y;
              range < width - 2; range++, x = ((x + 1) % width))
         {
            if (range == 0)
            {
               mx = x;
               my = y;
            }
            if (gameOfLife.cells[x][y] > 0) { break; }
         }
      }
      else if (mox.direction == Mox.DIRECTION.SOUTH.getValue())
      {
         x = mox.x;
         y = mox.y - 1;
         if (y < 0) { y += height; }
         for (range = 0; range < height - 2; range++)
         {
            if (range == 0)
            {
               mx = x;
               my = y;
            }
            if (gameOfLife.cells[x][y] > 0) { break; }
            y -= 1;
            if (y < 0) { y += height; }
         }
      }
      else
      {
         x = mox.x - 1;
         if (x < 0) { x += width; }
         y = mox.y;
         for (range = 0; range < width - 2; range++)
         {
            if (range == 0)
            {
               mx = x;
               my = y;
            }
            if (gameOfLife.cells[x][y] > 0) { break; }
            x -= 1;
            if (x < 0) { x += width; }
         }
      }
      if (mox.species == Mox.SPECIES.FORAGER.getValue())
      {
         if ((ForagerMox.MAX_SENSOR_RANGE >= 0.0f) &&
             ((float)range > ForagerMox.MAX_SENSOR_RANGE))
         {
            sensors[colorHueIndex]       = 0.0f;
            sensors[colorIntensityIndex] = 0.0f;
         }
         else
         {
            sensors[colorHueIndex]       = (float)gameOfLife.cells[x][y];
            sensors[colorIntensityIndex] = 1.0f / (float)(range + 1);
         }
      }
      else
      {
         if ((PredatorMox.MAX_SENSOR_RANGE >= 0.0f) &&
             ((float)range > PredatorMox.MAX_SENSOR_RANGE))
         {
            sensors[colorHueIndex]       = 0.0f;
            sensors[colorIntensityIndex] = 0.0f;
         }
         else
         {
            sensors[colorHueIndex]       = (float)gameOfLife.cells[x][y];
            sensors[colorIntensityIndex] = 1.0f / (float)(range + 1);
         }
      }

      // Get distance to nearest goal.
      if (mox.species == Mox.SPECIES.FORAGER.getValue())
      {
         if (ForagerMox.MAX_SENSOR_RANGE >= 0.0f)
         {
            sensors[rangeIndex] = ForagerMox.MAX_SENSOR_RANGE *
                                  ForagerMox.MAX_SENSOR_RANGE;
            if ((width / 2) < (int)ForagerMox.MAX_SENSOR_RANGE)
            {
               x1 = mx - (width / 2);
               x2 = mx + (width / 2);
            }
            else
            {
               x1 = mx - (int)ForagerMox.MAX_SENSOR_RANGE;
               x2 = mx + (int)ForagerMox.MAX_SENSOR_RANGE;
            }
            if ((height / 2) < (int)ForagerMox.MAX_SENSOR_RANGE)
            {
               y1 = my - (height / 2);
               y2 = my + (height / 2);
            }
            else
            {
               y1 = my - (int)ForagerMox.MAX_SENSOR_RANGE;
               y2 = my + (int)ForagerMox.MAX_SENSOR_RANGE;
            }
            for (x = x1; x <= x2; x++)
            {
               x3 = x;
               if (x < 0) { x3 += width; }
               if (x >= width) { x3 = x % width; }
               for (y = y1; y <= y2; y++)
               {
                  y3 = y;
                  if (y < 0) { y3 += height; }
                  if (y >= height) { y3 = y % height; }
                  if (gameOfLife.cells[x3][y3] == GameOfLife.BLUE_CELL_COLOR_VALUE)
                  {
                     distance  = (float)((mx - x) * (mx - x));
                     distance += (float)((my - y) * (my - y));
                     if (distance < sensors[rangeIndex])
                     {
                        sensors[rangeIndex] = distance;
                     }
                  }
               }
            }
            sensors[rangeIndex] = (float)Math.sqrt(sensors[rangeIndex]);
            if (sensors[rangeIndex] > ForagerMox.MAX_SENSOR_RANGE)
            {
               sensors[rangeIndex] = ForagerMox.MAX_SENSOR_RANGE;
            }
         }
         else
         {
            sensors[rangeIndex] = (float)((width + height) * (width + height));
            x1 = mx - (width / 2);
            x2 = mx + (width / 2);
            y1 = my - (height / 2);
            y2 = my + (height / 2);
            for (x = x1; x <= x2; x++)
            {
               x3 = x;
               if (x < 0) { x3 += width; }
               if (x >= width) { x3 = x % width; }
               for (y = y1; y <= y2; y++)
               {
                  y3 = y;
                  if (y < 0) { y3 += height; }
                  if (y >= height) { y3 = y % height; }
                  if (gameOfLife.cells[x3][y3] == GameOfLife.BLUE_CELL_COLOR_VALUE)
                  {
                     distance  = (float)((mx - x) * (mx - x));
                     distance += (float)((my - y) * (my - y));
                     if (distance < sensors[rangeIndex])
                     {
                        sensors[rangeIndex] = distance;
                     }
                  }
               }
            }
            sensors[rangeIndex] = (float)Math.sqrt(sensors[rangeIndex]);
         }
      }
      else if (mox.species == Mox.SPECIES.PREDATOR.getValue())
      {
         if (PredatorMox.MAX_SENSOR_RANGE >= 0.0f)
         {
            sensors[rangeIndex] = PredatorMox.MAX_SENSOR_RANGE *
                                  PredatorMox.MAX_SENSOR_RANGE;
            if ((width / 2) < (int)PredatorMox.MAX_SENSOR_RANGE)
            {
               x1 = mx - (width / 2);
               x2 = mx + (width / 2);
            }
            else
            {
               x1 = mx - (int)PredatorMox.MAX_SENSOR_RANGE;
               x2 = mx + (int)PredatorMox.MAX_SENSOR_RANGE;
            }
            if ((height / 2) < (int)PredatorMox.MAX_SENSOR_RANGE)
            {
               y1 = my - (height / 2);
               y2 = my + (height / 2);
            }
            else
            {
               y1 = my - (int)PredatorMox.MAX_SENSOR_RANGE;
               y2 = my + (int)PredatorMox.MAX_SENSOR_RANGE;
            }
            for (x = x1; x <= x2; x++)
            {
               x3 = x;
               if (x < 0) { x3 += width; }
               if (x >= width) { x3 = x % width; }
               for (y = y1; y <= y2; y++)
               {
                  y3 = y;
                  if (y < 0) { y3 += height; }
                  if (y >= height) { y3 = y % height; }
                  if (gameOfLife.cells[x3][y3] == ForagerMox.FORAGER_COLOR_VALUE)
                  {
                     distance  = (float)((mx - x) * (mx - x));
                     distance += (float)((my - y) * (my - y));
                     if (distance < sensors[rangeIndex])
                     {
                        sensors[rangeIndex] = distance;
                     }
                  }
               }
            }
            sensors[rangeIndex] = (float)Math.sqrt(sensors[rangeIndex]);
            if (sensors[rangeIndex] > PredatorMox.MAX_SENSOR_RANGE)
            {
               sensors[rangeIndex] = PredatorMox.MAX_SENSOR_RANGE;
            }
         }
         else
         {
            sensors[rangeIndex] = (float)((width + height) * (width + height));
            x1 = mx - (width / 2);
            x2 = mx + (width / 2);
            y1 = my - (height / 2);
            y2 = my + (height / 2);
            for (x = x1; x <= x2; x++)
            {
               x3 = x;
               if (x < 0) { x3 += width; }
               if (x >= width) { x3 = x % width; }
               for (y = y1; y <= y2; y++)
               {
                  y3 = y;
                  if (y < 0) { y3 += height; }
                  if (y >= height) { y3 = y % height; }
                  if (gameOfLife.cells[x3][y3] == ForagerMox.FORAGER_COLOR_VALUE)
                  {
                     distance  = (float)((mx - x) * (mx - x));
                     distance += (float)((my - y) * (my - y));
                     if (distance < sensors[rangeIndex])
                     {
                        sensors[rangeIndex] = distance;
                     }
                  }
               }
            }
            sensors[rangeIndex] = (float)Math.sqrt(sensors[rangeIndex]);
         }
      }

      // Check for goal.
      gotGoal = false;
      if (mox.species == Mox.SPECIES.FORAGER.getValue())
      {
         if ((range == 0) &&
             (sensors[colorHueIndex] == GameOfLife.BLUE_CELL_COLOR_VALUE))
         {
            gotGoal = true;
         }
      }
      else if (mox.species == Mox.SPECIES.PREDATOR.getValue())
      {
         if ((range == 0) &&
             (sensors[colorHueIndex] == ForagerMox.FORAGER_COLOR_VALUE))
         {
            gotGoal = true;
         }
      }

      // Cycle mox.
      response = mox.cycle(sensors);

      // Process response.
      if (response == Mox.RESPONSE_TYPE.FORWARD.getValue())
      {
         if (range > 0)
         {
            if (moveTo[mx][my] == -1)
            {
               moveTo[mx][my] = moxIndex;
            }
            else
            {
               moveTo[mx][my] = -2;
            }
         }
      }
      else if (response == Mox.RESPONSE_TYPE.RIGHT.getValue())
      {
         mox.direction = (mox.direction + 1) % Mox.DIRECTION.NUM_DIRECTIONS.getValue();
      }
      else if (response == Mox.RESPONSE_TYPE.LEFT.getValue())
      {
         mox.direction -= 1;
         if (mox.direction < 0)
         {
            mox.direction += Mox.DIRECTION.NUM_DIRECTIONS.getValue();
         }
      }

      if (gotGoal)
      {
         if (mox.species == Mox.SPECIES.FORAGER.getValue())
         {
            if (gameOfLife.cells[mx][my] == GameOfLife.BLUE_CELL_COLOR_VALUE)
            {
               eatCell[mx][my] = true;
            }
         }
         else
         {
            // Predator eats forager.
            numMox = moxen.size();
            for (int i = 0; i < numMox; i++)
            {
               preyMox = moxen.get(i);
               if ((preyMox.x == mx) && (preyMox.y == my))
               {
                  preyMox.isAlive = false;
                  if (dashboard != null)
                  {
                     dashboard.closeMoxDashboard(i);
                  }
                  eatCell[mx][my] = true;
                  break;
               }
            }
         }
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


   // Create dashboard.
   public void createDashboard()
   {
      if (dashboard == null)
      {
         if (moxen == null)
         {
            dashboard = new MoxWorldDashboard(gameOfLife);
         }
         else
         {
            dashboard = new MoxWorldDashboard(gameOfLife, moxen);
         }
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
      int     steps          = -1;
      boolean stepGameOfLife = false;
      int     width          = -1;
      int     height         = -1;
      float   liveCellProb   = -1.0f;
      int     numForagers    = 0;
      int     numPredators   = 0;
      int     randomSeed     = DEFAULT_RANDOM_SEED;
      String  cellsFilename  = null;
      String  loadfile       = null;
      String  savefile       = null;
      boolean dashboard      = false;

      for (int i = 0; i < args.length; i++)
      {
         if (args[i].equals("-steps"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid steps option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               steps = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid steps option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (steps < 0)
            {
               System.err.println("Invalid steps option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-stepGameOfLife"))
         {
            stepGameOfLife = true;
            continue;
         }
         if (args[i].equals("-gridSize"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid gridSize option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               width = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid gridSize width option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (width < 2)
            {
               System.err.println("Invalid gridSize width option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid gridSize option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               height = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid gridSize height option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (height < 2)
            {
               System.err.println("Invalid gridSize height option");
               System.err.println(MoxWorld.Usage);
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
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               liveCellProb = Float.parseFloat(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid liveCellProbability option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if ((liveCellProb < 0.0f) || (liveCellProb > 1.0f))
            {
               System.err.println("Invalid liveCellProbability option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-loadCells"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid loadCells option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (cellsFilename == null)
            {
               cellsFilename = args[i];
            }
            else
            {
               System.err.println("Duplicate loadCells option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-foragerMoxen"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid foragerMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               numForagers = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid foragerMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (numForagers <= 0)
            {
               System.err.println("Invalid foragerMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-predatorMoxen"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid predatorMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               numPredators = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid predatorMoxen option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (numPredators <= 0)
            {
               System.err.println("Invalid predatorMoxen option");
               System.err.println(MoxWorld.Usage);
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
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            try
            {
               randomSeed = Integer.parseInt(args[i]);
            }
            catch (NumberFormatException e) {
               System.err.println("Invalid randomSeed option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-load"))
         {
            i++;
            if (i >= args.length)
            {
               System.err.println("Invalid load option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (loadfile == null)
            {
               loadfile = args[i];
            }
            else
            {
               System.err.println("Duplicate load option");
               System.err.println(MoxWorld.Usage);
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
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            if (savefile == null)
            {
               savefile = args[i];
            }
            else
            {
               System.err.println("Duplicate save option");
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
            continue;
         }
         if (args[i].equals("-dashboard"))
         {
            dashboard = true;
            continue;
         }
         System.err.println(MoxWorld.Usage);
         System.exit(1);
      }

      // Check options.
      if (steps < 0)
      {
         System.err.println(MoxWorld.Usage);
         System.exit(1);
      }
      if (loadfile == null)
      {
         if (cellsFilename == null)
         {
            if ((width == -1) || (height == -1) || (liveCellProb < 0.0f))
            {
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
         }
         else
         {
            if ((width != -1) || (height != -1) || (liveCellProb >= 0.0f))
            {
               System.err.println(MoxWorld.Usage);
               System.exit(1);
            }
         }
      }
      else
      {
         if ((numForagers != 0) || (numPredators != 0) ||
             (width != -1) || (height != -1) ||
             (liveCellProb >= 0.0f) || (cellsFilename != null))
         {
            System.err.println(MoxWorld.Usage);
            System.exit(1);
         }
      }

      // Set look and feel.
      try {
         UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      }
      catch (Exception e)
      {
         System.err.println("Warning: cannot set look and feel");
      }

      // Create world.
      MoxWorld moxWorld = new MoxWorld();
      if (loadfile != null)
      {
         try
         {
            moxWorld.load(loadfile);
         }
         catch (Exception e)
         {
            System.err.println("Cannot load from file " + loadfile + ": " + e.getMessage());
            System.exit(1);
         }
      }
      else
      {
         try
         {
            if (cellsFilename == null)
            {
               moxWorld.initCells(width, height, liveCellProb, randomSeed);
            }
            else
            {
               moxWorld.loadCells(cellsFilename);
            }
            moxWorld.createMoxen(numForagers, numPredators, randomSeed);
         }
         catch (Exception e)
         {
            System.err.println("Cannot initialize: " + e.getMessage());
            System.exit(1);
         }
      }

      // Create dashboard?
      if (dashboard)
      {
         moxWorld.createDashboard();
      }

      // Run.
      moxWorld.run(steps, stepGameOfLife);

      // Save?
      if (savefile != null)
      {
         try
         {
            moxWorld.save(savefile);
         }
         catch (Exception e)
         {
            System.err.println("Cannot save to file " + savefile + ": " + e.getMessage());
         }
      }
      System.exit(0);
   }
}
