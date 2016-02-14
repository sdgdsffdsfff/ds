#include "../common.h"
#include "../iterators/php_vector_iterator.h"
#include "../handlers/php_vector_handlers.h"
#include "php_ds_vector.h"

zend_class_entry *ds_vector_ce;

zend_object *php_ds_vector_create_object_ex(ds_vector_t *vector)
{
    php_ds_vector_t *obj = ecalloc(1, sizeof(php_ds_vector_t));
    zend_object_std_init(&obj->std, ds_vector_ce);
    obj->std.handlers = &ds_vector_object_handlers;
    obj->vector = vector;
    return &obj->std;
}

zend_object *php_ds_vector_create_object(zend_class_entry *ce)
{
    return php_ds_vector_create_object_ex(ds_vector());
}

zend_object *php_ds_vector_create_clone(ds_vector_t *vector)
{
    return php_ds_vector_create_object_ex(ds_vector_clone(vector));
}

int php_ds_vector_serialize(zval *object, unsigned char **buffer, size_t *length, zend_serialize_data *data)
{
    ds_vector_t *vector = Z_VECTOR_P(object);

    php_serialize_data_t serialize_data = (php_serialize_data_t) data;
    PHP_VAR_SERIALIZE_INIT(serialize_data);

    if (VECTOR_IS_EMPTY(vector)) {
        SERIALIZE_SET_ZSTR(ZSTR_EMPTY_ALLOC());

    } else {
        zval *value;
        smart_str buf = {0};

        DS_VECTOR_FOREACH(vector, value) {
            php_var_serialize(&buf, value, &serialize_data);
        }
        DS_VECTOR_FOREACH_END();

        smart_str_0(&buf);
        SERIALIZE_SET_ZSTR(buf.s);
        zend_string_release(buf.s);
    }

    PHP_VAR_SERIALIZE_DESTROY(serialize_data);
    return SUCCESS;
}

int php_ds_vector_unserialize(zval *obj, zend_class_entry *ce, const unsigned char *buffer, size_t length, zend_unserialize_data *data)
{
    ds_vector_t *vector = ds_vector();

    php_unserialize_data_t unserialize_data = (php_unserialize_data_t) data;

    const unsigned char *pos = buffer;
    const unsigned char *max = buffer + length;

    PHP_VAR_UNSERIALIZE_INIT(unserialize_data);

    while (*pos != '}') {
        zval *value = var_tmp_var(&unserialize_data);

        if (php_var_unserialize(value, &pos, max, &unserialize_data)) {
            var_push_dtor(&unserialize_data, value);
        } else {
            goto error;
        }

        ds_vector_push(vector, value);
    }

    if (*(++pos) != '\0') {
        goto error;
    }

    ZVAL_VECTOR(obj, vector);
    PHP_VAR_UNSERIALIZE_DESTROY(unserialize_data);
    return SUCCESS;

error:
    PHP_VAR_UNSERIALIZE_DESTROY(unserialize_data);
    UNSERIALIZE_ERROR();
    return FAILURE;
}
