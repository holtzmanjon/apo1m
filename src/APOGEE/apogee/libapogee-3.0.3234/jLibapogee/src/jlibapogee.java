/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public class jlibapogee {
  public static SWIGTYPE_p_std__vectorT_std__string_t MkStrVectFromStrDb(StrDb DbStruct) {
    return new SWIGTYPE_p_std__vectorT_std__string_t(jlibapogeeJNI.MkStrVectFromStrDb(StrDb.getCPtr(DbStruct), DbStruct), true);
  }

  public static StrDb MkStrDbFromStrVect(SWIGTYPE_p_std__vectorT_std__string_t strVect) {
    return new StrDb(jlibapogeeJNI.MkStrDbFromStrVect(SWIGTYPE_p_std__vectorT_std__string_t.getCPtr(strVect)), true);
  }

  public static StrDb GetNoOpDb() {
    return new StrDb(jlibapogeeJNI.GetNoOpDb(), true);
  }

  public static long getNET_MAGIC_VALID() {
    return jlibapogeeJNI.NET_MAGIC_VALID_get();
  }

  public static SWIGTYPE_p_std__vectorT_unsigned_char_t MkU8VectFromNetDb(NetDb DbStruct) {
    return new SWIGTYPE_p_std__vectorT_unsigned_char_t(jlibapogeeJNI.MkU8VectFromNetDb(NetDb.getCPtr(DbStruct), DbStruct), true);
  }

  public static NetDb MkNetDbFromU8Vect(SWIGTYPE_p_std__vectorT_unsigned_char_t u8Vect) {
    return new NetDb(jlibapogeeJNI.MkNetDbFromU8Vect(SWIGTYPE_p_std__vectorT_unsigned_char_t.getCPtr(u8Vect)), true);
  }

  public static boolean IsAlta(int FirmwareRev) {
    return jlibapogeeJNI.IsAlta(FirmwareRev);
  }

  public static boolean IsGen2Platform(int FirmwareRev) {
    return jlibapogeeJNI.IsGen2Platform(FirmwareRev);
  }

  public static boolean IsFirmwareRevGood(int FirmwareRev) {
    return jlibapogeeJNI.IsFirmwareRevGood(FirmwareRev);
  }

  public static int MaskRawId(int FirmwareRev, int CamId) {
    return jlibapogeeJNI.MaskRawId(FirmwareRev, CamId);
  }

  public static PlatformType GetPlatformType(int FixedId, boolean IsEthernet) {
    return PlatformType.swigToEnum(jlibapogeeJNI.GetPlatformType__SWIG_0(FixedId, IsEthernet));
  }

  public static PlatformType GetPlatformType(int FixedId) {
    return PlatformType.swigToEnum(jlibapogeeJNI.GetPlatformType__SWIG_1(FixedId));
  }

  public static PlatformType GetPlatformType(String cameraLine) {
    return PlatformType.swigToEnum(jlibapogeeJNI.GetPlatformType__SWIG_2(cameraLine));
  }

  public static String GetPlatformStr(int FixedId, boolean IsEthernet) {
    return jlibapogeeJNI.GetPlatformStr__SWIG_0(FixedId, IsEthernet);
  }

  public static String GetPlatformStr(int FixedId) {
    return jlibapogeeJNI.GetPlatformStr__SWIG_1(FixedId);
  }

  public static String GetModelStr(int CamId) {
    return jlibapogeeJNI.GetModelStr(CamId);
  }

  public static String GetNoOpFirmwareRev() {
    return jlibapogeeJNI.GetNoOpFirmwareRev();
  }

  public static int getFIRMWARE_PLATFORM_MASK() {
    return jlibapogeeJNI.FIRMWARE_PLATFORM_MASK_get();
  }

  public static int getMAX_ALTA_FIRMWARE_REV() {
    return jlibapogeeJNI.MAX_ALTA_FIRMWARE_REV_get();
  }

  public static int getMIN_GEN2_FIRMWARE() {
    return jlibapogeeJNI.MIN_GEN2_FIRMWARE_get();
  }

  public static int getMAX_GEN2_FIRMWARE() {
    return jlibapogeeJNI.MAX_GEN2_FIRMWARE_get();
  }

  public static int getALTA_CAMERA_ID_MASK() {
    return jlibapogeeJNI.ALTA_CAMERA_ID_MASK_get();
  }

  public static int getGEN2_CAMERA_ID_MASK() {
    return jlibapogeeJNI.GEN2_CAMERA_ID_MASK_get();
  }

  public static int getNO_OP_FRMWR_REV() {
    return jlibapogeeJNI.NO_OP_FRMWR_REV_get();
  }

}
