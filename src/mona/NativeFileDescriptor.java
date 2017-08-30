// Native file IO.
package mona;

public class NativeFileDescriptor
{
   static {
      System.loadLibrary("mona_jni");
   }

   public String filename;
   public String mode;
   public long   fp;
   public NativeFileDescriptor(String filename, String mode)
   {
      this.filename = filename;
      this.mode     = mode;
      fp            = 0;
   }


   public native boolean open();
   public native boolean close();
   public native boolean seek(long position);
   public native long tell();
}
