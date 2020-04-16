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

/* Forward declarations */
typedef void *(*quadiron_fnt32_new_func)(int word_size, int n_data, int n_parities, int systematic);
typedef void (*quadiron_fnt32_delete_func)(void *fec);
typedef int (*quadiron_fnt32_encode_func)(void *fec, char **data, char **parity, int *qwanted_idxs, size_t block_size);
typedef int (*quadiron_fnt32_decode_func)(void *fec, char **data, char **parity, int *qmissing_idxs, size_t block_size);
typedef int (*quadiron_fnt32_reconstruct_func)(void *fec, char **data, char **parity, int *qmissing_idxs, unsigned int destination_idx, size_t block_size);
typedef int (*quadiron_fnt32_get_metadata_size_func)(void *fec, size_t block_size);

typedef struct {
	int k;
	int m;
	int w;

	void *fec; // the instance

	quadiron_fnt32_new_func fnt32_new_func;
	quadiron_fnt32_delete_func fnt32_delete_func;
	quadiron_fnt32_encode_func fnt32_encode_func;
	quadiron_fnt32_decode_func fnt32_decode_func;
	quadiron_fnt32_reconstruct_func fnt32_reconstruct_func;
	quadiron_fnt32_get_metadata_size_func fnt32_get_metadata_size_func;

} quadiron_fnt_descriptor;

int quadiron_fnt_encode(void *desc, char **data, char **parity, int blocksize);
int quadiron_fnt_decode(void *desc, char **data, char **parity, int *missing_idxs,
		int blocksize);
int quadiron_fnt_reconstruct(void *desc, char **data, char **parity,
		int *missing_idxs, int destination_idx, int blocksize);
int quadiron_fnt_min_fragments(void *desc, int *missing_idxs,
		int *fragments_to_exclude, int *fragments_needed);
int quadiron_fnt_element_size(void* desc);
int quadiron_fnt_exit(void *desc);
void * quadiron_fnt_common_init(struct ec_backend_args *args, void *backend_sohandle,
		int systematic);
size_t quadiron_fnt_common_get_metadata_size(void *desc, int block_size);
size_t quadiron_fnt_common_get_encode_offset(void *desc, int metadata_size);

extern void (*quadiron_hex_dump)(uint8_t *buf, size_t size);
