/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.33
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


public class Gee extends CamGen2Base {
  private long swigCPtr;

  protected Gee(long cPtr, boolean cMemoryOwn) {
    super(JavalibapogeeJNI.SWIGGeeUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Gee obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      JavalibapogeeJNI.delete_Gee(swigCPtr);
    }
    swigCPtr = 0;
    super.delete();
  }

  public Gee() {
    this(JavalibapogeeJNI.new_Gee(), true);
  }

  public void OpenConnection(String ioType, String DeviceAddr, SWIGTYPE_p_uint16_t FirmwareRev, SWIGTYPE_p_uint16_t Id) {
    JavalibapogeeJNI.Gee_OpenConnection(swigCPtr, this, ioType, DeviceAddr, SWIGTYPE_p_uint16_t.getCPtr(FirmwareRev), SWIGTYPE_p_uint16_t.getCPtr(Id));
  }

  public void CloseConnection() {
    JavalibapogeeJNI.Gee_CloseConnection(swigCPtr, this);
  }

  public void StartExposure(double Duration, boolean IsLight) {
    JavalibapogeeJNI.Gee_StartExposure(swigCPtr, this, Duration, IsLight);
  }

  public SWIGTYPE_p_int32_t GetNumAdChannels() {
    return new SWIGTYPE_p_int32_t(JavalibapogeeJNI.Gee_GetNumAdChannels(swigCPtr, this), true);
  }

  public double GetTempHeatsink() {
    return JavalibapogeeJNI.Gee_GetTempHeatsink(swigCPtr, this);
  }

  public String GetMacAddress() {
    return JavalibapogeeJNI.Gee_GetMacAddress(swigCPtr, this);
  }

}