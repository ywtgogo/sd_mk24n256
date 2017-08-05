#ifndef _VERSION_H_
#define _VERSION_H_

#include <stdint.h>


#ifdef	PRODUCE_TEST
	#define VERSION_MAJ 0
	#define VERSION_MIN 0
	#define VERSION_REV 0
	#define VERSION_BUI 1
#else
	#define VERSION_MAJ 1
	#define VERSION_MIN 1
	#define VERSION_REV 0
	#define VERSION_BUI 0
#endif

#define FW_VERSION	(uint32_t)(VERSION_MAJ<<24|VERSION_MIN<<16|VERSION_REV<<8|VERSION_BUI)

typedef struct
{
	uint32_t dt;
	uint32_t hwver;
	char hw_info[24];
	uint32_t swver;
	char sw_info[92];
} version_info;

extern const version_info ver_info;

#endif
