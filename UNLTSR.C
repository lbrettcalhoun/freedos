#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compile with Open Watcom C. IA-16 GNU C compiler does not have dos.h
// Don't forget to setup Open Watcom C with owsetenv.bat
// wcl -ldos unltsr.c
// wcl -ldos -d2 -"debug all" unltsr.c for debug with wd

// Use unsigned short cause the structure needs 2 bytes for wOwner and wSz
// Got to tell the compiler not to pad the struct
#pragma pack(push, 1)
struct DOS_MCB {
  unsigned char cSig;		// Type: M or Z
  unsigned short wOwner;	// PSP
  unsigned short wSz;		// Size in paragraphs (16 bytes)
  unsigned char res[3];		// Reserved
  unsigned char szOwn[8]; 	// Program name
};
#pragma pack(pop)

int main (int argc, char **argv) {

  unsigned short mcb_seg, env_seg;
  struct DOS_MCB far *mcb;
  union REGS regs = {0};	// CPU registers
  struct SREGS sregs = {0};	// Segment registers
  int i;

  // Get the List of Lists: The list of MCBs. For most other DOS
  // functions, you would first need to load the segment registers
  // into sregs with segread(). But you don't need to do this for
  // the undocumented function 0x52. 
  regs.h.ah = 0x52;
  intdosx(&regs, &regs, &sregs);

  // Get the first MCB; it sits 2 bytes in front of the LoL
  // Note the use of *: Cast output of MK_FP as unsigned int __far *
  // and then dereference with * to get the value (which is an address)
  mcb_seg = *((unsigned short __far *) MK_FP(sregs.es, regs.x.bx - 2));

  // Walk the MCB chain
  while (1) {
    
    mcb = (struct DOS_MCB __far *) MK_FP(mcb_seg, 0);
    
    // Stop when we reach the end of the chain
    if (mcb->cSig == 'Z') {
      printf("\nEnd of chain!\n\n");
      break;
    }

    // PSP 0 is free memory and 8 is DOS
    if (mcb->wOwner != 0 && mcb->wOwner != 8 && mcb->cSig == 'M') {
      printf("\n");
      printf("MCB Segment:  %#4x\n", mcb_seg);
      printf("Type:         %c\n", mcb->cSig);
      printf("PSP:          %#4x\n", mcb->wOwner);
      printf("Size:         %u\n", mcb->wSz);
      printf("Program name: ");
      if (mcb->wSz == 43) { 
      	printf("N/A (environment)\n");
      } else {
        for (i = 0; i < 8; i++) {
  	  if (mcb->szOwn[i] >= 'A' && mcb->szOwn[i] <= 'Z') {
            putchar(mcb->szOwn[i]);
          } else {
            break;
          }
        }
	if (_fstrncmp(mcb->szOwn,"MEMTSR", 6) == 0) {
          env_seg = *((unsigned short __far *) MK_FP(mcb->wOwner, 0x002C));
	  printf("\n");
	  printf("Env:          %#4x\n", env_seg);
	  printf("Unloading environment ...\n");
          segread(&sregs);
	  regs.h.ah = 0x49;
	  sregs.es = env_seg;
	  intdosx(&regs, &regs, &sregs);
	  if (regs.w.cflag) {
	    printf("Error unloading environment!\n");
	  }
	  printf("Unloading program ...\n");
	  regs.h.ah = 0x49;
	  sregs.es = mcb->wOwner;
	  intdosx(&regs, &regs, &sregs);
	  if (regs.w.cflag) {
	    printf("Error unloading program!\n");
	  }
	  break;
	}
	printf("\n");
      }
    }

    // Advance to next MCB
    mcb_seg = mcb_seg + 1 + mcb->wSz;

  }

  return 0;
}
