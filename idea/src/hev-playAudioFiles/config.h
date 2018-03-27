#ifndef	CONFIG_H
#define	CONFIG_H

#define __linux__ 1
#define HAVE_ALSA_ASOUNDLIB_H 1

#ifdef sgscomment
#define	HAS_FCNTL_H	1		/* /usr/include/fcntl.h		*/

#define	HAS_STDLIB_H	1		/* /usr/include/stdlib.h	*/
#define	HAS_FSTAT 	1		/* fstat syscall		*/
#define	HAS_FCHMOD 	1		/* fchmod syscall		*/
#define	HAS_CHMOD 	1		/* chmod syscall		*/
#define	HAS_FCHOWN 	1		/* fchown syscall		*/
#define	HAS_CHOWN 	1		/* chown syscall		*/

#define	HAS_STRING_H 	1		/* /usr/include/string.h 	*/

#define	HAS_UNISTD_H	1		/* /usr/include/unistd.h	*/
#define	HAS_UTIME	1		/* POSIX utime(path, times)	*/
#define	HAS_UTIME_H	1		/* UTIME header file		*/


#endif

#endif	/* CONFIG_H */

