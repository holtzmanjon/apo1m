#!/bin/bash
printenv SHELL
date
echo $#argv
nbell=5
#if ( $#argv < 1 ) then
#  set nbell = 30
#else
#  set nbell = $1
#endif
echo nbell
echo $nbell
i=0
echo $i
while [ $i -lt $nbell ] ; do
  echo $i $nbell
  echo -n  >/dev/tty
  usleep 300000
  echo -n  >/dev/tty
  usleep 300000
  echo -n  >/dev/tty
  sleep 1
  i=$[$i+1]
done

