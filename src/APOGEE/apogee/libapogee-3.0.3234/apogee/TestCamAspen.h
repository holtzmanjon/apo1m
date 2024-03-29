/*! 
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright(c) 2010 Apogee Instruments, Inc. 
* \class TestCamAspen 
* \brief Test object for the Alta-G (Aspen) camera line to be used with the Apex application 
* 
*/ 


#ifndef TESTCAMASPEN_INCLUDE_H__ 
#define TESTCAMASPEN_INCLUDE_H__ 

#include "Aspen.h" 
#include <string>

class DLL_EXPORT TestCamAspen : public Aspen
{ 
    public: 
        TestCamAspen(const std::string & ioType,
             const std::string & DeviceAddr);

        virtual ~TestCamAspen(); 

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

        void ProgramAspen(const std::string & FilenameFpga,
            const std::string & FilenameFx2, const std::string & FilenameDescriptor,
            const std::string & FilenameWebPage, const std::string & FilenameWebServer,
            const std::string & FilenameWebCfg, bool Print2StdOut=false);

        uint8_t ReadBufConReg( uint16_t reg );
	    void WriteBufConReg( uint16_t reg, uint8_t val );

        uint8_t ReadFx2Reg( uint16_t reg );
        void WriteFx2Reg( uint16_t reg, uint8_t val );

        CamInfo::StrDb GetCamInfo();
        void SetCamInfo( CamInfo::StrDb & info );

        CamInfo::NetDb GetCamNetDb();
        void SetCamNetDb( CamInfo::NetDb & input );

        void ReadAndSaveFlash( uint32_t addr, uint32_t numBytes, const std::string & fileName );

    private:
         void UpdateCam();

        const std::string m_fileName;
        double m_GetImgTime;
}; 

#endif
