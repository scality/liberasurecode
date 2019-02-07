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
 * quadiron_fnt backend implementation
 *
 * vi: set noai tw=79 ts=4 sw=4:
 */

#include <stdio.h>
#include <stdlib.h>

#include "erasurecode.h"
#include "erasurecode_backend.h"
#include "erasurecode_helpers.h"
#include "erasurecode_helpers_ext.h"
#include "quadiron_fnt_common.h"

typedef void (*quadiron_hex_dump_func)(uint8_t *buf, size_t size);
quadiron_hex_dump_func quadiron_hex_dump;

int quadiron_fnt_encode(void *desc, char **data, char **parity,
		int blocksize)
{
	quadiron_fnt_descriptor *qdesc = (quadiron_fnt_descriptor*) desc;
	int qwanted_idxs[qdesc->k + qdesc->m];
	unsigned int i;

	for (i = 0; i < qdesc->k + qdesc->m; i++) {
		qwanted_idxs[i] = 1;
	}
	return qdesc->fnt32_encode_func(qdesc->fec,
			data, parity, qwanted_idxs, blocksize);
}

int quadiron_fnt_decode(void *desc, char **data, char **parity,
		int *missing_idxs, int blocksize)
{
	quadiron_fnt_descriptor *qdesc = (quadiron_fnt_descriptor*)desc;
	int qmissing_idxs[qdesc->k + qdesc->m];

	convert_idx_list_to_bitvalues(missing_idxs, qmissing_idxs,
			qdesc->k + qdesc->m);
	return qdesc->fnt32_decode_func(qdesc->fec,
			data, parity, qmissing_idxs,
			blocksize);
}

int quadiron_fnt_reconstruct(void *desc, char **data, char **parity,
		int *missing_idxs, int destination_idx, int blocksize)
{
	quadiron_fnt_descriptor *qdesc = (quadiron_fnt_descriptor*)desc;
	int qmissing_idxs[qdesc->k + qdesc->m];

	convert_idx_list_to_bitvalues(missing_idxs, qmissing_idxs,
			qdesc->k + qdesc->m);
	return qdesc->fnt32_reconstruct_func(qdesc->fec,
			data, parity, qmissing_idxs,
			destination_idx,
			blocksize);
}

int quadiron_fnt_min_fragments(void *desc, int *missing_idxs,
		int *fragments_to_exclude, int *fragments_needed)
{
	quadiron_fnt_descriptor *qdesc = (quadiron_fnt_descriptor*) desc;
	int qexclude[qdesc->k + qdesc->m];
	int qmissing[qdesc->k + qdesc->m];
	int i;
	int j = 0;
	int ret = -1;

	convert_idx_list_to_bitvalues(fragments_to_exclude, qexclude, qdesc->k + qdesc->m);
	convert_idx_list_to_bitvalues(missing_idxs, qmissing, qdesc->k + qdesc->m);

	for (i = 0; i < (qdesc->k + qdesc->m); i++) {
		if (!(qmissing[i])) {
			fragments_needed[j] = i;
			j++;
		}
		if (j == qdesc->k) {
			ret = 0;
			fragments_needed[j] = -1;
			break;
		}
	}

	return ret;
}

/**
 * Return the element-size, which is the number of bits stored
 * on a given device, per codeword.
 *
 * Returns the size in bits!
 */
int quadiron_fnt_element_size(void* desc)
{
	return 8;
}

int quadiron_fnt_exit(void *desc)
{
	quadiron_fnt_descriptor *quadiron_fnt_desc = NULL;

	quadiron_fnt_desc = (quadiron_fnt_descriptor*) desc;

	quadiron_fnt_desc->fnt32_delete_func(quadiron_fnt_desc->fec);

	free(quadiron_fnt_desc);

	return 0;
}

size_t quadiron_fnt_common_get_metadata_size(void *desc, int block_size)
{
	quadiron_fnt_descriptor *quadiron_fnt_desc = NULL;

	quadiron_fnt_desc = (quadiron_fnt_descriptor*) desc;

	return quadiron_fnt_desc->fnt32_get_metadata_size_func(quadiron_fnt_desc->fec, block_size);
}

size_t quadiron_fnt_common_get_encode_offset(void *desc, int metadata_size)
{
	return metadata_size;
}

void * quadiron_fnt_common_init(struct ec_backend_args *args, void *backend_sohandle,
		int systematic)
{
	quadiron_fnt_descriptor *desc = NULL;

	desc = (quadiron_fnt_descriptor *)malloc(sizeof(quadiron_fnt_descriptor));
	if (NULL == desc) {
		return NULL;
	}

	desc->k = args->uargs.k;
	desc->m = args->uargs.m;
	desc->w = args->uargs.w;

	/* validate EC arguments */

	/*
	 * ISO C forbids casting a void* to a function pointer.
	 * Since dlsym return returns a void*, we use this union to
	 * "transform" the void* to a function pointer.
	 */
	union {
		quadiron_fnt32_new_func quadiron_fnt32_new_funcp;
		quadiron_fnt32_delete_func quadiron_fnt32_delete_funcp;
		quadiron_fnt32_encode_func quadiron_fnt32_encode_funcp;
		quadiron_fnt32_decode_func quadiron_fnt32_decode_funcp;
		quadiron_fnt32_reconstruct_func quadiron_fnt32_reconstruct_funcp;
		quadiron_fnt32_get_metadata_size_func quadiron_fnt32_get_metadata_size_funcp;
		quadiron_hex_dump_func quadiron_hex_dump_funcp;
		void *vptr;
	} func_handle = {.vptr = NULL};

	/* fill in function addresses */
	func_handle.vptr = NULL;
	func_handle.vptr = dlsym(backend_sohandle, "quadiron_fnt32_new");
	desc->fnt32_new_func = func_handle.quadiron_fnt32_new_funcp;
	if (NULL == desc->fnt32_new_func) {
		goto error;
	}

	func_handle.vptr = NULL;
	func_handle.vptr = dlsym(backend_sohandle, "quadiron_fnt32_delete");
	desc->fnt32_delete_func = func_handle.quadiron_fnt32_delete_funcp;
	if (NULL == desc->fnt32_delete_func) {
		goto error;
	}

	func_handle.vptr = NULL;
	func_handle.vptr = dlsym(backend_sohandle, "quadiron_fnt32_encode");
	desc->fnt32_encode_func = func_handle.quadiron_fnt32_encode_funcp;
	if (NULL == desc->fnt32_encode_func) {
		goto error;
	}

	func_handle.vptr = NULL;
	func_handle.vptr = dlsym(backend_sohandle, "quadiron_fnt32_decode");
	desc->fnt32_decode_func = func_handle.quadiron_fnt32_decode_funcp;
	if (NULL == desc->fnt32_decode_func) {
		goto error;
	}

	func_handle.vptr = NULL;
	func_handle.vptr = dlsym(backend_sohandle, "quadiron_fnt32_reconstruct");
	desc->fnt32_reconstruct_func = func_handle.quadiron_fnt32_reconstruct_funcp;
	if (NULL == desc->fnt32_reconstruct_func) {
		goto error;
	}

	func_handle.vptr = NULL;
	func_handle.vptr = dlsym(backend_sohandle, "quadiron_fnt32_get_metadata_size");
	desc->fnt32_get_metadata_size_func = func_handle.quadiron_fnt32_get_metadata_size_funcp;
	if (NULL == desc->fnt32_get_metadata_size_func) {
		goto error;
	}

	func_handle.vptr = dlsym(backend_sohandle, "quadiron_hex_dump");
	quadiron_hex_dump = func_handle.quadiron_hex_dump_funcp;

	desc->fec = desc->fnt32_new_func(args->uargs.w / 8, args->uargs.k,
			args->uargs.m, systematic);
	if (NULL == desc->fec) {
		goto error;
	}

	return desc;

error:
	free(desc);

	return NULL;
}
