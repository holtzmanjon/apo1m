alias tv 'tv flip clip'
alias tvred 'tvred flip clip'
alias tvgreen 'tvgreen flip clip'
alias tvblue 'tvblue flip clip'
alias blink 'blink clip'
alias postit 'postit clip'
alias cop 'copy'
alias at 'rd dst'
alias rw 'rd wfpc'
alias plot 'plot hist pixel'
alias printf nprintf
alias ls '$ls'
alias pwd '$pwd'
alias cp '$cp'
alias mv '$mv'
alias gzip '$gzip'
alias more '$more'
alias load 'call load'
alias phot 'call phot'
alias color 'call color'
alias stack 'call stack'
alias diffphot 'call difphot'
alias setp 'setdir pr dir=procedure'
setdir im dir=/home/holtz/ccd/
setdir da dir=/home/holtz/data/
setdir ph dir=/home/holtz/data/
setdir ps dir=/home/holtz/data/
alias
print dir
pi=arccos[-1]
daofiles file=/home/tcomm/1m/data/default.opt
opt
daofiles none
bell n
setdir pr dir=/home/tcomm/1m/procedure
setup checkair
setup apo
call startup
!call /home/holtz/procedure/getproc 1

end
