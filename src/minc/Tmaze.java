// T-maze.

package mona;

import java.util.*;

public class Tmaze
{
   // Maze junction.
   class Junction
   {
      int            mark;
      int            direction;
      double         probability;
      boolean        highlight;
      int            choice;
      Vector<String> annotations;

      Junction(int mark, int direction, double probability)
      {
         this.mark        = mark;
         this.direction   = direction;
         this.probability = probability;
         highlight        = false;
         choice           = -1;
         annotations      = null;
      }


      Junction()
      {
         mark        = direction = -1;
         highlight   = false;
         probability = 0.0;
         choice      = -1;
         annotations = null;
      }
   }

   Vector<Junction> path;

   Tmaze()
   {
      path = new Vector<Junction>();
   }


   void addJunction(int mark, int direction, double probability)
   {
      path.add(new Junction(mark, direction, probability));
   }


   void addJunction(Junction junction)
   {
      path.add(junction);
   }


   // Is given maze a duplicate?
   boolean isDuplicate(Tmaze maze)
   {
      if (path.size() != maze.path.size())
      {
         return(false);
      }
      for (int i = 0; i < path.size(); i++)
      {
         if (path.get(i).mark != maze.path.get(i).mark)
         {
            return(false);
         }
         if (path.get(i).direction != maze.path.get(i).direction)
         {
            return(false);
         }
      }
      return(true);
   }


   void clearAnnotations()
   {
      for (int i = 0; i < path.size(); i++)
      {
         path.get(i).highlight   = false;
         path.get(i).choice      = -1;
         path.get(i).annotations = null;
      }
   }


   void print()
   {
      Junction junction;

      System.out.println("Mark\tDirection\tProbabilities");
      for (int i = 0; i < path.size(); i++)
      {
         junction = path.get(i);
         if (junction.direction == 0)
         {
            System.out.println(junction.mark + "\tleft\t" +
                               junction.probability + " " + (1.0 - junction.probability));
         }
         else
         {
            System.out.println(junction.mark + "\right\t" +
                               junction.probability + " " + (1.0 - junction.probability));
         }
      }
   }
};
