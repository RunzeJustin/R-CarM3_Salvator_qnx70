#ifndef VSPI_H
#define VSPI_H

#include <sys/siginfo.h>

const struct sigevent* vspi0_intr (void* arg, int id);

#endif
