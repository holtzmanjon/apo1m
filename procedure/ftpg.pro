parameter start end

if (end==0)
  end=start
end_if

err call getroot
r={root}

do iftp=start,end

!string machine 'tycho.apo.nmsu.edu'
!string login 'visitor1'
!string password 'astro5mc'
!string imdir '/home/export/images'
!string exten '.fits'
string machine 'uhuru.apo.nmsu.edu'
string login 'observe'
string password 'clr$sks'
string imdir '/home2/observe/{root}'
string exten ' '

string file '{root}g.%i3.3' iftp
string name '{file}{exten}'

nprintf 'machine {machine}' >.netrc
nprintf 'login {login}' >>.netrc
nprintf 'password {password}' >>.netrc
nprintf 'macdef init' >>.netrc
nprintf 'cd {imdir}' >>.netrc
nprintf 'binary' >>.netrc
nprintf 'prompt' >>.netrc
nprintf 'get {name}' >>.netrc
nprintf 'bye' >>.netrc
$cat /home/avalon2/holtz/procedure/3.5m/blank >>.netrc
$printenv HOME >./home.inp
open home ./home.inp
string homedir {home}
close home
$mv .netrc {homedir}
$chmod 600 {homedir}/.netrc
$ftp {machine}
$'rm' {homedir}/.netrc

end_do
end
