'''
Created on Oct 21, 2010

@author: ltraynor
'''
'''
Created on Oct 19, 2010

@author: ltraynor
'''

import sys
import os
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import PyQt4.QtNetwork as qtNet
import numpy

PORT = 9407
     
class SocketThread(qtCore.QThread):
    def __init__(self, socketId, parent):
        super(SocketThread, self).__init__(parent)
        self.socketId = socketId
        
    def run(self):
        try:
            socket = qtNet.QTcpSocket()
            
            if not socket.setSocketDescriptor(self.socketId):
                self.emit(SIGNAL("error(int)"), socket.error())
                return
            
            nextBlockSize = 0
            stream = qtCore.QDataStream(socket)
            stream.setVersion(qtCore.QDataStream.Qt_4_5)      
    
            socket.waitForConnected()
            
            num = 0
            while True:
                socket.waitForReadyRead(-1)
                if nextBlockSize == 0:
                    if socket.bytesAvailable() >= 4:
                        nextBlockSize = stream.readUInt32()
                        break
                    
            if socket.bytesAvailable() < nextBlockSize:
                while True:
                    socket.waitForReadyRead(-1)
                    if socket.bytesAvailable() >= nextBlockSize:
                        break
                
            data = qtGui.QImage()
            stream >> data
            
            print "sending image"
            self.emit(qtCore.SIGNAL("output(QImage)"), data)
                            
        except:
            print "exception in thread %d" % (num, qtCore.QThread.currentThreadId())
     
class MyServer(qtNet.QTcpServer):

    def __init__(self, parent=None):
        super(MyServer, self).__init__(parent)
        self.Ui = parent

    def incomingConnection(self, socketId):
        thread = SocketThread(socketId, self)
        self.connect(thread, qtCore.SIGNAL("finished()"),
                     thread, qtCore.SLOT("deleteLater()"))
        
        self.connect( thread, qtCore.SIGNAL("output(QImage)"),
                     self.Ui.View )
        thread.start()
        
class ImgLabel(qtGui.QLabel):
    def __init__(self, parent=None):
        super(qtGui.QLabel, self).__init__(parent)
        
        
    def load(self,im):
        self.setPixmap(qtGui.QPixmap.fromImage(im))


class Form(qtGui.QDialog):
    def __init__(self, parent=None):
        super(Form,self).__init__(parent)
        
        self.img = ImgLabel()
               
        self.responseLabel = qtGui.QLabel()
         
        layout = qtGui.QVBoxLayout()
        layout.addWidget(self.img)
        layout.addWidget(self.responseLabel )

        self.setLayout(layout)
        
        self.server = MyServer(self)
        
        if not self.server.listen(qtNet.QHostAddress("0.0.0.0"), PORT):
            raise "failed to start server"
        
        
    def View(self, data):    
            print "loading image"   
            self.img.load(data)

       
            
#-----------------------------
# MAIN     
if __name__ == '__main__':
    app = qtGui.QApplication(sys.argv)
    f = Form()
    f.show()
    app.exec_()
    pass


