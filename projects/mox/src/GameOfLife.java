/**
 * Conway's Game of Life.
 */

import java.io.*;
import java.util.*;
import java.awt.*;

// Game of Life.
public class GameOfLife
{
   // Colors.
   public static final int GREEN_CELL_COLOR_VALUE = 2;
   public static final int BLUE_CELL_COLOR_VALUE  = 3;

   // Automaton.
   public Dimension size;
   public int[][]   cells;
   public int[][]   restoreCells;
   public Object    lock;

   // Constructors.
   public GameOfLife(Dimension size)
   {
      // Create automaton.
      this.size    = size;
      cells        = new int[size.width][size.height];
      restoreCells = new int[size.width][size.height];
      int x;
      int y;
      for (x = 0; x < size.width; x++)
      {
         for (y = 0; y < size.height; y++)
         {
            cells[x][y] = restoreCells[x][y] = 0;
         }
      }

      // Initialize mutex.
      lock = new Object();
   }


   public GameOfLife()
   {
      size = new Dimension();
      lock = new Object();
   }


   // Get grid width.
   public int getWidth()
   {
      return(size.width);
   }


   // Get grid height.
   public int getHeight()
   {
      return(size.height);
   }


   // Load automaton from file.
   public void load(String fileName) throws IOException
   {
      DataInputStream in;

      // Open the file.
      try {
         in = new DataInputStream(new FileInputStream(fileName));
      }
      catch (Exception e) {
         throw new IOException("Cannot open input file " + fileName +
                               ":" + e.getMessage());
      }

      // Load the file.
      load(in);

      in.close();
   }


   // Load automaton.
   @SuppressWarnings({ "deprecation" })
   public void load(DataInputStream in) throws IOException
   {
      synchronized (lock)
      {
         String          s;
         StringTokenizer t;
         int             w;
         int             h;
         int             n;
         int             x;
         int             y;

         // Load the automaton dimensions.
         if ((s = in.readLine()) == null)
         {
            throw (new IOException("Unexpected EOF"));
         }

         t = new StringTokenizer(s, " ");

         if (!t.hasMoreTokens())
         {
            throw (new IOException("Invalid dimensions"));
         }

         try
         {
            s = t.nextToken().trim();
            w = Integer.parseInt(s, 10);

            if (w <= 0)
            {
               throw (new IOException("Invalid width value " + s));
            }
         }
         catch (NumberFormatException e) {
            throw (new IOException("Invalid width value " + s));
         }

         if (!t.hasMoreTokens())
         {
            throw (new IOException("Invalid dimensions"));
         }

         try
         {
            s = t.nextToken().trim();
            h = Integer.parseInt(s, 10);

            if (h <= 0)
            {
               throw (new IOException("Invalid height value " + s));
            }
         }
         catch (NumberFormatException e) {
            throw (new IOException("Invalid height value " + s));
         }

         size.width   = w;
         size.height  = h;
         cells        = new int[size.width][size.height];
         restoreCells = new int[size.width][size.height];
         clear();

         // Load the cell values.
         if ((s = in.readLine()) == null)
         {
            throw (new IOException("Unexpected EOF"));
         }

         t = new StringTokenizer(s, " ");

         if (!t.hasMoreTokens())
         {
            throw (new IOException("Invalid dimensions"));
         }

         try
         {
            s = t.nextToken().trim();
            n = Integer.parseInt(s, 10);

            if (n < 0)
            {
               throw (new IOException("Invalid number of cell values " + s));
            }
         }
         catch (NumberFormatException e) {
            throw (new IOException("Invalid number of cell values " + s));
         }
         for (int i = 0; i < n; i++)
         {
            if ((s = in.readLine()) == null)
            {
               throw (new IOException("Unexpected EOF"));
            }
            t = new StringTokenizer(s, " ");

            if (!t.hasMoreTokens())
            {
               continue;
            }

            try
            {
               s = t.nextToken().trim();
               x = Integer.parseInt(s, 10);

               if ((x < 0) || (x >= w))
               {
                  throw (new IOException("Invalid x value " + s));
               }
            }
            catch (NumberFormatException e) {
               throw (new IOException("Invalid x value " + s));
            }

            if (!t.hasMoreTokens())
            {
               throw (new IOException("Invalid cell values"));
            }

            try
            {
               s = t.nextToken().trim();
               y = Integer.parseInt(s, 10);

               if ((y < 0) || (y >= h))
               {
                  throw (new IOException("Invalid y value " + s));
               }
            }
            catch (NumberFormatException e) {
               throw (new IOException("Invalid height y " + s));
            }

            if (!t.hasMoreTokens())
            {
               throw (new IOException("Invalid cell values"));
            }

            try
            {
               s           = t.nextToken().trim();
               cells[x][y] = Integer.parseInt(s, 10);
            }
            catch (NumberFormatException e) {
               throw (new IOException("Invalid cell value " + s));
            }
         }

         if ((s = in.readLine()) == null)
         {
            throw (new IOException("Unexpected EOF"));
         }

         t = new StringTokenizer(s, " ");

         if (!t.hasMoreTokens())
         {
            throw (new IOException("Invalid dimensions"));
         }

         try
         {
            s = t.nextToken().trim();
            n = Integer.parseInt(s, 10);

            if (n < 0)
            {
               throw (new IOException("Invalid number of restore cell values " + s));
            }
         }
         catch (NumberFormatException e) {
            throw (new IOException("Invalid number of restore cell values " + s));
         }
         for (int i = 0; i < n; i++)
         {
            if ((s = in.readLine()) == null)
            {
               throw (new IOException("Unexpected EOF"));
            }
            t = new StringTokenizer(s, " ");

            if (!t.hasMoreTokens())
            {
               continue;
            }

            try
            {
               s = t.nextToken().trim();
               x = Integer.parseInt(s, 10);

               if ((x < 0) || (x >= w))
               {
                  throw (new IOException("Invalid x value " + s));
               }
            }
            catch (NumberFormatException e) {
               throw (new IOException("Invalid x value " + s));
            }

            if (!t.hasMoreTokens())
            {
               throw (new IOException("Invalid restore cell values"));
            }

            try
            {
               s = t.nextToken().trim();
               y = Integer.parseInt(s, 10);

               if ((y < 0) || (y >= h))
               {
                  throw (new IOException("Invalid y value " + s));
               }
            }
            catch (NumberFormatException e) {
               throw (new IOException("Invalid height y " + s));
            }

            if (!t.hasMoreTokens())
            {
               throw (new IOException("Invalid restore cell values"));
            }

            try
            {
               s = t.nextToken().trim();
               restoreCells[x][y] = Integer.parseInt(s, 10);
            }
            catch (NumberFormatException e) {
               throw (new IOException("Invalid restore cell value " + s));
            }
         }
      }
   }


   // Save automaton.
   public void save(String fileName) throws IOException
   {
      PrintWriter out;

      try
      {
         out = new PrintWriter(new BufferedWriter(new FileWriter(fileName)));
      }
      catch (Exception e) {
         throw new IOException("Cannot open output file " + fileName +
                               ":" + e.getMessage());
      }

      // Save the file.
      save(out);

      out.close();
   }


   // Save automaton.
   public void save(PrintWriter out) throws IOException
   {
      synchronized (lock)
      {
         int n;
         int x;
         int y;

         out.println(size.width + " " + size.height);

         n = 0;
         for (x = 0; x < size.width; x++)
         {
            for (y = 0; y < size.height; y++)
            {
               if (cells[x][y] > 0)
               {
                  n++;
               }
            }
         }
         out.println(n + "");

         for (x = 0; x < size.width; x++)
         {
            for (y = 0; y < size.height; y++)
            {
               if (cells[x][y] > 0)
               {
                  out.println(x + " " + y + " " + cells[x][y]);
               }
            }
         }

         n = 0;
         for (x = 0; x < size.width; x++)
         {
            for (y = 0; y < size.height; y++)
            {
               if (restoreCells[x][y] > 0)
               {
                  n++;
               }
            }
         }
         out.println(n + "");

         for (x = 0; x < size.width; x++)
         {
            for (y = 0; y < size.height; y++)
            {
               if (restoreCells[x][y] > 0)
               {
                  out.println(x + " " + y + " " + restoreCells[x][y]);
               }
            }
         }
      }
   }


   // Step automaton.
   public void step()
   {
      int x;
      int y;
      int x2;
      int y2;
      int w;
      int h;
      int count;

      int[][] tempCells = new int[size.width][size.height];

      synchronized (lock)
      {
         for (x = 0; x < size.width; x++)
         {
            for (y = 0; y < size.height; y++)
            {
               tempCells[x][y] = cells[x][y];
               cells[x][y]     = 0;
            }
         }

         // Apply game of life rules.
         w = 1;
         h = 1;

         for (x = 0; x < size.width; x++)
         {
            for (y = 0; y < size.height; y++)
            {
               count = 0;
               x2    = x - w;

               while (x2 < 0)
               {
                  x2 += size.width;
               }

               y2 = y;

               if (tempCells[x2][y2] > 0)
               {
                  count++;
               }

               y2 = y - h;

               while (y2 < 0)
               {
                  y2 += size.height;
               }

               if (tempCells[x2][y2] > 0)
               {
                  count++;
               }

               y2 = y + h;

               while (y2 >= size.height)
               {
                  y2 -= size.height;
               }

               if (tempCells[x2][y2] > 0)
               {
                  count++;
               }

               x2 = x;
               y2 = y - h;

               while (y2 < 0)
               {
                  y2 += size.height;
               }

               if (tempCells[x2][y2] > 0)
               {
                  count++;
               }

               y2 = y + h;

               while (y2 >= size.height)
               {
                  y2 -= size.height;
               }

               if (tempCells[x2][y2] > 0)
               {
                  count++;
               }

               x2 = x + w;

               while (x2 >= size.width)
               {
                  x2 -= size.width;
               }

               y2 = y;

               if (tempCells[x2][y2] > 0)
               {
                  count++;
               }

               y2 = y - h;

               while (y2 < 0)
               {
                  y2 += size.height;
               }

               if (tempCells[x2][y2] > 0)
               {
                  count++;
               }

               y2 = y + h;

               while (y2 >= size.height)
               {
                  y2 -= size.height;
               }


               if (tempCells[x2][y2] > 0)
               {
                  count++;
               }

               if (tempCells[x][y] > 0)
               {
                  if ((count > 3) || (count < 2))
                  {
                     cells[x][y] = 0;
                  }
                  else
                  {
                     cells[x][y] = count;
                  }
               }
               else
               {
                  if (count == 3)
                  {
                     cells[x][y] = count;
                  }
                  else
                  {
                     cells[x][y] = 0;
                  }
               }
            }
         }
      }
   }


   // Step N times.
   public void step(int steps)
   {
      for (int i = 0; i < steps; i++)
      {
         step();
      }
   }


   // Clear cells.
   public void clear()
   {
      int x;
      int y;

      synchronized (lock)
      {
         for (x = 0; x < size.width; x++)
         {
            for (y = 0; y < size.height; y++)
            {
               cells[x][y] = 0;
            }
         }
      }
   }


   // Checkpoint cells.
   public void checkpoint()
   {
      int x;
      int y;

      synchronized (lock)
      {
         for (x = 0; x < size.width; x++)
         {
            for (y = 0; y < size.height; y++)
            {
               restoreCells[x][y] = cells[x][y];
            }
         }
      }
   }


   // Restore cells.
   public void restore()
   {
      int x;
      int y;

      synchronized (lock)
      {
         for (x = 0; x < size.width; x++)
         {
            for (y = 0; y < size.height; y++)
            {
               cells[x][y] = restoreCells[x][y];
            }
         }
      }
   }
}
