#run with indiserver running tutorial_two
#cd src/examples
#indiserver ./tutorial two
from indiclient import *
import sys
try:
	class demohandler(indi_custom_element_handler):
		def on_indiobject_changed(self,vector,element):
			print "RA= "+element.get_text()
			print " has just been received on port "+str(self.indi.port)
	# The actual script starts here 
	indi=indiclient("localhost",7624)
	print "Installing and calling hander"
	indi.add_custom_element_handler(demohandler("Telescope Simulator","EQUATORIAL_COORD","RA"))
	print "Done"
	indi.set_and_send_float("Telescope Simulator","EQUATORIAL_COORD","RA",2.0)
	time.sleep(0.0001)
	indi.set_and_send_float("Telescope Simulator","EQUATORIAL_COORD","RA",1.0)
	print "Staring hander"
	indi.process_events()
	print "Done"
	indi.quit()
	# The actual script ends here 
except:
	a,b,c =sys.exc_info()
	sys.excepthook(  a,b,c	)
	indi.quit()