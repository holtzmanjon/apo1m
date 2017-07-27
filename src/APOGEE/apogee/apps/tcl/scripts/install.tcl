#
# Per-user installation script to configure an Apogee camera.
# Creates a script "startapogee" in the users home directory
# which sets up the environment for the appropriate camera
# model, and then starts the gui.
#
#

wm withdraw .
if { [file exists $env(HOME)/.apccd.ini] == 0 } {
    puts stdout "Please choose the correct ini file for your camera"
    set cfg [tk_getOpenFile -initialdir /opt/apogee/config/ -filetypes {{{Apogee cameras} {.ini}}}]
    if { [file exists "$cfg"] } {
      set i [string trim [lindex [split [exec grep Interface $cfg] =] 1]]
      switch $i {
           PPI { 
                 set port "none" ; set p2 "none"
                 catch {set port [string range [exec grep apppi /proc/ioports] 0 3]}
                 catch {set p2   [string range [exec grep apppi /proc/devices] 0 3]}
                 if { $port == "none" && $p2 == "none" } {
                     set it [ tk_dialog .d "No module" "Please load the kernel module for your interface" {} -1 OK] 
                     exit
                 }
                 if { $p2 != "none" } {
                    set port "0x378"
                 } else {
                   set port "0x$port" 
                 }
              }
           ISA { 
                 set port "none"
                 catch {set port [string range [exec grep apisa /proc/ioports] 0 3]}
                 if { $port == "none" } {
                     set it [ tk_dialog .d "No module" "Please load the kernel module for your interface" {} -1 OK] 
                     exit
                 }
                 set port "0x$port"
               }
           USB - 
           NET -
           PCI { set port auto
               } 
      }
      if { $port == "none" } {
         puts stdout "Camera configuration not completed"
         exit
      } else {
          switch $i {
		ISA -
	        PPI {
	             set fin [open $cfg r]
	             set fout [open $env(HOME)/.apccd.ini w]
	             while { [gets $fin rec] > -1 } {
	                if { [string range $rec 0 4] == "Base=" } {
         	          puts $fout "Base=$port"
         	        } else {
         	          puts $fout $rec
         	        }
         	     }
        	     close $fin
                     close $fout
		    }
		PCI -
	        USB { exec cp $cfg $env(HOME)/.apccd.ini }
		NET { exec /opt/apogee/apps/tcl/scripts/altaDiscover }
          }
          set fout [open $env(HOME)/startapogee w]
          puts $fout "#!/bin/csh"
          puts $fout "echo Preparing environment for Apogee camera driver"
          if { [file exists /etc/udev/udev.conf] } {
		puts $fout "setenv USB_DEVFS_PATH /dev/bus/usb"
          }
          puts $fout "setenv APGTCL /opt/apogee/apps/tcl/scripts"
          puts $fout "source \$APGTCL/setup.env"
          puts $fout "wish -f \$APGTCL/gui.tcl"
          close $fout
          exec chmod 755 $env(HOME)/startapogee
          puts stdout "Camera configuration complete"
          puts stdout "Use the following command to start the control program"
          puts stdout " "
          puts stdout "		$env(HOME)/startapogee"
          puts stdout " "
          exec rm -f $env(HOME)/.apgui.tcl
          exit
      }
    } else {
        puts stdout "Configuration $cfg does not exist - giving up"
        exit
    }
} else {
   bell
   puts stdout "The Apogee camera driver is already configured"
   puts stdout "To alter the configuration, delete $env(HOME)/.apccd.ini"
   puts stdout "and re-run this script"
}

exit


