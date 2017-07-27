'''
Copyright(c) 2010 Apogee Instruments, Inc. 
Created on Feb 24, 2010

@author: ltraynor
'''
import TabBase
from cStringIO import StringIO
import difflib
import datetime
import random
import traceback
import array
import re
import os
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi
import time
import pyApg as apg
import time

class ApexTestTab(TabBase.TabBase):
    '''
    Supports the setup testing tab widget
    '''


    def __init__(self,theUi,parent=None):
        '''
        Constructor
        '''
        super(ApexTestTab,self).__init__(theUi,parent)
        self.OnTestSize()
        self.OnTestType()
        
        self.SerialParityDict = { 0: apg.SerialParity_None,
                                1 : apg.SerialParity_Odd,
                                2 : apg.SerialParity_Even }
        
        self.SerialBaudRateDict = { 0: 1200,
                                   1 : 2400,
                                   2 : 4800,
                                   3 : 9600,
                                   4 : 19200,
                                   5 : 38400,
                                   5 : 57600,
                                   6: 115200 }
        
        self.SerialFlowCrtlDict = { 0 : apg.SerialFC_Off,
                                   1 : apg.SerialFC_On  }
        
        self.fwStatusDict = {  apg.ApogeeFilterWheel.NOT_CONNECTED : "NotConnected",
                            apg.ApogeeFilterWheel.READY : "Ready",
                             apg.ApogeeFilterWheel.ACTIVE : "Moving" }
        
    def PrintMsg(self, msg):
        self.ui.editIoTestStatus.setText( msg )
        self.Write2OutputWin( msg )
        
    def Update4Cam(self):
        self.OnTestType()
        self.ui.spinIoTestImgRows.setValue( self.camRef.GetMaxImgRows() )
        self.ui.spinIoTestImgCols.setValue( self.camRef.GetMaxImgCols() )
        
    def OnAscentFwConnect(self):
        if( self.ui.btnTestAscentFwConnect.isChecked() ):
            #mapping fan enums to fw combo box indices
            self.FwTypeDict = {  0 : apg.Ascent.CFW31_8R,
                                           1 : apg.Ascent.CFW25_6R }
            index = self.ui.cbTestAscentFwType.currentIndex()
            self.camRef.FilterWheelOpen( self.FwTypeDict[ index ] )
            
            self.ui.cbTestAscentFwType.setEnabled( False )
            self.ui.spinTestAscentFwPos.setEnabled( True )
            self.ui.btnTestAscentFwConnect.setText( unicode("Disconnect") )
            self.ui.spinTestAscentFwPos.setMaximum(
                                      self.camRef.GetFilterWheelMaxPositions() )  
            self.ui.spinTestAscentFwPos.setValue( 0 )       
            self.camRef.SetFilterWheelPos(
                                     self.ui.spinTestAscentFwPos.value() )
        else:
            self.camRef.FilterWheelClose()
            self.ui.cbTestAscentFwType.setEnabled( True )
            self.ui.spinTestAscentFwPos.setEnabled( False )
            self.ui.btnTestAscentFwConnect.setText( unicode("Connect") )

    def OnAscentFwPos(self):
        self.camRef.SetFilterWheelPos(
                                 self.ui.spinTestAscentFwPos.value() )
        
        # wait for the fw to complete moving
        fwStatus = self.camRef.GetFilterWheelStatus()
        while apg.ApogeeFilterWheel.READY != fwStatus:
            fwStatus = self.camRef.GetFilterWheelStatus()
            self.ui.editTestAscentFwStatus.setText( 
                                                    unicode( self.fwStatusDict[ fwStatus ]) )
            qtGui.QApplication.instance().processEvents()
        
    def OnTestType(self):
        if( self.ui.rbIoTestFifo.isChecked() ):
            self.ui.spinIoTestImgRows.setMaximum( 16384 )
            self.ui.spinIoTestImgCols.setMaximum( 16384 )
            self.ui.cBoxIoTestSpeed.setEnabled( True )
        elif( self.ui.rbIoTestAds.isChecked() ):
            if( True == self.camRef.IsInitialized() ):
                self.ui.spinIoTestImgRows.setMaximum( self.camRef.GetMaxImgRows() )
                self.ui.spinIoTestImgCols.setMaximum( self.camRef.GetMaxImgCols() )
                self.ui.cBoxIoTestSpeed.setEnabled( False )
        else:
                raise RuntimeError("invalid test type selected")
                    
    def OnTestSize(self):
        if( self.ui.rbIoTestFixed.isChecked() ):
            self.ui.lblIoTestImgRows.setText("Rows")
            self.ui.lblIoTestImgCols.setText("Columns")
        
        if( self.ui.rbIoTestRandom.isChecked() ):
            self.ui.lblIoTestImgRows.setText("Max Rows")
            self.ui.lblIoTestImgCols.setText("Max Columns")
                      
    def OnBrowse(self):
        dd = qtGui.QFileDialog()
        dd.setFileMode(qtGui.QFileDialog.Directory)
        dd.setOption(qtGui.QFileDialog.ShowDirsOnly)
        fname = unicode( dd.getExistingDirectory() )
        if fname:
            self.ui.editIoTestOutDir.setText( fname )
    
    def GetTestSpeed(self):
        speed = self.ui.cBoxIoTestSpeed.currentText()
        
        if( 0 == speed .compare( qtCore.QString(unicode("Fast") ) ) ):
            return 0x2000
        else:
            return 0x6000
    
    def GetImgSize(self):
        r = self.ui.spinIoTestImgRows.value()
        c = self.ui.spinIoTestImgCols.value()
            
        if( self.ui.rbIoTestFixed.isChecked() ):
            return (r,c)
        
        if( self.ui.rbIoTestRandom.isChecked() ):
            random.seed()
            return (random.randint(1, r), random.randint(1, c))
        
        #error if we get here
        raise RuntimeError("invalid test image size selected")
    
    def OnTest(self):
        if( True == self.ui.btnIoTestToggle.isChecked() ):
            if( self.ui.rbIoTestFifo.isChecked() ):
                self.OnFifoTest()
            elif( self.ui.rbIoTestAds.isChecked() ):
                self.OnAdsTest()
            else:
                raise RuntimeError("invalid test type selected")
                
    def OnFifoTest(self):
        speed = self.GetTestSpeed()
        numRuns = self.ui.spinIoTestNumRuns.value()
        fname =  unicode( self.ui.editIoTestOutDir.text() )
        
        self.ui.btnIoTestToggle.setText("Stop")
        
        try:
            self.PrintMsg( "FIFO Test: Starting a %d run fifo test on the %s interface." 
                           % (numRuns, self.GetInterfaceStr() ) )
            for i in range(0,numRuns):
                runStr = "Run %d" % (i + 1)
                qtGui.QApplication.instance().processEvents()
                
                if( False == self.ui.btnIoTestToggle.isChecked() ):
                    self.PrintMsg("FIFO Test: Stopping FIFO test") 
                    self.ui.btnIoTestToggle.setText("Start Test")
                    break;
                
                (r,c) = self.GetImgSize()
                 
                progress = "FIFO Test: %s: r=%d, c=%d, speed=%d" % (runStr, r, c, speed)
                self.PrintMsg( progress )
                actual = self.camRef.RunFifoTest(r, c, speed)
                self.PrintMsg("FIFO Test: %s: Received data from camera " % runStr)
                
                #get the speed information
                transferTime = self.camRef.GetTestingGetImgTime()
                transferRate = (r*c) / transferTime 
                self.PrintMsg("FIFO Test: %s: Download time = %.3f" %  (runStr, transferTime) )
                self.PrintMsg("FIFO Test: %s: Rate (Pixels/s):  %.3f" %  (runStr, transferRate) )
                    
                qtGui.QApplication.instance().processEvents()
                
               
                result = self.AreFifoResultsGood( actual )
               
                
                if( True == result[0] ):
                    self.PrintMsg("FIFO Test: %s: SUCCESSFULLY completed" % runStr)
                    #we are good go to the top and run again
                    continue
                    
                self.PrintMsg("FIFO Test: %s: Errors at %d" % (runStr, result[1]) )
       
                self.WriteFifoErrors( actual )
                self.PrintMsg("FIFO Test: Test stopped due to errors")
                #kill the test
                break
                                
            if( i == numRuns-1 ):
                    self.PrintMsg("FIFO Test: All tests completed successfully")
                                 
        except RuntimeError:
            self.PrintMsg(  traceback.format_exc() )
            outputDir = unicode( self.ui.editIoTestOutDir.text() ) 
            timeStamp = datetime.datetime.now().strftime("%Y-%m-%d-%H%M")        
            #save the output win to file
            self.PrintMsg("FIFO Test: Writting output to file")
            winFileName = os.path.join(outputDir,"%s-outputWin.txt" % timeStamp)
            self.SaveOutputWin2File( winFileName )
            self.PrintMsg("FIFO Test: Test stopped on exception")
        
        finally:
            self.ui.btnIoTestToggle.setText("Start")
            self.ui.btnIoTestToggle.setChecked(False)

    def AreFifoResultsGood(self,data):
        ll = len( data )
        val = data[0]
        for i in range(0,ll):
            if( val < 0 ):
                val = 65535
                
            if( val != data[i] ):
                return [False, i]
            
            val = val-1
                
        return [True,i]
        
    def WriteFifoErrors(self,actual):
        
        outputDir = unicode( self.ui.editIoTestOutDir.text() ) 
        timeStamp = datetime.datetime.now().strftime("%Y-%m-%d-%H%M")
        
        #save the output win to file
        self.PrintMsg("Writting output to file")
        winFileName = os.path.join(outputDir,"%s-outputWin.txt" % timeStamp)
        self.SaveOutputWin2File( winFileName )
        
        #write the file from the hardware to disk
        self.PrintMsg("Writing the received data to binary") 
        modelStr = self.camRef.GetModel()
        fname = os.path.join(outputDir,"%s-%s.bin" % (timeStamp, modelStr) )
        
        actualData = array.array('H')
        actualData.fromlist( list(actual) )

        fp = open(fname,'wb')
        actualData.tofile(fp)
        fp.close()
        
        del actualData
                        
        ll = len( actual )
        val = actual[0]
        diffList = []
        for i in range(0,ll):
            if( val < 0 ):
                val = 65535
                
            if( val != actual[i] ):
                diffList.append( (i,val,actual[i]) )
            
            val = val-1
            
        # create the difference strings
        fileErrors = StringIO()

        self.PrintMsg("Creating the actual and difference string") 
        fileErrors.write("Byte    \t Expected \t Actual\n")
        for i in range(0,len(diffList)):
            bb = diffList[i][0] * 2
            fileErrors.write("%08x \t %04x \t\t %04x\n" % ( bb, diffList[i][1], diffList[i][2] ) )
            

        self.PrintMsg("writing errors to file")
        fname = os.path.join(outputDir,"%s-data-errors.txt" % timeStamp)
        fp = open(fname,'w')
        fp.write( fileErrors.getvalue()  )
        fp.close()
        
    def GetInterfaceStr(self):
        info = self.camRef.GetInfo()
        mm = re.search("Interface: (.*?)\n", info)
        
        if mm:
            return mm.group(1)
        else:
            return "Unknown"
      
    def OnAdsTest(self):
        try:
            if( False == self.camRef.IsInitialized() ):
                raise RuntimeError("Cannot run ADS test on uninitialized camera")
        
            numRuns = self.ui.spinIoTestNumRuns.value()
            fname =  unicode( self.ui.editIoTestOutDir.text() )
            self.ui.btnIoTestToggle.setText("Stop")
        
            self.PrintMsg( "ADS Test: Starting a %d run ADS test on the %s interface." 
                           % (numRuns, self.GetInterfaceStr() ) )
             
            for i in range(0,numRuns):
                runStr = "Run %d" % (i + 1)
                qtGui.QApplication.instance().processEvents()
                
                if( False == self.ui.btnIoTestToggle.isChecked() ):
                    self.PrintMsg("ADS Test: Test manually terminated") 
                    self.ui.btnIoTestToggle.setText("Start Test")
                    break;
                
                #waiting for flush
                numWaits = 0
                while( self.camRef.GetImagingStatus() != apg.Status_Flushing):
                    if( numWaits > 10):
                        raise RuntimeError("Timed out waiting for camera to return to flushing state.")
                    else:
                        numWaits += 1
                        time.sleep( 0.5 )
                    
                (r,c) = self.GetImgSize()
                
                # 40x40 is the min image size
                if( r < 40 ):
                    r = 40
                    
                if( c < 40 ):
                    c = 40
                 
                progress = "ADS Test: %s: r=%d, c=%d" % (runStr, r, c)
                self.PrintMsg( progress )
                actual = self.camRef.RunAdsTest(r, c)
                self.PrintMsg("ADS Test: %s: Received data from camera " % runStr)
                
                #get the speed information
                transferTime = self.camRef.GetTestingGetImgTime()
                transferRate = (r*c) / transferTime 
                self.PrintMsg("ADS Test: %s: Download time = %.3f" %  (runStr, transferTime) )
                self.PrintMsg("ADS Test: %s: Rate (Pixels/s):  %.3f" %  (runStr, transferRate) )
                    
                qtGui.QApplication.instance().processEvents()
                
                val = 0
                count = 0
                results = []
                for item in actual:
                    if( item != val ):
                        bad = (count,val,item)
                        results.append(bad)
                    
                    if( val < c-1 ):
                        val += 1
                    else:
                        val = 0
                    
                    count += 1
                    
                #there are errors
                if( len(results) > 0 ):
                    self.PrintMsg("ADS Test: Error detected" )
                    self.PrintMsg("Position\tExpected\tActual" )
                    for item in results:
                        self.PrintMsg("%08x \t %04x \t %04x\n" %(item[0], item[1], item[2]) )
                        
                    outputDir = unicode( self.ui.editIoTestOutDir.text() ) 
                    timeStamp = datetime.datetime.now().strftime("%Y-%m-%d-%H%M")
        
                    #save the output win to file
                    self.PrintMsg("Writting output to file")
                    winFileName = os.path.join(outputDir,"%s-outputWin.txt" % timeStamp)
                    self.SaveOutputWin2File( winFileName )
                    
                    #write the file from the hardware to disk
                    self.PrintMsg("Writing the received data to binary") 
                    modelStr = self.camRef.GetModel()
                    fname = os.path.join(outputDir,"%s-%s.bin" % (timeStamp, modelStr) )
                    actualData = array.array('H')
                    actualData.fromlist( list(actual) )
            
                    fp = open(fname,'wb')
                    actualData.tofile(fp)
                    fp.close()
                    
                    del actualData
                    
                    #kill the test
                    break
                                
            if( i == numRuns-1 ):
                    self.PrintMsg("ADS Test: All tests completed successfully")
                                 
        except RuntimeError:
            self.PrintMsg(  traceback.format_exc() )
            outputDir = unicode( self.ui.editIoTestOutDir.text() ) 
            timeStamp = datetime.datetime.now().strftime("%Y-%m-%d-%H%M")        
            #save the output win to file
            self.PrintMsg("ADS Test: Writting output to file")
            winFileName = os.path.join(outputDir,"%s-outputWin.txt" % timeStamp)
            self.SaveOutputWin2File( winFileName )
            self.PrintMsg("ADS Test: Test stopped on exception")
        finally:
            
            self.ui.btnIoTestToggle.setText("Start")
            self.ui.btnIoTestToggle.setChecked(False)
            
    def OnRegRead(self):
        if( self.ui.rbTestRegIoCamCon.isChecked() ):
            value = self.camRef.ReadReg( self.GetReg() )
        elif( self.ui.rbTestRegIolBufCon.isChecked() ):
            value = self.camRef.ReadBufConReg( self.GetReg() )
        elif( self.ui.rbTestRegIoFx2.isChecked() ):
            value = self.camRef.ReadFx2Reg( self.GetReg() )
        else:
            raise RuntimeError( "Invalid Register Type" )
            
            
        self.SetRegValue( value )
    
    def OnRegWrite(self):
        value = self.GetRegValue()
        
        if( self.ui.rbTestRegIoCamCon.isChecked() ):
            self.camRef.WriteReg( self.GetReg(), value )
        elif( self.ui.rbTestRegIolBufCon.isChecked() ):
            self.camRef.WriteBufConReg( self.GetReg(), value )
        elif( self.ui.rbTestRegIoFx2.isChecked()  ):
            self.camRef.WriteFx2Reg( self.GetReg(), value )
        else:
            raise RuntimeError( "Invalid Register Type" )

        self.SetRegValue( 0 )
            
    def GetReg(self):
        txtStr = unicode( self.ui.editTestRegIoReg.text() )
        result = None
        if self.ui.rbTestRegIoHex.isChecked():
            result = self.HexStr2Int( txtStr )
        else:
            result = self.IntStr2Int( txtStr )
            
        return result
    
    def GetRegValue(self):
        txtStr = unicode( self.ui.editTestRegIoValue.text() )
        result = None
        if self.ui.rbTestRegIoHex.isChecked():
            result = self.HexStr2Int( txtStr )
        else:
            result = self.IntStr2Int( txtStr )
            
        return result
            
    def SetRegValue(self, value):
        if self.ui.rbTestRegIoHex.isChecked():
            ss = unicode( self.Int2HexStr( value) ) 
        else:
            ss = unicode( self.Int2Str(value) )
            
        self.ui.editTestRegIoValue.setText( ss )
    
    def Int2HexStr(self, value):
        result = "%x" % ( value )
        return result
    
    def HexStr2Int(self, hexStr):
        result = int( hexStr, 16)
        return result
    
    def Int2Str(self, value):
        result = "%d" % (value)
        return result
    
    def IntStr2Int(self, intStr):
        result = int( intStr )
        return result
    
    def OnSpeedTest(self):
        
        try:
            self.ui.btnTestSpeedStartStop.setText("Stop")
            #setup the camera for the test
            if( self.ui.rbTestSpeedRateNorm.isChecked() ):
                self.camRef.SetCcdAdcSpeed( apg.AdcSpeed_Normal )
            else:
                self.camRef.SetCcdAdcSpeed( apg.AdcSpeed_Fast )
             
            #just use the max rows and cols for testing
            self.camRef.SetRoiStartRow( 0 )
            self.camRef.SetRoiNumRows( self.camRef.GetMaxImgRows() )
        
            self.camRef.SetRoiStartCol( 0 )
            self.camRef.SetRoiNumCols( self.camRef.GetMaxImgCols() )
            
            isImgSeq = self.ui.cbTestSpeedImgSeq.isChecked()
            numImgs = self.ui.spinTestSpeedNumImgs.value()
                        
            if( isImgSeq ):
                self.camRef.SetImageCount( numImgs )
            else:
                self.camRef.SetImageCount( 1 )

            sumDownloadTime = 0
            exposeTime = self.ui.spinTestSpeedExposure.value()

            self.OnWriteSpeedStatus("Starting speed test")
            totalTestTimeStart = time.time()
            for i in range(0, numImgs):
                
                #break the loop on the user command
                qtGui.QApplication.instance().processEvents()
                if( False == self.ui.btnTestSpeedStartStop.isChecked() ):
                    self.OnWriteSpeedStatus("Test stopped by user")
                    break
                
                #issue the expose command
                if( False == isImgSeq or 0 == i ):
                    self.camRef.StartExposure( exposeTime,  True )
                
                #todo add error handling
                while(self.camRef.GetImagingStatus() != apg.Status_ImageReady):
                    pass
                
                t0 = time.time()
                theImg = self.camRef.GetImage()
                t1 = time.time()
                sumDownloadTime += (t1-t0)
                
            totalTestTimeStop = time.time()    
            
            avgTime = sumDownloadTime / numImgs
            self.OnWriteSpeedStatus( "avg download time = %f" %( avgTime ) )
            
            megaPixelPerSec = ( 
                                (self.camRef.GetMaxImgRows()*self.camRef.GetMaxImgCols() ) / avgTime ) / 1000000.0
            self.OnWriteSpeedStatus( "MPixel / Sec = %f" % (megaPixelPerSec) )
    
            fps = numImgs / (totalTestTimeStop-totalTestTimeStart)
    
            self.OnWriteSpeedStatus( "fps = %f" %( fps ) )
    
        finally:
            self.ui.btnTestSpeedStartStop.setText("Start")
            self.ui.btnTestSpeedStartStop.setChecked(False)
            
            
    def OnWriteSpeedStatus(self,text):
        #add newline if there isn't one
        if( -1 == text.find("\n") ):
            text += "\n"
            
        #make sure we at the end of the document
        #enum QTextCursor::End = 11
        self.ui.editTestSpeedResult.moveCursor(11)
        self.ui.editTestSpeedResult.insertPlainText( unicode(text) )
        
    def OnConnectSerialPort(self):
        newState = True
        if( self.ui.btnTestSerialConnect.isChecked() ):
            
            self.camRef.OpenSerial( self.GetPortNum() )
            
            self.SetBaudRate()
            self.UpdateBaudRate()
            
            self.SetFlowCtrl()
            self.UpdateFlowCtrl()
            
            self.SetParity()
            self.UpdateParity()
            
            newState = False
            self.ui.btnTestSerialConnect.setText( unicode("Disconnect") )
            
        else:
            self.camRef.CloseSerial( self.GetPortNum() )
            newState = True
            self.ui.btnTestSerialConnect.setText( unicode("Connect") )
            
        self.ui.cbTestSerialBaudRate.setEnabled( newState )
        self.ui.cbTestSerialFlow.setEnabled( newState )
        self.ui.cbTestSerialParity.setEnabled( newState )
        self.ui.rbTestSerialPortA.setEnabled( newState )
        self.ui.rbTestSerialPortB.setEnabled( newState )
            
    def GetPortNum(self):
        if( self.ui.rbTestSerialPortA.isChecked() ):
            return 0
        else:
            return 1
        
    def SetBaudRate(self):
        index = self.ui.cbTestSerialBaudRate.currentIndex()
        
        self.camRef.SetSerialBaudRate( self.GetPortNum(),
                                       self.SerialBaudRateDict [ index ] )
        
    def UpdateBaudRate(self):
        rate = self.camRef.GetSerialBaudRate( self.GetPortNum() )
        
        index = -1
        for k, v in self.SerialBaudRateDict.iteritems():
            if rate  == v:
                index = k
                
        if -1 == index:
            raise RuntimeError(" Invalid baud rate %d" % rate )  
              
        self.ui.cbTestSerialBaudRate.setCurrentIndex( index )
        
    def SetFlowCtrl(self):
        index = self.ui.cbTestSerialFlow.currentIndex()
        self.camRef.SetSerialFlowControl( self.GetPortNum(),
                                       self.SerialFlowCrtlDict[ index ] )
        
    def UpdateFlowCtrl(self):
        flow = self.camRef.GetSerialFlowControl( self.GetPortNum() )
        
        index = -1
        for k, v in self.SerialFlowCrtlDict.iteritems():
            if flow  == v:
                index = k
                
        if -1 == index:
            raise RuntimeError(" Invalid flow control %d" % flow )  
              
        self.ui.cbTestSerialFlow.setCurrentIndex( index )
                
    def SetParity(self):
        index = self.ui.cbTestSerialParity.currentIndex()
        self.camRef.SetSerialParity( self.GetPortNum(),
                                       self.SerialParityDict[ index ] )
        
    def UpdateParity(self):
        parity = self.camRef.GetSerialParity( self.GetPortNum() )
        
        index = -1
        for k, v in self.SerialParityDict.iteritems():
            if parity  == v:
                index = k
                
        if -1 == index:
            raise RuntimeError(" Invalid parity %d" % parity )  
              
        self.ui.cbTestSerialParity.setCurrentIndex( index )
        
    def OnSerialWrite(self):
        data = unicode( self.ui.editTestSerialWrite.text() ).encode()
        self.camRef.WriteSerial( self.GetPortNum(), data )
         
    def OnSerialRead(self):
        data = self.camRef.ReadSerial( self.GetPortNum() )
        self.ui.editTestSerialRead.setText( unicode(data) )
        
        
        
            
        
        
    
        
        
                
        
        