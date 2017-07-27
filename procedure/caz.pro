printf 'umask 000' >./caz.csh

printf 'echo priv >/home/export/tocc/tocccmd.doc' >>./caz.csh
printf 'touch /home/export/tocc/tocccmd.fin' >>./caz.csh
printf 'sleep 5' >>./caz.csh

printf 'echo clyde >/home/export/tocc/tocccmd.doc' >>./caz.csh
printf 'touch /home/export/tocc/tocccmd.fin' >>./caz.csh
printf 'sleep 5' >>./caz.csh

printf 'echo caz >/home/export/tocc/tocccmd.doc' >>./caz.csh
printf 'touch /home/export/tocc/tocccmd.fin' >>./caz.csh
printf 'sleep 5' >>./caz.csh

printf 'echo y >/home/export/tocc/tocccmd.doc' >>./caz.csh
printf 'touch /home/export/tocc/tocccmd.fin' >>./caz.csh
printf 'sleep 480' >>./caz.csh

$csh caz.csh
end
