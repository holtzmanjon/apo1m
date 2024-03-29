/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public final class PlatformType {
  public final static PlatformType UNKNOWN_PLATFORM = new PlatformType("UNKNOWN_PLATFORM");
  public final static PlatformType ALTAU = new PlatformType("ALTAU");
  public final static PlatformType ALTAE = new PlatformType("ALTAE");
  public final static PlatformType ASCENT = new PlatformType("ASCENT");
  public final static PlatformType ASPEN = new PlatformType("ASPEN");
  public final static PlatformType HIC = new PlatformType("HIC");
  public final static PlatformType ALTAF = new PlatformType("ALTAF");
  public final static PlatformType QUAD = new PlatformType("QUAD");

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static PlatformType swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + PlatformType.class + " with value " + swigValue);
  }

  private PlatformType(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private PlatformType(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private PlatformType(String swigName, PlatformType swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static PlatformType[] swigValues = { UNKNOWN_PLATFORM, ALTAU, ALTAE, ASCENT, ASPEN, HIC, ALTAF, QUAD };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}

