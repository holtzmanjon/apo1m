/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public final class Resolution {
  public final static Resolution Resolution_SixteenBit = new Resolution("Resolution_SixteenBit", jlibapogeeJNI.Resolution_SixteenBit_get());
  public final static Resolution Resolution_TwelveBit = new Resolution("Resolution_TwelveBit", jlibapogeeJNI.Resolution_TwelveBit_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static Resolution swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + Resolution.class + " with value " + swigValue);
  }

  private Resolution(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private Resolution(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private Resolution(String swigName, Resolution swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static Resolution[] swigValues = { Resolution_SixteenBit, Resolution_TwelveBit };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

