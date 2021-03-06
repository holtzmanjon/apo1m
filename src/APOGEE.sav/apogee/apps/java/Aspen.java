/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public class Aspen extends CamGen2Base {
  private long swigCPtr;

  protected Aspen(long cPtr, boolean cMemoryOwn) {
    super(jlibapogeeJNI.Aspen_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Aspen obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        jlibapogeeJNI.delete_Aspen(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public Aspen() {
    this(jlibapogeeJNI.new_Aspen(), true);
  }

  public void OpenConnection(String ioType, String DeviceAddr, int FirmwareRev, int Id) {
    jlibapogeeJNI.Aspen_OpenConnection(swigCPtr, this, ioType, DeviceAddr, FirmwareRev, Id);
  }

  public void CloseConnection() {
    jlibapogeeJNI.Aspen_CloseConnection(swigCPtr, this);
  }

  public void StartExposure(double Duration, boolean IsLight) {
    jlibapogeeJNI.Aspen_StartExposure(swigCPtr, this, Duration, IsLight);
  }

  public int GetNumAdChannels() {
    return jlibapogeeJNI.Aspen_GetNumAdChannels(swigCPtr, this);
  }

  public String GetMacAddress() {
    return jlibapogeeJNI.Aspen_GetMacAddress(swigCPtr, this);
  }

  public void Init() {
    jlibapogeeJNI.Aspen_Init(swigCPtr, this);
  }

  public FanMode GetFanMode() {
    return FanMode.swigToEnum(jlibapogeeJNI.Aspen_GetFanMode(swigCPtr, this));
  }

  public void SetFanMode(FanMode mode, boolean PreCondCheck) {
    jlibapogeeJNI.Aspen_SetFanMode__SWIG_0(swigCPtr, this, mode.swigValue(), PreCondCheck);
  }

  public void SetFanMode(FanMode mode) {
    jlibapogeeJNI.Aspen_SetFanMode__SWIG_1(swigCPtr, this, mode.swigValue());
  }

  public boolean IsDualReadoutSupported() {
    return jlibapogeeJNI.Aspen_IsDualReadoutSupported(swigCPtr, this);
  }

  public void SetDualReadout(boolean TurnOn) {
    jlibapogeeJNI.Aspen_SetDualReadout(swigCPtr, this, TurnOn);
  }

  public boolean GetDualReadout() {
    return jlibapogeeJNI.Aspen_GetDualReadout(swigCPtr, this);
  }

}
