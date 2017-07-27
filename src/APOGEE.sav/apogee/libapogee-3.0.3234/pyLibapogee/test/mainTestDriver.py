import logging
import os
import randomImgCount
import irregularRoiAndBinning
import asymBinning
import normFastSwitching
import fullThenSubFrame
import randomExposeTime
import startStopImgCountExp
import startStopImgCountBulk
import startStopSingleExp
import tdiStress
import mpiAlta

# set up logging to file - see previous section for more details
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    datefmt='%m-%d %H:%M',
                    filename='apogeeTest.log',
                    filemode='w')
# define a Handler which writes INFO messages or higher to the sys.stderr
console = logging.StreamHandler()
console.setLevel(logging.INFO)
# set a format which is simpler for console use
formatter = logging.Formatter('%(name)-12s: %(levelname)-8s %(message)s')
# tell the handler to use this format
console.setFormatter(formatter)
# add the handler to the root logger
logging.getLogger('').addHandler(console)

testLog = logging.getLogger('apogeeTest')

outDirBase = os.getcwd()

#run the tests...
#what do to with resutls
result = fullThenSubFrame.run( outDirBase )

result = asymBinning.run( outDirBase )

result = normFastSwitching.run( outDirBase )

result = randomImgCount.run( outDirBase )

result = irregularRoiAndBinning.run( outDirBase )

result = randomExposeTime.run( outDirBase )

result = startStopImgCountBulk.run( outDirBase )

result = startStopImgCountExp.run( outDirBase )

result = startStopSingleExp.run( outDirBase )

#result = tdiStress.run( outDirBase )

result = mpiAlta.run( outDirBase )

