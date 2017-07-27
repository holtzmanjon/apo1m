'''
Created on Feb 26, 2010

@author: ltraynor
'''
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi

class TabBase(qtCore.QObject):
    '''
    Base class for the Apex Tab classes
    '''


    def __init__(self,theUi,parent=None):
        '''
        Constructor
        '''
        super(TabBase,self).__init__(parent)
        self.ui = theUi
        self.camRef = None
        self.lastDir = qtCore.QDir()
        
    def SetCamera(self,cam):
        self.camRef = cam
        
    def RemoveCamera(self):
        self.camRef = None
        
    def Write2OutputWin(self, msg):
        if( -1 == msg.find("\n") ):
            msg += "\n"
        self.ui.editSetupOutput.moveCursor(11)
        self.ui.editSetupOutput.insertPlainText( unicode(msg) )
        
    def FileSelect(self, typeStr):
        name = qtGui.QFileDialog.getOpenFileName(None,
                                                 "Select File", self.lastDir.path(), 
                                                 typeStr)
        
        if( None != name and 
            qtCore.QString( unicode("") ) != name):
            
            lastFile = qtCore.QFileInfo( name )
            self.lastDir.setPath( lastFile.path() ) 
            
        return name
    
    def DirSelect(self):
        name = qtGui.QFileDialog.getExistingDirectory(None,
                                                 "Select directory", self.lastDir.path() )
        if( None != name and 
            qtCore.QString( unicode("") ) != name):
            
            self.lastDir.setPath( name ) 
            
        return name
    
    def FileSave(self, typeStr):
        name = qtGui.QFileDialog.getSaveFileName(None,
                                                 "Save File", self.lastDir.path(), 
                                                 typeStr)
        if( None != name and 
            qtCore.QString( unicode("") ) != name):
            
            lastFile = qtCore.QFileInfo( name )
            self.lastDir.setPath( lastFile.path() ) 
            
        return name
    
    def SaveOutputWin2File(self, name):
        fp = open( unicode(name) ,'w')
        fp.write( unicode( self.ui.editSetupOutput.toPlainText() ).encode() )
        fp.close()
        
        