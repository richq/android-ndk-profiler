/*
 * Part of the android-ndk-profiler library.
 * Copyright (C) Richard Quirk
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#ifndef ucontext_h_seen
#define ucontext_h_seen

#include <asm/sigcontext.h>       /* for sigcontext */
#include <asm/signal.h>           /* for stack_t */

typedef struct ucontext {
	unsigned long uc_flags;
	struct ucontext *uc_link;
	stack_t uc_stack;
	struct sigcontext uc_mcontext;
	unsigned long uc_sigmask;
} ucontext_t;

#endif
