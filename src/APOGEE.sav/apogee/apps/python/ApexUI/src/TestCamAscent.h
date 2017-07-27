/*! 
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright(c) 2010 Apogee Instruments, Inc. 
* \class TestCamAscent 
* \brief Test object for the Ascent camera line to be used with the Apex application 
* 
*/ 


#ifndef TESTCAMASCENT_INCLUDE_H__ 
#define TESTCAMASCENT_INCLUDE_H__ 

#include "Ascent.h" 
#include "CameraInfo.h" 
#include <string>

class DLL_EXPORT TestCamAscent : public Ascent
{ 
    public: 
        TestCamAscent(const std::string & ioType,
            const std::string & DeviceAddr);

        virtual ~TestCamAscent(); 

        void CfgCamFromId( uint16_t CameraId );

        void CfgCamFromFile( const std::string & path,
            const std::string & cfgFileName );

        void CfgCamFromIni( const std::string & input );

        std::string GetFirmwareHdr();

        void SetSerialNumber(const std::string & num);

        std::vector<uint16_t>  RunFifoTest(const uint16_t rows,
            const uint16_t cols,  uint16_t speed);

        std::vector<uint16_t>  RunAdsTest(const uint16_t rows,
            const uint16_t cols);

        double GetTestingGetImgTime() { return m_GetImgTime; }

        void ProgramAscent(const std::string & FilenameFpga,
            const std::string & FilenameFx2, 
            const std::string & FilenameDescriptor,
            bool Print2StdOut=false);

        uint8_t ReadBufConReg( uint16_t reg );
        void WriteBufConReg( uint16_t reg, uint8_t val );

        uint8_t ReadFx2Reg( uint16_t reg );
        void WriteFx2Reg( uint16_t reg, uint8_t val );

        CamInfo::StrDb GetCamInfo();
        void SetCamInfo( CamInfo::StrDb & info );

    private:
        void UpdateCam();

        const std::string m_fileName;
        double m_GetImgTime;
}; 

#endif
