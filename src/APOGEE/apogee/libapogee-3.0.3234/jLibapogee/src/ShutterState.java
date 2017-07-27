/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public final class ShutterState {
  public final static ShutterState ShutterState_Unkown = new ShutterState("ShutterState_Unkown");
  public final static ShutterState ShutterState_Normal = new ShutterState("ShutterState_Normal");
  public final static ShutterState ShutterState_ForceOpen = new ShutterState("ShutterState_ForceOpen");
  public final static ShutterState ShutterState_ForceClosed = new ShutterState("ShutterState_ForceClosed");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static ShutterState swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + ShutterState.class + " with value " + swigValue);
  }

  private ShutterState(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private ShutterState(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private ShutterState(String swigName, ShutterState swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static ShutterState[] swigValues = { ShutterState_Unkown, ShutterState_Normal, ShutterState_ForceOpen, ShutterState_ForceClosed };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}
