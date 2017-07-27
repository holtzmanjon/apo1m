'''
Created on Apr 29, 2010

@author: ltraynor
'''
import numpy
import pyApg as apg
import getopt
import sys
import traceback

def main():
    try:
        #parse command line options
        opts, args = getopt.getopt(sys.argv[1:],"s:", ["sub="])
         
        subnet = ""
        for o, a in opts:
            if o in ("-s","--sub"):
                subnet = a
              
        efind = apg.FindDeviceEthernet()  
        print efind.Find( subnet )
    except:
        traceback.print_exc(file=sys.stderr)
        sys.exit(2)
        
if __name__ == '__main__':
    main()