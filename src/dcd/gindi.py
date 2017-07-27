from gtkindiclient import *
import socket
import time
import base64
import gtk
import math
import os
import platform

class winstarter:
	def __init__(self,command):
		self.command=command
		self.receivetimer = threading.Timer(0.01, self.callback)
		self.receivetimer.start()
	def callback(self):
		os.system(self.command)
		

class myindiclient(gtkindiclient):
	def __init__(self,host,port,widget):
		self.widget=widget
		gtkindiclient.__init__(self,host,port)
		#self.set_verbose()
			
	def _blob_received(self,vector,blob):
		gtkindiclient._blob_received(self,vector,blob)
		self.enable_blob()		
		if  blob.get_plain_format()==".fits":
			i=1
			while os.path.exists("image%09d" % i+".fits"):
				i=i+1
			out_file = open("image%09d" % i+".fits","wb")
			out_file.write(blob.get_data())
			out_file.close()
			if platform.system()=="Windows":
				winstarter(self.widget.get_text()+" "+"image%09d" % i+".fits")
			if platform.system()=="Linux":
				os.system(self.widget.get_text()+" "+"image%09d" % i+".fits"+" &")		
	


class gindi_device:
	def __init__(self,name,notebook):
		self.name=name
		self.notebook=notebook
		self.grouplist=[]

class gindi_group:
	def __init__(self,name,vbox):
		self.name=name
		self.vbox=vbox
		self.vectorlist=[]
		self.vectorwidgetlist=[]
		
class gindi_numvector:
	def __init__(self,indivector):
		self.indivector=indivector
		self.entrylist=[]

class gindi_switchvector:
	def __init__(self,indivector,indi):
		self.indivector=indivector
		self.buttonlist=[]
		self.indi=indi
	def set_togglebuttons(self):
		for i, element in enumerate(self.indivector.elements):
			element.set_active(self.buttonlist[i].get_active())	

class defhandler:
	def __init__(self,vector,indi,parent):
		self.indi=indi
		self.indivector=vector
		self.widgetlist=[]
		self.parent=parent
		self.hbox = gtk.HBox(False,0)
		self.on_hbox_created()
		self.table=gtk.Table(len(vector.elements),4, False)
		self.table.show()
		for i, element in enumerate(vector.elements):
			label=self.makelabel(element.label)
			self.table.attach(label, 0, 1, i, i+1,gtk.EXPAND, gtk.EXPAND, 5, 5)
			label=self.makelabel("undefined")
			self.table.attach(label, 1, 2, i, i+1,gtk.EXPAND, gtk.EXPAND, 5, 5)
			indi.add_label(vector.device,vector.name,element.name,label)
			if vector.get_permissions().is_writeable():
				widget=self.make_changeblewidget(element,i)
		vbox = gtk.VBox(False,0)
		if vector.get_permissions().is_writeable():
			button = gtk.Button("Set")
			button.show()
			button.connect("clicked", self.gtk_callback )
			button.set_flags(gtk.CAN_FOCUS)
			vbox.pack_end(button, True, False, 0)	
		vbox.show()
		self.hbox.pack_end(vbox, False, False, 5)
		self.hbox.pack_end(self.table, False, False, 5)
		self.hbox.show()
		self.make_vector()
	def make_vector(self):
		box = gtk.HBox(False,0)
		box.show()
		label=gtk.Label("void")
		label.set_size_request(50,-1)
		label.show()
		self.indi.add_statuslabel(self.indivector.device,self.indivector.name,label)
		box.pack_start(label, False, False, 5)
		label=gtk.Label(self.indivector.label)
		label.set_size_request(150,-1)
		label.show()
		self.indi.add_vectorlabel(self.indivector.device,self.indivector.name,label)
		frame = gtk.Frame()
		frame.show()
		frame.add(label)
		box.pack_start(frame, False, False, 5)
		box.pack_start(self.hbox, False, False, 5)
		self.widget=box
	def on_hbox_created(self):
		None
	def make_changeblewidget(self,element,i):
		entry = gtk.Entry()
		entry.show()
		entry.set_size_request(150,-1)
		entry.connect("activate", self.gtk_callback )
		self.table.attach(entry, 2, 3, i, i+1,gtk.EXPAND, gtk.EXPAND, 5, 5)
		self.widgetlist.append(entry)
	def makelabel(self,label):
		label=gtk.Label(label)
		label.set_line_wrap(True)
		label.show()
		label.set_size_request(200, -1)
		return label
	def gtk_callback(self, widget):
		for i, entry in enumerate(self.widgetlist):
			self.update_element(self.indivector.get_element(self.indivector.elements[i].name),self.widgetlist[i])
		self.indi.send_vector(self.indivector)
	def update_element(self,element,widget):
		element.set_text(widget.get_text())
		
class text_defhandler(defhandler):
	None
class blob_defhandler(defhandler):
	None

class number_defhandler(defhandler):
	def on_hbox_created(self):
		if self.indivector.name=="CCDPREVIEW_CTRL":
			ccdpreview_def_handler(self.indivector,self.hbox,self.indi)
		if self.indivector.name=="IMAGE_SIZE":
			streampreview_def_handler(self.indivector,self.hbox,self.indi)
	def make_element(self,element,i):
		if self.indivector.get_permissions().is_writeable():
			widget=gtk.HScrollbar()
			widget.show()
			widget.set_size_request(150,-1)
			self.indi.setup_range(self.indivector,element,widget)
			self.table.attach(widget, 2, 3, i, i+1,gtk.EXPAND, gtk.EXPAND, 5, 5)
	def _setup_range(self,element,widget,rangelist):
		self.indi.setup_range(self.indivector,element,widget)
		widget.show()
		self.rangebox.pack_end(widget)
		widget.connect("value_changed", self._range_changed ,rangelist)
		rangelist.append(widget)
	def _range_changed(self, widget,rangelist):
		v=widget.get_value()
		adj=widget.get_adjustment()
		step=adj.step_increment
		min=adj.lower
		if (v-min)%step!=0:
			s=int(round((v-min)/step))
			widget.set_value(min+s*step)
		for range in rangelist:
			range.set_value(widget.get_value())
	def make_changeblewidget(self,element,i):
		self.rangebox=gtk.HBox(False,0)
		if (not element.is_range()): # or (element.get_number_of_steps()>300):
			defhandler.make_changeblewidget(self,element,i)
			return
		widget = gtk.SpinButton()
		widget.set_digits(element.get_digits_after_point())
		#widget.set_snap_to_ticks(True)
		#widget.set_numeric(True)
		self.widgetlist.append(widget)
		rangelist=[]
		self._setup_range(element,widget,rangelist)
		widget=gtk.HScale()
		widget.set_draw_value(False)
		widget.set_size_request(50,-1)
		self._setup_range(element,widget,rangelist)
		self.table.attach(self.rangebox, 2, 3, i, i+1,gtk.EXPAND, gtk.EXPAND, 5, 5)
		self.rangebox.show()
		self.rangebox.set_size_request(150,-1)
class preview_def_handler:
	def __init__(self,vector,hbox,indi):
		indi.enable_blob()
		self.vbox = gtk.VBox(False,0)
		self.vbox.show()
		self.drawing_area = gtk.DrawingArea()
		xsize=vector.get_element("WIDTH").get_int()
		ysize=vector.get_element("HEIGHT").get_int()
		if xsize<100:
			xsize=100
		if ysize<100:
			ysize=100
		self.drawing_area.show()
		self.vbox.pack_end(self.drawing_area, False, False, 5)
		self.table=gtk.Table(3,2, False)
		self.table.show()
	
class streampreview_def_handler(preview_def_handler):
	def __init__(self,vector,hbox,indi):
		preview_def_handler.__init__(self,vector,hbox,indi)
		self.vbox.pack_end(self.table, False, False, 5)		
		hbox.pack_end(self.vbox, False, False, 5)
		preview=indi.add_indistream(vector.device,self.drawing_area)
	
class ccdpreview_def_handler(preview_def_handler):
	def __init__(self,vector,hbox,indi):
		preview_def_handler.__init__(self,vector,hbox,indi)
		scrollbars=[]
		labels=[]
		labelnames=["Brightness","Contrast","Gamma"]
		
		for i in range(3):
			scrollbars.append(gtk.HScrollbar())
			scrollbars[i].set_size_request(100,-1)
			scrollbars[i].show()
			labels.append(gtk.Label(labelnames[i]))
			labels[i].show()
			self.table.attach(labels[i], 0, 1, i, i+1,gtk.EXPAND | gtk.FILL, gtk.EXPAND |gtk.FILL, 0, 0)
			self.table.attach(scrollbars[i], 1, 2, i, i+1,gtk.EXPAND | gtk.FILL, gtk.EXPAND |gtk.FILL, 0, 0)
		self.vbox.pack_end(self.table, False, False, 5)		
		hbox.pack_end(self.vbox, False, False, 5)
		self.preview=indi.add_ccdpreview(vector.device,self.drawing_area,
			scrollbars[0],scrollbars[1],scrollbars[2])

	def _scrollbar_changed(self, widget):
		self.preview.update()
	def on_previewarea_configure_event(self, widget, *args):
		x, y, width, height = widget.get_allocation()
		self.pixmap = gtk.create_pixmap(widget.get_window(), width, height, -1)
		gtk.draw_rectangle(pixmap, widget.get_style().white_gc,
		gtk.TRUE, 0, 0, width, height)
		return True
		
class gindi:
	def __init__(self,widget):
		self.widget=widget
		self.connections=[]
		self.mainwindow = gtk.Window(gtk.WINDOW_TOPLEVEL)
		self.devicelist=[]
		self.mainwindow.connect("delete_event", self.hide, "")
		self.running=True
		self.notebook = gtk.Notebook()
		self.notebook.set_tab_pos(gtk.POS_TOP)
		self.notebook.show()
		box = gtk.VBox(False,0)
		box.show()
		box.pack_start(self.notebook, False, False, 5)
		scrolled_window = gtk.ScrolledWindow()
		scrolled_window.show()
		textview = gtk.TextView()
		textview.unset_flags(gtk.CAN_FOCUS)
		textview.set_editable(False)
		textview.show()
		self.textbuffer= textview.get_buffer()
		self.scrolled_window=scrolled_window
		scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		scrolled_window.add_with_viewport(textview)
		#textview.set_size_request(200, 100)
		box.pack_start(scrolled_window, False, False, 5)
		#box.pack_start(textview, False, False, 5)
		self.mainwindow.add(box)
		#self.mainwindow.show()
	
	def hide(self, widget, *args):
		self.mainwindow.hide()
		return True
	
	def process_events(self):
		for con in self.connections:
			con.process_events()
	
	def connect(self,host,port):
		self.mainwindow.show()
		for i in self.connections:
			if (i.host==host) and (i.port==port):
				return
		indi=myindiclient(host,port,self.widget)
		#indi.verbose=True
		self.connections.append(indi)
		#indi.verbose= True
		indi.verbose=False
		indi.blob_def_handler=self.blob_def_handler
		indi.number_def_handler=self.number_def_handler
		indi.switch_def_handler=self.switch_def_handler
		indi.text_def_handler=self.text_def_handler
		indi.blob_def_handler=self.text_def_handler
		indi.message_handler=self.message_handler
		indi.light_def_handler=self.light_def_handler
	def quit(self):
		self.running=False
		for con in self.connections:
			con.quit()
		return False
		
	def get_device(self,devicename):
		for dev in self.devicelist:
			if dev.name==devicename:
				return dev
		return None
		
	def get_group(self,devicename,groupname):
		dev=self.get_device(devicename)
		if dev!=None:
			for group in dev.grouplist:
				if group.name==groupname:
					return group
		return None
			
	def get_vector(self,devicename,groupname,vectorname):
		group=self.get_group(devicename,groupname)
		if group!=None:
			for vec in group.vectorlist:
				if vec.name==vectorname:
					return vec
		return None
	
	def get_or_create_device(self,vector):
		dev=self.get_device(vector.device)
		if dev==None:
			notebook = gtk.Notebook()
			notebook.set_tab_pos(gtk.POS_TOP)
			#notebook.set_size_request(100, 75)
			notebook.show()
			label = gtk.Label(vector.device)
			self.notebook.append_page(notebook, label)
			dev=gindi_device(vector.device,notebook)
			self.devicelist.append(dev)
		return self.get_device(vector.device)
	
	def get_or_create_group(self,vector):
		dev=self.get_or_create_device(vector)
		group=self.get_group(vector.device,vector.group)
		if group==None:
			box = gtk.VBox(False,0)
			#box.set_size_request(200, 200)
			box.show()
			label = gtk.Label(vector.group)
			dev.notebook.append_page(box, label)
			group=gindi_group(vector.group,box)
			dev.grouplist.append(group)
		return self.get_group(vector.device,vector.group)
		
	def blob_def_handler(self,vector,indi):
		widget=blob_defhandler(vector,indi,self).widget
		group=self.get_or_create_group(vector)
		group.vbox.pack_start(widget, False, False, 5)

	def number_def_handler(self,vector,indi):
		widget=number_defhandler(vector,indi,self).widget
		group=self.get_or_create_group(vector)
		group.vbox.pack_start(widget, False, False, 5)
		
	def text_def_handler(self,vector,indi):
		widget=text_defhandler(vector,indi,self).widget
		group=self.get_or_create_group(vector)
		group.vbox.pack_start(widget, False, False, 5)

	def get_or_create_vector(self,vector,widget,indi):
		group=self.get_or_create_group(vector)
		vec=self.get_vector(vector.device,vector.group,vector.name)
		if vec==None:
			box = gtk.HBox(False,0)
			#box.set_size_request(200, 50)
			box.show()
			label=gtk.Label("void")
			label.set_size_request(50,-1)
			label.show()
			indi.add_statuslabel(vector.device,vector.name,label)
			box.pack_start(label, False, False, 5)
			label=gtk.Label(vector.label)
			label.set_size_request(150,-1)
			label.show()
			indi.add_vectorlabel(vector.device,vector.name,label)
			frame = gtk.Frame()
			frame.show()
			frame.add(label)
			box.pack_start(frame, False, False, 5)
			box.pack_start(widget, False, False, 5)
			group.vbox.pack_start(box, False, False, 5)
	

	def switch_def_handler(self,vector,indi):
		swvec=gindi_switchvector(vector,indi)
		size=len(vector.elements)
		y=int(math.ceil(size/5))
		if y<=1:
			table=gtk.Table(1,size, False)
		else:
			table=gtk.Table(y,4, False)
		table.show()
		row=0
		col=0
		lastbutton=None
		for i, element in enumerate(vector.elements):
			if (vector.rule=="AtMostOne"):
				button=gtk.ToggleButton(element.label)
			if (vector.rule=="OneOfMany"):
				#button=gtk.ToggleButton(element.label)
				button=gtk.RadioButton(lastbutton, element.label)
			if (vector.rule=="AnyOfMany"):
				button=gtk.CheckButton(element.label)
			indi.add_label(vector.device,vector.name,element.name,button)
			lastbutton=button
			button.set_size_request(200, -1)
			button.show()
			button.set_flags(gtk.CAN_FOCUS)	
			button.set_sensitive(vector.get_permissions().is_writeable())
			table.attach(button, row, row+1, col, col+1,gtk.EXPAND, gtk.EXPAND, 5, 5)
			swvec.buttonlist.append(button)
			row=row+1
			if row>=4:
				row=0
				col=col+1
		self.get_or_create_vector(vector,table,indi)
		swvec.togglebuttonshandler=indi.add_togglebuttons(swvec.indivector.device,swvec.indivector.name,swvec.buttonlist)
		swvec.togglebuttonshandler.set_bidirectional()
		
	def light_def_handler(self,vector,indi):
		print "DEF LIGHT"
		
	def message_handler(self,message,indi):
		iter=self.textbuffer.get_iter_at_offset(0)
		iter.forward_to_end()
		self.textbuffer.insert(iter,"\n" +message.timestamp+" " + message.device +
			" : "+ message.get_text())
		adjust=self.scrolled_window.get_vadjustment()
		adjust.set_value(adjust.upper)