#
#  This file contains the most Apogee camera specific parts of the interface.
#  It is designed so that other camera types can easily be incorporated in
#  future (hopefully)
#
#


#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : debuglog
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-4-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  Redirect debug messages, currently just print them out
#
#  Arguments  :
#
#               msg	-	message text
 
proc debuglog { msg } {
 
#
#  Globals    :		n/a
#  
   puts stdout "$msg"
}





#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : fileopen
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-4-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure lets the user select a configuration file then 
#  parses it to initialize the GUI widgets.
#
#  Arguments  :
#
 
proc fileopen { } {
 
#
#  Globals    :
#  
#               CFGFILE	-	Configuraion file in use
#               DEBUG	-	Set to 1 for verbose logging
#               TKAPOGEE	-	Base directory for installation
#               env	-	The shell environment
global CFGFILE DEBUG TKAPOGEE env
   set cfg [tk_getOpenFile -initialdir $TKAPOGEE/config -filetypes {{{Apogee cameras} {.ini}}}]
   if { $DEBUG } {debuglog "Loading configuration from $cfg"}
   loadconfig $cfg
   exec ln -sf $CFGFILE $env(HOME)/.apccd.ini
}




#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : loadconfig
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-4-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure opens a named Apogee .ini configuration file and
#  parses the contents. Items are entered in the global array CONFIG, 
#  which has automatic callbacks to the GUI widgets.
#
#  Arguments  :
#
#               cfg	-	Configuration file name
 
proc loadconfig { cfg } {
 
#
#  Globals    :
#  
#               CFGFILE	-	Configuraion file in use
#               CONFIG	-	GUI configuration
#               DEBUG	-	Set to 1 for verbose logging
global CFGFILE CONFIG DEBUG
   set CFGFILE $cfg
   set fcfg [open $CFGFILE r]
   while { [gets $fcfg rec] > -1 } {
      if { [string range $rec 0 0] == "\[" } {
         set subsys [lindex [split $rec "\[\]"] 1]
      }
      if { [llength [split $rec "="]] == 2 } {
         set par [string trim [lindex [split $rec "="] 0]]
         if { $par == "ImgRows" } {set par Imgrows}
         if { $par == "ImgCols" } {set par Imgcols}
         set val [lindex [split $rec "="] 1]
         set CONFIG($subsys.$par) [string trim $val]
         if { $DEBUG } {debuglog "Configuration $subsys,$par = $val"}
      }
   }
   close $fcfg
}




#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : shutdown
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-4-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  Set the camera to  ramp-up to ambient, and then exit
#
#  Arguments  :
#
#               id	-	Camera id (for multi-camera use) (optional, default is 0)
 
proc shutdown { {id 0} } {
 
#
#  Globals    :
#  
#               CAMERAS	-	Camera id's
global CAMERAS ALTA
   set it [tk_dialog .d "Exit" "Confirm exit" {} -1 "Cancel" "EXIT"]
   if { $it } {
     if { $ALTA } {
        set cooler SetCooler
     } else {
        set cooler write_CoolerMode
     }
     set camera $CAMERAS($id)
     $camera $cooler 2
     catch { exec xpaset -p ds9 exit }
     if { $ALTA } {$CAMERAS($id) CloseConnection}
     exit
   }
}






#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : apogeeSetVal
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-4-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure is a simple wrapper to easily set Apogee camera parameters
#  using the C++ wrapper interface.
#
#  Arguments  :
#
#               configPar	-	Name of a .ini configuration parameter
#               objPar	-	Name of C++ instance variable
#               id	-	Camera id (for multi-camera use) (optional, default is 0)
 
proc apogeeSetVal { configPar objPar {id 0} } {
 
#
#  Globals    :
#  
#               CAMERAS	-	Camera id's
#               DEBUG	-	Set to 1 for verbose logging
#               CONFIG	-	GUI configuration
global CAMERAS DEBUG CONFIG
  set camera $CAMERAS($id)
  if { [info exists CONFIG($configPar)] } {
     if { $DEBUG } {debuglog "Setting value for $objPar = $CONFIG($configPar)"}  
     $camera Set[set objPar] $CONFIG($configPar)     
  } else {
     if { $DEBUG } {debuglog "No value available for $configPar"}
  }
}




#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : apogeeSetBool
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-4-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This routine is a simple wrapper to set boolean Apogee camera variables.
#
#  Arguments  :
#
#               configPar	-	Name of a .ini configuration parameter
#               objPar	-	Name of C++ instance variable
#               id	-	Camera id (for multi-camera use) (optional, default is 0)
 
proc apogeeSetBool { configPar objPar {id 0} } {
 
#
#  Globals    :
#  
#               CAMERAS	-	Camera id's
#               DEBUG	-	Set to 1 for verbose logging
#               CONFIG	-	GUI configuration
global CAMERAS DEBUG CONFIG
  set camera $CAMERAS($id)
  if { [info exists CONFIG($configPar)] } {
     if { $DEBUG } {debuglog "Setting value for $objPar = $CONFIG($configPar)"}
     set state [string toupper  $CONFIG($configPar)]
     switch $state {
             0         -
             OFF       -
             CCD       -
             FALSE     { $camera configure -m_$objPar 0 }
             1         -
             ON        -
             CMOS      -
             TRUE      { $camera configure -m_$objPar 1 }
     }
  } else {
     if { $DEBUG } {debuglog "No value available for $configPar"}
  }
}


#
# Define the global environment, everything lives under /opt/apogee
# Change TKAPOGEE to move the code somewhere else
#
set TKAPOGEE $env(TKAPOGEE)
if { $tcl_platform(os) != "Darwin" } {
   load $TKAPOGEE/lib/libfitstcl.so
   load $TKAPOGEE/lib/libccd.so
} else {
   load $TKAPOGEE/lib/libccd.dylib
}

#
#  Load the Apogee ini file
#

loadconfig $env(HOME)/.apccd.ini

#
#  Load the appropriate wrapper library
#
if { $tcl_platform(os) == "Darwin" } {
   set ldext dylib
   load $TKAPOGEE/lib/tcllibapogee.$ldext
} else {
   set ldext so
   switch $CONFIG(system.Interface) {
      ISA {load $TKAPOGEE/lib/apogee_ISA.$ldext}
      PPI {load $TKAPOGEE/lib/apogee_PPI.$ldext}
      PCI {load $TKAPOGEE/lib/apogee_PCI.$ldext}
      USB {load $TKAPOGEE/lib/tcllibapogee.$ldext}
      NET {load $TKAPOGEE/lib/tcllibapogee.$ldext}
   }
}

if { $CONFIG(system.Interface) == "USB" || $CONFIG(system.Interface) == "NET" } {
   set ALTA 1
   source $TKAPOGEE/apps/tcl/scripts/alta_init.tcl
} else {
   set ALTA 0
   source $TKAPOGEE/apps/tcl/scripts/legacy_init.tcl
}







