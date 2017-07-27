parameter iproc
if iproc==0
  printf 'Choose default procedure directory (default is reduce): '
  printf ' 0: /home/holtz/procedure/reduce '
  printf ' 1: /home/holtz/procedure/1m '
  printf ' 2: /home/holtz/procedure/3.5m '
  printf ' 3: /home/holtz/procedure/wfpc2 '
  printf ' 4: /home/holtz/procedure/dust '
  printf ' 5: /home/holtz/procedure/ctio '
  string option '?Option (default is 0): '
  err goto default
  opt={option}
else
  opt=iproc
end_if
if (opt==1)
  setdir pr dir=/home/holtz/procedure/1m
  setup apo
else_if (opt==2)
  setdir pr dir=/home/holtz/procedure/3.5m
  remote=1
  setup apo
else_if (opt==3)
  setdir pr dir=/home/holtz/procedure/wfpc2
else_if (opt==4)
  setdir pr dir=/home/dust/proc
else_if (opt==5)
  setdir pr dir=/home/holtz/procedure/ctio
  setup ctio
end_if
goto finish
default:
setdir pr dir=/home/holtz/procedure/reduce
finish:
end
