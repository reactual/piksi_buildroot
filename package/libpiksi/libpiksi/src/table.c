/*
 * Copyright (C) 2018 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Adapted from: https://github.com/novaua/ExpC/blob/master/glibc_hash_tbl/main.c */

#define _GNU_SOURCE
#include <assert.h>
#include <search.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libpiksi/logging.h>
#include <libpiksi/table.h>

typedef struct table_s {
  struct hsearch_data htab;
  size_t size;
  table_destroy_entry_fn_t destroy_entry;
  char *keys[];
} table_t;

#define TABLE_T_INITIALIZER                     \
  (table_t)                                     \
  {                                             \
    .htab = (struct hsearch_data){0}, .size = 0 \
  }

table_t *table_create(size_t size, table_destroy_entry_fn_t destroy_entry_fn)
{
  table_t *table = NULL;
  size_t sizeof_keys = size * sizeof(*table->keys[0]);

  table = malloc(sizeof(*table) + sizeof_keys);
  *table = TABLE_T_INITIALIZER;

  hcreate_r(size, &table->htab);

  table->size = size;
  table->destroy_entry = destroy_entry_fn;

  bzero(table->keys, sizeof_keys);

  return table;
}

void table_destroy(table_t **ptable)
{
  if (ptable == NULL || *ptable == NULL) return;

  table_t *table = *ptable;

  int i = 0;
  for (; i < table->htab.filled; ++i) {
    void *dt = table_get(table, table->keys[i]);
    assert(dt && "no data!");

    table->destroy_entry(table, &dt);
    assert(dt == NULL);

    free(table->keys[i]);
    table->keys[i] = NULL;
  }

  hdestroy_r(&table->htab);
  free(table);

  *ptable = NULL;
}

bool table_put(table_t *table, const char *key, void *data)
{
  unsigned n = 0;
  ENTRY e, *ep = NULL;

  e.key = strdup(key);
  e.data = data;

  unsigned int fb = table->htab.filled;
  n = hsearch_r(e, ENTER, &ep, &table->htab);

  if (fb < table->htab.filled) {
    assert(table->keys[fb] == 0 && "free space");
    table->keys[fb] = e.key;
  } else {
    PK_LOG_ANNO(LOG_ERR, "overwrite is not supported");
    return false;
  }

  return n != 0;
}

void *table_get(table_t *table, const char *key_in)
{
  unsigned n = 0;
  ENTRY e = {0}, *ep = NULL;

  e.key = (char *)key_in;
  n = hsearch_r(e, FIND, &ep, &table->htab);

  if (!n) return NULL;

  return ep->data;
}
