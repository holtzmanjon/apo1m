/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.apogee.driver;

public class ApogeeCam {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected ApogeeCam(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(ApogeeCam obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        jlibapogeeJNI.delete_ApogeeCam(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public void Reset() {
    jlibapogeeJNI.ApogeeCam_Reset(swigCPtr, this);
  }

  public int ReadReg(int reg) {
    return jlibapogeeJNI.ApogeeCam_ReadReg(swigCPtr, this, reg);
  }

  public void WriteReg(int reg, int value) {
    jlibapogeeJNI.ApogeeCam_WriteReg(swigCPtr, this, reg, value);
  }

  public void SetRoiNumRows(int rows) {
    jlibapogeeJNI.ApogeeCam_SetRoiNumRows(swigCPtr, this, rows);
  }

  public void SetRoiNumCols(int cols) {
    jlibapogeeJNI.ApogeeCam_SetRoiNumCols(swigCPtr, this, cols);
  }

  public int GetRoiNumRows() {
    return jlibapogeeJNI.ApogeeCam_GetRoiNumRows(swigCPtr, this);
  }

  public int GetRoiNumCols() {
    return jlibapogeeJNI.ApogeeCam_GetRoiNumCols(swigCPtr, this);
  }

  public void SetRoiStartRow(int row) {
    jlibapogeeJNI.ApogeeCam_SetRoiStartRow(swigCPtr, this, row);
  }

  public void SetRoiStartCol(int col) {
    jlibapogeeJNI.ApogeeCam_SetRoiStartCol(swigCPtr, this, col);
  }

  public int GetRoiStartRow() {
    return jlibapogeeJNI.ApogeeCam_GetRoiStartRow(swigCPtr, this);
  }

  public int GetRoiStartCol() {
    return jlibapogeeJNI.ApogeeCam_GetRoiStartCol(swigCPtr, this);
  }

  public void SetRoiBinRow(int bin) {
    jlibapogeeJNI.ApogeeCam_SetRoiBinRow(swigCPtr, this, bin);
  }

  public int GetRoiBinRow() {
    return jlibapogeeJNI.ApogeeCam_GetRoiBinRow(swigCPtr, this);
  }

  public void SetRoiBinCol(int bin) {
    jlibapogeeJNI.ApogeeCam_SetRoiBinCol(swigCPtr, this, bin);
  }

  public int GetRoiBinCol() {
    return jlibapogeeJNI.ApogeeCam_GetRoiBinCol(swigCPtr, this);
  }

  public int GetFirmwareRev() {
    return jlibapogeeJNI.ApogeeCam_GetFirmwareRev(swigCPtr, this);
  }

  public void SetImageCount(int count) {
    jlibapogeeJNI.ApogeeCam_SetImageCount(swigCPtr, this, count);
  }

  public int GetImageCount() {
    return jlibapogeeJNI.ApogeeCam_GetImageCount(swigCPtr, this);
  }

  public int GetImgSequenceCount() {
    return jlibapogeeJNI.ApogeeCam_GetImgSequenceCount(swigCPtr, this);
  }

  public void SetSequenceDelay(double delay) {
    jlibapogeeJNI.ApogeeCam_SetSequenceDelay(swigCPtr, this, delay);
  }

  public double GetSequenceDelay() {
    return jlibapogeeJNI.ApogeeCam_GetSequenceDelay(swigCPtr, this);
  }

  public void SetVariableSequenceDelay(boolean variable) {
    jlibapogeeJNI.ApogeeCam_SetVariableSequenceDelay(swigCPtr, this, variable);
  }

  public boolean GetVariableSequenceDelay() {
    return jlibapogeeJNI.ApogeeCam_GetVariableSequenceDelay(swigCPtr, this);
  }

  public void SetTdiRate(double TdiRate) {
    jlibapogeeJNI.ApogeeCam_SetTdiRate(swigCPtr, this, TdiRate);
  }

  public double GetTdiRate() {
    return jlibapogeeJNI.ApogeeCam_GetTdiRate(swigCPtr, this);
  }

  public void SetTdiRows(int TdiRows) {
    jlibapogeeJNI.ApogeeCam_SetTdiRows(swigCPtr, this, TdiRows);
  }

  public int GetTdiRows() {
    return jlibapogeeJNI.ApogeeCam_GetTdiRows(swigCPtr, this);
  }

  public int GetTdiCounter() {
    return jlibapogeeJNI.ApogeeCam_GetTdiCounter(swigCPtr, this);
  }

  public void SetTdiBinningRows(int bin) {
    jlibapogeeJNI.ApogeeCam_SetTdiBinningRows(swigCPtr, this, bin);
  }

  public int GetTdiBinningRows() {
    return jlibapogeeJNI.ApogeeCam_GetTdiBinningRows(swigCPtr, this);
  }

  public void SetKineticsSectionHeight(int height) {
    jlibapogeeJNI.ApogeeCam_SetKineticsSectionHeight(swigCPtr, this, height);
  }

  public int GetKineticsSectionHeight() {
    return jlibapogeeJNI.ApogeeCam_GetKineticsSectionHeight(swigCPtr, this);
  }

  public void SetKineticsSections(int sections) {
    jlibapogeeJNI.ApogeeCam_SetKineticsSections(swigCPtr, this, sections);
  }

  public int GetKineticsSections() {
    return jlibapogeeJNI.ApogeeCam_GetKineticsSections(swigCPtr, this);
  }

  public void SetKineticsShiftInterval(double interval) {
    jlibapogeeJNI.ApogeeCam_SetKineticsShiftInterval(swigCPtr, this, interval);
  }

  public double GetKineticsShiftInterval() {
    return jlibapogeeJNI.ApogeeCam_GetKineticsShiftInterval(swigCPtr, this);
  }

  public void SetShutterStrobePosition(double position) {
    jlibapogeeJNI.ApogeeCam_SetShutterStrobePosition(swigCPtr, this, position);
  }

  public double GetShutterStrobePosition() {
    return jlibapogeeJNI.ApogeeCam_GetShutterStrobePosition(swigCPtr, this);
  }

  public void SetShutterStrobePeriod(double period) {
    jlibapogeeJNI.ApogeeCam_SetShutterStrobePeriod(swigCPtr, this, period);
  }

  public double GetShutterStrobePeriod() {
    return jlibapogeeJNI.ApogeeCam_GetShutterStrobePeriod(swigCPtr, this);
  }

  public void SetShutterCloseDelay(double delay) {
    jlibapogeeJNI.ApogeeCam_SetShutterCloseDelay(swigCPtr, this, delay);
  }

  public double GetShutterCloseDelay() {
    return jlibapogeeJNI.ApogeeCam_GetShutterCloseDelay(swigCPtr, this);
  }

  public void SetCoolerBackoffPoint(double point) {
    jlibapogeeJNI.ApogeeCam_SetCoolerBackoffPoint(swigCPtr, this, point);
  }

  public double GetCoolerBackoffPoint() {
    return jlibapogeeJNI.ApogeeCam_GetCoolerBackoffPoint(swigCPtr, this);
  }

  public void SetCoolerSetPoint(double point) {
    jlibapogeeJNI.ApogeeCam_SetCoolerSetPoint(swigCPtr, this, point);
  }

  public double GetCoolerSetPoint() {
    return jlibapogeeJNI.ApogeeCam_GetCoolerSetPoint(swigCPtr, this);
  }

  public CameraMode GetCameraMode() {
    return CameraMode.swigToEnum(jlibapogeeJNI.ApogeeCam_GetCameraMode(swigCPtr, this));
  }

  public void SetCameraMode(CameraMode mode) {
    jlibapogeeJNI.ApogeeCam_SetCameraMode(swigCPtr, this, mode.swigValue());
  }

  public void SetFastSequence(boolean TurnOn) {
    jlibapogeeJNI.ApogeeCam_SetFastSequence(swigCPtr, this, TurnOn);
  }

  public boolean IsFastSequenceOn() {
    return jlibapogeeJNI.ApogeeCam_IsFastSequenceOn(swigCPtr, this);
  }

  public void SetBulkDownload(boolean TurnOn) {
    jlibapogeeJNI.ApogeeCam_SetBulkDownload(swigCPtr, this, TurnOn);
  }

  public boolean IsBulkDownloadOn() {
    return jlibapogeeJNI.ApogeeCam_IsBulkDownloadOn(swigCPtr, this);
  }

  public void SetPipelineDownload(boolean TurnOn) {
    jlibapogeeJNI.ApogeeCam_SetPipelineDownload(swigCPtr, this, TurnOn);
  }

  public boolean IsPipelineDownloadOn() {
    return jlibapogeeJNI.ApogeeCam_IsPipelineDownloadOn(swigCPtr, this);
  }

  public void SetIoPortAssignment(int assignment) {
    jlibapogeeJNI.ApogeeCam_SetIoPortAssignment(swigCPtr, this, assignment);
  }

  public int GetIoPortAssignment() {
    return jlibapogeeJNI.ApogeeCam_GetIoPortAssignment(swigCPtr, this);
  }

  public void SetIoPortBlankingBits(int blankingBits) {
    jlibapogeeJNI.ApogeeCam_SetIoPortBlankingBits(swigCPtr, this, blankingBits);
  }

  public int GetIoPortBlankingBits() {
    return jlibapogeeJNI.ApogeeCam_GetIoPortBlankingBits(swigCPtr, this);
  }

  public void SetIoPortDirection(int direction) {
    jlibapogeeJNI.ApogeeCam_SetIoPortDirection(swigCPtr, this, direction);
  }

  public int GetIoPortDirection() {
    return jlibapogeeJNI.ApogeeCam_GetIoPortDirection(swigCPtr, this);
  }

  public void SetIoPortData(int data) {
    jlibapogeeJNI.ApogeeCam_SetIoPortData(swigCPtr, this, data);
  }

  public int GetIoPortData() {
    return jlibapogeeJNI.ApogeeCam_GetIoPortData(swigCPtr, this);
  }

  public void SetPreFlash(boolean TurnOn) {
    jlibapogeeJNI.ApogeeCam_SetPreFlash(swigCPtr, this, TurnOn);
  }

  public boolean GetPreFlash() {
    return jlibapogeeJNI.ApogeeCam_GetPreFlash(swigCPtr, this);
  }

  public void SetExternalTrigger(boolean TurnOn, TriggerMode trigMode, TriggerType trigType) {
    jlibapogeeJNI.ApogeeCam_SetExternalTrigger(swigCPtr, this, TurnOn, trigMode.swigValue(), trigType.swigValue());
  }

  public boolean IsTriggerNormEachOn() {
    return jlibapogeeJNI.ApogeeCam_IsTriggerNormEachOn(swigCPtr, this);
  }

  public boolean IsTriggerNormGroupOn() {
    return jlibapogeeJNI.ApogeeCam_IsTriggerNormGroupOn(swigCPtr, this);
  }

  public boolean IsTriggerTdiKinEachOn() {
    return jlibapogeeJNI.ApogeeCam_IsTriggerTdiKinEachOn(swigCPtr, this);
  }

  public boolean IsTriggerTdiKinGroupOn() {
    return jlibapogeeJNI.ApogeeCam_IsTriggerTdiKinGroupOn(swigCPtr, this);
  }

  public boolean IsTriggerExternalShutterOn() {
    return jlibapogeeJNI.ApogeeCam_IsTriggerExternalShutterOn(swigCPtr, this);
  }

  public boolean IsTriggerExternalReadoutOn() {
    return jlibapogeeJNI.ApogeeCam_IsTriggerExternalReadoutOn(swigCPtr, this);
  }

  public void SetShutterState(ShutterState state) {
    jlibapogeeJNI.ApogeeCam_SetShutterState(swigCPtr, this, state.swigValue());
  }

  public ShutterState GetShutterState() {
    return ShutterState.swigToEnum(jlibapogeeJNI.ApogeeCam_GetShutterState(swigCPtr, this));
  }

  public boolean IsShutterForcedOpen() {
    return jlibapogeeJNI.ApogeeCam_IsShutterForcedOpen(swigCPtr, this);
  }

  public boolean IsShutterForcedClosed() {
    return jlibapogeeJNI.ApogeeCam_IsShutterForcedClosed(swigCPtr, this);
  }

  public boolean IsShutterOpen() {
    return jlibapogeeJNI.ApogeeCam_IsShutterOpen(swigCPtr, this);
  }

  public void SetShutterAmpCtrl(boolean TurnOn) {
    jlibapogeeJNI.ApogeeCam_SetShutterAmpCtrl(swigCPtr, this, TurnOn);
  }

  public boolean IsShutterAmpCtrlOn() {
    return jlibapogeeJNI.ApogeeCam_IsShutterAmpCtrlOn(swigCPtr, this);
  }

  public void SetCooler(boolean TurnOn) {
    jlibapogeeJNI.ApogeeCam_SetCooler(swigCPtr, this, TurnOn);
  }

  public CoolerStatus GetCoolerStatus() {
    return CoolerStatus.swigToEnum(jlibapogeeJNI.ApogeeCam_GetCoolerStatus(swigCPtr, this));
  }

  public boolean IsCoolerOn() {
    return jlibapogeeJNI.ApogeeCam_IsCoolerOn(swigCPtr, this);
  }

  public double GetTempCcd() {
    return jlibapogeeJNI.ApogeeCam_GetTempCcd(swigCPtr, this);
  }

  public void SetCcdAdcResolution(Resolution res) {
    jlibapogeeJNI.ApogeeCam_SetCcdAdcResolution(swigCPtr, this, res.swigValue());
  }

  public Resolution GetCcdAdcResolution() {
    return Resolution.swigToEnum(jlibapogeeJNI.ApogeeCam_GetCcdAdcResolution(swigCPtr, this));
  }

  public void SetCcdAdcSpeed(AdcSpeed speed) {
    jlibapogeeJNI.ApogeeCam_SetCcdAdcSpeed(swigCPtr, this, speed.swigValue());
  }

  public AdcSpeed GetCcdAdcSpeed() {
    return AdcSpeed.swigToEnum(jlibapogeeJNI.ApogeeCam_GetCcdAdcSpeed(swigCPtr, this));
  }

  public int GetMaxBinCols() {
    return jlibapogeeJNI.ApogeeCam_GetMaxBinCols(swigCPtr, this);
  }

  public int GetMaxBinRows() {
    return jlibapogeeJNI.ApogeeCam_GetMaxBinRows(swigCPtr, this);
  }

  public int GetMaxImgCols() {
    return jlibapogeeJNI.ApogeeCam_GetMaxImgCols(swigCPtr, this);
  }

  public int GetMaxImgRows() {
    return jlibapogeeJNI.ApogeeCam_GetMaxImgRows(swigCPtr, this);
  }

  public int GetTotalRows() {
    return jlibapogeeJNI.ApogeeCam_GetTotalRows(swigCPtr, this);
  }

  public int GetTotalCols() {
    return jlibapogeeJNI.ApogeeCam_GetTotalCols(swigCPtr, this);
  }

  public int GetNumOverscanCols() {
    return jlibapogeeJNI.ApogeeCam_GetNumOverscanCols(swigCPtr, this);
  }

  public boolean IsInterline() {
    return jlibapogeeJNI.ApogeeCam_IsInterline(swigCPtr, this);
  }

  public PlatformType GetPlatformType() {
    return PlatformType.swigToEnum(jlibapogeeJNI.ApogeeCam_GetPlatformType(swigCPtr, this));
  }

  public void SetLedAState(LedState state) {
    jlibapogeeJNI.ApogeeCam_SetLedAState(swigCPtr, this, state.swigValue());
  }

  public LedState GetLedAState() {
    return LedState.swigToEnum(jlibapogeeJNI.ApogeeCam_GetLedAState(swigCPtr, this));
  }

  public void SetLedBState(LedState state) {
    jlibapogeeJNI.ApogeeCam_SetLedBState(swigCPtr, this, state.swigValue());
  }

  public LedState GetLedBState() {
    return LedState.swigToEnum(jlibapogeeJNI.ApogeeCam_GetLedBState(swigCPtr, this));
  }

  public void SetLedMode(LedMode mode) {
    jlibapogeeJNI.ApogeeCam_SetLedMode(swigCPtr, this, mode.swigValue());
  }

  public LedMode GetLedMode() {
    return LedMode.swigToEnum(jlibapogeeJNI.ApogeeCam_GetLedMode(swigCPtr, this));
  }

  public String GetInfo() {
    return jlibapogeeJNI.ApogeeCam_GetInfo(swigCPtr, this);
  }

  public String GetModel() {
    return jlibapogeeJNI.ApogeeCam_GetModel(swigCPtr, this);
  }

  public String GetSensor() {
    return jlibapogeeJNI.ApogeeCam_GetSensor(swigCPtr, this);
  }

  public void SetFlushCommands(boolean Disable) {
    jlibapogeeJNI.ApogeeCam_SetFlushCommands(swigCPtr, this, Disable);
  }

  public boolean AreFlushCmdsDisabled() {
    return jlibapogeeJNI.ApogeeCam_AreFlushCmdsDisabled(swigCPtr, this);
  }

  public void SetPostExposeFlushing(boolean Disable) {
    jlibapogeeJNI.ApogeeCam_SetPostExposeFlushing(swigCPtr, this, Disable);
  }

  public boolean IsPostExposeFlushingDisabled() {
    return jlibapogeeJNI.ApogeeCam_IsPostExposeFlushingDisabled(swigCPtr, this);
  }

  public double GetPixelWidth() {
    return jlibapogeeJNI.ApogeeCam_GetPixelWidth(swigCPtr, this);
  }

  public double GetPixelHeight() {
    return jlibapogeeJNI.ApogeeCam_GetPixelHeight(swigCPtr, this);
  }

  public double GetMinExposureTime() {
    return jlibapogeeJNI.ApogeeCam_GetMinExposureTime(swigCPtr, this);
  }

  public double GetMaxExposureTime() {
    return jlibapogeeJNI.ApogeeCam_GetMaxExposureTime(swigCPtr, this);
  }

  public boolean IsColor() {
    return jlibapogeeJNI.ApogeeCam_IsColor(swigCPtr, this);
  }

  public boolean IsCoolingSupported() {
    return jlibapogeeJNI.ApogeeCam_IsCoolingSupported(swigCPtr, this);
  }

  public boolean IsCoolingRegulated() {
    return jlibapogeeJNI.ApogeeCam_IsCoolingRegulated(swigCPtr, this);
  }

  public double GetInputVoltage() {
    return jlibapogeeJNI.ApogeeCam_GetInputVoltage(swigCPtr, this);
  }

  public InterfaceType GetInterfaceType() {
    return InterfaceType.swigToEnum(jlibapogeeJNI.ApogeeCam_GetInterfaceType(swigCPtr, this));
  }

  public void GetUsbVendorInfo(int[] VendorId, int[] ProductId, int[] DeviceId) {
    jlibapogeeJNI.ApogeeCam_GetUsbVendorInfo(swigCPtr, this, VendorId, ProductId, DeviceId);
  }

  public boolean IsCCD() {
    return jlibapogeeJNI.ApogeeCam_IsCCD(swigCPtr, this);
  }

  public void PauseTimer(boolean TurnOn) {
    jlibapogeeJNI.ApogeeCam_PauseTimer(swigCPtr, this, TurnOn);
  }

  public boolean IsSerialASupported() {
    return jlibapogeeJNI.ApogeeCam_IsSerialASupported(swigCPtr, this);
  }

  public boolean IsSerialBSupported() {
    return jlibapogeeJNI.ApogeeCam_IsSerialBSupported(swigCPtr, this);
  }

  public void SetFlushBinningRows(int bin) {
    jlibapogeeJNI.ApogeeCam_SetFlushBinningRows(swigCPtr, this, bin);
  }

  public int GetFlushBinningRows() {
    return jlibapogeeJNI.ApogeeCam_GetFlushBinningRows(swigCPtr, this);
  }

  public boolean IsOverscanDigitized() {
    return jlibapogeeJNI.ApogeeCam_IsOverscanDigitized(swigCPtr, this);
  }

  public void SetDigitizeOverscan(boolean TurnOn) {
    jlibapogeeJNI.ApogeeCam_SetDigitizeOverscan(swigCPtr, this, TurnOn);
  }

  public void SetAdcGain(int gain, int ad, int channel) {
    jlibapogeeJNI.ApogeeCam_SetAdcGain(swigCPtr, this, gain, ad, channel);
  }

  public int GetAdcGain(int ad, int channel) {
    return jlibapogeeJNI.ApogeeCam_GetAdcGain(swigCPtr, this, ad, channel);
  }

  public void SetAdcOffset(int offset, int ad, int channel) {
    jlibapogeeJNI.ApogeeCam_SetAdcOffset(swigCPtr, this, offset, ad, channel);
  }

  public int GetAdcOffset(int ad, int channel) {
    return jlibapogeeJNI.ApogeeCam_GetAdcOffset(swigCPtr, this, ad, channel);
  }

  public boolean IsInitialized() {
    return jlibapogeeJNI.ApogeeCam_IsInitialized(swigCPtr, this);
  }

  public boolean IsConnected() {
    return jlibapogeeJNI.ApogeeCam_IsConnected(swigCPtr, this);
  }

  public void SetAdSimMode(boolean TurnOn) {
    jlibapogeeJNI.ApogeeCam_SetAdSimMode(swigCPtr, this, TurnOn);
  }

  public boolean IsAdSimModeOn() {
    return jlibapogeeJNI.ApogeeCam_IsAdSimModeOn(swigCPtr, this);
  }

  public void SetLedBrightness(double PercentIntensity) {
    jlibapogeeJNI.ApogeeCam_SetLedBrightness(swigCPtr, this, PercentIntensity);
  }

  public double GetLedBrightness() {
    return jlibapogeeJNI.ApogeeCam_GetLedBrightness(swigCPtr, this);
  }

  public String GetDriverVersion() {
    return jlibapogeeJNI.ApogeeCam_GetDriverVersion(swigCPtr, this);
  }

  public String GetUsbFirmwareVersion() {
    return jlibapogeeJNI.ApogeeCam_GetUsbFirmwareVersion(swigCPtr, this);
  }

  public String GetSerialNumber() {
    return jlibapogeeJNI.ApogeeCam_GetSerialNumber(swigCPtr, this);
  }

  public StrDb ReadStrDatabase() {
    return new StrDb(jlibapogeeJNI.ApogeeCam_ReadStrDatabase(swigCPtr, this), true);
  }

  public void WriteStrDatabase(StrDb info) {
    jlibapogeeJNI.ApogeeCam_WriteStrDatabase(swigCPtr, this, StrDb.getCPtr(info), info);
  }

  public void OpenConnection(String ioType, String DeviceAddr, int FirmwareRev, int Id) {
    jlibapogeeJNI.ApogeeCam_OpenConnection(swigCPtr, this, ioType, DeviceAddr, FirmwareRev, Id);
  }

  public void CloseConnection() {
    jlibapogeeJNI.ApogeeCam_CloseConnection(swigCPtr, this);
  }

  public void Init() {
    jlibapogeeJNI.ApogeeCam_Init(swigCPtr, this);
  }

  public void StartExposure(double Duration, boolean IsLight) {
    jlibapogeeJNI.ApogeeCam_StartExposure(swigCPtr, this, Duration, IsLight);
  }

  public CameraStatusRegs GetStatus() {
    return new CameraStatusRegs(jlibapogeeJNI.ApogeeCam_GetStatus(swigCPtr, this), true);
  }

  public Status GetImagingStatus() {
    return Status.swigToEnum(jlibapogeeJNI.ApogeeCam_GetImagingStatus(swigCPtr, this));
  }

  public void GetImage(Uint16Vector out) {
    jlibapogeeJNI.ApogeeCam_GetImage(swigCPtr, this, Uint16Vector.getCPtr(out), out);
  }

  public void StopExposure(boolean Digitize) {
    jlibapogeeJNI.ApogeeCam_StopExposure(swigCPtr, this, Digitize);
  }

  public long GetAvailableMemory() {
    return jlibapogeeJNI.ApogeeCam_GetAvailableMemory(swigCPtr, this);
  }

  public int GetNumAds() {
    return jlibapogeeJNI.ApogeeCam_GetNumAds(swigCPtr, this);
  }

  public int GetNumAdChannels() {
    return jlibapogeeJNI.ApogeeCam_GetNumAdChannels(swigCPtr, this);
  }

  public double GetCoolerDrive() {
    return jlibapogeeJNI.ApogeeCam_GetCoolerDrive(swigCPtr, this);
  }

  public void SetFanMode(FanMode mode, boolean PreCondCheck) {
    jlibapogeeJNI.ApogeeCam_SetFanMode__SWIG_0(swigCPtr, this, mode.swigValue(), PreCondCheck);
  }

  public void SetFanMode(FanMode mode) {
    jlibapogeeJNI.ApogeeCam_SetFanMode__SWIG_1(swigCPtr, this, mode.swigValue());
  }

  public FanMode GetFanMode() {
    return FanMode.swigToEnum(jlibapogeeJNI.ApogeeCam_GetFanMode(swigCPtr, this));
  }

  public double GetTempHeatsink() {
    return jlibapogeeJNI.ApogeeCam_GetTempHeatsink(swigCPtr, this);
  }

  public void UpdateAlta(String FilenameCamCon, String FilenameBufCon, String FilenameFx2, String FilenameGpifCamCon, String FilenameGpifBufCon, String FilenameGpifFifo) {
    jlibapogeeJNI.ApogeeCam_UpdateAlta(swigCPtr, this, FilenameCamCon, FilenameBufCon, FilenameFx2, FilenameGpifCamCon, FilenameGpifBufCon, FilenameGpifFifo);
  }

  public void UpdateAscentOrAltaF(String FilenameFpga, String FilenameFx2, String FilenameDescriptor) {
    jlibapogeeJNI.ApogeeCam_UpdateAscentOrAltaF(swigCPtr, this, FilenameFpga, FilenameFx2, FilenameDescriptor);
  }

  public void UpdateAspen(String FilenameFpga, String FilenameFx2, String FilenameDescriptor, String FilenameWebPage, String FilenameWebServer, String FilenameWebCfg) {
    jlibapogeeJNI.ApogeeCam_UpdateAspen(swigCPtr, this, FilenameFpga, FilenameFx2, FilenameDescriptor, FilenameWebPage, FilenameWebServer, FilenameWebCfg);
  }

}
