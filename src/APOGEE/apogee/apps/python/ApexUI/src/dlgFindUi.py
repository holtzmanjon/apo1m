# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'findDlg.ui'
#
# Created: Mon Jan 23 17:42:58 2012
#      by: PyQt4 UI code generator 4.8.5
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_dlgFind(object):
    def setupUi(self, dlgFind):
        dlgFind.setObjectName(_fromUtf8("dlgFind"))
        dlgFind.setEnabled(True)
        dlgFind.resize(709, 531)
        dlgFind.setWindowTitle(_fromUtf8("Find Apogee Devices"))
        self.layoutWidget = QtGui.QWidget(dlgFind)
        self.layoutWidget.setGeometry(QtCore.QRect(20, 130, 671, 381))
        self.layoutWidget.setObjectName(_fromUtf8("layoutWidget"))
        self.verticalLayout = QtGui.QVBoxLayout(self.layoutWidget)
        self.verticalLayout.setSpacing(25)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.btnFind = QtGui.QPushButton(self.layoutWidget)
        self.btnFind.setMaximumSize(QtCore.QSize(75, 25))
        self.btnFind.setText(QtGui.QApplication.translate("dlgFind", "Find", None, QtGui.QApplication.UnicodeUTF8))
        self.btnFind.setObjectName(_fromUtf8("btnFind"))
        self.verticalLayout.addWidget(self.btnFind)
        self.pBarFind = QtGui.QProgressBar(self.layoutWidget)
        self.pBarFind.setMaximum(10)
        self.pBarFind.setProperty("value", 0)
        self.pBarFind.setObjectName(_fromUtf8("pBarFind"))
        self.verticalLayout.addWidget(self.pBarFind)
        self.tblDevices = QtGui.QTableWidget(self.layoutWidget)
        self.tblDevices.setMinimumSize(QtCore.QSize(589, 0))
        self.tblDevices.setAutoFillBackground(False)
        self.tblDevices.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.tblDevices.setObjectName(_fromUtf8("tblDevices"))
        self.tblDevices.setColumnCount(4)
        self.tblDevices.setRowCount(0)
        item = QtGui.QTableWidgetItem()
        item.setText(QtGui.QApplication.translate("dlgFind", "Model", None, QtGui.QApplication.UnicodeUTF8))
        self.tblDevices.setHorizontalHeaderItem(0, item)
        item = QtGui.QTableWidgetItem()
        item.setText(QtGui.QApplication.translate("dlgFind", "Interface", None, QtGui.QApplication.UnicodeUTF8))
        self.tblDevices.setHorizontalHeaderItem(1, item)
        item = QtGui.QTableWidgetItem()
        item.setText(QtGui.QApplication.translate("dlgFind", "Address", None, QtGui.QApplication.UnicodeUTF8))
        self.tblDevices.setHorizontalHeaderItem(2, item)
        item = QtGui.QTableWidgetItem()
        item.setText(QtGui.QApplication.translate("dlgFind", "Status", None, QtGui.QApplication.UnicodeUTF8))
        self.tblDevices.setHorizontalHeaderItem(3, item)
        self.tblDevices.horizontalHeader().setCascadingSectionResizes(False)
        self.tblDevices.horizontalHeader().setDefaultSectionSize(100)
        self.tblDevices.horizontalHeader().setHighlightSections(False)
        self.tblDevices.horizontalHeader().setMinimumSectionSize(100)
        self.tblDevices.horizontalHeader().setStretchLastSection(True)
        self.tblDevices.verticalHeader().setDefaultSectionSize(30)
        self.tblDevices.verticalHeader().setMinimumSectionSize(22)
        self.verticalLayout.addWidget(self.tblDevices)
        self.btnSelect = QtGui.QPushButton(self.layoutWidget)
        self.btnSelect.setEnabled(False)
        self.btnSelect.setMinimumSize(QtCore.QSize(0, 25))
        self.btnSelect.setMaximumSize(QtCore.QSize(75, 25))
        self.btnSelect.setText(QtGui.QApplication.translate("dlgFind", "Select", None, QtGui.QApplication.UnicodeUTF8))
        self.btnSelect.setAutoDefault(True)
        self.btnSelect.setObjectName(_fromUtf8("btnSelect"))
        self.verticalLayout.addWidget(self.btnSelect)
        self.layoutWidget1 = QtGui.QWidget(dlgFind)
        self.layoutWidget1.setGeometry(QtCore.QRect(20, 20, 401, 101))
        self.layoutWidget1.setObjectName(_fromUtf8("layoutWidget1"))
        self.gridLayout = QtGui.QGridLayout(self.layoutWidget1)
        self.gridLayout.setMargin(0)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.cBoxUsb = QtGui.QCheckBox(self.layoutWidget1)
        self.cBoxUsb.setText(QtGui.QApplication.translate("dlgFind", "USB", None, QtGui.QApplication.UnicodeUTF8))
        self.cBoxUsb.setChecked(True)
        self.cBoxUsb.setObjectName(_fromUtf8("cBoxUsb"))
        self.gridLayout.addWidget(self.cBoxUsb, 0, 0, 1, 1)
        self.cBoxEthernet = QtGui.QCheckBox(self.layoutWidget1)
        self.cBoxEthernet.setText(QtGui.QApplication.translate("dlgFind", "Ethernet", None, QtGui.QApplication.UnicodeUTF8))
        self.cBoxEthernet.setObjectName(_fromUtf8("cBoxEthernet"))
        self.gridLayout.addWidget(self.cBoxEthernet, 1, 0, 1, 1)
        self.editEthernetSubnet = QtGui.QLineEdit(self.layoutWidget1)
        self.editEthernetSubnet.setObjectName(_fromUtf8("editEthernetSubnet"))
        self.gridLayout.addWidget(self.editEthernetSubnet, 1, 1, 1, 1)

        self.retranslateUi(dlgFind)
        QtCore.QObject.connect(self.btnSelect, QtCore.SIGNAL(_fromUtf8("clicked()")), dlgFind.accept)
        QtCore.QObject.connect(self.tblDevices, QtCore.SIGNAL(_fromUtf8("itemClicked(QTableWidgetItem*)")), dlgFind.OnDevicePickedInTable)
        QtCore.QMetaObject.connectSlotsByName(dlgFind)

    def retranslateUi(self, dlgFind):
        item = self.tblDevices.horizontalHeaderItem(0)
        item = self.tblDevices.horizontalHeaderItem(1)
        item = self.tblDevices.horizontalHeaderItem(2)
        item = self.tblDevices.horizontalHeaderItem(3)

