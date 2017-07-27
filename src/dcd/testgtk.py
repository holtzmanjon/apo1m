#run with indiserver running tutorial_two
#cd src/examples
#indiserver ./tutorial two
from gtkindiclient import *
import gtk
try:
	def on_delete(*args):
		global running
		running=False
	# The actual script starts here 
	global running
	running=True
	indi=gtkindiclient("localhost",7624)
	mainwindow = gtk.Window(gtk.WINDOW_TOPLEVEL)
	mainwindow.connect("delete_event", on_delete)
	mainwindow.show()
	label=gtk.Label()
	label.show()
	indi.add_label("Telescope Simulator","EQUATORIAL_COORD","RA",label)
	mainwindow.add(label)
	while running:
		indi.process_events()
		while gtk.events_pending():
			gtk.main_iteration_do(False)
		time.sleep(0.01)
	indi.quit()
	# The actual script ends here 
except:
	a,b,c =sys.exc_info()
	sys.excepthook(  a,b,c	)
	indi.quit()