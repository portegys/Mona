// Process information.

public abstract class ProcessInformation
{
   // Get CPU (kernel + user) time used by the current process.
   // The time unit is platform dependent.
   public static native long getProcessCPUTime();

   // Load JNI library.
   private static final String PILIB = "ProcessInformation";

   static
   {
      try
      {
         System.loadLibrary(PILIB);
      }
      catch (UnsatisfiedLinkError e)
      {
         System.out.println("native lib '" + PILIB + "' not found in 'java.library.path': " + System.getProperty("java.library.path"));
         throw e;
      }
   }

   // Prevent instantiation.
   private ProcessInformation() {}
}
