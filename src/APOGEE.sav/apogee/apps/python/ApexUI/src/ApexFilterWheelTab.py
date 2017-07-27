'''
Created on Oct 27, 2011

@author: ltraynor
'''

import TabBase
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi
import os
import pyApg as apg


class ApexFilterWheelTab(TabBase.TabBase):
    '''
    Supports the programming ui tab widget
    '''


    def __init__(self,theUi,parent=None):
        '''
        Constructor
        '''
        super(ApexFilterWheelTab,self).__init__(theUi,parent)
        self.theWheel = None
                
    def Refresh(self):
        self.ui.editFwFirmwareDir.setText( self.lastDir.path() )
        
    def PrintMsg(self, msg):
        self.ui.editFwStatus.setText( msg )
        self.Write2OutputWin( msg )
        self.ui.editFwStatus.update()
        qtGui.QApplication.instance().processEvents()
        
    def OnConnect(self):
        if( self.ui.btnFwConnect.isChecked() ):
            self.theWheel = apg.TestFilterWheel()
            #just putting dummy type for now
            self.theWheel.Init( apg.ApogeeFilterWheel.FW50_7S, "0" )
            self.PrintMsg( "Filter wheel connected" )
            self.ui.btnFwConnect.setText( "Disconnect" )
            self.ui.btnFwProgram.setEnabled( True )
        else:
            self.ui.btnFwConnect.setText( "Connect" )
            self.ui.btnFwConnect.setChecked( False )
            self.ui.btnFwProgram.setEnabled( False )
            self.theWheel = None
            
    
    def OnBrowse(self):
        name = self.DirSelect()
        self.ui.editFwFirmwareDir.setText( unicode( name ) )
    
    def OnProgram(self):
        self.PrintMsg( "Verifying firmware files" )
        baseDir = unicode( self.ui.editFwFirmwareDir.text() ).encode()
        iicFile = os.path.join(baseDir, "filter_8051.iic")
        self.VerifyFile( iicFile )
        
        descrFile = os.path.join(baseDir, "filter_descriptors.bin")
        self.VerifyFile( descrFile )
        
        
        self.PrintMsg( "Programming starting" )
        self.theWheel.ProgramFx2( iicFile, descrFile )
        
        self.PrintMsg( "Programming completed successfully. Disconnecting filter wheel." )
        self.ui.btnFwConnect.setChecked( False )
        self.OnConnect()
        
    def VerifyFile(self, name):
        if( False == os.path.isfile(name) ):
            msg = "Invalid filter wheel firmware file %s" %s ( name )
            self.PrintMsg( msg )
            raise RuntimeError( msg )
                

       