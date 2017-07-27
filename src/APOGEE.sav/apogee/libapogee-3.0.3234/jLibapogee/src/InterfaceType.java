/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public final class InterfaceType {
  public final static InterfaceType UNKNOWN_INTERFACE = new InterfaceType("UNKNOWN_INTERFACE");
  public final static InterfaceType USB = new InterfaceType("USB");
  public final static InterfaceType ETHERNET = new InterfaceType("ETHERNET");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static InterfaceType swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + InterfaceType.class + " with value " + swigValue);
  }

  private InterfaceType(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private InterfaceType(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private InterfaceType(String swigName, InterfaceType swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static InterfaceType[] swigValues = { UNKNOWN_INTERFACE, USB, ETHERNET };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

