'''
Copyright(c) 2010 Apogee Instruments, Inc.
Created on Feb 11, 2010

@author: ltraynor
'''
import TabBase
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi
import pyApg as apg
import FindDlg
import os
import numpy as np

class ApexSetupTab(TabBase.TabBase):
    '''
    Supports the setup ui tab widget
    '''

    def __init__(self,theUi,parent=None):
        '''
        Constructor
        '''
        super(ApexSetupTab,self).__init__(theUi,parent)
        self.ui.cbCamCfg.addItems( self.GetIdQList() )
  
    def GetIdQList(self):
        '''
        Fetch the current list of support camera ids
        '''
        ids = apg.GetListOfIds()
        qStrList = qtCore.QStringList()
        qStrList.append(unicode("None"))
        for num in ids.flat:
            qStrList.append(unicode("%d" % num))
            
        return qStrList
            
    def OnFindBtn(self):
        '''
        Launches the camera find dialog
        '''
        find = FindDlg.FindDlg()
        if find.exec_() :
            #update the ui from the find dialog
            info = find.selectedDevice
            self.ui.editCamAddress.setText( unicode(info["address"]) )
            
            #setting cam id
            #convert hex string to int
            idInt = int(info["id"], 16)
            idStr = qtCore.QString( unicode(idInt) )
            index = self.ui.cbCamCfg.findText( idStr )
            
            if( -1 == index):
                #setting type to None if string not found
                index = 0
                
            self.ui.cbCamCfg.setCurrentIndex( index )
            
            #setting cam type
            typeStr = qtCore.QString( unicode(info["camType"]) )
            
            #checks for quad camera                                    
			#pmo id
            if( 412 == idInt ):
                typeStr = "Quad"
                
            index = self.ui.cbCamType.findText( typeStr )                            
            self.ui.cbCamType.setCurrentIndex( index )
            
            #setting interface
            if( "usb" == info["interface"]):
                self.ui.rbCamUsb.setChecked(True)
            else:
                self.ui.rbCamEthernet.setChecked(True)
                           
        
    def GetInterfaceType(self):
        if( self.ui.rbCamUsb.isChecked() ):
            return "usb"
        else:
            return "ethernet"
        
    def GetCamType(self):
        camGeneration = {0 : apg.ALTAU, 
                         1 : apg.ALTAE,
                         2 : apg.ASCENT, 
                         3 : apg.ALTAF,
                         4 : apg.ASPEN,
                         5 : apg.QUAD,
                         6 : apg.HIC }
        val = self.ui.cbCamType.currentIndex()
        return camGeneration[ val ]
        
        
    def GetCamTypeStr(self):
        return( unicode( self.ui.cbCamType.currentText() ).encode() )
    
    def GetCamInfo(self):
        
        result = (self.GetInterfaceType(), 
                  unicode( self.ui.editCamAddress.text() ).encode(),
                  self.GetCamType() )
        
        return result
    
    def OnScriptBrowse(self):
        scriptName = self.FileSelect("Python Files (*.py)")
        if None != scriptName:
            self.ui.cbSetupScript.insertItem( 0, scriptName )
            self.ui.cbSetupScript.setCurrentIndex( 0 )
            
    def OnMatrixBrowse(self):
        cgfName = self.FileSelect("Comma Delimited (*.csv)")
        if None != cgfName:
            self.ui.cbIniFile.addItem( cgfName  )   
            
    def OnSaveOutput(self):
        txtName = self.FileSave("Text File (*.txt)")
        
        if None != txtName:
          self.SaveOutputWin2File( txtName )
            
    def OnClearOutput(self):
        self.ui.editSetupOutput.clear()
        
    def OnGetInfo(self):
        self.ui.setupInfoTextWnd.clear()
        details = "Total rows = %d, cols = %d\n\
Imaging rows = %d, cols = %d\n\
Overscan cols = %d\n\
Is Interline = %d\n " % (self.camRef.GetTotalRows(), self.camRef.GetTotalCols(), self.camRef.GetMaxImgRows(), 
                       self.camRef.GetMaxImgCols(), self.camRef.GetNumOverscanCols(), self.camRef.IsInterline())
        msg = self.camRef.GetInfo() + details
        self.ui.setupInfoTextWnd.append( msg )
            
             
        