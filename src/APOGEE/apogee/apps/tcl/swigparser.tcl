#!/usr/bin/tclsh
#
#  Scan for  _wrap_[type]_GetImage( where type can be ApogeeCam/Alta/CamGen2Base
#  Foreach scan for "  std::vector< uint16_t > data2 ;"
#  and then add
#"
#  int nx, ny;
#  std::vector<uint16_t> pImageData;
#  unsigned short *pccdData;
#"
#
#  then scan for and replace
#       (arg1)->GetImage(*arg2);
#  with 
#"
#       nx = (arg1)->GetRoiNumCols();
#       ny = (arg1)->GetRoiNumRows();
#       pccdData = (unsigned short *)CCD_locate_buffer("tempobs", 2 , nx, ny, 1, 1 );
#       (arg1)->GetImage(pImageData);
#       copy(pImageData.begin(), pImageData.end(), pccdData);
#"
#
#
puts stdout "Editing tcllibapogee_wrap.cpp"
set fin [open tcllibapogee_wrap.cpp r]
set fout [open tcllibapogee_wrap.cpp.modded w]
while { [gets $fin rec] > -1 } {
   set doit 0
   if { [string range $rec 0 24] == "_wrap_ApogeeCam_GetImage(" } {set doit 1}
   if { [string range $rec 0 19] == "_wrap_Alta_GetImage(" } {set doit 1}
   if { [string range $rec 0 26] == "_wrap_CamGen2Base_GetImage(" } {set doit 1}
   if { $doit } {
      puts stdout "    Processing [lindex [split $rec (] 0]"
      puts $fout $rec
      gets $fin rec ; puts $fout $rec
      gets $fin rec ; puts $fout $rec
      gets $fin rec ; puts $fout $rec
      gets $fin rec ; puts $fout $rec
      gets $fin rec ; puts $fout $rec
      puts $fout "  int nx, ny;
  std::vector<uint16_t> pImageData;
  unsigned short *pccdData;
  char tbuffer\[8\];"
      gets $fin rec
      while { [string trim $rec] != "(arg1)->GetImage(*arg2);" } {
         puts $fout $rec ; gets $fin rec
      }
      puts $fout "       nx = (arg1)->GetRoiNumCols();
       ny = (arg1)->GetRoiNumRows();
       strcpy(tbuffer,\"tempobs\");
       pccdData = (unsigned short *)CCD_locate_buffer(tbuffer, 2 , nx, ny, 1, 1 );
       (arg1)->GetImage(pImageData);
       copy(pImageData.begin(), pImageData.end(), pccdData);"
   } else {
      puts $fout $rec
   }
   if { [string trim $rec] == "#define SWIGTCL" } {
      puts $fout "#include \"tcl.h\""
      puts $fout "#include \"ccd.h\""
   }
}

close $fin
close $fout
exec mv tcllibapogee_wrap.cpp tcllibapogee_wrap.cpp.original
exec mv tcllibapogee_wrap.cpp.modded tcllibapogee_wrap.cpp

puts stdout "Original moved to tcllibapogee_wrap.cpp.original"

