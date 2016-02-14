#ifndef DS_CE_SEQUENCE_H
#define DS_CE_SEQUENCE_H

#include "../common.h"
#include "php_ce_collection.h"

extern zend_class_entry *sequence_ce;

#define SEQUENCE_ME(cls, name) PHP_ME(cls, name, arginfo_Sequence_##name, ZEND_ACC_PUBLIC)

#define SEQUENCE_ME_LIST(cls) \
SEQUENCE_ME(cls, __construct) \
SEQUENCE_ME(cls, allocate) \
SEQUENCE_ME(cls, capacity) \
SEQUENCE_ME(cls, contains) \
SEQUENCE_ME(cls, filter) \
SEQUENCE_ME(cls, find) \
SEQUENCE_ME(cls, first) \
SEQUENCE_ME(cls, get) \
SEQUENCE_ME(cls, insert) \
SEQUENCE_ME(cls, join) \
SEQUENCE_ME(cls, last) \
SEQUENCE_ME(cls, map) \
SEQUENCE_ME(cls, pop) \
SEQUENCE_ME(cls, push) \
SEQUENCE_ME(cls, pushAll) \
SEQUENCE_ME(cls, reduce) \
SEQUENCE_ME(cls, remove) \
SEQUENCE_ME(cls, reverse) \
SEQUENCE_ME(cls, rotate) \
SEQUENCE_ME(cls, set) \
SEQUENCE_ME(cls, shift) \
SEQUENCE_ME(cls, slice) \
SEQUENCE_ME(cls, sort) \
SEQUENCE_ME(cls, unshift)

/* Argument info */

ARGINFO_LONG(
    Sequence_rotate, rotations
);

ARGINFO_OPTIONAL_ZVAL(
    Sequence___construct, values
);

ARGINFO_LONG(
    Sequence_allocate, capacity
);

ARGINFO_NONE_RETURN_LONG(
    Sequence_capacity
);

ARGINFO_VARIADIC_ZVAL_RETURN_BOOL(
    Sequence_contains, values
);

ARGINFO_OPTIONAL_CALLABLE_RETURN_COLLECTION(
    Sequence_filter, callback, Sequence
);

ARGINFO_ZVAL(
    Sequence_find, value
);

ARGINFO_NONE(
    Sequence_first
);

ARGINFO_OPTIONAL_STRING(
    Sequence_join, glue
);

ARGINFO_LONG(
    Sequence_get, index
);

ARGINFO_LONG_VARIADIC_ZVAL(
    Sequence_insert, index, values
);

ARGINFO_NONE(
    Sequence_last
);

ARGINFO_CALLABLE_RETURN_COLLECTION(
    Sequence_map, callback, Sequence
);

ARGINFO_NONE(
    Sequence_pop
);

ARGINFO_VARIADIC_ZVAL(
    Sequence_push, values
);

ARGINFO_ZVAL(
    Sequence_pushAll, values
);

ARGINFO_CALLABLE_OPTIONAL_ZVAL(
    Sequence_reduce, callback, initial
);

ARGINFO_LONG(
    Sequence_remove, index
);

ARGINFO_NONE_RETURN_COLLECTION(
    Sequence_reverse, Sequence
);

ARGINFO_LONG_ZVAL(
    Sequence_set, index, value
);

ARGINFO_NONE(
    Sequence_shift
);

ARGINFO_LONG_OPTIONAL_LONG_RETURN_COLLECTION(
    Sequence_slice, index, length, Sequence
);

ARGINFO_OPTIONAL_CALLABLE_RETURN_COLLECTION(
    Sequence_sort,  comparator, Sequence
);

ARGINFO_VARIADIC_ZVAL(
    Sequence_unshift, values
);

void register_sequence();

#endif
