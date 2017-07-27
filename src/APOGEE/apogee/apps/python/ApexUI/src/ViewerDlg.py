'''
Created on Oct 21, 2010

@author: ltraynor
'''

import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import pyApg as apg
import numpy
            
class ViewerDlg(qtGui.QDialog):
    '''
    Simple dialog for searching for Apogee devices
    on the USB bus and on the local network
    '''
    def __init__(self,parent=None):
        '''
        Constructor
        '''
        super(ViewerDlg,self).__init__(parent)
        
        self.apgIcon = qtGui.QIcon("apogeeIcon.ico")
        self.setWindowIcon( self.apgIcon  )
        self.setWindowTitle("Viewer")
        self.lblViewer = qtGui.QLabel(self)
        #self.lblViewer.setMinimumHeight(256)
        #self.lblViewer.setMinimumWidth(256)
        
        self.lblMaxName = qtGui.QLabel( unicode("Max Value"), self )
        
        self.spinMaxValue = qtGui.QSpinBox(self)
        self.spinMaxValue.setRange(1,255)
        self.spinMaxValue.setValue(250)
        self.connect(self.spinMaxValue, 
                     qtCore.SIGNAL("valueChanged(int)"),
                     self.OnSpinMaxValue) 
                     
        layout = qtGui.QVBoxLayout()
        layout.addWidget(self.lblViewer)
        layout.addWidget(self.lblMaxName)
        layout.addWidget(self.spinMaxValue)
        
        self.setLayout(layout)
        
        self.UpdateColorMap(250)
        
    def closeEvent(self, event):
        print "on close event"
        
    def OnSpinMaxValue(self):
        self.UpdateColorMap( self.spinMaxValue.value() )
        
    def UpdateColorMap(self, value):
        self.colormapQt = []
        for i in range(0, 256):
            s = float(value) / 255.0
            v = s*i
            
            if( v > 255.0 ):
                v = 255.0
                print v
                
            self.colormapQt.append(qtGui.qRgb(int(v), int(v), int(v)))      
    
    def convert16To8Bit(self, npData):
        maxValue = numpy.max( npData )
        
        #div zero protection
        if( 0 == maxValue ):
            maxValue = 0.00001
            
        slope = 255.0 / maxValue
        result = slope*npData
        return result.astype(numpy.uint8)
    
    def DisplayImage(self, data, sizeTuple):
        self.npData = self.convert16To8Bit( data )
        image = qtGui.QImage( self.npData,
                             sizeTuple[0], 
                             sizeTuple[1], 
                             sizeTuple[0],
                             qtGui.QImage.Format_Indexed8)
        
        gray = image.isGrayscale()
        ct = image.colorTable()
        image.setColorTable( self.colormapQt )
        
        image = image.scaledToWidth( 512 )
        
        self.lblViewer.setPixmap(qtGui.QPixmap.fromImage(image))