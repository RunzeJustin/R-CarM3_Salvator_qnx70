#ifndef VSPD_H
#define VSPD_H

#include <sys/siginfo.h>

const struct sigevent* vspd0_intr (void* arg, int id);
const struct sigevent* vspd1_intr (void* arg, int id);
const struct sigevent* vspd2_intr (void* arg, int id);


#endif
