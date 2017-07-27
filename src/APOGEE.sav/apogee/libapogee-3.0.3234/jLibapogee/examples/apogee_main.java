import java.util.regex.Matcher;
import java.util.regex.Pattern;

import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;


import com.apogee.driver.*;

public class apogee_main {
   static {
      try {

         System.loadLibrary("jlibapogee");
         
        } catch (UnsatisfiedLinkError e) {
         
         System.err.println("Native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n" + e);
         System.exit(1);
         
        }
  }
  
  public static void main(String argv[]) {
     
     // search for the camera on the USB bus
     FindDeviceUsb look4cam = new FindDeviceUsb();
     String msg = look4cam.Find();
      
     String item = "interface=(.*?)[,|<]";
     Pattern r = Pattern.compile(item);
     Matcher m = r.matcher(msg);
     String interfaceStr = "";
     
     // parse the resulting find string
     // for the camera.  assuming only one
     // camera is connected for this example
     try {
      if(m.find()){
         interfaceStr = m.group(1);
      }
      else{
         String errStr = item + " not found";
         throw new apgException( errStr );
      }
      
      item = "address=(.*?)[,|<]";
      r = Pattern.compile(item);
      m = r.matcher(msg);
      String addrStr = "";
      
      if(m.find()){
         addrStr = m.group(1);
      }
      else{
         String errStr = item + " not found";
         throw new apgException( errStr );
      }
      
      item = "id=(.*?)[,|<]";
      r = Pattern.compile(item);
      m = r.matcher(msg);
      int id = 0;
      
      if(m.find()){
         id = Integer.decode(m.group(1));
      }
      else{
         String errStr = item + " not found";
         throw new apgException( errStr );
      }
      
      item = "firmwareRev=(.*?)[,|<]";
      r = Pattern.compile(item);
      m = r.matcher(msg);
      int frwRev = 0;
      
      if(m.find()){
         frwRev = Integer.decode(m.group(1));
      }
      else{
         String errStr = item + " not found";
         throw new apgException( errStr );
      }
      
      System.out.println(interfaceStr + "," + addrStr + "," + frwRev + "," + id );
      
      // now that we have the connection information
      // create the camera object, open a connection
      // and intialize the camera
      Alta cam = new Alta();
      
      cam.OpenConnection( interfaceStr, addrStr, frwRev, id);
      
      cam.Init();
      
      System.out.println( "Successfully connected to " + cam.GetModel()  );
      
      // take an exposure
      int rows = cam.GetRoiNumRows();
      int cols = cam.GetRoiNumCols();
      System.out.println( "Staring light exposure of size rows = " + rows +
                     " x cols = " + cols );
      
      double exposeTime = 0.2;
      
      cam.StartExposure( exposeTime, true );
      
      Status camStatus = Status.Status_Idle;
      
      while( camStatus != Status.Status_ImageReady) {
      
         camStatus = cam.GetImagingStatus();
         
         if( Status.Status_ConnectionError == camStatus ||
            Status.Status_DataError == camStatus ||
            Status.Status_PatternError == camStatus ) {
            
            String errStr = "IMAGING FAILED - error in camera status = " + camStatus;
            throw new apgException( errStr );         
         }
      }
   
      // data is ready now download it from the camera
      System.out.println("Getting image");
      Uint16Vector data = new Uint16Vector();
      
      cam.GetImage( data );
      
      // save the file in 32bit signed int format
      String fname = "img.bin";
      System.out.println( "writing image to file " + fname );
      
      DataOutputStream os = new DataOutputStream(new FileOutputStream(
        fname));
      
      for( int i=0; i < data.size(); i++ ) {
          os.write((byte) (data.get(i) >> 8));
	  os.write((byte) data.get(i));
      }
      
      // close up everything and exit
      os.close();
    
      System.out.println( "Closing connection to camera.");
      cam.CloseConnection();
     
   } catch(apgException e) {
      System.out.println("caught " + e);
   } catch(FileNotFoundException e ) {
      e.printStackTrace();
   } catch(IOException e) {
      e.printStackTrace();
   }

  }
} // end main

class apgException extends Exception {
   apgException(String strMessage){
      super(strMessage);
   }
}
