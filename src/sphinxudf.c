//
// $Id$
//

//
// Copyright (c) 2001-2021, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//

//
// Sphinx UDF helpers implementation
//

#include "sphinxudf.h"

#include <memory.h>
#include <stdlib.h>

#define SPH_UDF_MAX_FIELD_FACTORS	256
#define SPH_UDF_MAX_TERM_FACTORS	2048
#define SPH_UDF_MAX_FIELD_SIZE		8 ///< how many ints to store a bitmask for 256 fields?

/// helper function that must be called to initialize the SPH_UDF_FACTORS structure
/// before it is passed to sphinx_factors_unpack
/// returns 0 on success
/// returns an error code on error
int sphinx_factors_init(SPH_UDF_FACTORS * out)
{
	if (!out)
		return 1;

	memset(out, 0, sizeof(SPH_UDF_FACTORS));
	return 0;
}


/// helper function that unpacks PACKEDFACTORS() blob into SPH_UDF_FACTORS structure
/// MUST be in sync with PackFactors() method in sphinxsearch.cpp
/// returns 0 on success
/// returns an error code on error
int sphinx_factors_unpack(const unsigned int * in, SPH_UDF_FACTORS * out)
{
	const unsigned int * pack = in;
	SPH_UDF_FIELD_FACTORS * f;
	SPH_UDF_TERM_FACTORS * t;
	int i, size, fields, fields_size;
	unsigned int exact_hit_mask[SPH_UDF_MAX_FIELD_SIZE];
	unsigned int exact_order_mask[SPH_UDF_MAX_FIELD_SIZE];
	unsigned int exact_field_hit_mask[SPH_UDF_MAX_FIELD_SIZE];
	unsigned int full_field_hit_mask[SPH_UDF_MAX_FIELD_SIZE];

	if (!in || !out)
		return 1;

	if (out->field || out->term)
		return 1;

	// extract size, extract document-level factors
	size = *in++;

	out->doc_bm15 = *in++;
	out->doc_bm25a = *(float *)in++;
	out->matched_fields = *in++;
	out->doc_word_count = *in++;
	out->num_fields = *in++;

	// extract field-level factors
	if (out->num_fields > SPH_UDF_MAX_FIELD_FACTORS)
		return 1;

	fields_size = (out->num_fields + 31) / 32;

	for (i = 0; i < fields_size; i++)
		exact_hit_mask[i] = *in++;

	for (i = 0; i < fields_size; i++)
		exact_order_mask[i] = *in++;

	for (i = 0; i < fields_size; i++)
		exact_field_hit_mask[i] = *in++;

	for (i = 0; i < fields_size; i++)
		full_field_hit_mask[i] = *in++;

	if (out->num_fields > 0)
	{
		i = out->num_fields * sizeof(SPH_UDF_FIELD_FACTORS);
		out->field = (SPH_UDF_FIELD_FACTORS *)malloc(i);
		memset(out->field, 0, i);
	}

	for (i = 0; i < out->num_fields; i++)
	{
		f = &(out->field[i]);
		f->hit_count = *in++;

		if (f->hit_count)
		{
			f->id = *in++;
			f->lcs = *in++;
			f->word_count = *in++;
			f->tf_idf = *(float *)in++;
			f->min_idf = *(float *)in++;
			f->max_idf = *(float *)in++;
			f->sum_idf = *(float *)in++;
			f->min_hit_pos = (int)*in++;
			f->min_best_span_pos = (int)*in++;
			f->max_window_hits = (int)*in++;
			f->min_gaps = (int)*in++;
			f->atc = *(float *)in++;
			f->lccs = *in++;
			f->wlccs = *(float *)in++;
			f->sum_idf_boost = *(float *)in++;
			f->is_noun_hits = *in++;
			f->is_latin_hits = *in++;
			f->is_number_hits = *in++;
			f->has_digit_hits = *in++;
			f->trf_qt = *(float *)in++;
			f->trf_i2u = *(float *)in++;
			f->trf_i2q = *(float *)in++;
			f->trf_i2f = *(float *)in++;
			f->trf_aqt = *(float *)in++;
			f->trf_naqt = *(float *)in++;
			f->phrase_decay10 = *(float *)in++;
			f->phrase_decay30 = *(float *)in++;

			f->exact_hit = (char)((exact_hit_mask[i >> 5] & (1UL << (i & 31))) != 0);
			f->exact_order = (char)((exact_order_mask[i >> 5] & (1UL << (i & 31))) != 0);
			f->exact_field_hit = (char)((exact_field_hit_mask[i >> 5] & (1UL << (i & 31))) != 0);
			f->full_field_hit = (char)((full_field_hit_mask[i >> 5] & (1UL << (i & 31))) != 0);
		} else
		{
			// everything else is already zeroed out by memset() above
			f->id = i;
		}
	}

	// extract term-level factors
	out->max_uniq_qpos = *in++;
	if (out->max_uniq_qpos > SPH_UDF_MAX_TERM_FACTORS)
		return 1;

	if (out->max_uniq_qpos > 0)
		out->term = (SPH_UDF_TERM_FACTORS *)malloc(out->max_uniq_qpos * sizeof(SPH_UDF_TERM_FACTORS));

	for (i = 0; i < out->max_uniq_qpos; i++)
	{
		t = &(out->term[i]);
		t->keyword_mask = *in++;
		if (t->keyword_mask)
		{
			t->id = *in++;
			t->tf = (int)*in++;
			t->idf = *(float *)in++;
			t->idf_boost = *(float *)in++;
			t->flags = (unsigned char)*in++;
		}
	}

	out->query_max_lcs = (int)*in++;
	out->query_word_count = (int)*in++;
	out->query_is_noun_words = (int)*in++;
	out->query_is_latin_words = (int)*in++;
	out->query_is_number_words = (int)*in++;
	out->query_has_digit_words = (int)*in++;
	out->query_max_idf = *(float *)in++;
	out->query_min_idf = *(float *)in++;
	out->query_sum_idf = *(float *)in++;

	// extract field_tf factors
	// perform size safety check to avoid allocating and copying too much
	fields = *in++;
	if (in + sizeof(int) * fields <= pack + size)
	{
		out->field_tf = (int *)malloc(fields * sizeof(int));
		memcpy(out->field_tf, in, fields * sizeof(int));
	}
	in += fields;

	// do a final safety check, and return
	return (size != (int)((in - pack) * sizeof(int))) ? 1 : 0;
}


/// helper function that must be called to free the memory allocated by the sphinx_factors_unpack
/// function call
/// returns 0 on success
/// returns an error code on error
int sphinx_factors_deinit(SPH_UDF_FACTORS * out)
{
	if (!out)
		return 1;

	free(out->term);
	free(out->field);
	free(out->field_tf);

	return 0;
}

//////////////////////////////////////////////////////////////////////////

static const unsigned int * skip_fields(const unsigned int * in, int n)
{
	in += 6 + ((in[5] + 31) / 32) * 4; // skip heading document factors and 4 exact/full masks
	while (n-- > 0)
		in += (in[0] > 0) ? 28 : 1; // skip all factors in matched field, or just 1 in unmatched
	return in;
}


static const unsigned int * skip_terms(const unsigned int * in, int n)
{
	in += 1; // skip max_uniq_qpos
	while (n-- > 0)
		in += (in[0] > 0) ? 6 : 1; // skip 6 ints per matched term, or 1 per unmatched
	return in;
}


const unsigned int * sphinx_get_field_factors(const unsigned int * in, int field)
{
	if (!in || field < 0 || field >= (int)in[5])
		return NULL; // blob[5] is num_fields, do a sanity check
	in = skip_fields(in, field);
	if (!in[0])
		return NULL; // no hits, no fun
	if ((int)in[1] != field)
		return NULL; // field[1] is field_id, do a sanity check
	return in; // all good
}


const unsigned int * sphinx_get_term_factors(const unsigned int * in, int term)
{
	if (!in || term < 0)
		return NULL;
	in = skip_fields(in, in[5]); // skip all fields
	if (term > (int)in[0])
		return NULL; // sanity check vs max_uniq_qpos ( qpos and terms range - [1, max_uniq_qpos]
	in = skip_terms(in, term - 1);
	if (!in[0])
		return NULL; // unmatched term
	if ((int)in[1] != term)
		return NULL; // term[1] is keyword_id, sanity check failed
	return in;
}


int sphinx_get_doc_factor_int(const unsigned int * in, enum sphinx_doc_factor f)
{
	int fields_size;
	switch (f)
	{
		case SPH_DOCF_BM15: return (int)in[1];
		case SPH_DOCF_BM25A: return (int)in[2];
		case SPH_DOCF_MATCHED_FIELDS: return (int)in[3];
		case SPH_DOCF_DOC_WORD_COUNT: return (int)in[4];
		case SPH_DOCF_NUM_FIELDS: return (int)in[5];
		case SPH_DOCF_MAX_UNIQ_QPOS:
			in = skip_fields(in, in[5]);
			return (int)in[0];
		case SPH_DOCF_EXACT_HIT_MASK: return (int)in[6];
		case SPH_DOCF_EXACT_ORDER_MASK: fields_size = ((int)in[5] + 31) / 32; return (int)in[6 + fields_size];
		case SPH_DOCF_EXACT_FIELD_HIT_MASK: fields_size = ((int)in[5] + 31) / 32; return (int)in[6 + fields_size * 2];
		case SPH_DOCF_FULL_FIELD_HIT_MASK: fields_size = ((int)in[5] + 31) / 32; return (int)in[6 + fields_size * 3];
	}
	return 0;
}


const unsigned int * sphinx_get_doc_factor_ptr(const unsigned int * in, enum sphinx_doc_factor f)
{
	int fields_size;

	if (f == SPH_DOCF_EXACT_HIT_MASK)
		return in + 6;

	fields_size = ((int)in[5] + 31) / 32;
	if (f == SPH_DOCF_EXACT_ORDER_MASK)
		return in + 6 + fields_size;

	if (f == SPH_DOCF_EXACT_FIELD_HIT_MASK)
		return in + 6 + fields_size * 2;

	if (f == SPH_DOCF_FULL_FIELD_HIT_MASK)
		return in + 6 + fields_size * 3;

	return 0;
}


float sphinx_get_doc_factor_float(const unsigned int * in, enum sphinx_doc_factor f)
{
	if (f == SPH_DOCF_BM25A)
		return *(float *)&in[2];
	else
		return 0.0f;
}


int sphinx_get_field_factor_int(const unsigned int * in, enum sphinx_field_factor f)
{
	if (!in)
		return 0;

	// in[1] is id
	switch (f)
	{
		case SPH_FIELDF_HIT_COUNT: return (int)in[0];
		case SPH_FIELDF_LCS: return (int)in[2];
		case SPH_FIELDF_WORD_COUNT: return (int)in[3];
		case SPH_FIELDF_TF_IDF: return (int)in[4];
		case SPH_FIELDF_MIN_IDF: return (int)in[5];
		case SPH_FIELDF_MAX_IDF: return (int)in[6];
		case SPH_FIELDF_SUM_IDF: return (int)in[7];
		case SPH_FIELDF_MIN_HIT_POS: return (int)in[8];
		case SPH_FIELDF_MIN_BEST_SPAN_POS: return (int)in[9];
		case SPH_FIELDF_MAX_WINDOW_HITS: return (int)in[10];
		case SPH_FIELDF_MIN_GAPS: return (int)in[11];
		case SPH_FIELDF_ATC: return (int)in[12];
		case SPH_FIELDF_LCCS: return (int)in[13];
		case SPH_FIELDF_WLCCS: return (int)in[14];
		case SPH_FIELDF_SUM_IDF_BOOST: return (int)in[15];
		case SPH_FIELDF_IS_NOUN_HITS: return (int)in[16];
		case SPH_FIELDF_IS_LATIN_HITS: return (int)in[17];
		case SPH_FIELDF_IS_NUMBER_HITS: return (int)in[18];
		case SPH_FIELDF_HAS_DIGIT_HITS: return (int)in[19];
		case SPH_FIELDF_TRF_QT: return (int)in[20];
		case SPH_FIELDF_TRF_I2U: return (int)in[21];
		case SPH_FIELDF_TRF_I2Q: return (int)in[22];
		case SPH_FIELDF_TRF_I2F: return (int)in[23];
		case SPH_FIELDF_TRF_AQT: return (int)in[24];
		case SPH_FIELDF_TRF_NAQT: return (int)in[25];
		case SPH_FIELDF_PHRASE_DECAY10: return (int)in[26];
		case SPH_FIELDF_PHRASE_DECAY30: return (int)in[27];
	}
	return 0;
}


int sphinx_get_term_factor_int(const unsigned int * in, enum sphinx_term_factor f)
{
	if (!in)
		return 0;
	switch (f)
	{
		case SPH_TERMF_KEYWORD_MASK: return (int)in[0];
		case SPH_TERMF_TF: return (int)in[2];
		case SPH_TERMF_IDF: return (int)in[3];
		case SPH_TERMF_IDF_BOOST: return (int)in[4];
		case SPH_TERMF_FLAGS: return (int)in[5];
	}
	return 0;
}


float sphinx_get_field_factor_float(const unsigned int * in, enum sphinx_field_factor f)
{
	int r = sphinx_get_field_factor_int(in, f);
	void * pvoid = &r;
	return *(float *)pvoid;
}


float sphinx_get_term_factor_float(const unsigned int * in, enum sphinx_term_factor f)
{
	int r = sphinx_get_term_factor_int(in, f);
	void * pvoid = &r;
	return *(float *)pvoid;
}


const unsigned int * sphinx_get_query_factors(const unsigned int * in)
{
	in = skip_fields(in, in[5]);
	in = skip_terms(in, (int)in[0]);
	return in;
}


int sphinx_get_query_factor_int(const unsigned int * in, enum sphinx_query_factor f)
{
	switch (f)
	{
		case SPH_QUERYF_MAX_LCS: return (int)in[0];
		case SPH_QUERYF_WORD_COUNT: return (int)in[1];
		case SPH_QUERYF_IS_NOUN_WORDS: return (int)in[2];
		case SPH_QUERYF_IS_LATIN_WORDS: return (int)in[3];
		case SPH_QUERYF_IS_NUMBER_WORDS: return (int)in[4];
		case SPH_QUERYF_HAS_DIGIT_WORDS: return (int)in[5];
		case SPH_QUERYF_MAX_IDF: return (int)in[6];
		case SPH_QUERYF_MIN_IDF: return (int)in[7];
		case SPH_QUERYF_SUM_IDF: return (int)in[8];
	}
	return 0;
}


float sphinx_get_query_factor_float(const unsigned int * in, enum sphinx_query_factor f)
{
	int r = sphinx_get_query_factor_int(in, f);
	void * pvoid = &r;
	return *(float *)pvoid;
}

//
// $Id$
//
