

#
# This script assists the user in selecting the appropriate usb hotplugging
# for an Apogee filter wheel driver
#
wm withdraw .

set it 1
                 set marg ""
                 if { [file exists /usr/share/hal/fdi] } {
                     exec cp /opt/apogee/config/61-filterwheel-altausb.fdi /usr/share/hal/fdi/information/20thirdparty/.
                     exec cp /opt/apogee/config/61-altausb-filterwheel-policy.fdi /usr/share/hal/fdi/policy/20thirdparty/.
                     exec mkdir -p /usr/libexec
                     exec cp /opt/apogee/config/altafw-set-procperm /usr/libexec/.
		     set it [tk_dialog .d "AFW" "Apogee Filterwheel HAL support configured" {} -1 OK]
                     exit 
                 }
                 if { [file exists /etc/udev/udev.conf] } {
		     set fout [open /etc/udev/rules.d/61-alta.rules w]
		     puts $fout "
# alta.rules a udev rules file for apogee alta usb filterwheel 
SUBSYSTEM!=\"usb_device\", ACTION!=\"add\", GOTO=\"alta_rules_end\" 
# Apogee AFW
SYSFS\{idVendor\}==\"125c\", SYSFS\{idProduct\}==\"0100\", SYMLINK+=\"altafw-\%k\", MODE=\"666\" 
LABEL=\"alta_rules_end\"
"
		     close $fout
		     set it [ tk_dialog .d "AFW" "Apogee Filterwheel udev support configured" {} -1 OK]   
                 } else {
   		   if { [file exists /etc/hotplug/usb.usermap] } {
                    set fusb [open /etc/hotplug/usb.usermap a]
                    puts $fusb "# Apogee AFW series Filterwheel"
		    puts $fusb "apogee-afw  0x0003      0x125c   0x00100    0x0000       0x0000      0x00         0x00            0x00            0x00            0x00               0x00               0x00000000"
                    close $fusb
		    exec cp /opt/apogee/config/apogee-altafw /etc/hotplug/usb/.
		    exec chmod 755 /etc/hotplug/usb/apogee-altafw
		    set it [ tk_dialog .d "AFW" "Apogee Filterwheel hotplug support configured" {} -1 OK]
                   } else {
  		     set it [ tk_dialog .d "AFW" "Apogee AFW models require hotplug support,\n please install it first" {} -1 OK]           
                   }
                 
                 }


