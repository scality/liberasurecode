/*
 * Copyright 2018 Scality
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.  THIS SOFTWARE IS PROVIDED BY
 * THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * quadiron_fnt_sys backend implementation
 *
 * vi: set noai tw=79 ts=4 sw=4:
 */

#include <stdlib.h>
#include "erasurecode_backend.h"
#include "quadiron_fnt_common.h"

#define QUADIRON_FNT_SYS_LIB_MAJOR 0
#define QUADIRON_FNT_SYS_LIB_MINOR 1
#define QUADIRON_FNT_SYS_LIB_REV   0
#define QUADIRON_FNT_SYS_LIB_VER_STR "0.1.0"
#define QUADIRON_FNT_SYS_LIB_NAME "quadiron_fnt_sys"
#if defined(__MACOS__) || defined(__MACOSX__) || defined(__OSX__) || defined(__APPLE__)
#define QUADIRON_FNT_SYS_SO_NAME "libquadiron.dylib"
#else
#define QUADIRON_FNT_SYS_SO_NAME "libquadiron.so"
#endif

/* Forward declarations */
struct ec_backend_op_stubs quadiron_fnt_sys_ops;
struct ec_backend quadiron_fnt_sys;
struct ec_backend_common backend_quadiron_fnt_sys;

static void * quadiron_fnt_sys_init(struct ec_backend_args *args,
		void *backend_sohandle)
{
	return quadiron_fnt_common_init(args, backend_sohandle, 1);
}

/*
 * For the time being, we only claim compatibility with versions that
 * match exactly
 */
static bool quadiron_fnt_sys_is_compatible_with(uint32_t version) {
	return version == backend_quadiron_fnt_sys.ec_backend_version;
}

struct ec_backend_op_stubs quadiron_fnt_sys_op_stubs = {
	.INIT                       = quadiron_fnt_sys_init,
	.EXIT                       = quadiron_fnt_exit,
	.ENCODE                     = quadiron_fnt_encode,
	.DECODE                     = quadiron_fnt_decode,
	.FRAGSNEEDED                = quadiron_fnt_min_fragments,
	.RECONSTRUCT                = quadiron_fnt_reconstruct,
	.ELEMENTSIZE                = quadiron_fnt_element_size,
	.ISCOMPATIBLEWITH           = quadiron_fnt_sys_is_compatible_with,
	.GETMETADATASIZE            = quadiron_fnt_common_get_metadata_size,
	.GETENCODEOFFSET            = quadiron_fnt_common_get_encode_offset,
};

struct ec_backend_common backend_quadiron_fnt_sys = {
	.id                         = EC_BACKEND_QUADIRON_FNT_SYS,
	.name                       = QUADIRON_FNT_SYS_LIB_NAME,
	.soname                     = QUADIRON_FNT_SYS_SO_NAME,
	.soversion                  = QUADIRON_FNT_SYS_LIB_VER_STR,
	.ops                        = &quadiron_fnt_sys_op_stubs,
	.ec_backend_version         = _VERSION(QUADIRON_FNT_SYS_LIB_MAJOR,
			QUADIRON_FNT_SYS_LIB_MINOR,
			QUADIRON_FNT_SYS_LIB_REV),
};
