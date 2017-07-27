#run with indiserver running tutorial_two
#cd src/examples
#indiserver ./tutorial two
from indiclient import *
try:
	def def_vector(vector,indiclientobject):
		print "new vector defined by host: "+indiclientobject.host+" : "
		vector.tell()
	def msg(message,indiclientobject):
		print "got message by host :"+indiclientobject.host+" : "
		message.tell()
	# The actual script starts here 
	indi=indiclient("localhost",7624)
	indi.set_def_handlers(def_vector,def_vector,def_vector,def_vector,def_vector)
	indi.set_message_handler(msg)
	time.sleep(1)
	indi.process_events()
	indi.quit()
	# The actual script ends here 
except:
	a,b,c =sys.exc_info()
	sys.excepthook(  a,b,c	)
	indi.quit()
