

#
# This script assists the user in selecting the appropriate kernel module
# for an Apogee CCD camera driver.
#
wm withdraw .

set kext "o"
puts stdout "Please choose the correct ini file for your camera"
set cfg [tk_getOpenFile -initialdir /opt/apogee/config/ -filetypes {{{Apogee cameras} {.ini}}}]
if { [file exists "$cfg"] } {
      set i [string trim [lindex [split [exec grep Interface $cfg] =] 1]]
      switch $i {
           PPI { set l "Cancel 0x378 0x278 0x3BC"
                 set it [ tk_dialog .d "Select IO address" "Please specify the IO address for the camera ?" {} -1 Cancel "0x378" "0x278" "0x3BC"]
                 set marg "apppi_addr=[lindex $l $it]"
                 catch {
                    foreach n "0 1 2 3" {
                       exec mknod /dev/apppi$n c 61 $n
                       exec chmod og+rw /dev/apppi$n
                    }
                 }
               }
           ISA { set l "Cancel 0x290 0x200 0x210 0x280 0x300 0x310 0x390"
                 set it [ tk_dialog .d "Select IO address" "Please specify the IO address for the camera ?" {} -1 Cancel "0x290" "0x200" "0x210" "0x280" "0x300" "0x310" "0x390"]
                 set marg "apisa_addr=[lindex $l $it]"
                 catch {
                    foreach n "0 1 2 3" {
                       exec mknod /dev/apisa$n c 62 $n
                       exec chmod og+rw /dev/apisa$n
                    }
                 }
               }
           PCI { set it 1
                 set marg ""
                 catch {
                    foreach n "0 1 2 3" {
                       exec mknod /dev/appci$n c 60 $n
                       exec chmod og+rw /dev/appci$n
                    }
                 }
               }
           USB { set it 1
                 set marg ""
                 if { [file exists /usr/share/hal/fdi] } {
                     exec sudo mkdir -p /usr/share/hal/fdi/information/20thirdparty
                     exec sudo mkdir -p /usr/share/hal/fdi/policy/20thirdparty
                     exec sudo cp /opt/apogee/config/60-camera-altausb.fdi /usr/share/hal/fdi/information/20thirdparty/.
                     exec sudo cp /opt/apogee/config/60-altausb-camera-policy.fdi /usr/share/hal/fdi/policy/20thirdparty/.
                     exec sudo cp /opt/apogee/config/61-filterwheel-altausb.fdi /usr/share/hal/fdi/information/20thirdparty/.
                     exec sudo cp /opt/apogee/config/61-altausb-filterwheel-policy.fdi /usr/share/hal/fdi/policy/20thirdparty/.
                     exec sudo mkdir -p /usr/libexec
                     exec sudo cp /opt/apogee/config/altausb-set-procperm /usr/libexec/.
		     set it [tk_dialog .d "ALTA" "ALTA HAL support configured" {} -1 OK]
                 }
                 if { [file exists /etc/udev/udev.conf] } {
                     exec sudo cp /opt/apogee/config/60-apogee.rules /etc/udev/rules.d/.
		     set it [ tk_dialog .d "ALTA-U" "ALTA-U udev support configured" {} -1 OK]   
                 } else {
   		   if { [file exists /etc/hotplug/usb.usermap] } {
                    set fusb [open /etc/hotplug/usb.usermap a]
                    puts $fusb "# Apogee ALTA-U series CCD camera"
		    puts $fusb "apogee-altau  0x0003      0x125c   0x0010    0x0000       0x0000      0x00         0x00            0x00            0x00            0x00               0x00               0x00000000"
		    puts $fusb "apogee-ascent  0x0003      0x125c   0x0020    0x0000       0x0000      0x00         0x00            0x00            0x00            0x00               0x00               0x00000000"
                    close $fusb
		    exec cp /opt/apogee/config/apogee-altau /etc/hotplug/usb/.
		    exec cp /opt/apogee/config/apogee-ascent /etc/hotplug/usb/.
		    exec chmod 755 /etc/hotplug/usb/apogee-altau
		    exec chmod 755 /etc/hotplug/usb/apogee-ascent
		    set it [ tk_dialog .d "ALTA-U" "ALTA-U hotplug support configured" {} -1 OK]
                   } else {
  		     set it [ tk_dialog .d "ALTA-U" "ALTA-U models require hotplug support,\n please install it first" {} -1 OK]           
                   }
                 }
                 exit
               }
           NET { set it [ tk_dialog .d "ALTA-E" "ALTA-E models do not require kernel module installation" {} -1 OK]
                 exit
               }
      }
      set fapp [open /opt/apogee/apps/tcl/scripts/tele_init.tcl a]
      puts $fapp "set SCOPE(camera) \"Apogee [file rootname [file tail $cfg]]\"" 
      close $fapp
}

#
# Check for a known installation
#

set modpath none
set rcfile rc.local
set rcdir /etc/rc.d

if { [file exists /etc/fedora-release] } {
   set fin [open /etc/fedora-release r] ; gets $fin rec ; close $fin
   set v fedora[lindex $rec 3]
   if { [file exists /opt/apogee/src/apogee/module/$v] } {
      puts stdout "Located prebuilt modules ($v)"
      set modpath /opt/apogee/src/apogee/module/$v
   }
   if { [file exists /opt/apogee/src/apogee/module26/$v] } {
      puts stdout "Located prebuilt modules ($v)"
      set modpath /opt/apogee/src/apogee/module26/$v
      set kext "ko"
   }
}

if { [file exists /etc/redhat-release] } {
   set fin [open /etc/redhat-release r] ; gets $fin rec ; close $fin
   set v redhat[lindex $rec 4]
   if { [file exists /opt/apogee/src/apogee/module/$v] } {
      puts stdout "Located prebuilt modules ($v)"
      set modpath /opt/apogee/src/apogee/module/$v
   }
}

if { [file exists /etc/mandrake-release] } {
   set fin [open /etc/mandrake-release r] ; gets $fin rec ; close $fin
   set v mandrake[lindex $rec 3]
   set kv [lindex [split [exec uname -r] .] 1]
   if { $kv < 5 } {
     if { [file exists /opt/apogee/src/apogee/module/$v] } {
        puts stdout "Located prebuilt modules ($v)"
        set modpath /opt/apogee/src/apogee/module/$v
     }
   } else {
     set kext "ko"
     if { [file exists /opt/apogee/src/apogee/module26/$v] } {
        puts stdout "Located prebuilt modules ($v)"
        set modpath /opt/apogee/src/apogee/module26/$v
     }
   }
}

if { [file exists /etc/SuSE-release] } {
   set fin [open /etc/SuSE-release r] ; gets $fin rec ; close $fin
   set v suse[lindex $rec 2]
   if { [file exists /opt/apogee/src/apogee/module/$v] } {
      puts stdout "Located prebuilt modules ($v)"
      set modpath /opt/apogee/src/apogee/module/$v
   }
   if { [file exists /opt/apogee/src/apogee/module26/$v] } {
      puts stdout "Located prebuilt modules ($v)"
      set modpath /opt/apogee/src/apogee/module26/$v
      set kext "ko"
   }
   set rcfile boot.local
   set rcdir /etc/init.d
}

if { [file exists /etc/sun-release] } {
   set fin [open /etc/SuSE-release r] ; gets $fin rec ; get $fin rec; close $fin
   set v sun[lindex $rec 2]
   if { [file exists /opt/apogee/src/apogee/module/$v] } {
      puts stdout "Located prebuilt modules ($v)"
      set modpath /opt/apogee/src/apogee/module/$v
   }
}

if { $modpath == "none" } {
      set it [ tk_dialog .d "Load failure" "There is no prebuilt module\n - building a custom version" {} -1 OK]
      cd /opt/apogee/src/apogee/module
      exec make
      set modpath /opt/apogee/src/apogee/module
}

switch $v {
	mandrake8.1 -
	mandrake8.2 -
	mandrake9.0 -
	mandrake9.1 -
	redhat6.2 -
	redhat7.0 -
	redhat7.1 -
	redhat7.2 -
	redhat7.2smp -
	redhat7.3 -
	redhat8.0 -
	redhat9.0 -
	suse7.0 -
	suse7.3 -
	suse8.0 -
	suse8.2 {
		   exec cp -f /opt/apogee/lib/oldversions/* /opt/apogee/lib/.
		}
}

catch {exec ldd /opt/apogee/bin/xpans} xpres
set xpok [lsearch $xpres "`GLIBC_2.3'"]
if { $xpok > -1 } {
	exec cp -f /opt/apogee/bin/ds9-old /opt/apogee/bin/ds9
	exec cp -f /opt/apogee/bin/xpans-old /opt/apogee/bin/xpans
	exec cp -f /opt/apogee/bin/xpaset-old /opt/apogee/bin/xpaset
	exec cp -f /opt/apogee/bin/xpaget-old /opt/apogee/bin/xpaget
}


if { $modpath != "none" } {
   catch {exec /sbin/rmmod apogee$i}
   if { $marg == "" } {
	   exec /sbin/insmod $modpath/apogee$i.$kext
   } else {
	   exec /sbin/insmod $modpath/apogee$i.$kext $marg
   }
   set ok 0
   catch {
     set mods [exec /sbin/lsmod | grep apogee$i]
     set ok 1
   }
   if { $ok } {
   } else {
      set makeres ""
      set vers [split [exec uname -r] .]
      set major [lindex $vers 0]
      set minor [lindex $vers 1]
      if { $major == 2 && $minor > 4 } {set kext "ko"}
      set it [ tk_dialog .d "Load failure" "The chosen module failed to load\n - building a custom version" {} -1 OK]
      cd /opt/apogee/src/apogee/module
      exec rm *.o *.ko
      if { $kext == "o" } {
        catch {exec make} makeres
      } else {
        if { [file exists /usr/src/linux] == 0 } {
 	        set it [ tk_dialog .d "Compile failure" "The module cannot be compiled\n - You need to install the linux kernel sources\n corresponding to the currently running kernel,\n at /usr/src/linux" {} -1 OK]
        } else {
		catch {make -C /usr/src/linux SUBDIRS=$PWD modules} makeres
 	}
      }
      if { [file exists apogee$i.$kext] == 0 } {
         puts stdout "$makeres"
	 exit
      }
      set modpath /opt/apogee/src/apogee/module
      if { $marg == "" } {
	      exec /sbin/insmod $modpath/apogee$i.$kext
       } else {
	      exec /sbin/insmod $modpath/apogee$i.$kext $marg
      }
      set ok 0
      catch {
        set mods [exec /sbin/lsmod | grep apogee$i]
        set ok 1
      }
   }
   if { $ok } {
      set it [ tk_dialog .d "Load OK" "The chosen module loads OK\n - Do you want it loaded automatically after a reboot?" {} -1 No Yes]
      if { $it ==1 } {
         cd $rcdir
         if { [file exists $rcfile.preapogee] } {
            exec cp $rcfile.preapogee $rcfile
         }
         exec cp $rcfile $rcfile.preapogee
         set fout [open $rcfile a]
         puts $fout " "
         puts $fout "echo \"Loading Apogee CCD driver module\""
         puts $fout "/sbin/insmod $modpath/apogee$i.$kext $marg"
         close $fout
      }
      set fout [open /usr/bin/apogee_load w]
      puts $fout "#!/bin/sh"
      puts $fout "echo \"Loading Apogee CCD driver module\""
      puts $fout "/sbin/insmod $modpath/apogee$i.$kext $marg"
      close $fout
      set fout [open /usr/bin/apogee_remove w]
      puts $fout "#!/bin/sh"
      puts $fout "/sbin/rmmod apogee$i"
      close $fout
      exec chmod 755 /usr/bin/apogee_load
      exec chmod 755 /usr/bin/apogee_remove
      set it [ tk_dialog .d "Done" "The module can be manually loaded or removed\n using the commands\n\n/usr/bin/apogee_load\n/usr/bin/apogee_remove" {} -1 OK]
   }
}


exit








