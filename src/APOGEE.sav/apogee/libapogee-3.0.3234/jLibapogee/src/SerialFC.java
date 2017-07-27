/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public final class SerialFC {
  public final static SerialFC SerialFC_Unknown = new SerialFC("SerialFC_Unknown", jlibapogeeJNI.SerialFC_Unknown_get());
  public final static SerialFC SerialFC_Off = new SerialFC("SerialFC_Off", jlibapogeeJNI.SerialFC_Off_get());
  public final static SerialFC SerialFC_On = new SerialFC("SerialFC_On", jlibapogeeJNI.SerialFC_On_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static SerialFC swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + SerialFC.class + " with value " + swigValue);
  }

  private SerialFC(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private SerialFC(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private SerialFC(String swigName, SerialFC swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static SerialFC[] swigValues = { SerialFC_Unknown, SerialFC_Off, SerialFC_On };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}
