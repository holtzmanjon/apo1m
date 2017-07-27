#
# Script to create a udev configuration
#

if { [file exists /etc/udev/rules.d] } {
   set fout [open /etc/udev/rules.d/11-alta.rules w]
   puts $fout "
SUBSYSTEM!=\"usb_device\", ACTION!=\"add\", GOTO=\"alta_rules_end\"
# Apogee ALTA-U ccd cameras
SYSFS\{idVendor\}==\"125c\", SYSFS\{idProduct\}==\"0010\", MODE=\"0666\"
LABEL=\"cell_rules_end\" SYMLINK=\"altau\"
"
} else {
   puts stdout "No udev capability"
}


