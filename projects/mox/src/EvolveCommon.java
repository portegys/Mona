/*
 * Evolution common.
 */

import java.util.*;
import java.io.*;
import mona.NativeFileDescriptor;

public class EvolveCommon
{
   // Parameters.
   public static final int FORAGER_FIT_POPULATION_SIZE = 20;
   public static final int FORAGER_NUM_MUTANTS         = 10;
   public static final int FORAGER_NUM_OFFSPRING       = 10;
   public static final int FORAGER_POPULATION_SIZE     =
      (FORAGER_FIT_POPULATION_SIZE + FORAGER_NUM_MUTANTS + FORAGER_NUM_OFFSPRING);
   public static final int PREDATOR_FIT_POPULATION_SIZE = 20;
   public static final int PREDATOR_NUM_MUTANTS         = 10;
   public static final int PREDATOR_NUM_OFFSPRING       = 10;
   public static final int PREDATOR_POPULATION_SIZE     =
      (PREDATOR_FIT_POPULATION_SIZE + PREDATOR_NUM_MUTANTS + PREDATOR_NUM_OFFSPRING);
   public static final double DEFAULT_MUTATION_RATE        = 0.25;
   public static double       MutationRate                 = DEFAULT_MUTATION_RATE;
   public static final double DEFAULT_RANDOM_MUTATION_RATE = 0.5;
   public static double       RandomMutationRate           = DEFAULT_RANDOM_MUTATION_RATE;
   public static final float  DEFAULT_MAX_SENSOR_RANGE     = 10.0f;
   public static float        MaxSensorRange               = DEFAULT_MAX_SENSOR_RANGE;
   public static int          MAX_PREPARATION_TRIALS       = 5;
   public static final int    DEFAULT_RANDOM_SEED          = 4517;
   public static int          RandomSeed     = DEFAULT_RANDOM_SEED;
   public static final int    SAVE_FREQUENCY = 1;

   // Mox populations.
   public static enum MOX_POPULATIONS
   {
      FORAGERS_ONLY(0, "foragers_only"),
      FORAGERS_AND_PREDATORS(1, "foragers_and_predators");

      private int value;
      private String name;

      MOX_POPULATIONS(int value, String name)
      {
         this.value = value;
         this.name  = name;
      }

      public int getValue()
      {
         return(value);
      }


      public void setValue(int value)
      {
         this.value = value;
      }


      public String getName()
      {
         return(name);
      }


      public void setName(String name)
      {
         this.name = name;
      }
   }

   // Mox parameter genome.
   public static class MoxParmGenome extends Genome
   {
      Gene maxMediatorLevel;
      Gene maxResponseEquippedMediatorLevel;
      Gene minResponseUnequippedMediatorLevel;

      // Constructor.
      MoxParmGenome(Random randomizer)
      {
         super(MutationRate, RandomMutationRate, randomizer.nextInt());

         // INITIAL_ENABLEMENT.
         genes.add(
            new Gene("INITIAL_ENABLEMENT", 0.1, 0.1, 1.0, 0.1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // DRIVE_ATTENUATION.
         genes.add(
            new Gene("DRIVE_ATTENUATION", 0.0, 0.0, 1.0, 0.1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // LEARNING_DECREASE_VELOCITY.
         genes.add(
            new Gene("LEARNING_DECREASE_VELOCITY", 0.1, 0.1, 0.9, 0.1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // LEARNING_INCREASE_VELOCITY.
         genes.add(
            new Gene("LEARNING_INCREASE_VELOCITY", 0.1, 0.1, 0.9, 0.1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // FIRING_STRENGTH_LEARNING_DAMPER.
         genes.add(
            new Gene("FIRING_STRENGTH_LEARNING_DAMPER", 0.1, 0.05, 0.9, 0.05,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // UTILITY_ASYMPTOTE.
         genes.add(
            new Gene("UTILITY_ASYMPTOTE", 10.0, 0.0, 100.0, 10.0,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // RESPONSE_RANDOMNESS.
         genes.add(
            new Gene("RESPONSE_RANDOMNESS", 0.01, 0.01, 0.2, 0.01,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL.
         genes.add(
            new Gene("DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL", 1, 1, 3, 1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // DEFAULT_NUM_EFFECT_EVENT_INTERVALS.
         genes.add(
            new Gene("DEFAULT_NUM_EFFECT_EVENT_INTERVALS", 1, 1, 3, 1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // MAX_ASSOCIATOR_EVENTS.
         genes.add(
            new Gene("MAX_ASSOCIATOR_EVENTS", 1, 1, 5, 1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // MAX_MEDIATORS.
         genes.add(
            new Gene("MAX_MEDIATORS", 100, 50, 500, 50,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // MAX_MEDIATOR_LEVEL.
         maxMediatorLevel =
            new Gene("MAX_MEDIATOR_LEVEL", 2, 1, 5, 1,
                     MutationRate, RandomMutationRate, randomizer.nextInt());
         genes.add(maxMediatorLevel);

         // MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL.
         maxResponseEquippedMediatorLevel =
            new Gene("MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL", 2, 1, 5, 1,
                     MutationRate, RandomMutationRate, randomizer.nextInt());
         genes.add(maxResponseEquippedMediatorLevel);

         // MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL.
         minResponseUnequippedMediatorLevel =
            new Gene("MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL", 1, 1, 5, 1,
                     MutationRate, RandomMutationRate, randomizer.nextInt());
         genes.add(minResponseUnequippedMediatorLevel);
      }


      // Mutate.
      void mutate()
      {
         // Mutate.
         super.mutate();

         // Sanity checks.
         if (maxResponseEquippedMediatorLevel.ivalue > maxMediatorLevel.ivalue)
         {
            maxResponseEquippedMediatorLevel.ivalue = maxMediatorLevel.ivalue;
         }
         if (minResponseUnequippedMediatorLevel.ivalue > maxMediatorLevel.ivalue)
         {
            minResponseUnequippedMediatorLevel.ivalue = maxMediatorLevel.ivalue;
         }
         if (minResponseUnequippedMediatorLevel.ivalue >
             (maxResponseEquippedMediatorLevel.ivalue + 1))
         {
            if (randomizer.nextBoolean())
            {
               minResponseUnequippedMediatorLevel.ivalue =
                  maxResponseEquippedMediatorLevel.ivalue;
            }
            else
            {
               maxResponseEquippedMediatorLevel.ivalue =
                  minResponseUnequippedMediatorLevel.ivalue;
            }
         }
      }
   }

   // Mox homeostat genome.
   public static class MoxHomeostatGenome extends Genome
   {
      float[] sensors;
      int    sensorMode;
      int    response;
      double goalValue;
      int    frequency;
      double periodicNeed;

      // Constructors.
      MoxHomeostatGenome(Random randomizer)
      {
         super(MutationRate, RandomMutationRate, randomizer.nextInt());

         sensors = new float[Mox.SENSOR_CONFIG.NUM_SENSORS.getValue()];

         // RANGE_SENSOR.
         genes.add(
            new Gene("RANGE_SENSOR", 0.0f, 0.0f, MaxSensorRange, 1.0f,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // COLOR_SENSOR.
         // Mapping: 0=empty, 1=green, 2=blue, 3=forager, 4=predator
         genes.add(
            new Gene("COLOR_SENSOR", 0, 0, 4, 1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // RESPONSE.
         // NULL_RESPONSE=0, WAIT=1, ...
         genes.add(
            new Gene("RESPONSE", 0, 0, 4, 1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // GOAL_VALUE.
         genes.add(
            new Gene("GOAL_VALUE", 0.1, 0.1, 1.0, 0.1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // FREQUENCY.
         genes.add(
            new Gene("FREQUENCY", 0, 0, 50, 5,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));

         // PERIODIC_NEED.
         genes.add(
            new Gene("PERIODIC_NEED", 0.1, 0.1, 1.0, 0.1,
                     MutationRate, RandomMutationRate, randomizer.nextInt()));
      }


      // Mutate.
      void mutate()
      {
         // Mutate.
         super.mutate();
      }


      // Extract values.
      void extractValues()
      {
         for (int i = 0; i < genes.size(); i++)
         {
            if (genes.get(i).name.equals("RANGE_SENSOR"))
            {
               sensors[0] = genes.get(i).fvalue;
            }
            else if (genes.get(i).name.equals("COLOR_SENSOR"))
            {
               switch (genes.get(i).ivalue)
               {
               case 0:
                  sensors[1] = 0.0f;
                  break;

               case 1:
                  sensors[1] = (float)GameOfLife.GREEN_CELL_COLOR_VALUE;
                  break;

               case 2:
                  sensors[1] = (float)GameOfLife.BLUE_CELL_COLOR_VALUE;
                  break;

               case 3:
                  sensors[1] = (float)ForagerMox.FORAGER_COLOR_VALUE;
                  break;

               case 4:
                  sensors[1] = (float)PredatorMox.PREDATOR_COLOR_VALUE;
                  break;
               }
            }
            else if (genes.get(i).name.equals("RESPONSE"))
            {
               switch (genes.get(i).ivalue)
               {
               case 0:
                  response = Mox.RESPONSE_TYPE.NULL_RESPONSE.getValue();
                  break;

               case 1:
                  response = Mox.RESPONSE_TYPE.WAIT.getValue();
                  break;

               case 2:
                  response = Mox.RESPONSE_TYPE.FORWARD.getValue();
                  break;

               case 3:
                  response = Mox.RESPONSE_TYPE.RIGHT.getValue();
                  break;

               case 4:
                  response = Mox.RESPONSE_TYPE.LEFT.getValue();
                  break;
               }
            }
            else if (genes.get(i).name.equals("GOAL_VALUE"))
            {
               goalValue = genes.get(i).dvalue;
            }
            else if (genes.get(i).name.equals("FREQUENCY"))
            {
               frequency = genes.get(i).ivalue;
            }
            else if (genes.get(i).name.equals("PERIODIC_NEED"))
            {
               periodicNeed = genes.get(i).dvalue;
            }
         }
      }


      // Load values.
      void loadValues(DataInputStream reader) throws IOException
      {
         super.loadValues(reader);

         // Cap RANGE_SENSOR value at current maximum.
         Gene gene;
         for (int i = 0; i < genes.size(); i++)
         {
            gene = genes.get(i);
            if (gene.name.equals("RANGE_SENSOR"))
            {
               if (gene.fvalue > MaxSensorRange)
               {
                  gene.fvalue = MaxSensorRange;
               }
               break;
            }
         }
      }
   }

   // Mox ID dispenser.
   public static int MoxIdDispenser = 0;

   // Population member.
   public static class Member
   {
      int    species;
      int    generation;
      double fitness;

      // Mox parameters.
      MoxParmGenome moxParmGenome;

      // Instincts.
      Gene numInstincts;
      Vector<MoxHomeostatGenome> instinctHomeostats;

      // Mox.
      Mox mox;

      // Constructors.
      Member(int species, int generation, int x, int y, int direction, Random randomizer)
      {
         this.species    = species;
         this.generation = generation;
         fitness         = 0.0;

         // Create parameter genome.
         moxParmGenome = new MoxParmGenome(randomizer);

         // Create instinct genomes.
         numInstincts = new Gene("NUM_INSTINCTS", 0, 0, 5, 1,
                                 MutationRate, RandomMutationRate, randomizer.nextInt());
         instinctHomeostats = new Vector<MoxHomeostatGenome>();
         for (int i = 0; i < numInstincts.ivalue; i++)
         {
            instinctHomeostats.add(new MoxHomeostatGenome(randomizer));
         }

         // Initialize mox.
         initMox(x, y, direction, randomizer);
      }


      // Construct mutation of given member.
      Member(Member member, int generation, Random randomizer)
      {
         species         = member.species;
         this.generation = generation;
         fitness         = 0.0;

         // Create and mutate parameter genome.
         moxParmGenome = new MoxParmGenome(randomizer);
         moxParmGenome.copyValues(member.moxParmGenome);
         moxParmGenome.mutate();

         // Create and mutate instinct genomes.
         numInstincts = new Gene("NUM_INSTINCTS", 0, 0, 5, 1,
                                 MutationRate, RandomMutationRate, randomizer.nextInt());
         numInstincts.copyValue(member.numInstincts);
         numInstincts.mutate();
         instinctHomeostats = new Vector<MoxHomeostatGenome>();
         for (int i = 0; i < numInstincts.ivalue; i++)
         {
            instinctHomeostats.add(new MoxHomeostatGenome(randomizer));
            if (i < member.numInstincts.ivalue)
            {
               instinctHomeostats.get(i).copyValues(member.instinctHomeostats.get(i));
               instinctHomeostats.get(i).mutate();
            }
         }

         // Initialize mox.
         initMox(member.mox.x2, member.mox.y2, member.mox.direction2, randomizer);
      }


      // Construct by melding given members.
      Member(Member member1, Member member2, int generation, Random randomizer)
      {
         species         = member1.species;
         this.generation = generation;
         fitness         = 0.0;

         // Create and meld parameter genome.
         moxParmGenome = new MoxParmGenome(randomizer);
         moxParmGenome.meldValues(member1.moxParmGenome, member2.moxParmGenome);

         // Create and meld instinct genomes.
         numInstincts = new Gene("NUM_INSTINCTS", 0, 0, 5, 1,
                                 MutationRate, RandomMutationRate, randomizer.nextInt());
         if (randomizer.nextBoolean())
         {
            numInstincts.copyValue(member1.numInstincts);
            instinctHomeostats = new Vector<MoxHomeostatGenome>();
            for (int i = 0; i < numInstincts.ivalue; i++)
            {
               instinctHomeostats.add(new MoxHomeostatGenome(randomizer));
               instinctHomeostats.get(i).copyValues(member1.instinctHomeostats.get(i));
            }
            initMox(member1.mox.x2, member1.mox.y2, member1.mox.direction2, randomizer);
         }
         else
         {
            numInstincts.copyValue(member2.numInstincts);
            instinctHomeostats = new Vector<MoxHomeostatGenome>();
            for (int i = 0; i < numInstincts.ivalue; i++)
            {
               instinctHomeostats.add(new MoxHomeostatGenome(randomizer));
               instinctHomeostats.get(i).copyValues(member2.instinctHomeostats.get(i));
            }
            initMox(member2.mox.x2, member2.mox.y2, member2.mox.direction2, randomizer);
         }
      }


      // Initialize mox.
      void initMox(int x, int y, int direction, Random randomizer)
      {
         // Get parameters.
         Vector<String> parameterKeys   = new Vector<String>();
         Vector<Object> parameterValues = new Vector<Object>();
         moxParmGenome.getKeyValues(parameterKeys, parameterValues);

         // Construct mox.
         parameterKeys.add("SENSOR_MODES");
         Object[] modes = new Object[Mox.SENSOR_CONFIG.NUM_SPECIFIC_SENSOR_MODES.getValue()];
         modes[0]       = Mox.SENSOR_CONFIG.NUM_RANGE_SENSORS.getValue() + "";
         modes[1]       = Mox.SENSOR_CONFIG.NUM_COLOR_SENSORS.getValue() + "";
         parameterValues.add(modes);
         parameterKeys.add("NUM_RESPONSES");
         parameterValues.add(Mox.RESPONSE_TYPE.NUM_RESPONSES.getValue() + "");
         parameterKeys.add("RANDOM_SEED");
         parameterValues.add(randomizer.nextInt() + "");
         parameterKeys.add("NUM_NEEDS");
         int numNeeds;
         if (species == Mox.SPECIES.FORAGER.getValue())
         {
            numNeeds = ForagerMox.NEED_TYPE.NUM_NEEDS.getValue();
            parameterValues.add(numNeeds + numInstincts.ivalue + "");
            mox = new ForagerMox(MoxIdDispenser, x, y, direction,
                                 parameterKeys, parameterValues);
            addInstincts(numNeeds);
         }
         else
         {
            numNeeds = PredatorMox.NEED_TYPE.NUM_NEEDS.getValue();
            parameterValues.add(numNeeds + numInstincts.ivalue + "");
            mox = new PredatorMox(MoxIdDispenser, x, y, direction,
                                  parameterKeys, parameterValues);
            addInstincts(numNeeds);
         }
         MoxIdDispenser++;
      }


      // Add instincts to mox.
      void addInstincts(int startIndex)
      {
         MoxHomeostatGenome homeostat;

         for (int i = 0; i < instinctHomeostats.size(); i++)
         {
            homeostat = instinctHomeostats.get(i);
            homeostat.extractValues();
            mox.addGoal(i + startIndex, homeostat.sensors,
                        homeostat.sensorMode, homeostat.response,
                        homeostat.goalValue, homeostat.frequency,
                        homeostat.periodicNeed);
         }
      }


      // Load member.
      void load(FileInputStream input, NativeFileDescriptor fd)
      throws IOException
      {
         // DataInputStream is for unbuffered input.
         DataInputStream reader = new DataInputStream(input);

         species    = Utility.loadInt(reader);
         generation = Utility.loadInt(reader);
         fitness    = Utility.loadDouble(reader);

         // Load parameter genome.
         moxParmGenome.loadValues(reader);

         // Load instincts.
         numInstincts.loadValue(reader);
         instinctHomeostats = new Vector<MoxHomeostatGenome>();
         for (int i = 0; i < numInstincts.ivalue; i++)
         {
            instinctHomeostats.add(new MoxHomeostatGenome(new Random()));
            instinctHomeostats.get(i).loadValues(reader);
         }

         // Load mox.
         mox.load(input, fd);
         if (mox.id >= MoxIdDispenser)
         {
            MoxIdDispenser = mox.id + 1;
         }
      }


      // Save member.
      void save(FileOutputStream output, NativeFileDescriptor fd) throws IOException
      {
         PrintWriter writer = new PrintWriter(new OutputStreamWriter(output));

         Utility.saveInt(writer, species);
         Utility.saveInt(writer, generation);
         Utility.saveDouble(writer, fitness);
         writer.flush();

         // Save parameter genome.
         moxParmGenome.saveValues(writer);
         writer.flush();

         // Save instincts.
         numInstincts.saveValue(writer);
         for (int i = 0; i < numInstincts.ivalue; i++)
         {
            instinctHomeostats.get(i).saveValues(writer);
         }
         writer.flush();

         // Save mox.
         mox.save(output, fd);
      }


      // Clear.
      void clear()
      {
         species            = 0;
         generation         = 0;
         fitness            = 0.0;
         moxParmGenome      = null;
         numInstincts       = null;
         instinctHomeostats = null;
         mox.clear();
         mox = null;
      }


      // Print properties.
      void printProperties()
      {
         System.out.println(getInfo());
         System.out.println("parameters:");
         moxParmGenome.print();
         for (int i = 0; i < numInstincts.ivalue; i++)
         {
            System.out.println("instinct " + i + ":");
            instinctHomeostats.get(i).print();
         }
      }


      // Get information.
      String getInfo()
      {
         return("mox=" + mox.id +
                ", fitness=" + fitness +
                ", generation=" + generation);
      }
   }
}
