amoeba
a demo by excess
released at underscore 2002 (1st place), may 2002
final version released june 2002

___________________________________________________ demo

as with the last demo, we've been working international,
with gloom designing the demo in norway while we were
busy coding and ironing out the last bugs at the
partyplace in sweden.. and as if putting this demo
together wasn't hard enough as it is, gloom actually
had to -break into- his own office to work on it during
the night.

the things we do for the demoscene.. :)

anyways; here it is - we hope you like it.

________________________________________________ credits

gloom 				design, music, graphics
<bent@gathering.org>

sesse 				code
<sgunderson@bigfoot.com>

neuromancer			graphics
<neuromancerskulls@poczta.onet.pl>

kusma 				graphics, code
<eflund@online.no>	  

tick 				3d modelling
<tick@psychoproject.net>	  

satcom101 			music
<mattias@satcom101.com>

________________________________________________ machine

should work more or less equally well on linux and
win32. opengl and a relatively fast 3d accelerator
needed -- this final version has been tested on various
geforce cards (including geforce2go), matrox
g200/g400/g450/g550 (g200/g400 only under linux, no
opengl support for those on windows), radeon (also
mobile), trident cyberblade (!), tnt2, i815 etc. etc.
etc., so we hope most people should be able to watch the
demo. if you don't have hardware stencil buffers some
parts (like the shadows) will be incredibly slow,
though... tough luck. perhaps a -nostencil option next
time.

_________________________________________________ binary

the default settings should be okay, just click the
binary for windows or execute it (remember chmod +x)
under linux (the -oldg++ version is for those of you who
still don't have libstdc++ v4 installed -- sorry,
libstdc++ v3 isn't supported anymore, we can't have
_three_ different versions ;-) ). if you use linux and
have libgtk+ somewhere, we have a colourful
configuration dialog, if not, you can still use the good
old unfancy (but working) command line switches. try
"-help", for instance. ;-) the final adds a windows
config dialog too, so you can enjoy easy setup on both
platforms (if you don't want the dialog box, simply
specify command line switches and they will override the
dialog box code). (note: most drivers don't expose
stencil visuals if you're in 16bpp mode. try changing
to 24/32bpp if you get errors or the box scene is very
slow. note to matrox users on windows: if the shadows
are buggy, check that "use 32-bit z-buffers" is checked
in the powerdesk control panel.)

_________________________________________________ source

yes, you guessed it, the src/ directory includes
complete source. see gpldemo.txt for more information.
don't complain to us if it doesn't compile -- demosource
was never intended to be portable anyhow. (the final
should be slightly better than the party version, though
:-) )

_________________________________________________ we use

linux. windows. vim. ultraedit. expat. freetype.
libjpeg. libpng. gcc. (sometimes msvc.) various ogg
vorbis libraries (go vorbis!). gimp. photoshop. acid.
soundforge. coke. beer. (no drugs.) computers.

__________________________________________________ party

for some reason, underscore2002 is... dark. after we
logged in (the sign-in terminals were showing all
passwords in cleartext in the URL and thus in the
history too, nice going ;-) ) we found a more or less
empty place, tried to get used to the dark and set up
our equipment. of course, sesse almost lost his digicam,
but the taxi driver had found it (and we had to pay for
him to drive back with it -- boo). kusma and the others
went to drink as always, while some weirdo in the
democrew decided to play with some sound generator so
nobody could go to sleep in the main hall... of course,
the showers were good to sleep in -- but cold. 3/4th of
the norwegian posse have forgotten their sleeping bags,
but fortunately, some swede was nice enough to lend out
a sleeping mat, so we didn't have to get to know the
floor _that_ well... we're actually handing in something
like five productions from four people here, and as
usual, lug00ber looks like he could go on stage quite a
few times.

the network here is, well, almost non-existant, but
heck, it's a sceneparty, no lame-ass lan party (there
are still people playing games, here, though). still
could use some light though, the weather outside is real
good this weekend and there's a barbeque planned or
something... oh well, signing off, hopefully gloom will
finish his stuff in norway soon so we can get to an
internet café or something, fetch the stuff, finish the
demo and just wait for the show to start :-)

_______________________________________________ party II

(written for the final, we didn't have time to update
readme.txt before release)

okay, so the problem was getting the data from gloom to
the party. we talked to lator/dxm (stefan), member of
the underscore democrew, and he agreed to fetch some
files for us later that day when he was going home.
after some cell phone stress (the (borrowed) cellphone
sesse was using didn't work in sweden) we managed to get
in touch with gloom, and get an url... there was little
to do with the demo, since the source was already
largely ready, so the norwegian posse went out for the
usual scene-grill (although it rained, it was definitely
a success), keeping an eye on the arriving cars, waiting
for lator to return.

when he finally returned, he just gave us a notebook,
saying "the data is there", so we took it to our place
in the hall and tried to transfer the files. the first
problem was finding some way to transfer the files --
sesse didn't have samba installed on his linux machine,
so "windows file sharing" was out of the question... the
portable (running windows xp home edition -- swedish)
didn't have anything useful on it except ftp.exe, so we
started up an ftp daemon and started transferring the
files... it froze after a few hundred kB. we tried like
_everything_ -- switching ftp daemon, switching ip
ranges, splitting the file into multiple smaller ones,
using samba the other way (ie. smbclient, with the
notebook as server -- enabling file sharing in xp isn't
exactly intuitive, the "shared files" folder isn't
shared by default even though the help tells you it is),
etc... we tried moving the files to the win2000 notebook
nelius had brought with him -- but still _exactly the
same_ problem. we even tried uploading, cancelling and
resuming multiple times, which didn't work out either
(the file got corrupted).

we were quite out of ideas when neon suggested that we
could use the usb compactflash reader for sesse's
digicam (which you might recall was lost earlier) to
transfer the files... it worked first time, and quickly.
THEN we discovered that gloom suddenly had used the font
support in the demolib, which had been removed (ie.
deleted) only hours earlier -- fortunately, there was an
old backup lying around still containing the code
(underscore did, as you've probably noticed by now, not
have internet access, otherwise we could simply have
fetched an old copy from somewhere -- hadn't the CVS
server, which was situated in germany, been down, of
course ;-) )... problem fixed. but gloom still wasn't
content -- he wanted to transfer an update to the demo
script, and neon's gsm phone (which _did_ work for voice
calls) didn't work for data calls abroad... fortunately,
inm was nice enough to transfer the remaining 39kB using
his cell phone phoning home (of course, after his
machine had had major problems, having to phone home to
get somebody to reset mgetty, having problems with
gnokii etc.), and after getting the remaining files over
(FTP worked well first time this time, happily --
probably the network card was what was fscked, by the
way, we tried multiple different cables ;-) ) we could
finalize the demo, fix a few remaining bugs (although
there were still some left, including a radeon mobility
hang we still haven't managed to fix/reproduce --
probably caused by outdated drivers, and it magically
fixed itself later on anyhow ;-) ), test it on the compo
machine (or rather, test it on the _second_ compo
machine, we never got to test it properly on the first
one ;-) ) and sit back and wait for the demo show :-)
(we even reached the second deadline this time, which
was more or less identical to the original showing time
-- earlier, we've been up to twelve hours late ;-) ) we
really thought we could make a demo without all the
stress this time, though, but it turned out to be almost
exactly as bad as usual ;-)

amoeba placed 1st in the pc demo competition at
underscore, something we were very pleased with,
although the prize was a ten year old old sparcstation
we still haven't managed to get any contact with ;-)

______________________________________________ party III

perhaps the most interesting part of the party was the
trip home -- after taking the bus to the train station,
we discovered that the train was crammed, so we asked
for some place to put our luggage while looking for
somewhere to sit -- something we later discovered would
have been quite hopeless, as there was people almost
_everywhere_. some were crouched in a corner for over
four hours straight...

but... sceners always know a way. we soon discovered
that the luggage room would be ideal to sit in -- it
wasn't too clean, it bumped a bit and the lights were
flickering, but at least there was plenty of space, so
we simply closed the door, found something comfortable
to sit on and opened lug00ber's portable so we could get
some music... so while the others were sitting outside
in all sorts of weird uncomfortable positions were we
quite happy, having a lot of fun with the "expedition
robinson" game (which lug00ber and kusma won at
underscore, and, by the way, I'd guess nobody will ever
play again ;-) ). the friendly train personnel even
allowed us to recharge the portable when the battery was
empty, so we had music almost the whole journey (if the
220v hadn't been in a different wagon, we would have
started up the flatscreen and sesse's pc, and watched
movies or something ;-) ). people outside were getting
more and more irritated -- now and then, we even went
around taking pictures of tired and beat people. they
must have thought we were drunk or something, and they
must _definitely_ have heard the music and the laughter.
nobody really understood why none of them came into the
room asking to sit there with us ;-)

pictures, video, more text etc. will be included in the
forthcoming party report. ;-)

_________________________________________________ efnet?

*** Now talking in #underscore
<gloom> stefan?
<@inm> gloom
<@inm> Känner inte jag igen ditt nick ? 
<gloom> bent
<gloom> fra excess
<gloom> det var du som skulle ta med demoen vår til
        underscore?
<gloom> ..som jeg nettopp fikk mail fra?
<@inm> Nope.. vet jag ingenting om
<@inm> Tyvärr
<gloom> okei, men vet du hvem stefan er?
<@inm> Sorry, nope.. 
<gloom> lator / deus ex machina?
*** stefandxm (stefan@as2-4-1.bdn.g.bonet.se) has joined
    #underscore
<stefandxm> gloom
<stefandxm> !
<gloom> der
<gloom> ah
<gloom> takk gud.. :)
<stefandxm> ircnet i meant
<gloom> hehe
<stefandxm> but ok ;)

_________________________________________________ greets

acme, appendix, array, bypass, coma, contraz, darkzone,
deux ex machina, ephidrena, fairlight, gollum, granma,
haujobb, inf, komplex, kvasigen, moondreamers,
nocturnal, purple, progress, prone, proxima, razor 1911,
rebels, sorrox, skulls, spaceballs, squirrelz, tbl,
therapii, the silents, tls, unique, yaphan, #scene.no,
#amigascne

_________________________________________________ sweden

det heter "jordbær" og "gulrot"! og ikke minst...
"pølse"! mens vi snakker om pølse, hvorfor har ikke
dette landet lompe? på togstasjonen henger det plakater
med "slut" rett over der det står jenter... brødet er
dyrt også, og sære svensker forstår ikke norsk engang
(bytte over på engelsk når noen snakker til deg)... og
angående jämtland og härjedalen -- få tilbake støffet
vårt, for helvete!!! sees på rrrrrymdtorget...

__________________ 3.14159265358979323846264338327950288

yes, it's pi. no, we didn't look it up.

__________________________________________________ final

this final is actually the first we got out -- a lot of
problems have been fixed, most of them in the
performance area, but also in the source release, some
size optimizations (the final version shaves some 3MB
off the party version's .dat file), some artifact fixes
and some (at times) really weird compatibility fixes
(shame on companies making opengl drivers that don't
return errors on non-supported flags, but instead decide
to magically crash at more or less random spots later
on). in short, it should be quite a lot nicer to your
system, especially on fillrate- and texture
memory-limited systems. in addition, we have done the
design-changes that were planned for the original 
release, but never got finished in time. deadlines suck.

___________________ excess - a very very ninja demogroup
