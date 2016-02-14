#include "../common.h"
#include "../iterators/php_vector_iterator.h"
#include "../handlers/php_vector_handlers.h"
#include "../classes/php_ce_vector.h"
#include "ds_vector.h"

static inline bool index_out_of_range(zend_long index, zend_long max)
{
    if (index < 0 || index >= max) {
        INDEX_OUT_OF_RANGE(index, max);
        return true;
    }
    return false;
}

static inline void ds_vector_reallocate(ds_vector_t *vector, zend_long capacity)
{
    REALLOC_ZVAL_BUFFER(vector->buffer, capacity);
    vector->capacity = capacity;
}

ds_vector_t *ds_vector_ex(zend_long capacity)
{
    ds_vector_t *vector = ecalloc(1, sizeof(ds_vector_t));

    capacity = MAX(capacity, DS_VECTOR_MIN_CAPACITY);

    vector->buffer   = ALLOC_ZVAL_BUFFER(capacity);
    vector->capacity = capacity;
    vector->size     = 0;

    return vector;
}

ds_vector_t *ds_vector()
{
    return ds_vector_ex(DS_VECTOR_MIN_CAPACITY);
}

static ds_vector_t *ds_vector_from_buffer_ex(
    zval *buffer,
    zend_long size,
    zend_long capacity
) {
    ds_vector_t *vector   = ecalloc(1, sizeof(ds_vector_t));
    vector->buffer   = buffer;
    vector->capacity = capacity;
    vector->size     = size;

    return vector;
}

ds_vector_t *ds_vector_from_buffer(zval *buffer, zend_long length)
{
    zend_long capacity = length;

    if (capacity < DS_VECTOR_MIN_CAPACITY) {
        capacity = DS_VECTOR_MIN_CAPACITY;
        REALLOC_ZVAL_BUFFER(buffer, capacity);

    } else if (length < capacity >> 2) {
        capacity = capacity >> 1;
        REALLOC_ZVAL_BUFFER(buffer, capacity);
    }

    return ds_vector_from_buffer_ex(buffer, length, capacity);
}

void ds_vector_user_allocate(ds_vector_t *vector, zend_long capacity)
{
    if (capacity > vector->capacity) {
        ds_vector_reallocate(vector, capacity);
    }
}

ds_vector_t *ds_vector_clone(ds_vector_t *vector)
{
    if (VECTOR_IS_EMPTY(vector)) {
        return ds_vector();

    } else {
        zend_long size     = vector->size;
        zend_long capacity = vector->capacity;

        ds_vector_t *copy = ds_vector_ex(capacity);

        copy->buffer   = ALLOC_ZVAL_BUFFER(capacity);
        copy->capacity = capacity;
        copy->size     = size;

        COPY_ZVAL_BUFFER(copy->buffer, vector->buffer, size);

        return copy;
    }
}

static inline void ds_vector_increase_capacity(ds_vector_t *vector)
{
    ds_vector_reallocate(vector, vector->capacity + (vector->capacity >> 1));
}

static inline void ds_vector_ensure_capacity(ds_vector_t *vector, zend_long capacity)
{
    if (capacity > vector->capacity) {
        zend_long boundary = vector->capacity + (vector->capacity >> 1);
        ds_vector_reallocate(vector, MAX(capacity, boundary));
    }
}

static inline void ds_vector_check_compact(ds_vector_t *vector)
{
    if (vector->size < vector->capacity >> 2) {

        if (vector->capacity >> 1 > DS_VECTOR_MIN_CAPACITY) {
            ds_vector_reallocate(vector, vector->capacity >> 1);
        }
    }
}

void ds_vector_remove(ds_vector_t *vector, zend_long index, zval *return_value)
{
    if (index_out_of_range(index, vector->size)) {
        return;
    }

    if (index == vector->size - 1) {
        ds_vector_pop(vector, return_value);

    } else {
        zval *pos = vector->buffer + index;

        if (return_value) {
            ZVAL_COPY(return_value, pos);
        }

        if ( ! Z_ISUNDEF_P(pos)) {
            zval_ptr_dtor(pos);
        }

        memmove(pos, pos + 1, sizeof(zval) * (vector->size - index));
        vector->size--;

        ds_vector_check_compact(vector);
    }
}

zval *ds_vector_get(ds_vector_t *vector, zend_long index)
{
    if (index_out_of_range(index, vector->size)) {
        return NULL;
    }

    return vector->buffer + index;
}

static inline void increase_capacity_if_full(ds_vector_t *vector)
{
    if (vector->size == vector->capacity) {
        ds_vector_increase_capacity(vector);
    }
}

void ds_vector_clear(ds_vector_t *vector)
{
    zval *pos = vector->buffer;
    zval *end = pos + vector->size;

    for (; pos != end; ++pos) {
        zval_ptr_dtor(pos);
    }

    vector->size = 0;
    ds_vector_reallocate(vector, DS_VECTOR_MIN_CAPACITY);
}

void ds_vector_set(ds_vector_t *vector, zend_long index, zval *value)
{
    if ( ! index_out_of_range(index, vector->size)) {
        zval *current = vector->buffer + index;
        zval_ptr_dtor(current);
        ZVAL_COPY(current, value);
    }
}

/**
 *
 */
void ds_vector_to_array(ds_vector_t *vector, zval *return_value)
{
    zend_long n = vector->size;

    if (n == 0) {
        array_init(return_value);

    } else {
        zval *pos = vector->buffer;
        zval *end = pos + n;

        array_init_size(return_value, n);

        for (; pos != end; ++pos) {
            add_next_index_zval(return_value, pos);
            Z_TRY_ADDREF_P(pos);
        }
    }
}

static inline zend_long ds_vector_find_index(ds_vector_t *vector, zval *value)
{
    zval *pos = vector->buffer;
    zval *end = vector->buffer + vector->size;

    for (; pos != end; ++pos) {
        if (zend_is_identical(value, pos)) {
            return pos - vector->buffer;
        }
    }

    return FAILURE;
}

void ds_vector_find(ds_vector_t *vector, zval *value, zval *return_value)
{
    zend_long index = ds_vector_find_index(vector, value);

    if (index >= 0) {
        ZVAL_LONG(return_value, index);
        return;
    }

    ZVAL_FALSE(return_value);
}

bool ds_vector_contains(ds_vector_t *vector, zval *value)
{
    return ds_vector_find_index(vector, value) != FAILURE;
}

bool ds_vector_contains_va(ds_vector_t *vector, VA_PARAMS)
{
    if (argc == 0) return false;

    while (argc--) {
        if ( ! ds_vector_contains(vector, argv++)) return false;
    }

    return true;
}

void ds_vector_join(ds_vector_t *vector, char *str, size_t len, zval *return_value)
{
    zend_string *s;
    s = join_zval_buffer(vector->buffer, VECTOR_SIZE(vector), str, len);
    ZVAL_STR(return_value, s);
}

void ds_vector_insert_va(ds_vector_t *vector, zend_long index, VA_PARAMS)
{
    if ( ! index_out_of_range(index, vector->size + 1) && argc > 0) {
        zend_long len;
        zval *src;
        zval *dst;
        zval *end;

        ds_vector_ensure_capacity(vector, vector->size + argc);

        src = argv;
        dst = vector->buffer + index;
        end = dst + argc;
        len = vector->size - index;

        if (len > 0) {
            memmove(end, dst, (vector->size - index) * sizeof(zval));
        }

        for (; dst != end; ++dst, ++src) {
            ZVAL_COPY(dst, src);
        }

        vector->size += argc;
    }
}

void ds_vector_insert(ds_vector_t *vector, zend_long index, zval *value)
{
    ds_vector_insert_va(vector, index, 1, value);
}

void ds_vector_push(ds_vector_t *vector, zval *value)
{
    increase_capacity_if_full(vector);
    ZVAL_COPY(&vector->buffer[vector->size++], value);
}

void ds_vector_push_va(ds_vector_t *vector, VA_PARAMS)
{
    if (argc == 1) {
        ds_vector_push(vector, argv);

    } else if (argc > 0) {
        ds_vector_ensure_capacity(vector, vector->size + argc);

        do {
            ZVAL_COPY(&vector->buffer[vector->size++], argv++);
        } while (--argc != 0);
    }
}

void ds_vector_unshift_va(ds_vector_t *vector, VA_PARAMS)
{
    ds_vector_ensure_capacity(vector, vector->size + argc);

    if (argc > 0) {
        zval *pos = vector->buffer;

        memmove(pos + argc, pos, vector->size * sizeof(zval));
        vector->size += argc;

        do {
            ZVAL_COPY(pos++, argv++);
        } while (--argc != 0);
    }
}

void ds_vector_unshift(ds_vector_t *vector, zval *value)
{
    ds_vector_insert(vector, 0, value);
}

void ds_vector_sort_callback(ds_vector_t *vector)
{
    user_sort_zval_buffer(vector->buffer, vector->size);
}

void ds_vector_sort(ds_vector_t *vector)
{
    sort_zval_buffer(vector->buffer, vector->size);
}

bool ds_vector_isset(ds_vector_t *vector, zend_long index, int check_empty)
{
    if (index < 0 || index >= vector->size) {
        return 0;
    }

    return zval_isset(vector->buffer + index, check_empty);
}

bool ds_vector_index_exists(ds_vector_t *vector, zend_long index)
{
    return index >= 0 && index < vector->size;
}

static int iterator_add(zend_object_iterator *iterator, void *puser)
{
    ds_vector_push((ds_vector_t *) puser, iterator->funcs->get_current_data(iterator));
    return SUCCESS;
}

static inline void add_traversable_to_vector(ds_vector_t *vector, zval *obj)
{
    spl_iterator_apply(obj, iterator_add, (void*) vector);
}

static inline void add_array_to_vector(ds_vector_t *vector, HashTable *array)
{
    zval *value;
    ds_vector_ensure_capacity(vector, vector->size + array->nNumOfElements);

    ZEND_HASH_FOREACH_VAL(array, value) {
        ds_vector_push(vector, value);
    }
    ZEND_HASH_FOREACH_END();
}

void ds_vector_rotate(ds_vector_t *vector, zend_long r)
{
    zval *a, *b, *c;

    zend_long n = vector->size;

         // Negative rotation should rotate in the opposite direction
         if (r < 0) r = n - (llabs(r) % n);
    else if (r > n) r = r % n;

    // There's no need to rotate if the sequence won't be affected.
    if (r == 0 || r == n) return;

    a = vector->buffer;
    b = a + r;
    c = a + n;

    reverse_zval_range(a, b);
    reverse_zval_range(b, c);
    reverse_zval_range(a, c);
}

void ds_vector_push_all(ds_vector_t *vector, zval *values)
{
    if ( ! values) {
        return;
    }

    if (Z_TYPE_P(values) == IS_ARRAY) {
        add_array_to_vector(vector, Z_ARRVAL_P(values));
        return;
    }

    if (is_traversable(values)) {
        add_traversable_to_vector(vector, values);
        return;
    }

    ARRAY_OR_TRAVERSABLE_REQUIRED();
}

void ds_vector_pop(ds_vector_t *vector, zval *return_value)
{
    if (VECTOR_IS_EMPTY(vector)) {
        NOT_ALLOWED_WHEN_EMPTY();
        return;

    } else {
        zval *value = &vector->buffer[--vector->size];

        if (return_value) {
            ZVAL_COPY(return_value, value);
        }

        zval_ptr_dtor(value);
        ds_vector_check_compact(vector);
    }
}

void ds_vector_shift(ds_vector_t *vector, zval *return_value)
{
    zval *first = vector->buffer;

    if (VECTOR_IS_EMPTY(vector)) {
        NOT_ALLOWED_WHEN_EMPTY();
        return;
    }

    ZVAL_COPY(return_value, first);
    zval_ptr_dtor(first);

    memmove(first, first + 1, sizeof(zval) * (--vector->size));

    ds_vector_check_compact(vector);
}

zval *ds_vector_get_last(ds_vector_t *vector)
{
    if (VECTOR_IS_EMPTY(vector)) {
        NOT_ALLOWED_WHEN_EMPTY();
        return NULL;
    }

    return &vector->buffer[vector->size - 1];
}

zval *ds_vector_get_first(ds_vector_t *vector)
{
    if (VECTOR_IS_EMPTY(vector)) {
        NOT_ALLOWED_WHEN_EMPTY();
        return NULL;
    }

    return &vector->buffer[0];
}

void ds_vector_reverse(ds_vector_t *vector)
{
    reverse_zval_range(vector->buffer, vector->buffer + vector->size);
}

ds_vector_t *ds_vector_map(ds_vector_t *vector, FCI_PARAMS)
{
    zval *value;
    zval *buf = ALLOC_ZVAL_BUFFER(vector->size);
    zval *pos = buf;

    DS_VECTOR_FOREACH(vector, value) {
        zval param;
        zval retval;

        ZVAL_COPY_VALUE(&param, value);

        fci.param_count = 1;
        fci.params      = &param;
        fci.retval      = &retval;

        if (zend_call_function(&fci, &fci_cache) == FAILURE || Z_ISUNDEF(retval)) {
            efree(buf);
            return NULL;
        } else {
            ZVAL_COPY(pos, &retval);
        }

        pos++;
    }
    DS_VECTOR_FOREACH_END();

    return ds_vector_from_buffer_ex(buf, vector->size, vector->capacity);
}

ds_vector_t *ds_vector_filter(ds_vector_t *vector)
{
    if (VECTOR_IS_EMPTY(vector)) {
        return ds_vector();

    } else {
        zval *value;
        zval *buf = ALLOC_ZVAL_BUFFER(vector->size);
        zval *pos = buf;

        zend_long size = 0;

        DS_VECTOR_FOREACH(vector, value) {
            if (zend_is_true(value)) {
                ZVAL_COPY(pos++, value);
                size++;
            }
        }
        DS_VECTOR_FOREACH_END();

        return ds_vector_from_buffer_ex(buf, size, vector->size);
    }
}

ds_vector_t *ds_vector_filter_callback(ds_vector_t *vector, FCI_PARAMS)
{
    if (VECTOR_IS_EMPTY(vector)) {
        return ds_vector();

    } else {
        zval *value;
        zval *buf = ALLOC_ZVAL_BUFFER(vector->size);
        zval *pos = buf;

        DS_VECTOR_FOREACH(vector, value) {
            zval param;
            zval retval;

            ZVAL_COPY_VALUE(&param, value);

            fci.param_count = 1;
            fci.params      = &param;
            fci.retval      = &retval;

            if (zend_call_function(&fci, &fci_cache) == FAILURE || Z_ISUNDEF(retval)) {
                efree(buf);
                return NULL;
            } else if (zend_is_true(&retval)) {
                ZVAL_COPY(pos++, value);
            }
        }
        DS_VECTOR_FOREACH_END();

        return ds_vector_from_buffer_ex(buf, (pos - buf), vector->size);
    }
}

void ds_vector_reduce(ds_vector_t *vector, zval *initial, zval *return_value, FCI_PARAMS)
{
    zval carry;

    zval *pos = vector->buffer;
    zval *end = pos + vector->size;

    if (initial == NULL) {
        ZVAL_NULL(&carry);
    } else {
        ZVAL_COPY_VALUE(&carry, initial);
    }

    for (; pos < end; ++pos) {
        zval params[2];
        zval retval;

        ZVAL_COPY_VALUE(&params[0], &carry);
        ZVAL_COPY_VALUE(&params[1], pos);

        fci.param_count = 2;
        fci.params      = params;
        fci.retval      = &retval;

        if (zend_call_function(&fci, &fci_cache) == FAILURE || Z_ISUNDEF(retval)) {
            ZVAL_NULL(return_value);
            return;
        } else {
            ZVAL_COPY_VALUE(&carry, &retval);
        }
    }

    ZVAL_COPY(return_value, &carry);
}

ds_vector_t *ds_vector_slice(ds_vector_t *vector, zend_long index, zend_long length)
{
    normalize_slice_params(&index, &length, vector->size);

    if (length == 0) {
        return ds_vector();

    } else {
        zval *src, *dst, *end;
        zval *buffer = ALLOC_ZVAL_BUFFER(length);

        src = vector->buffer + index;
        dst = buffer;
        end = src + length;

        for (; src < end; ++src, ++dst) {
            ZVAL_COPY(dst, src);
        }

        return ds_vector_from_buffer(buffer, length);
    }
}

void ds_vector_destroy(ds_vector_t *vector)
{
    ds_vector_clear(vector);
    efree(vector->buffer);
    efree(vector);
}

