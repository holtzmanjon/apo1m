# Copyright (C) 2001-2003 Dirk Huenniger dhun (at) astro (dot) uni-bonn (dot) de
#	This program is free software; you can redistribute it and/or
#	modify it under the terms of the GNU General Public License as
#	published by the Free Software Foundation, version 2.

"""
An INDI GUI Library using GTK
=============================
	It provides functions to connect U{INDI <http://indi.sourceforge.net/>} objects with
	U{GTK <http://www.pygtk.org/>} objects. \n
	Each time an INDI object is received the connected GTK element is updated. Some widgets can be changed by
	the user. In this case an optional handler can be installed that will send the INDI object to the server each time
	the user changes widget.
	The main applications are:
		- development of general INDI GUIs constructing themselves  dynamically.
		- development of special INDI GUIs to be constructed with e. g.
			U{Glade <http://primates.ximian.com/~sandino/python-glade/>} 
	Supported platforms are:
		- Linux
		- Windows
		- any other platform supporting Python and GTK
	
	Connecting INDI objects to GTK widgets
	--------------------------------------
		Connecting in this sense means, that the widget will be updated with the data from its associated
		INDIobject each time it the INDI object is received. The way how it is updated depends on the type 
		of widget and the type of the INDIobject course. The philosophy used for this update is, to
		set as many parameters of the widget as possible. A nice example is the Spinbutton. The following
		parameters of the widget are changed according to the information contained in an L{indinumber}:
		
			- current value
			- minimum value
			- maximum value
			- step size
			- digits after the decimal dot
			
		The advantage of this approach is that you don't have to configure these things. The disadvantage
		is that you can not customise these parameters of the widget anymore.
		The methods to connect widgets are listed below:
		
			- L{gtkindiclient.add_ccdpreview}
			- L{gtkindiclient.add_comboboxentry}
			- L{gtkindiclient.add_entry}
			- L{gtkindiclient.add_indistream}
			- L{gtkindiclient.add_label}
			- L{gtkindiclient.add_range}
			- L{gtkindiclient.add_spinbutton}
			- L{gtkindiclient.add_statuslabel}
			- L{gtkindiclient.add_switchlabel}
			- L{gtkindiclient.add_togglebuttons}
			- L{gtkindiclient.add_vectorlabel}
		
		Example:
			
			>>> indi.add_spinbutton(dev,"CCDPREVIEW_CTRL","WIDTH",self.width)
		
		
	Bidirectional Connections
	-------------------------
		The C{add_*} functions mentioned in section I{Connecting INDI objects to GTK widgets} return
		a handler object. If the widget can be changed by the user, the handler object has got a 
		L{gui_indi_object_handler.set_bidirectional} method. If you call it, the handler will link to
		GTK such a way that it sends an L{indiobject} to the server each time the user changes the widget.
		
		Example:
		
			>>> indi.add_spinbutton(dev,"CCDPREVIEW_CTRL","WIDTH",self.width).set_bidirectional()
		
		This works fine as long as GTK provides a the C{button-release-event} C{key-release-event}
		signals for the widget in question. If not (which is currently only the case for L{gtkindiclient.add_comboboxentry})
		we use the C{changed} signal. But this can cause a very unpleasant loopback. We correct for this.
		The details of this correction are documented in L{indiclient.gui_indi_object_handler}
		
	Building a GUI manually
	-----------------------
	We strongly recommend to use Glade instead of this approach (see section I{Building a GUI in a visual way}).
	The example below demonstrates how to build a gui manually. \n 
	B{Important:} C{indiserver} B{must} running C{tutorial_two} in order
	to use this example):
		
		>>> from gtkindiclient import *
		>>> import gtk
		>>> try:
		>>>	def on_delete(*args):
		>>>		global running
		>>>		running=False
		>>> global running
		>>> running=True
		>>> indi=gtkindiclient("localhost",7624)
		>>> mainwindow = gtk.Window(gtk.WINDOW_TOPLEVEL)
		>>> mainwindow.connect("delete_event", on_delete)
		>>> mainwindow.show()
		>>> label=gtk.Label()
		>>> label.show()
		>>> indi.add_label("Telescope Simulator","EQUATORIAL_COORD","RA",label)
		>>> mainwindow.add(label)
		>>> while running:
		>>>	indi.process_events()
		>>>	while gtk.events_pending():
		>>>		gtk.main_iteration_do(False)
		>>>	time.sleep(0.01)
		>>> indi.quit()
	
	This results in a window showing a sexagesimal number: the right ascension.
	(see U{screenshot<http://pygtkindiclient.sourceforge.net/testgtk.jpg>})
		
	Building a GUI in a visual way
	------------------------------
	With U{Glade <http://glade.gnome.org/>} you can design the GUI of your
	application in a visual way. Glade produces a C{.glade} file.
	U{SimpleGladeApp <http://primates.ximian.com/~sandino/python-glade/SimpleGladeApp.html>} 
	generates the basic Python code for your GUI writing a C{.py} file, providing callbacks
	to be implemented by you. You can edit this C{.py} file writing your custom code. If you add a new
	callback in Glade you can run SimpleGladeApp and the new callback will be added to the
	C{.py} file and the changes you already made to the C{.py} file will be preserved.
	In these callbacks you can add the connections between INDIobjects and GTKwidgets.
	An example for these callbacks is given below. The complete example is given in the file C{testccd.py}
	B{Important:} The indiserver must be running C{tutorial_ccdpreview} in order to run this example.	
	
	We need to include some files:
	
		>>> # Put your modules and data here
		>>> import gtk
		>>> from SimpleGladeApp import SimpleGladeApp
		>>> from gtkindiclient import *
		>>> # From here through main() codegen inserts/updates a class for
		>>> # every top-level widget in the .glade file.
		
	We need to implement two callbacks
	
		>>> class Testccd(SimpleGladeApp):
		>>> def __init__(self, path="testccd.glade", root="testccd", domain=None, **kwargs):
		>>>	path = os.path.join(glade_dir, path)
		>>>	SimpleGladeApp.__init__(self, path, root, domain, **kwargs)
		>>>
		>>> def new(self):
		>>>	#context Testccd.new {
		>>>	indi.add_spinbutton(dev,"CCDPREVIEW_CTRL","WIDTH",self.width).set_bidirectional()
		>>>	indi.add_spinbutton(dev,"CCDPREVIEW_CTRL","HEIGHT",self.height).set_bidirectional()
		>>>	indi.add_spinbutton(dev,"CCDPREVIEW_CTRL","MAXGOODDATA",self.mgd).set_bidirectional()
		>>>	indi.add_spinbutton(dev,"CCDPREVIEW_CTRL","BYTEORDER",self.bo).set_bidirectional()
		>>>	indi.add_spinbutton(dev,"CCDPREVIEW_CTRL","BYTESPERPIXEL",self.bpp).set_bidirectional()
		>>>	indi.add_label(dev,"CCD_INFO","CCD_FWHM_PIXEL",self.fwhm)
		>>>	indi.add_label(dev,"CCD_INFO","CCD_PIXEL_SIZE",self.pixsize)
		>>>		indi.add_spinbutton(dev,"Telescope","Focus",self.focus).set_bidirectional()
		>>>	list=[self.startbutton,self.stopbutton]
		>>>	indi.add_togglebuttons(dev,"readout",list).set_bidirectional()		  	
		>>>	indi.add_ccdpreview(dev,self.drawingarea,self.brightness,self.contrast,self.gamma)	
		>>>	#context Testccd.new }
		>>>
		>>> def on_testccd_delete_event(self, widget, *args):
		>>>	#context Testccd.on_testccd_delete_event {
		>>>	global running
		>>>	running=False
		>>>	return True
		>>>	#context Testccd.on_testccd_delete_event }
	
	And the main method needs to be changed like this:
	
		>>> def main():
		>>>	global indi
		>>>	global dev
		>>>	global running
		>>>	try:
		>>>		dev="Device with Data Transfer"
		>>>		indi=gtkindiclient("localhost",7624)
		>>>		indi.enable_blob()
		>>>		testccd = Testccd()
		>>>		running=True
		>>>		while running:
		>>>			indi.process_events()
		>>>			while gtk.events_pending():
		>>>				gtk.main_iteration_do(False)
		>>>			time.sleep(0.01)
		>>>		indi.quit()
		>>>	except:
		>>>		a,b,c =sys.exc_info()
		>>>		sys.excepthook(  a,b,c	)
		>>>		indi.quit()

	And thats it. The GUI is ready to use. 
	(see U{screenshot<http://pygtkindiclient.sourceforge.net/testccd.jpg>})
	It is very important to keep the C{#context} statement as they are and
	to implement you code only between the respective C{#context} statements as SimpleGladeApp uses them to
	determine where your code started and ended, in order to avoid overwriting it. \n
	There are three important commands to call:
	
		- C{glade-2 testccd.glade} (opens the graphical editor)
		- C{python simple-glade-codegen.py  testccd.glade} (generate C{.py} file)
		- C{python testccd.py}
	
	Have a lot of fun
	
	
@author: Dirk Huenniger
@organization: "Hoher List" observatory Daun (Germany)
@license: GNU General Public License as published by the Free Software Foundation, version 2
@contact: dhun (at) astro (dot) uni-bonn (dot) de
@version: 0.13
@var _have_previewcrunch  : C{True} if the previewcrunch library has been installed, C{False} otherwise
@type _have_previewcrunch  : BooleanType 
"""
from indiclient import *
import string
import gtk
import sys
import array
import os
import math
import time
import math 
_have_previewcrunch=True

try:
	import previewcrunch
except:
	have_previewcrunch=False
		
class _ccdpreview( indi_custom_vector_handler):
	"""
	A bandwidth optimised online display for CCDs. \n
	A special python library must be installed in order to use this feature. \n
	@ivar area :  A GTK Drawingarea widget 
	@type area : Gtk.DrawingArea 
	@ivar maxgooddata : The maximum value which is still an accepted measurement within the
	limits of the CCD chip    
	@type maxgooddata : IntType
	@ivar previewwidth :  The width of the preview 
	@type previewwidth : IntType
	@ivar previewheight : The height of the preview  
	@type previewheight : IntType
	@ivar brightnesswidget : A GTK Range to adjust the brightness 
	@type brightnesswidget : GTK.Range 
	@ivar contrastwidget : A GTK Range to adjust the contrast
	@type contrastwidget : GTK.Range 
	@ivar gammawidget : A GTK Range to adjust gamma
	@type gammawidget : GTK.Range
	@ivar pbuf : a RGB buffer containing the current image 
	@type pbuf : array 
	@ivar greybuf : a buffer containing the current image in up to 32 bit greyscale  
	@type greybuf: array
	@ivar greypos : The writing position within greybuf  
	@type greypos: IntType
	@ivar byteorder : The Byteorder
	@type byteorder : IntType 
	@ivar bytesperpixel : The number of Bytes per pixel  
	@type bytesperpixel : IntType
	@ivar enabled : The preview will not be updated if set to C{False} 
	@type enabled : BooleanType
	"""
	def __init__(self,devicename,drawingareawidget
			,brightnesswidget,contrastwidget,gammawidget):
		"""
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param drawingareawidget:  A GTK Drawingarea widget
		@type drawingareawidget: Gtk.DrawingArea
		@param brightnesswidget:  A GTK Range to adjust the brightness 
		@type brightnesswidget: GTK.Range 
		@param contrastwidget:  A GTK Range to adjust the contrast 
		@type contrastwidget: GTK.Range 
		@param gammawidget:  A GTK Range to adjust gamma
		@type gammawidget: GTK.Range 
		"""
		indi_custom_vector_handler.__init__(self,devicename,"CCDPREVIEW_CTRL")
		self.type="CCDPreviewHandler"
		self.area=drawingareawidget
		self.maxgooddata=65000
		self.previewwidth=256
		self.previewheight=256
		self.brightnesswidget=brightnesswidget
		self.contrastwidget=contrastwidget
		self.gammawidget=gammawidget
		scrollbars=[brightnesswidget,contrastwidget,gammawidget]
		values=[0.3,0.1,0.33]
		for i in range(3):			
			scrollbars[i].set_adjustment(gtk.Adjustment(values[i], 0, 1.2, 0.01, 0.2, 0.1))
			scrollbars[i].connect("value_changed", self._scrollbar_changed )
			scrollbars[i].set_size_request(100,-1)
			scrollbars[i].set_update_policy(gtk.UPDATE_DELAYED)
		self.area.connect("expose-event", self._area_expose_cb)
		self.pbuf=array.array("c",(self.previewwidth*self.previewheight*3)*'\0')
		self.greybuf=array.array("c",self.previewwidth*self.previewheight*4*'\0')
		self.greypos=0
		self.byteorder=1;
		self.bytesperpixel=2;
		self.update()
		self.enabled=True
		
	def _scrollbar_changed(self,widget) :
		self.update()
		
	def _get_normed_value(self,widget):
		"""
		@param widget:  A GTK.Range object (like a scrollbar or a slider)
		@type widget: GTK.Range (or inherited)
		@return: The value of the GTK.Range object, scaled to be in the range from 0  to 1
		@rtype: FloatType
		"""		
		adjust=widget.get_adjustment()
		return (widget.get_value()-adjust.lower)/(1.0*adjust.upper-adjust.lower)
		
	def update(self):
		"""
		Update brightness contrast gamma, given by the GTK.Range widgets
		and queue the image for redrawing 
		@return: B{None}
		@rtype: NoneType
		"""
		b=self._get_normed_value(self.brightnesswidget);
		c=self._get_normed_value(self.contrastwidget);
		gamma=3.0*self._get_normed_value(self.gammawidget);
		if _have_previewcrunch:
			buf=previewcrunch.downconvert(self.greybuf.tostring(),
				b,c,gamma,self.bytesperpixel,self.maxgooddata)
			if (self.byteorder==2):
				buf=previewcrunch.reorder(buf)
			self.pbuf=array.array("c",buf)
		self.area.queue_draw()
		
	def _area_expose_cb( self,area, event):
		"""A callback method called by GTK in order to ask the image to redraw itself \n
		@param area: The GTK.DrawingArea to be redrawn.
		@type area: GTK.DrawingArea
		@param event: A GTK.Event (not used, but required by GTK).
		@type event: GTK.Event
		@return: C{True}
		@rtype: BooleanType
		"""
		style = area.get_style()
		gc = style.fg_gc[gtk.STATE_NORMAL]
		buff=self.pbuf.tostring()
		area.window.draw_rgb_image(gc, 1, 1, self.previewwidth, self.previewheight,
			gtk.gdk.RGB_DITHER_NONE, buff, self.previewwidth*3)
		return True
	
	def on_indiobject_changed(self,vector):
		"""called when the INumberVector containing the parameters of the preview has been received
		for the server \n
		@param vector: INumberVector containing the parameters of the preview.
		@type vector: L{indivector}
		@return: B{None}
		@rtype: NoneType
		"""
		for element in vector.elements:
			if element.name=="WIDTH":
				self.previewwidth=element.get_int()
			if element.name=="HEIGHT":
				self.previewheight=element.get_int()
			if element.name=="MAXGOODDATA":
				self.maxgooddata=element.get_int()
			if element.name=="BYTEORDER":
				self.byteorder=element.get_int()
			if element.name=="BYTESPERPIXEL":
				self.bytesperpixel=element.get_int()
		self.pbuf=array.array("c",(self.previewwidth*self.previewheight*3)*'\0')
		self.greybuf=array.array("c",self.previewwidth*self.previewheight*self.bytesperpixel*'\0')
		self.greypos=0
		self.enabled=True
		self.area.set_size_request(self.previewwidth,self.previewheight)
			
	def reset(self):
		"""
		Clear all buffers, and enable ccdpreview
		@return: B{None}
		@rtype: NoneType
		"""
		self.pbuf=array.array("c",(self.previewwidth*self.previewheight*3)*'\0')
		self.greybuf=array.array("c",self.previewwidth*self.previewheight*self.bytesperpixel*'\0')
		self.greypos=0
		self.enabled==True
		
	def disable(self):
		""" 
		Disables data reception
		@return: B{None}
		@rtype: NoneType
		"""
		self.enabled=False
		
	def blob_received(self,blob):
		"""called when an L{indiblob} with format C{.ccdpreview} has been received from the server. \n
		It appends the data to the buffer and queues the image for redrawing 
		@param blob: the L{indiblob} containing the data for the preview.
		@type blob: L{indiblob}
		@return: B{None}
		@rtype: NoneType
		"""
		if blob.get_size()==0:
			return
		if self.enabled==False:
			return
		b=self._get_normed_value(self.brightnesswidget);
		c=self._get_normed_value(self.contrastwidget);
		gamma=3.0*self._get_normed_value(self.gammawidget);
		data=blob.get_data()
		for i in range(len(data)):
			self.greybuf[self.greypos+i]=data[i]
		if _have_previewcrunch:
			converted=previewcrunch.downconvert(data,b,c,
				gamma,self.bytesperpixel,self.maxgooddata)
			if (self.byteorder==1):
				for i in range(len(data)/self.bytesperpixel):
					self.pbuf[3*(self.greypos/self.bytesperpixel)+3*i]=converted[3*i]
					self.pbuf[3*(self.greypos/self.bytesperpixel)+3*i+1]=converted[3*i+1]
					self.pbuf[3*(self.greypos/self.bytesperpixel)+3*i+2]=converted[3*i+2]
			if (self.byteorder==2):
				for i in range(len(data)/self.bytesperpixel):
					if (i%2==0) :
						offs=i/2;
					if ((i%2)<1):
						pos=3*((self.greypos/(2*self.bytesperpixel))+i-offs);
						self.pbuf[pos]=converted[3*i]
						self.pbuf[pos+1]=converted[3*i+1]
						self.pbuf[pos+2]=converted[3*i+2]
					else:
						pos=3*(self.previewwidth*self.previewheight-(i+self.greypos/(2*self.bytesperpixel)-offs)-1)
						self.pbuf[pos]=converted[3*i]
						self.pbuf[pos+1]=converted[3*i+1]
						self.pbuf[pos+2]=converted[3*i+2]
		self.greypos=self.greypos+len(data)
		self.area.queue_draw()
		

class _indistream( indi_custom_vector_handler):
	"""
	A general purpose preview. \n
	@ivar  devicename : The name of the device 
	@type  devicename:  StringType
	@ivar area :  A GTK Drawingarea widget 
	@type  area :  Gtk.DrawingArea 
	@ivar  enabled : The preview will not be updated if set to C{False} 
	@type  enabled:  BooleanType
	@ivar  pbuf : a 24 bit RGB  8 bit greyscale buffer containing the current image  
	@type  pbuf :  array 
	"""
	def __init__(self,devicename,drawingareawidget):
		"""
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param drawingareawidget:  A GTK Drawingarea widget
		@type drawingareawidget: Gtk.DrawingArea
		"""
		indi_custom_vector_handler.__init__(self,devicename,"IMAGE_SIZE")
		self.area=drawingareawidget
		self.previewwidth=256
		self.previewheight=256
		self.area.connect("expose-event", self._area_expose_cb)
		self.pbuf=array.array("c",(self.previewwidth*self.previewheight*4)*'\0')
		self.enabled=True
		self.area.queue_draw()
			
	def _area_expose_cb( self,area, event):
		"""A callback method called by GTK in order to ask the image to redraw itself \n
		@param area: The GTK.DrawingArea to be redrawn.
		@type area: GTK.DrawingArea
		@param event: A GTK.Event (not used, but required by GTK).
		@type event: GTK.Event
		@return: C{True}
		@rtype: BooleanType
		"""
		style = area.get_style()
		gc = style.fg_gc[gtk.STATE_NORMAL]
		buff=self.pbuf
		if len(buff)==self.previewheight*self.previewwidth:
			area.window.draw_gray_image(gc,1,1,self.previewwidth, self.previewheight,
				gtk.gdk.RGB_DITHER_NONE, buff, self.previewwidth)
		else:
			area.window.draw_rgb_32_image(gc, 1, 1, self.previewwidth, self.previewheight,
				gtk.gdk.RGB_DITHER_NONE, buff, self.previewwidth*4)
		return True
		
	def on_indiobject_changed(self,vector):
		"""called when the L{indinumbervector} containing the parameters of the preview has been received
		from the server \n
		@param vector: L{indinumbervector} containing the parameters of the preview.
		@type vector: L{indinumbervector}
		@return: B{None}
		@rtype: NoneType
		"""
		for element in vector.elements:
			if element.name=="WIDTH":
				self.previewwidth=element.get_int()
			if element.name=="HEIGHT":
				self.previewheight=element.get_int()
		self.pbuf=array.array("c",(self.previewwidth*self.previewheight*4)*'\0')
		self.enabled=True
		self.area.set_size_request(self.previewwidth,self.previewheight)


	def reset(self):
		"""
		Clear buffer, and enable stream
		@return: B{None}
		@rtype: NoneType
		"""
		self.pbuf=array.array("c",(self.previewwidth*self.previewheight*4)*'\0')
		self.enabled==True
		
	def disable(self):
		""" 
		Disables data reception
		@return: B{None}
		@rtype: NoneType
		"""
		self.enabled=False
		
	def blob_received(self,blob):
		"""called when an L{indiblob} with format C{.stream} has been received from the server. \n
		It copies its contends into L{pbuf} and queues the image for redrawing. \n
		@param blob: BLOB containing the data for the preview.
		@type blob: L{indiblob}
		@return: B{None}
		@rtype: NoneType
		"""
		if blob.get_size()==0:
			return
		if self.enabled==False:
			return
		self.pbuf=blob.get_data()
		self.area.queue_draw()

		
class _gtkindi_widgetlist_handler(gui_indi_object_handler):
	"""
	A base class for handlers connecting an INDI object with a list of GTK widgets
	@ivar widgetlist : The list GTK widgets to be connected 
	@type widgetlist : list of GTK.Widget
	"""
	def __init__(self,widgetlist):
		"""
		@param widgetlist: The list containing the GTK.widget objects to be handled
		@type widgetlist: list of Gtk.Widget 
		"""
		gui_indi_object_handler.__init__(self)
		self._gtk_handler_idlist=None
		self.widgetlist=widgetlist
		handler=None
		self._gtk_signals=["button-release-event","key-release-event"]
	def set_bidirectional(self):
		if self._gtk_handler_idlist==None:
			self._gtk_handler_idlist=[]
			for widget in self.widgetlist:
				hids=[]
				for signal in self._gtk_signals:
					#try:
					hids.append(
						widget.connect(signal,self._blocking_on_gui_changed))
					#except:
					#	print signal,self.vectorname
				self._gtk_handler_idlist.append(hids)
	def unset_bidirectional(self):
		if self._gtk_handler_idlist!=None:
			for i,widget in enumerate(self.widgetlist):
					hids=self._gtk_handler_idlist[i]
					for hig in hids:
						widget.handler_disconnect(hid)
			self._gtk_handler_idlist=None

class _gtkindi_widget_handler(_gtkindi_widgetlist_handler):
	"""
	A base class for handlers connecting an INDI object with a single GTK widget
	@ivar widget : The GTK widget
	@type widget : GTK.Widget
	"""
	def __init__(self,widget):
		"""
		@param widget: The GTK widget to be connected
		@type widget: Gtk.Widget 
		"""
		widgetlist=[]
		widgetlist.append(widget)
		self.widget=widget
		_gtkindi_widgetlist_handler.__init__(self,widgetlist)

class _gtkindi_widget_element_handler(_gtkindi_widget_handler,indi_vector_identifier,indi_element_identifier):
	"""
	A base class for handlers being called on reception of an INDI element which update a GTK widget
	@ivar widget : The GTK widget
	@type widget : GTK.Widget
	"""
	def __init__(self,devicename,vectorname,elementname,widget):
		"""
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param elementname: name of the INDI element
		@type elementname: StringType
		@param widget: The GTK widget to be connected
		@type widget: Gtk.Widget 
		"""
		_gtkindi_widget_handler.__init__(self,widget)
		indi_element_identifier.__init__(self,devicename,vectorname,elementname)
	def on_gui_changed(self,widget,event=None):
		vector=self.get_vector()
		element=self.get_element()
		self.update_indi_element(element)
		self.indi.send_vector(vector)
	def update_indi_element(self,element):
		"""
		Called when the widget has changed and the INDI element needs to be updated
		@param element : the L{indielement} to update
		@type element : L{indielement}
		@return: B{None}
		@rtype: NoneType
		"""
		None
	def get_vector(self):
		"""
		Returns the L{indivector} this handler is associated with.
		@return: the L{indivector} this handler is associated with
		@rtype: L{indivector}	
		"""
		return self.indi.get_vector(self.devicename,self.vectorname)
		
	def get_element(self):
		"""
		Returns the L{indielement} this handler is associated with.
		@return: the L{indielement} this handler is associated with
		@rtype: L{indielement}
		"""
		return self.indi.get_vector(self.devicename,self.vectorname).get_element(self.elementname)	

class _gtkindi_widget_vector_handler(_gtkindi_widget_handler,indi_vector_identifier):
	"""
	A base class for handlers dealing with and INDI vector and a single GTK widget
	@ivar widget : The GTK widget
	@type widget : GTK.Widget
	"""
	def __init__(self,devicename,vectorname,widget):
		"""
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param widget: The GTK widget to be connected
		@type widget: Gtk.Widget 
		"""
		_gtkindi_widget_handler.__init__(self,widget)
		indi_vector_identifier.__init__(self,devicename,vectorname)
		self.type="widget_vector_handler"
		self.widget=widget
		
	def on_gui_changed(self,widget,event=None):
		vector=self.get_vector()
		self.update_indi_vector(vector)
		self.indi.send_vector(vector)
	def update_indi_vector(self,vector):
		"""
		Called when the widget has changed and the INDI vector needs to be updated
		@param vector : the L{indivector} to update
		@type vector : L{indivector}
		@return: B{None}
		@rtype: NoneType
		"""
		None
	
	def get_vector(self):
		"""
		Returns the L{indivector} this handler is associated with.
		@return: the L{indivector} this handler is associated with
		@rtype: L{indivector}	
		"""
		return self.indi.get_vector(self.devicename,self.vectorname)

	
	
class _gtkindi_widgetlist_vector_handler(_gtkindi_widgetlist_handler,indi_vector_identifier):
	"""
	A base class for handlers dealing with and INDI vector and a single GTK widget
	"""
	def __init__(self,devicename,vectorname,widgetlist):
		"""
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param widgetlist: The a list containing the GTK.widget objects to be handled
		@type widgetlist: a list of Gtk.Widget 
		"""
		indi_vector_identifier.__init__(self,devicename,vectorname)
		_gtkindi_widgetlist_handler.__init__(self,widgetlist)
	def on_gui_changed(self,widget,event=None):
		vector=self.get_vector()
		self.update_indi_vector(vector)
		self.indi.send_vector(vector)
	def update_indi_vector(self,vector):
		"""
		Called when the widget has changed and the INDI vector needs to be updated
		@param vector : the L{indivector} to update
		@type vector : L{indivector}
		@return: B{None}
		@rtype: NoneType
		"""
		None
	
	def get_vector(self):
		"""
		Returns the L{indivector} this handler is associated with.
		@return: the L{indivector} this handler is associated with
		@rtype: L{indivector}	
		"""
		return self.indi.get_vector(self.devicename,self.vectorname)


class _labelhandler(_gtkindi_widget_element_handler):
	"""A handler for an L{indielement} object connected to GTK.label object"""
	def on_indiobject_changed(self,vector,element):		
		label="<b>"+element.get_text()+"</b>"
		try:
			self.widget.set_markup(label)
		except:
			return
		
class _statuslabelhandler(_gtkindi_widget_vector_handler):
	"""A handler for an L{indivector} who's L{indilight} property is connected to a GTK.Label"""
	def on_indiobject_changed(self,vector):
		light=vector.get_light()
		if light.is_idle():
			label="<b><span color=\"black\">Idle</span></b>"
		if light.is_alert():
			label="<b><span color=\"red\">Alert</span></b>"
		if light.is_busy():
			label="<b><span color=\"dark orange\">Busy</span></b>"
		if light.is_ok():
			label="<b><span color=\"dark green\">Ok</span></b>"
		self.widget.set_markup(label)
	
class _switchlabelhandler(_gtkindi_widget_element_handler):
	"""A handler for an L{indiswitch} object connected to a GTK.Label object, replacing C{On}  and C{Off} by  a customisable label string
	@ivar onlabel:  The Text to be displayed on the Label if the L{indiswitch} is C{On}, Pango markup is allowed
	@type onlabel : StringType
	@ivar offlabel : The Text to be displayed on the Label if the L{indiswitch} is C{Off}, Pango markup is allowed
	@type offlabel : StringType
	"""
	def __init__(self,devicename,vectorname,elementname,widget,onlabel,offlabel):
		"""
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param elementname: name of the INDI element
		@type elementname: StringType
		@param widget: The Label
		@type widget: Gtk.Label 
		@param onlabel: The Text to be displayed on the Label if the L{indiswitch} is C{On}, Pango markup is allowed
		@type onlabel: StringType
		@param offlabel: The Text to be displayed on the Label if the L{indiswitch} is C{Off}, Pango markup is allowed
		@type offlabel: StringType
		"""
		_gtkindi_widget_element_handler.__init__(self,devicename,vectorname,elementname,widget)
		self.widget=widget
		self.onlabel=onlabel
		self.offlabel=offlabel
		
	def on_indiobject_changed(self,vector,element):
		if element.get_active():
			label=self.onlabel
		else:
			label=self.offlabel
		self.widget.set_markup(label)


class _rangehandler(_gtkindi_widget_element_handler):
	"""A handler for an L{indinumber} objects connected to a GTK.Range object """
	def configure(self,vector,element):
		vector=self.get_vector()
		min=element.get_min()
		max=element.get_max()
		step=element.get_step()
		if step<=0:
			step=(max-min)/200
		steps=(max-min)/step
		if steps<=20:
			pagestep=1
		else:
			pagestep=step*int(steps/10)
		step=element.get_step()
		# this condition fullfills (pagestep%step ==0)
		# and (pagestep>=step) and (pagestep<steps)
		self.widget.set_increments(step,pagestep)
		self.widget.set_range(min,max)
		self.widget.set_value(element.get_float())
	
	def on_indiobject_changed(self,vector,element):
		if self.widget.get_value()!=element.get_float():
			self.widget.set_value(element.get_float())
			
	def update_indi_element(self,element):
		element.set_float(self.widget.get_value())
	

class _spinbuttonhandler(_rangehandler):
	"""A handler for an L{indinumber} objects connected to a GTK.Spinbutton object """
	def configure(self,vector,element):
		_rangehandler.configure(self,vector,element)
		self.widget.set_digits(element.get_digits_after_point())
		#in America we use "42.23" but in Germany we use "42,23" which we need to correct for this.
		#we need to correct for that avoiding to change our original widget
		self.valuechecker=gtk.SpinButton()
		self.valuechecker.set_adjustment(self.widget.get_adjustment())
		if element.get_step()>0:
			self.valuechecker.set_snap_to_ticks(True)
		else:
			self.valuechecker.set_snap_to_ticks(False)
		self.valuechecker.set_numeric(False)
		self.valuechecker.set_value(element.get_float())
	def on_indiobject_changed(self,vector,element):
		if self.widget.get_value()!=element.get_float():
			self.widget.set_value(element.get_float())
			self.valuechecker.set_value(element.get_float())

	def update_indi_element(self,element):
		#in America we use "42.23" but in Germany we use "42,23" which we need to correct for this.
		#we need to correct for that avoiding to change our original widget
		# I agree this is a horrible hack but it does exactly what we want to do.
		#copy old value and cursor position
		pos=self.widget.get_position()
		text=self.widget.get_text()
		#obtain the value meant by the text
		self.valuechecker.set_text(text)
		self.valuechecker.update()
		value=self.valuechecker.get_value()		
		#write the value out spinbutton
		if text!="":
			self.widget.set_value(value)
		else:
			self.widget.set_value(element.get_float())
		#restore text and cursor position
		self.widget.set_text(text)
		if (self.widget.get_position()==0) and (pos!=0):
			self.widget.set_position(pos)
		element.set_float(self.widget.get_value())
			
class _entryhandler(_gtkindi_widget_element_handler):
	"""A handler for an L{indielement} objects connected to a GTK.Entry object (can also be used to connect for any GTK 
	object providing a set_text method)"""
	def on_indiobject_changed(self,vector,element):
		try:
			text=element.get_text()
			if len(text)<40:
				self.widget.set_text(text)
		except:
			return
	def update_indi_element(self,element):
		element.set_text(self.widget.get_value())



class _comboboxentryhandler(_gtkindi_widget_vector_handler):
	"""
	A handler for an L{indiswitchvector} object (with rule C{OneOfMany}) connected to a GTK.ComboBoxEntry object. 
	The labels of L{indiswitch} objects will be fed into the popdown menu of the ComboBox, the label of the active switch will be 
	visable in the entry of . 
	"""
	#def __init__(self,devicename,vectorname,widget):
	#	self.comboboxentry=widget
	#	_gtkindi_widget_vector_handler.__init__(self,devicename,vectorname,widget.entry)
	def __init__(self,devicename,vectorname,widget):
		"""
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param widget: The Gtk.comboboxentryBox to be handled
		@type widget: Gtk.comboboxentryBox
		"""
		_gtkindi_widget_vector_handler.__init__(self,devicename,vectorname,widget)
		self._gtk_signals=["changed"]
		self.type="gktINDIcomboboxentryhandler"
		self.liststore = gtk.ListStore(str)
		self.widget.set_model(self.liststore)
		self.widget.set_text_column(0)
		self.index=None
		self.labels=[]
	def on_blocked(self,*args):
		self.indi_object_change_notify(self.get_vector())
	def configure(self,vector):
		self.liststore.clear() 
		for element in vector.elements:
			self.widget.append_text(element.label)
	
	def on_indiobject_changed(self,vector):
		changed=False
		if len(self.labels)==len(vector.elements):
			for i,element in enumerate(vector.elements):
				if self.labels[i]!=element.label:
					changed=True
		else:
			changed=True
		if changed:
			self.configure(vector)
			self.labels=[]
			for element in vector.elements:
				self.labels.append(element.label)
		
		
		index=vector.get_active_index()
		wigi=self.widget.get_active()
		self.index=index
		if index!=None:
			if wigi!=index:
				self.widget.set_active(index)
		
	def update_indi_vector(self,vector):
		vector.set_active_index(self.widget.get_active())

class _vectorlabelhandler(_gtkindi_widget_vector_handler):
	"""
	A handler for an L{indivector} object (with rule C{OneOfMany}) connected to a  Gtk.Label object. The label of the L{indivector}
	will be fed into the Gtk.Label. 
	"""
	#def __init__(self,devicename,vectorname,widget):
	#	self.comboboxentry=widget
	#	_gtkindi_widget_vector_handler.__init__(self,devicename,vectorname,widget.entry)
	def __init__(self,devicename,vectorname,widget):
		"""
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param widget: The Label Widget
		@type widget: List of Gtk.Label
		"""
		_gtkindi_widget_vector_handler.__init__(self,devicename,vectorname,widget)
		self.type="gtkvectorlabelhandler"
	def on_indiobject_changed(self,vector):
		label=vector.label
		try:
			self.widget.set_markup(label)
		except:
			return
		



class _togglebuttonshandler( _gtkindi_widgetlist_vector_handler):
	"""
	A handler for an L{indiswitchvector} object connected a list of GTK.ToggleButton objects. The togglebuttons
	are set active or inactive according to the states of the L{indiswitch} objects.Objects are associated by thier indices in L{widgetlist} and L{indivector.elements}. 
	@ivar widgetlist: The list of togglebuttons
	@type widgetlist: List of Gtk.ToggleButton
	"""
	def __init__(self,devicename,vectorname,widgetlist):
		"""
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param widgetlist: The list of togglebuttons
		@type widgetlist: List of Gtk.ToggleButton
		"""
		_gtkindi_widgetlist_vector_handler.__init__(self,devicename,vectorname,widgetlist)
		#self._gtk_signals=["toggled"]
		self._gtk_signals=["clicked","activate"]
		self.type="GTKINDITogglebuttonshandler"
		self.current_index=0  # not understood solution for a loopback problem KEEP THIS LINE!
		self.oldstatus=[]
		self.lasttime=time.time()
		for widget in self.widgetlist:
			self.oldstatus.append(widget.get_active())
	def on_blocked(self,*args):
		#BAD PATCH!!!
		#CORRECT:
		#self.indi_object_change_notify(self.get_vector())
		#but loopback!
		None
		
	def on_gui_changed(self,widget,event=None):
		vector=self.get_vector()
		if (vector.rule=="AtMostOne"):
			if widget.get_active():
				for button in self.widgetlist:
					if widget!=button:
						button.set_active(False)
		if (vector.rule=="OneOfMany"):
			if widget.get_active():
				for button in self.widgetlist:
					if widget!=button:
						button.set_active(False)
			else:
				any_active_button=False
				for button in self.widgetlist:
					if button.get_active():
						any_active_button=True
				if not any_active_button:
					widget.set_active(True)
		widgets_changed=False
		for i, element in enumerate(self.widgetlist):
			if self.widgetlist[i].get_active()!=self.oldstatus[i]:
				widgets_changed=True
				self.oldstatus[i]=self.widgetlist[i].get_active()
		quick=False
		if time.time()-self.lasttime<0.05:
			quick=True
		self.lasttime=time.time()
		if quick and (not widgets_changed):
			return
		self.update_indi_vector(vector)
		self.indi.send_vector(vector)
		
	def on_indiobject_changed(self,vector):
		changed=False
		for i,widget in enumerate(vector.elements):
			if vector.elements[i].get_active()!=self.widgetlist[i].get_active():
				#print vector.elements[i].name,self.widgetlist[i].get_label(),vector.elements[i].get_active(),self.widgetlist[i].get_active()
				changed=True
		if not changed:
			return
		#OK I don't understnand these lines but they solve a loopback problem  KEEP THESE LINES!
		self.current_index=self.current_index+1
		for i, element in enumerate(vector.elements):		
			if self.current_index==i:
				self.widgetlist[i].set_active(element.get_active())
		#End of the lines that I don't understand KEEP THESE LINES!
		
		for i, element in enumerate(vector.elements):		
			self.current_index=i # not understood solution for a loopback problem  KEEP THIS LINE!
			self.widgetlist[i].set_active(element.get_active())
			self.widgetlist[i].set_label(element.label)
			#print vector.elements[i].name,self.widgetlist[i].get_label(),vector.elements[i].get_active(),self.widgetlist[i].get_active()
		
	def update_indi_vector(self,vector):
		for i, element in enumerate(vector.elements):
			element.set_active(self.widgetlist[i].get_active())
		
class gtkindiclient(indiclient):
	"""
	@ivar ccdpreviewlist :  A list of ccdpreviews connected to INDI devices
	@type ccdpreviewlist : list of   L{_ccdpreview}
	@ivar indistreamlist :   A list of indistreams connected to INDI devices
	@type indistreamlist :  list of   L{_indistream}
	"""
	
	def __init__(self,host,port):
		"""
		@param host:  The hostname or IP address of the server you want to connect to
		@type host: StringType 
		@param port:  The port address of the server you want to connect to.
		@type port: IntType 				
		"""
		indiclient.__init__(self,host,port)
		self.ccdpreviewlist=[]
		self.indistreamlist=[]
	
	def _element_received(self,vector,element):
		""" Called during the L{process_events} method each time an INDI element has been received
		@param vector: The vector containing the element that has been received  
		@type vector: indivector
		@param element:  The element that has been received
		@type element: indielement
		@return: B{None}
		@rtype: NoneType
		"""
		indiclient._element_received(self,vector,element)
		if element.tag.get_type()=="BLOB":
			self._blob_received(vector,element)

	
	def _blob_received(self,vector,blob):
		""" 
		Distributes the BLOBs to the ccdpreview and stream objects
		@param vector: The vector that has been received  
		@type vector: indivector
		@param blob: The BLOB that has been received  
		@type blob: L{indiblob}
		@return: B{None}
		@rtype: NoneType
		"""
		for preview in self.ccdpreviewlist:
			if preview.devicename==vector.device and blob.get_plain_format()==".ccdpreview" :
				preview.blob_received(blob)
		for stream in self.indistreamlist:
			if stream.devicename==vector.device and blob.get_plain_format()==".stream":
				stream.blob_received(blob)
		
				
	def add_label(self,devicename,vectorname,elementname,widget):
		"""
		Connects an L{indielement} to a GTK.Label (see also L{_labelhandler})
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param elementname: name of the INDI element
		@type elementname: StringType
		@param widget: The Label
		@type widget: Gtk.Label
		@return: The handler object created
		@rtype: L{_labelhandler}
		"""
		return self.add_custom_element_handler(_labelhandler(devicename,vectorname,elementname,widget))

	def add_vectorlabel(self,devicename,vectorname,widget):
		"""
		Connects an L{indivector} object to a GTK.Label object (see also L{_vectorlabelhandler})
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param widget: The Label
		@type widget: Gtk.Label
		@return: The handler object created
		@rtype: L{_vectorlabelhandler}
		"""
		return self.add_custom_vector_handler(_vectorlabelhandler(devicename,vectorname,widget))

	def add_statuslabel(self,devicename,vectorname,widget):
		"""
		Connects the status property of an L{indivector} to a GTK.Label
		(see also L{_statuslabelhandler})
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param widget: The Label
		@type widget: Gtk.Label 
		@return: The handler object created
		@rtype: L{_statuslabelhandler}
		"""
		return self.add_custom_vector_handler(_statuslabelhandler(devicename,vectorname,widget))
			
	def add_switchlabel(self,devicename,vectorname,elementname,widget,onlabel,offlabel):
		"""Connects an L{indiswitch} to a GTK.Label, replacing C{On}  and C{Off} by  a customisable label string
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIVector
		@type vectorname: StringType
		@param elementname: name of the INDI element
		@type elementname: StringType
		@param widget: The Label
		@type widget: Gtk.Label 
		@param onlabel: The Text to be displayed on the Label if the L{indiswitch} is C{On}, Pango markup is allowed
		@type onlabel: StringType
		@param offlabel: The Text to be displayed on the Label if the L{indiswitch} is C{Off}, Pango markup is allowed
		@type offlabel: StringType
		@return: The handler object created
		@rtype: L{_switchlabelhandler}
		"""
		return self.add_custom_element_handler(_switchlabelhandler(devicename,vectorname,elementname,widget,onlabel,offlabel))

	def add_entry(self,devicename,vectorname,elementname,widget):
		"""
		Connects an L{indielement} to a GTK.Entry (can also be used with any GTK 
		object providing a set_text method) (see also L{_entryhandler})
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIvector
		@type vectorname: StringType
		@param elementname: name of the INDI element
		@type elementname: StringType
		@param widget: The GTK widget
		@type widget: Gtk.Widget 
		@return: The handler object created
		@rtype: L{_entryhandler}
		"""
		return self.add_custom_element_handler(_entryhandler(devicename,vectorname,elementname,widget))
	
	def add_range(self,devicename,vectorname,elementname,widget):
		"""
		Connects an L{indielement} to a GTK.Range (like slider scrollbar etc.) 
		(see also L{_rangehandler})
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIvector
		@type vectorname: StringType
		@param elementname: name of the INDI element
		@type elementname: StringType
		@param widget: The GTK Range
		@type widget: GTK.Range
		@return: The handler object created
		@rtype: L{_rangehandler}
		"""
		return self.add_custom_element_handler(_rangehandler(devicename,vectorname,elementname,widget))
		
	def add_spinbutton(self,devicename,vectorname,elementname,widget):
		"""
		Connects an L{indielement} to a GTK.Spinbutton  
		(see also L{_spinbuttonhandler})
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIvector
		@type vectorname: StringType
		@param elementname: name of the INDI element
		@type elementname: StringType
		@param widget: The GTK Spinbutton
		@type widget: Spinbutton
		@return: The handler object created
		@rtype: L{_spinbuttonhandler}
		"""
		return self.add_custom_element_handler(_spinbuttonhandler(devicename,vectorname,elementname,widget))
				
	def setup_range(self,vector,element,widget):
		"""
		Sets up a GTK.Range widget with the information contained in B{element}.
		Set minimum and maximum values as well as the incremental steps and the value of B{widget}
		are set up.
		@param vector: The L{indivector} in which B{element} is contained.
		@type vector: L{indivector}
		@param element: The L{indielement} which shall be used to setup the GTK.Range
		@type element: L{indielement}
		@param widget: The GTK Range to be set up.
		@type widget: GTK.Range
		@return: B{None}
		@rtype: NoneType
		"""
		handler=_rangehandler(vector.device,vector.name,element.name,widget)
		handler.indi=self
		handler.configure(vector,element)
		
	def add_comboboxentry(self,devicename,vectorname,widget):
		"""
		Connects an L{indiswitchvector} object (with rule C{OneOfMany})  to a GTK.comboboxentryBoxEntry (B{not}  a GTK.comboboxentryBox) object. 
		The labels of the L{indiswitch} objects will be fed into the popdown menu of the comboboxentrybox, the label of the active switch 
		will put into the comboboxentrybox entry.  (see also L{_comboboxentryhandler} )
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIvector
		@type vectorname: StringType
		@param widget: The GTK widget
		@type widget: Gtk.Widget
		@return: The handler object created
		@rtype: L{_comboboxentryhandler}
		"""
		return self.add_custom_vector_handler(_comboboxentryhandler(devicename,vectorname,widget))
		
	def add_ccdpreview(self,devicename,drawingareawidget,brightnesswidget,contrastwidget,gammawidget):
		"""
		Connects a Gtk.DrawingArea and 3 GTK.Range objects (like sliders, scrollbars etc.) to an INDI device.
		Any BLOBs with the extension C{.ccdpreview} or C{.ccdpreview.z} being received for
		this device will be drawn into L{drawingareawidget}, any L{indinumbervector} objects received for this device and a 
		who's name property matches C{CCDPREVIEW_CTRL} will be used in order to set the parameters of the ccdpreview.
		(see also L{_ccdpreview}) \n
		B{Important:} You have to call L{indiclient.enable_blob}
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param drawingareawidget: The Gtk.DrawingArea widget to be used in order to display the ccdpreview.
		@type drawingareawidget: Gtk.DrawingArea. 
		@param brightnesswidget:  A GTK Range to adjust the brightness 
		@type brightnesswidget: GTK.Range 
		@param contrastwidget:  A GTK Range to adjust the contrast 
		@type contrastwidget: GTK.Range 
		@param gammawidget:  A GTK Range to adjust gamma
		@type gammawidget: GTK.Range 
		@return: The L{_ccdpreview} object
		@rtype: L{_ccdpreview}
		"""
		preview=_ccdpreview(devicename,drawingareawidget,brightnesswidget,contrastwidget,gammawidget)
		self.ccdpreviewlist.append(preview)
		self.add_custom_vector_handler(preview)
		return preview
	
	def add_indistream(self,devicename,drawingareawidget):
		"""
		Connects a Gtk.DrawingArea to an INDI device. Any blobs with the extension C{.stream} or C{.stream.z} being received for
		this device will be drawn into L{drawingareawidget}, any L{indinumbervector} objects received for this device and a 
		who's name property matches C{IMAGE_SIZE} will be used in order to set the parameters of the stream.
		(see also L{_indistream}) \n
		B{Important:} You have to call L{indiclient.enable_blob}
		@param devicename:  The name of the device
		@type devicename: StringType
		@param drawingareawidget: The Gtk.DrawingArea widget to be used in order to display the stream.
		@type drawingareawidget: Gtk.DrawingArea.
		@return: The L{_indistream} object
		@rtype: L{_indistream} 
		"""
		s=_indistream(devicename,drawingareawidget)
		self.add_custom_vector_handler(s)
		self.indistreamlist.append(s)
		return s
	
	def add_togglebuttons(self,devicename,vectorname,widgetlist):
		"""
		Connects a list of Gtk.ToggleButton to an L{indiswitchvector} (see L{_togglebuttonshandler})
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIvector
		@type vectorname: StringType
		@param widgetlist: The list of togglebuttons.
		@type widgetlist: List of Gtk.ToggleButton or inherited type.
		@return: The handler object created
		@rtype: L{_togglebuttonshandler}
		"""
		return self.add_custom_vector_handler(_togglebuttonshandler(devicename,vectorname,widgetlist))
		
	def get_handler(self,devicename,vectorname,type):
		"""
		This function looks for installed L{indi_custom_vector_handler} objects, matching L{devicename}
		and L{vectorname} and L{type}. If exactly one is found, it is returned. Otherwise an exception is raised.
		@param devicename:  The name of the device 
		@type devicename: StringType 
		@param vectorname: The name of the INDIvector
		@type vectorname: StringType
		@param type: the type of the object to look for
		@type type: StringType
		@return: the handler found or B{None} on error.
		@rtype: L{gui_indi_object_handler}
		"""
		found=None
		for handler in self.custom_vector_handler_list:
			if ((handler.type==type) and 
				(handler.devicename==devicename) and (handler.vectorname==vectorname)):
				if found==None:
					found=handler
				else: 
					raise LookupError , ("gtkindiclient: There are at least two handlers installed for "
						+" device= " + devicename
						+" vector= " + vectorname
						+" handlertype= "+ type
						+ " . But only one is allowed")
					return None
		if found==None:
			raise LookupError , ("gtkindiclient: There is no handler installed for "
				+" device= " + devicename
				+" vector= " + vectorname
				+" handlertype= "+ type
				+ " . But exactly one has to be installed")
			return None
		return found
	
	def get_preview(self,devicename):
		"""
		@param devicename:  The name of the device
		@type devicename: StringType 
		@return: the L{_ccdpreview} object installed for this device.
		@rtype: L{_ccdpreview}
		"""
		return self.get_handler(devicename,"CCDPREVIEW_CTRL","CCDPreviewHandler")

				