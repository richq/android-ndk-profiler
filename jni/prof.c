/* VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
 * Copyright (C) 1999-2003 Forgotten
 * Copyright (C) 2004 Forgotten and the VBA development team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or(at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

/* adapted from gmon.c */
/*-
 * Copyright (c) 1991, 1998 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. [rescinded 22 July 1999]
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <pthread.h>
#include <android/log.h>

#include "prof.h"
#include "gmon.h"
#include "gmon_out.h"
#include "read_smaps.h"

/*
 * froms is actually a bunch of unsigned shorts indexing tos
 */
static int profiling = 3;
static unsigned short *froms;
static struct tostruct *tos = 0;
static long tolimit = 0;
static uint32_t s_lowpc = 0;
static uint32_t s_highpc = 0;
static unsigned long s_textsize = 0;

static int ssiz;
static char *sbuf;
static int s_scale;

static int hist_num_bins = 0;
static char hist_dimension[16] = "seconds";
static char hist_dimension_abbrev = 's';
struct smap *s_smaps = NULL;
static pthread_t s_runner;
volatile long s_pc;

/* see profil(2) where this is describe (incorrectly) */
#define  SCALE_1_TO_1 0x10000L
#define  FREQ_HZ 100

static void systemMessage(int a, const char *msg)
{
	__android_log_print(ANDROID_LOG_INFO, "PROFILING", "%d: %s", a,
			    msg);
}

static void profPut32(char *b, uint32_t v)
{
	b[0] = v & 255;
	b[1] = (v >> 8) & 255;
	b[2] = (v >> 16) & 255;
	b[3] = (v >> 24) & 255;
}

static void profPut16(char *b, uint16_t v)
{
	b[0] = v & 255;
	b[1] = (v >> 8) & 255;
}

static int profWrite8(FILE *f, uint8_t b)
{
	if (fwrite(&b, 1, 1, f) != 1) {
		return 1;
	}
	return 0;
}

static int profWrite32(FILE *f, uint32_t v)
{
	char buf[4];
	profPut32(buf, v);
	if (fwrite(buf, 1, 4, f) != 4) {
		return 1;
	}
	return 0;
}

static int profWrite(FILE *f, char *buf, unsigned int n)
{
	if (fwrite(buf, 1, n, f) != n) {
		return 1;
	}
	return 0;
}

static void check_profil(long frompcindex)
{
	if (sbuf && ssiz) {
		uint16_t *b = (uint16_t *)sbuf;
		int pc = ((frompcindex - (long)s_lowpc) * s_scale)/SCALE_1_TO_1;
		if(pc >= 0 && pc < ssiz)
			b[pc]++;
	}
}

static void *run_profil(void *ptr)
{
	do {
		struct timespec req;
		long pc = s_pc;
		if (pc)
			check_profil(pc);
		/* sleep for 1/100th of a second */
		req.tv_sec = 0;
		req.tv_nsec = 1e7;
		nanosleep(&req, NULL);
	} while (profiling != 3);
	pthread_exit(NULL);
	return NULL;
}

/* Control profiling;
   profiling is what mcount checks to see if
   all the data structures are ready.  */

static void profControl(int mode)
{
	if (mode) {
		/* start */
		profiling = 0;
		s_pc = 0;
		pthread_create(&s_runner, NULL, run_profil, NULL);
	} else {
		/* stop */
		profiling = 3;
		systemMessage(1, "stopping profile thread");
		pthread_join(s_runner, NULL);
	}
}

#define MSG ("No space for profiling buffer(s)\n")

void monstartup(const char *libname)
{
	int monsize;
	char *buffer;
	int o;
	uint32_t lowpc, highpc;
	FILE *self = fopen("/proc/self/smaps", "r");
	s_smaps = read_smaps(self, libname);
	if (s_smaps == NULL) {
		systemMessage(0, "No smaps found");
		return;
	}
	lowpc = s_smaps->lo;
	highpc = s_smaps->hi;
	__android_log_print(ANDROID_LOG_INFO, "PROFILING",
			"Profile %s %x-%x: %d",
			libname,
			lowpc, highpc,
			s_smaps->base);
	/*
	 * round lowpc and highpc to multiples of the density we're using
	 * so the rest of the scaling (here and in gprof) stays in ints.
	 */
	lowpc = ROUNDDOWN(lowpc, HISTFRACTION * sizeof(HISTCOUNTER));
	s_lowpc = lowpc;
	highpc = ROUNDUP(highpc, HISTFRACTION * sizeof(HISTCOUNTER));
	s_highpc = highpc;
	s_textsize = highpc - lowpc;
	monsize = (s_textsize / HISTFRACTION);
	buffer = calloc(1, 2 * monsize);
	if (buffer == NULL) {
		systemMessage(0, MSG);
		return;
	}
	froms = calloc(1, 4 * s_textsize / HASHFRACTION);
	if (froms == NULL) {
		systemMessage(0, MSG);
		free(buffer);
		buffer = NULL;
		return;
	}
	tolimit = s_textsize * ARCDENSITY / 100;
	if (tolimit < MINARCS) {
		tolimit = MINARCS;
	} else if (tolimit > 65534) {
		tolimit = 65534;
	}
	tos =
		(struct tostruct *) calloc(1,
					   tolimit * sizeof(struct tostruct));
	if (tos == NULL) {
		systemMessage(0, MSG);
		free(buffer);
		buffer = NULL;
		free(froms);
		froms = NULL;
		return;
	}
	tos[0].link = 0;
	sbuf = buffer;
	ssiz = monsize;
	if (monsize <= 0) {
		return;
	}
	o = highpc - lowpc;
	if (monsize < o) {
		s_scale = (int)(((float) monsize / o) * SCALE_1_TO_1);
	} else {
		s_scale = SCALE_1_TO_1;
	}
	profControl(1);
}

void moncleanup(void)
{
	FILE *fd;
	int fromindex;
	int endfrom;
	uint32_t frompc;
	int toindex;
	struct gmon_hdr ghdr;
	profControl(0);
	systemMessage(1, "writing gmon.out");
	fd = fopen("/sdcard/gmon.out", "wb");
	if (fd == NULL) {
		systemMessage(0, "mcount: gmon.out");
		return;
	}
	memcpy(&ghdr.cookie[0], GMON_MAGIC, 4);
	profPut32((char *) ghdr.version, GMON_VERSION);
	if (fwrite(&ghdr, sizeof(ghdr), 1, fd) != 1) {
		systemMessage(0, "mcount: gmon.out header");
		fclose(fd);
		return;
	}
	hist_num_bins = ssiz;
	if (profWrite8(fd, GMON_TAG_TIME_HIST) ||
	    profWrite32(fd, get_real_address(s_smaps, (uint32_t) s_lowpc)) ||
	    profWrite32(fd, get_real_address(s_smaps, (uint32_t) s_highpc)) ||
	    profWrite32(fd, hist_num_bins) ||
	    profWrite32(fd, FREQ_HZ) ||
	    profWrite(fd, hist_dimension, 15) ||
	    profWrite(fd, &hist_dimension_abbrev, 1)) {
		systemMessage(0, "mcount: gmon.out hist");
		fclose(fd);
		return;
	}
	uint16_t *hist_sample = (uint16_t *) sbuf;
	uint16_t count;
	int i;
	for (i = 0; i < hist_num_bins; ++i) {
		profPut16((char *) &count, hist_sample[i]);
		if (fwrite(&count, sizeof(count), 1, fd) != 1) {
			systemMessage(0, "mcount: gmon.out sample");
			fclose(fd);
			return;
		}
	}
	endfrom = s_textsize / (HASHFRACTION * sizeof(*froms));
	for (fromindex = 0; fromindex < endfrom; fromindex++) {
		if (froms[fromindex] == 0) {
			continue;
		}
		frompc =
			s_lowpc + (fromindex * HASHFRACTION * sizeof(*froms));
		frompc = get_real_address(s_smaps, frompc);
		for (toindex = froms[fromindex]; toindex != 0;
		     toindex = tos[toindex].link) {
			if (profWrite8(fd, GMON_TAG_CG_ARC)
			    || profWrite32(fd, (uint32_t) frompc)
			    || profWrite32(fd,
					   get_real_address(s_smaps,
							    (uint32_t)
							    tos[toindex].
							    selfpc))
			    || profWrite32(fd, tos[toindex].count)) {
				systemMessage(0, "mcount: arc");
				fclose(fd);
				return;
			}
		}
	}
	fclose(fd);
}

void profCount(unsigned short *frompcindex, char *selfpc)
{
	struct tostruct *top;
	struct tostruct *prevtop;
	long toindex;
	/*
	 * find the return address for mcount,
	 * and the return address for mcount's caller.
	 */
	/* selfpc = pc pushed by mcount call.
	   This identifies the function that was just entered.  */
	/*selfpc = (char *) reg[14].I; */
	/* frompcindex = pc in preceding frame.
	   This identifies the caller of the function just entered.  */
	/*frompcindex = (unsigned short *) reg[12].I; */
	/*
	 * check that we are profiling
	 * and that we aren't recursively invoked.
	 */
	if (profiling) {
		return;
	}
	profiling++;
	s_pc = (long)frompcindex;
	/*
	 * check that frompcindex is a reasonable pc value.
	 * for example: signal catchers get called from the stack,
	 *   not from text space.  too bad.
	 */
	frompcindex =
		(unsigned short *)((long) frompcindex - (long) s_lowpc);
	if ((unsigned long) frompcindex > s_textsize) {
		goto done;
	}
	frompcindex =
		&froms[((long) frompcindex) / (HASHFRACTION * sizeof(*froms))];
	toindex = *frompcindex;
	if (toindex == 0) {
		/*
		 * first time traversing this arc
		 */
		toindex = ++tos[0].link;
		if (toindex >= tolimit) {
			goto overflow;
		}
		*frompcindex = (unsigned short) toindex;
		top = &tos[toindex];
		top->selfpc = selfpc;
		top->count = 1;
		top->link = 0;
		goto done;
	}
	top = &tos[toindex];
	if (top->selfpc == selfpc) {
		/*
		 * arc at front of chain; usual case.
		 */
		top->count++;
		goto done;
	}
	/*
	 * have to go looking down chain for it.
	 * top points to what we are looking at,
	 * prevtop points to previous top.
	 * we know it is not at the head of the chain.
	 */
	for (; /* goto done */ ;) {
		if (top->link == 0) {
			/*
			 * top is end of the chain and none of the chain
			 * had top->selfpc == selfpc.
			 * so we allocate a new tostruct
			 * and link it to the head of the chain.
			 */
			toindex = ++tos[0].link;
			if (toindex >= tolimit) {
				goto overflow;
			}
			top = &tos[toindex];
			top->selfpc = selfpc;
			top->count = 1;
			top->link = *frompcindex;
			*frompcindex = (unsigned short) toindex;
			goto done;
		}
		/*
		 * otherwise, check the next arc on the chain.
		 */
		prevtop = top;
		top = &tos[top->link];
		if (top->selfpc == selfpc) {
			/*
			 * there it is.
			 * increment its count
			 * move it to the head of the chain.
			 */
			top->count++;
			toindex = prevtop->link;
			prevtop->link = top->link;
			top->link = *frompcindex;
			*frompcindex = (unsigned short) toindex;
			goto done;
		}
	}
done:
	profiling--;
	/* and fall through */
out:
	return;			/* normal return restores saved registers */
overflow:
	profiling++;		/* halt further profiling */
#define TOLIMIT "mcount: tos overflow\n"
	systemMessage(0, TOLIMIT);
	goto out;
}
