#! /bin/bash

# NOTE: paths are set to the gnu defaults.  modify if install is set
# to a different location

# only call this once
#export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/share/java

javac -classpath /usr/local/share/java/jlibapogee.jar *.java
java -classpath .:/usr/local/share/java/jlibapogee.jar apogee_main