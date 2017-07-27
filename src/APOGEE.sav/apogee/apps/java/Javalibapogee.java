/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.33
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


public class Javalibapogee {
  public static SWIGTYPE_p_std__vectorTstd__string_t MkStrVectFromStrDb(StrDb DbStruct) {
    return new SWIGTYPE_p_std__vectorTstd__string_t(JavalibapogeeJNI.MkStrVectFromStrDb(StrDb.getCPtr(DbStruct), DbStruct), true);
  }

  public static StrDb MkStrDbFromStrVect(SWIGTYPE_p_std__vectorTstd__string_t strVect) {
    return new StrDb(JavalibapogeeJNI.MkStrDbFromStrVect(SWIGTYPE_p_std__vectorTstd__string_t.getCPtr(strVect)), true);
  }

  public static StrDb GetNoOpDb() {
    return new StrDb(JavalibapogeeJNI.GetNoOpDb(), true);
  }

  public static boolean IsAlta(SWIGTYPE_p_uint16_t FirmwareRev) {
    return JavalibapogeeJNI.IsAlta(SWIGTYPE_p_uint16_t.getCPtr(FirmwareRev));
  }

  public static boolean IsGen2Platform(SWIGTYPE_p_uint16_t FirmwareRev) {
    return JavalibapogeeJNI.IsGen2Platform(SWIGTYPE_p_uint16_t.getCPtr(FirmwareRev));
  }

  public static boolean IsFirmwareRevGood(SWIGTYPE_p_uint16_t FirmwareRev) {
    return JavalibapogeeJNI.IsFirmwareRevGood(SWIGTYPE_p_uint16_t.getCPtr(FirmwareRev));
  }

  public static SWIGTYPE_p_uint16_t MaskRawId(SWIGTYPE_p_uint16_t FirmwareRev, SWIGTYPE_p_uint16_t CamId) {
    return new SWIGTYPE_p_uint16_t(JavalibapogeeJNI.MaskRawId(SWIGTYPE_p_uint16_t.getCPtr(FirmwareRev), SWIGTYPE_p_uint16_t.getCPtr(CamId)), true);
  }

  public static PlatformType GetPlatformType(SWIGTYPE_p_uint16_t FixedId, boolean IsEthernet) {
    return PlatformType.swigToEnum(JavalibapogeeJNI.GetPlatformType__SWIG_0(SWIGTYPE_p_uint16_t.getCPtr(FixedId), IsEthernet));
  }

  public static PlatformType GetPlatformType(SWIGTYPE_p_uint16_t FixedId) {
    return PlatformType.swigToEnum(JavalibapogeeJNI.GetPlatformType__SWIG_1(SWIGTYPE_p_uint16_t.getCPtr(FixedId)));
  }

  public static PlatformType GetPlatformType(String cameraLine) {
    return PlatformType.swigToEnum(JavalibapogeeJNI.GetPlatformType__SWIG_2(cameraLine));
  }

  public static String GetPlatformStr(SWIGTYPE_p_uint16_t FixedId, boolean IsEthernet) {
    return JavalibapogeeJNI.GetPlatformStr__SWIG_0(SWIGTYPE_p_uint16_t.getCPtr(FixedId), IsEthernet);
  }

  public static String GetPlatformStr(SWIGTYPE_p_uint16_t FixedId) {
    return JavalibapogeeJNI.GetPlatformStr__SWIG_1(SWIGTYPE_p_uint16_t.getCPtr(FixedId));
  }

  public static String GetModelStr(SWIGTYPE_p_uint16_t CamId) {
    return JavalibapogeeJNI.GetModelStr(SWIGTYPE_p_uint16_t.getCPtr(CamId));
  }

  public static String GetNoOpFirmwareRev() {
    return JavalibapogeeJNI.GetNoOpFirmwareRev();
  }

  public static SWIGTYPE_p_uint16_t getFIRMWARE_PLATFORM_MASK() {
    return new SWIGTYPE_p_uint16_t(JavalibapogeeJNI.FIRMWARE_PLATFORM_MASK_get(), true);
  }

  public static SWIGTYPE_p_uint16_t getMAX_ALTA_FIRMWARE_REV() {
    return new SWIGTYPE_p_uint16_t(JavalibapogeeJNI.MAX_ALTA_FIRMWARE_REV_get(), true);
  }

  public static SWIGTYPE_p_uint16_t getMIN_GEN2_FIRMWARE() {
    return new SWIGTYPE_p_uint16_t(JavalibapogeeJNI.MIN_GEN2_FIRMWARE_get(), true);
  }

  public static SWIGTYPE_p_uint16_t getMAX_GEN2_FIRMWARE() {
    return new SWIGTYPE_p_uint16_t(JavalibapogeeJNI.MAX_GEN2_FIRMWARE_get(), true);
  }

  public static SWIGTYPE_p_uint16_t getALTA_CAMERA_ID_MASK() {
    return new SWIGTYPE_p_uint16_t(JavalibapogeeJNI.ALTA_CAMERA_ID_MASK_get(), true);
  }

  public static SWIGTYPE_p_uint16_t getGEN2_CAMERA_ID_MASK() {
    return new SWIGTYPE_p_uint16_t(JavalibapogeeJNI.GEN2_CAMERA_ID_MASK_get(), true);
  }

  public static SWIGTYPE_p_uint16_t getNO_OP_FRMWR_REV() {
    return new SWIGTYPE_p_uint16_t(JavalibapogeeJNI.NO_OP_FRMWR_REV_get(), true);
  }

}