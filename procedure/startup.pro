open com2master ./com2master

alias focrun 'call focrun 1'
alias gfocrun 'call focrun 2'
alias saot 'call sao 0'
alias saoa 'call sao 1'
alias hipt 'call hip 0'
alias hipa 'call hip 1'
alias saofoc 'call sao 2'
alias saofoca 'call sao 3'
alias send 'call send'
setup computeair
!send +display
!send g+display
!send '-xfer'
!send 'g-xfer'
!send 'td 30'

scidet=17 guidedet=33
scidet=32 
pi=3.14159 
gloc0=1200 gfocus=0
focref=-7557 tref=15.5  !with 9/8 spacer
focref=-5000 tref=7     !Camera in normal position
focref=-4700 tref=1.1     !Camera in normal position
focref=-5490 tref=10     !Camera in normal position
focref=-4570 tref=12     !Camera in normal position
focref=-3400 tref=8.3     !Camera in normal position
focref=-220 tref=2
focref=-4000 tref=5
focref=-3700 tref=5
focref=-3800 tref=11
focref=-4100 tref=3.9
focref=-2500 tref=2
focref=-7800 tref=13
focref=-8100 tref=13
focref=-8350 tref=13

nfill=0

end
