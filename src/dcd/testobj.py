#run with indiserver running tutorial_two
#cd src/examples
#indiserver ./tutorial two
from indiclient import *

def waitForOkGen(vec,ind,checkinterval,timeout):
	t=time.time()
	while not(vec._light.is_ok()):
		time.sleep(checkinterval)
		ind.process_events()
		if (time.time()-t)>timeout:
			raise Exception, ("timeout waiting for state to turn Ok "+
				"devicename=" +vec.device+" vectorname= " + vec.name+
				" "+str(timeout)+ " "+str(time.time()-t)
				)

def waitForOk(vec,ind,timeout):
	checkinterval=0.1
	if (timeout/1000.0)<checkinterval:
		checkinterval=(timeout/1000.0)
	waitForOkGen(vec,ind,checkinterval,timeout)

try:
	# The actual script starts here 
	indi=indiclient("localhost",7624)
	time.sleep(1)
	indi.process_events()
	vector=indi.get_vector("Telescope Simulator","CONNECTION")
	vector.tell()
	vector.set_by_elementname("CONNECT")
	indi.send_vector(vector)
	waitForOk(vector,indi,0.1)
	vector.tell()
	vector.set_by_elementname("DISCONNECT")
	waitForOk(vector,indi,0.1)
	vector.tell()
	element=vector.get_element("CONNECT")
	print element.get_active()
	indi.quit()
	# The actual script ends here 
except:
	a,b,c =sys.exc_info()
	sys.excepthook(  a,b,c	)
	indi.quit()
