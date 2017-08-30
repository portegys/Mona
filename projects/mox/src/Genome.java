/*
 * Genome.
 */

import java.util.*;
import java.io.*;

public class Genome
{
   // Genes.
   Vector<Gene> genes;

   // Mutation rate.
   double mutationRate;

   // Probability of random mutation.
   double randomMutationRate;

   // Random numbers.
   int    randomSeed;
   Random randomizer;

   // Constructor.
   Genome(double mutationRate, double randomMutationRate, int randomSeed)
   {
      this.mutationRate       = mutationRate;
      this.randomMutationRate = randomMutationRate;
      this.randomSeed         = randomSeed;
      randomizer = new Random(randomSeed);
      genes      = new Vector<Gene>();
   }


   // Mutate.
   void mutate()
   {
      for (int i = 0; i < genes.size(); i++)
      {
         genes.get(i).mutate();
      }
   }


   // Randomly merge genome values from given genome.
   void meldValues(Genome from1, Genome from2)
   {
      Gene gene;

      for (int i = 0; i < genes.size(); i++)
      {
         gene = genes.get(i);
         if (randomizer.nextBoolean())
         {
            gene.copyValue(from1.genes.get(i));
         }
         else
         {
            gene.copyValue(from2.genes.get(i));
         }
      }
   }


   // Copy genome values from given genome.
   void copyValues(Genome from)
   {
      Gene gene;

      for (int i = 0; i < genes.size(); i++)
      {
         gene = genes.get(i);
         gene.copyValue(from.genes.get(i));
      }
   }


   // Get genome as key-value pairs.
   void getKeyValues(Vector<String> keys, Vector<Object> values)
   {
      Gene gene;

      keys.clear();
      values.clear();
      for (int i = 0; i < genes.size(); i++)
      {
         gene = genes.get(i);
         keys.add(new String(gene.name));
         switch (gene.type)
         {
         case INTEGER_VALUE:
            values.add(gene.ivalue + "");
            break;

         case FLOAT_VALUE:
            values.add(gene.fvalue + "");
            break;

         case DOUBLE_VALUE:
            values.add(gene.dvalue + "");
            break;
         }
      }
   }


   // Load values.
   void loadValues(DataInputStream reader) throws IOException
   {
      for (int i = 0; i < genes.size(); i++)
      {
         genes.get(i).loadValue(reader);
      }
   }


   // Save values.
   void saveValues(PrintWriter writer) throws IOException
   {
      for (int i = 0; i < genes.size(); i++)
      {
         genes.get(i).saveValue(writer);
      }
      writer.flush();
   }


   // Print genome.
   void print()
   {
      for (int i = 0; i < genes.size(); i++)
      {
         genes.get(i).print();
      }
   }
}
