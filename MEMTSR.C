#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

// Compile with Open Watcom C. IA-16 GNU C compiler does not have dos.h
// Don't forget to setup Open Watcom C with owsetenv.bat
// Use the tiny memory model and generate a COM file; this will
// put everything (code, data, etc) into a single 64K segment which can be
// loaded into residency
// wcl -mt -lcom memtsr.c

struct T_TSR_DATA {
  char signature[12];
  char tsr_data[32];
};

static struct T_TSR_DATA tsr_data = {
  "MEMTSR01",
  "This is the way!"
};

int main (int argc, char **argv) {

  unsigned int total_bytes, paragraphs = 0;

  total_bytes = FP_OFF(&tsr_data) + sizeof(struct T_TSR_DATA);
  paragraphs = (total_bytes / 16) + 1;

  printf("Signature loaded at offset: %x\n", FP_OFF(&tsr_data));
  printf("Going resident with %u paragraphs ...\n", paragraphs);

  // TSR
  _dos_keep(0,paragraphs);

  return 0;
}
