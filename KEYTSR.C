#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

// Example of TSR with chained keyboard interrupt to capture CTRL-ALT-F12
// Put red boxes in the upper portion of the screen
// Compile with Open Watcom C. IA-16 GNU C compiler does not have dos.h
// Don't forget to setup Open Watcom C with owsetenv.bat
// Generate an EXE file for this one and use small memory model
// Also, turn off stack detection (to help avoid crashes in interrupts)
// wcl -ms -ldos -s keytsr.c

#define KEYBINT 0x09
#define KEYPORT 0x60
#define KEYF12 0x58
#define BIOSDATA 0x0040
#define SHFTFLGS 0x0017
#define CTLALT 0x000C
// Text Mode Video Memory
#define TMVM 0xB800
#define REDBOX 0x0CDB

void (__interrupt __far *okeybint) (void);

void __interrupt __far nkeybint (void) {

  int i;
  unsigned int scan_code;
  unsigned short __far *shift_flags;
  unsigned short __far *video_memory;

  scan_code = inp(KEYPORT);
  
  if (scan_code == KEYF12) {

    shift_flags = (unsigned short __far *) MK_FP(BIOSDATA, SHFTFLGS);

    if ((*shift_flags & CTLALT) == CTLALT){
      video_memory = (unsigned short __far *) MK_FP(TMVM,0x0000);
      for (i = 0; i < 320; i++) {
        *(video_memory + i) = REDBOX;
      }
    }
  }

  // Make sure you chain the original keyboard interrupt!!!
  _chain_intr(okeybint);

}

int main (int argc, char **argv) {

  okeybint = _dos_getvect(KEYBINT);
  _dos_setvect(KEYBINT, nkeybint); 

  printf("Going resident ... CTRL-ALT-F12 to test\n");

  // TSR ... just use 64K for the size ... should be big enough for small
  // memory model
  _dos_keep(0, 0x1000);

  return 0;
}
