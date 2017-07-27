/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public final class LedMode {
  public final static LedMode LedMode_DisableAll = new LedMode("LedMode_DisableAll", jlibapogeeJNI.LedMode_DisableAll_get());
  public final static LedMode LedMode_DisableWhileExpose = new LedMode("LedMode_DisableWhileExpose", jlibapogeeJNI.LedMode_DisableWhileExpose_get());
  public final static LedMode LedMode_EnableAll = new LedMode("LedMode_EnableAll", jlibapogeeJNI.LedMode_EnableAll_get());
  public final static LedMode LedMode_Unknown = new LedMode("LedMode_Unknown", jlibapogeeJNI.LedMode_Unknown_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static LedMode swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + LedMode.class + " with value " + swigValue);
  }

  private LedMode(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private LedMode(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private LedMode(String swigName, LedMode swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static LedMode[] swigValues = { LedMode_DisableAll, LedMode_DisableWhileExpose, LedMode_EnableAll, LedMode_Unknown };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

