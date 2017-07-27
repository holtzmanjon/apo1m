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
	indi.tell()
	indi.set_and_send_switchvector_by_elementlabel("Telescope Simulator", "CONNECTION", "On")
	print time.time()
	print indi.get_text("Telescope Simulator","EQUATORIAL_EOD_COORD","RA")
	vector=indi.get_vector("Telescope Simulator","EQUATORIAL_EOD_COORD_REQUEST")
	vector.get_element("RA").set_float(2.0)
	vector.get_element("DEC").set_float(0.0)
	indi.send_vector(vector)
	waitForOk(indi.get_vector("Telescope Simulator","EQUATORIAL_EOD_COORD_REQUEST"),indi,60.0)
	print time.time()
	print indi.get_float("Telescope Simulator","EQUATORIAL_EOD_COORD","RA")
	print indi.get_text("Telescope Simulator","EQUATORIAL_EOD_COORD","RA")
	vector.get_element("RA").set_text("3:30:00")
	vector.get_element("DEC").set_float(0.0)
	indi.send_vector(vector)
	waitForOk(indi.get_vector("Telescope Simulator","EQUATORIAL_EOD_COORD_REQUEST"),indi,60.0)
	print indi.get_float("Telescope Simulator","EQUATORIAL_EOD_COORD","RA")
	indi.quit()
	# The actual script ends here 
except:
	a,b,c =sys.exc_info()
	sys.excepthook(  a,b,c	)
	indi.quit()
