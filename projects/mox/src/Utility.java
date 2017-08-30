// Utility class.

import java.util.*;
import java.io.*;

public abstract class Utility
{
   // Load integer.
   @SuppressWarnings({ "deprecation" })
   public static int loadInt(DataInputStream in) throws IOException
   {
      String          s;
      StringTokenizer t;
      int             value;

      if ((s = in.readLine()) == null)
      {
         throw (new IOException("Unexpected EOF"));
      }

      t = new StringTokenizer(s, " ");

      if (!t.hasMoreTokens())
      {
         throw (new IOException("Missing integer value"));
      }

      try
      {
         s     = t.nextToken().trim();
         value = Integer.parseInt(s);
      }
      catch (NumberFormatException e) {
         throw (new IOException("Invalid integer value " + s));
      }

      return(value);
   }


   // Load float.
   @SuppressWarnings({ "deprecation" })
   public static float loadFloat(DataInputStream in) throws IOException
   {
      String          s;
      StringTokenizer t;
      float           value;

      if ((s = in.readLine()) == null)
      {
         throw (new IOException("Unexpected EOF"));
      }

      t = new StringTokenizer(s, " ");

      if (!t.hasMoreTokens())
      {
         throw (new IOException("Missing float value"));
      }

      try
      {
         s     = t.nextToken().trim();
         value = Float.parseFloat(s);
      }
      catch (NumberFormatException e) {
         throw (new IOException("Invalid float value " + s));
      }

      return(value);
   }


   // Load double.
   @SuppressWarnings({ "deprecation" })
   public static double loadDouble(DataInputStream in) throws IOException
   {
      String          s;
      StringTokenizer t;
      double          value;

      if ((s = in.readLine()) == null)
      {
         throw (new IOException("Unexpected EOF"));
      }

      t = new StringTokenizer(s, " ");

      if (!t.hasMoreTokens())
      {
         throw (new IOException("Missing double value"));
      }

      try
      {
         s     = t.nextToken().trim();
         value = Double.parseDouble(s);
      }
      catch (NumberFormatException e) {
         throw (new IOException("Invalid double value " + s));
      }

      return(value);
   }


   // Save integer.
   public static void saveInt(PrintWriter writer, int value) throws IOException
   {
      writer.println(value + "");
      writer.flush();
   }


   // Save float.
   public static void saveFloat(PrintWriter writer, float value) throws IOException
   {
      writer.println(value + "");
      writer.flush();
   }


   // Save double.
   public static void saveDouble(PrintWriter writer, double value) throws IOException
   {
      writer.println(value + "");
      writer.flush();
   }


   // Prevent instantiation.
   private Utility() {}
}
