#include "../common.h"
// #include "../internal/ds_vector.h"
#include "../php/php_ds_vector.h"
#include "../iterators/php_vector_iterator.h"
#include "../handlers/php_vector_handlers.h"
#include "php_ce_sequence.h"
#include "php_ce_vector.h"

#define METHOD(name) PHP_METHOD(Vector, name)

METHOD(__construct)
{
    PARSE_OPTIONAL_ZVAL(values);

    if (values) {
        if (Z_TYPE_P(values) == IS_LONG) {
            ds_vector_user_allocate(THIS_VECTOR(), Z_LVAL_P(values));
        } else {
            ds_vector_push_all(THIS_VECTOR(), values);
        }
    }
}

METHOD(allocate)
{
    PARSE_LONG(capacity);
    ds_vector_user_allocate(THIS_VECTOR(), capacity);
}

METHOD(capacity)
{
    PARSE_NONE;
    RETURN_LONG((THIS_VECTOR())->capacity);
}

METHOD(clear)
{
    PARSE_NONE;
    ds_vector_clear(THIS_VECTOR());
}

METHOD(contains)
{
    PARSE_VARIADIC_ZVAL();
    RETURN_BOOL(ds_vector_contains_va(THIS_VECTOR(), argc, argv));
}

METHOD(copy)
{
    PARSE_NONE;
    RETURN_OBJ(php_ds_vector_create_clone(THIS_VECTOR()));
}

METHOD(count)
{
    PARSE_NONE;
    RETURN_LONG(VECTOR_SIZE(THIS_VECTOR()));
}

METHOD(filter)
{
    if (ZEND_NUM_ARGS()) {
        PARSE_CALLABLE();
        RETURN_VECTOR(ds_vector_filter_callback(THIS_VECTOR(), FCI_ARGS));
    } else {
        RETURN_VECTOR(ds_vector_filter(THIS_VECTOR()));
    }
}

METHOD(find)
{
    PARSE_ZVAL(value);
    ds_vector_find(THIS_VECTOR(), value, return_value);
}

METHOD(first)
{
    PARSE_NONE;
    RETURN_ZVAL_COPY(ds_vector_get_first(THIS_VECTOR()));
}

METHOD(get)
{
    PARSE_LONG(index);
    RETURN_ZVAL_COPY(ds_vector_get(THIS_VECTOR(), index));
}

METHOD(insert)
{
    PARSE_LONG_AND_VARARGS(index);
    ds_vector_insert_va(THIS_VECTOR(), index, argc, argv);
}

METHOD(isEmpty)
{
    PARSE_NONE;
    RETURN_BOOL(VECTOR_IS_EMPTY(THIS_VECTOR()));
}

METHOD(join)
{
    if (ZEND_NUM_ARGS()) {
        PARSE_STRING();
        ds_vector_join(THIS_VECTOR(), str, len, return_value);
    } else {
        ds_vector_join(THIS_VECTOR(), NULL, 0, return_value);
    }
}

METHOD(jsonSerialize)
{
    PARSE_NONE;
    ds_vector_to_array(THIS_VECTOR(), return_value);
}

METHOD(last)
{
    PARSE_NONE;
    RETURN_ZVAL_COPY(ds_vector_get_last(THIS_VECTOR()));
}

METHOD(map)
{
    PARSE_CALLABLE();
    RETURN_VECTOR(ds_vector_map(THIS_VECTOR(), FCI_ARGS));
}

METHOD(pop)
{
    PARSE_NONE;
    ds_vector_pop(THIS_VECTOR(), return_value);
}

METHOD(push)
{
    PARSE_VARIADIC_ZVAL();
    ds_vector_push_va(THIS_VECTOR(), argc, argv);
}

METHOD(pushAll)
{
    PARSE_ZVAL(values);
    ds_vector_push_all(THIS_VECTOR(), values);
}

METHOD(reduce)
{
    PARSE_CALLABLE_AND_OPTIONAL_ZVAL(initial);
    ds_vector_reduce(THIS_VECTOR(), initial, return_value, FCI_ARGS);
}

METHOD(remove)
{
    PARSE_LONG(index);
    ds_vector_remove(THIS_VECTOR(), index, return_value);
}

METHOD(reverse)
{
    PARSE_NONE;
    {
        ds_vector_t *vector = ds_vector_clone(THIS_VECTOR());
        ds_vector_reverse(vector);
        RETURN_VECTOR(vector);
    }
}

METHOD(rotate)
{
    PARSE_LONG(rotations);
    ds_vector_rotate(THIS_VECTOR(), rotations);
}

METHOD(set)
{
    PARSE_LONG_AND_ZVAL(index, value);
    ds_vector_set(THIS_VECTOR(), index, value);
}

METHOD(shift)
{
    PARSE_NONE;
    ds_vector_shift(THIS_VECTOR(), return_value);
}

METHOD(slice)
{
    ds_vector_t *vector = THIS_VECTOR();

    if (ZEND_NUM_ARGS() > 1) {
        PARSE_LONG_AND_LONG(index, length);
        RETURN_VECTOR(ds_vector_slice(vector, index, length));
    } else {
        PARSE_LONG(index);
        RETURN_VECTOR(ds_vector_slice(vector, index, vector->size));
    }
}

METHOD(sort)
{
    ds_vector_t *vector = ds_vector_clone(THIS_VECTOR());

    if (ZEND_NUM_ARGS()) {
        PARSE_COMPARE_CALLABLE();
        ds_vector_sort_callback(vector);
    } else {
        ds_vector_sort(vector);
    }

    RETURN_VECTOR(vector);
}

METHOD(toArray)
{
    PARSE_NONE;
    ds_vector_to_array(THIS_VECTOR(), return_value);
}

METHOD(unshift)
{
    PARSE_VARIADIC_ZVAL();
    ds_vector_unshift_va(THIS_VECTOR(), argc, argv);
}

void register_vector()
{
    zend_class_entry ce;

    zend_function_entry methods[] = {
        SEQUENCE_ME_LIST(Vector)
        COLLECTION_ME_LIST(Vector)
        PHP_FE_END
    };

    INIT_CLASS_ENTRY(ce, COLLECTION_NS(Vector), methods);

    ds_vector_ce = zend_register_internal_class(&ce);
    ds_vector_ce->ce_flags      |= ZEND_ACC_FINAL;
    ds_vector_ce->create_object  = php_ds_vector_create_object;
    ds_vector_ce->get_iterator   = ds_vector_get_iterator;
    ds_vector_ce->serialize      = php_ds_vector_serialize;
    ds_vector_ce->unserialize    = php_ds_vector_unserialize;

    zend_class_implements(ds_vector_ce, 1, sequence_ce);
    register_vector_handlers();
}
