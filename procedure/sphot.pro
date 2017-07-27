! Procedure to do aperture photometry on set of standard star frames
parameter string=filt inter fsbias fflat fdt fpixarea

printf 'Filter: {filt}'
printf 'Superbias from buffer: %i3' fsbias
printf 'Flat from buffer: %i3' fflat
printf 'Shutter shading from buffer: %i3' fdt
printf 'Pixel areas from buffer: %i3' fpixarea

if inter>-2
  string junk '?OK to continue (CTRL-C to quit): '
end_if


! Star and sky radii to use
rad=10/scale
s1=15/scale
s2=25/scale

opt ma=maxbad

string data '{root}.{filt}'

open input '{data}'
stat nlines=count[input]
$'rm' ./junk.
$touch ./junk.
do i=1,nlines
  read input
  jd=@input.1
  string file '{root}.%i3.3' jd
  string out '{root}s%i3.3' jd
  if inter<=1
   if inter==1
     error goto moveon
     open test {out}.nst
     goto nextframe
   end_if
   moveon:

   call reduce jd fsbias fflat fdt 0 fpixarea
   copy 1 $ireadbuf

   obsnum=jd
   fits 1 int=obsnum obsnum
   haveobs:
   sky 1 box=10
   if sky<0
     sky=100.
   end_if
   if (inter==1)
     tv 1 z=sky-50. l=150. noerase
     jd=@input.2
     if jd==0
       printf 'Unknown standard star: %i4' @input.1
       pause
     end_if
     mark new id=jd obsnum circ=rad radius=3/scale
     mark exit circ=s1
     mark exit circ=s2
     err goto nextframe
     save coo={out} sr=sr[1]-1 sc=sc[1]-1 low=-300 high=maxbad
   else_if (inter<=-2)
     box 1 n=150 cr=256 cc=256
     abx 1 1 high_row=hr high_col=hc high=high
     jd=@input.2
     if jd==0
       printf 'Unknown standard star: %i4' @input.1
       pause
     end_if
     if high>sky+5*skysig
       box 1 n=10 cr=hr cc=hc
       automark 1 range=high-10,high+10 box=1 new id=jd obsnum
       tv 1 z=sky-50. l=150. noerase
       if inter<-2
        mark circ=rad 
       else
        mark circ=rad exit
       end_if
     else
       goto nextframe
     end_if
     err goto nextframe
     save coo={out} sr=sr[1]-1 sc=sc[1]-1 low=-300 high=maxbad
   else
     err goto nextframe
     get coo={out} sr=sr[1]-1 sc=sc[1]-1 low=-300 high=maxbad
     automark 1 auto radius=3/scale
     if inter==-1
       tv 1 z=sky-50. l=150. noerase
       mark exit circ=rad
       mark exit circ=s1
       mark exit circ=s2
       pause
     end_if
     save coo={out} sr=sr[1]-1 sc=sc[1]-1 low=-300 high=maxbad
   end_if
   daofiles coo={out} mag={out} pro={out}
   unmask
   clip  1 min=lowbad maskonly
   photom 1 rad=rad skyrad=s1,s2 rn=rn gain=gain mask 
   unmask
   shortap
   $'rm' {out}.ap
  end_if
  printf '{out}.nst' >>./junk.
  nextframe:
end_do
magaver norm id file=./junk. out={root}{filt}.mag name={filt}

daofiles none
END
