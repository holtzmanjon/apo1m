/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public final class FanMode {
  public final static FanMode FanMode_Off = new FanMode("FanMode_Off", jlibapogeeJNI.FanMode_Off_get());
  public final static FanMode FanMode_Low = new FanMode("FanMode_Low", jlibapogeeJNI.FanMode_Low_get());
  public final static FanMode FanMode_Medium = new FanMode("FanMode_Medium", jlibapogeeJNI.FanMode_Medium_get());
  public final static FanMode FanMode_High = new FanMode("FanMode_High", jlibapogeeJNI.FanMode_High_get());
  public final static FanMode FanMode_Unknown = new FanMode("FanMode_Unknown", jlibapogeeJNI.FanMode_Unknown_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static FanMode swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + FanMode.class + " with value " + swigValue);
  }

  private FanMode(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private FanMode(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private FanMode(String swigName, FanMode swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static FanMode[] swigValues = { FanMode_Off, FanMode_Low, FanMode_Medium, FanMode_High, FanMode_Unknown };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}
