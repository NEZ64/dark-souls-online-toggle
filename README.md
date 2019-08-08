# dark-souls-online-toggle

toggles online mode in release, steamworks, and debug versions

to build:
# gcc -Wl,-subsystem,windows online.c resource.res -o DarkSoulsOnlineToggle.exe

make sure to do
# windres -O coff -i resource.rc -o resource.res
if changing the .rc file
whatever, nobody will see this








FrankerZ
