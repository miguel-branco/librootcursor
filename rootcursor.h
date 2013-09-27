#ifndef ROOT_WRAPPER_H
#define ROOT_WRAPPER_H

/*
 * Incomplete type definitions for C compatibility.
 * Types defined in the C++ source.
 */
typedef struct Root Root;
typedef struct RootTable RootTable;
typedef struct RootCursor RootCursor;
typedef struct RootAttribute RootAttribute;

typedef enum
{
  RootInvalidType,
  RootTreeId,
  RootCollectionId,
  RootInt,
  RootUInt,
  RootFloat,
  RootBool
} RootAttributeType;

#if !defined(__cplusplus) && !defined(POSTGRES_H)
typedef int bool;
#endif

#if !defined(POSTGRES_H)
#include <stdint.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

RootAttributeType get_root_type(char *atttype);

Root *init_root(char *fnames[], int n);
RootTable *get_root_table(Root *root, char *tree_name, bool is_collection);
void fini_root(Root *root);

int64_t get_root_table_approx_size(RootTable *table);
RootAttribute *get_root_table_attr(RootTable *table, char *attname, int key);
void fini_root_table(RootTable *table);

RootCursor *init_root_cursor(RootTable *table, int nattrs);
int set_root_cursor_attr(RootCursor *cursor, int pos, char *attname, RootAttributeType atttype);
RootAttributeType get_root_cursor_attr_type(RootCursor *cursor, int pos);
int open_root_cursor(RootCursor *cursor);
int advance_root_cursor(RootCursor *cursor);
int is_null(RootCursor *cursor, int pos);
int64_t get_tree_id(RootCursor *cursor, int pos);
int get_collection_id(RootCursor *cursor, int pos);
int get_int(RootCursor *cursor, int pos);
unsigned int get_uint(RootCursor *cursor, int pos);
float get_float(RootCursor *cursor, int pos);
bool get_bool(RootCursor *cursor, int pos);
void fini_root_cursor(RootCursor *cursor);

#if defined(__cplusplus)
}
#endif

#endif  /* ROOT_WRAPPER_H */

