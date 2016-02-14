
#include "php.h"
#include "ext/spl/spl_exceptions.h"
#include "../common.h"
#include "../internal/ds_vector.h"
#include "../php/php_ds_vector.h"

zend_object_handlers ds_vector_object_handlers;

static zval *ds_vector_read_dimension(zval *obj, zval *offset, int type, zval *return_value)
{
    if (Z_TYPE_P(offset) != IS_LONG) {
        INTEGER_INDEX_REQUIRED(offset);
        return NULL;

    } else {
        zval *value = ds_vector_get(Z_VECTOR_P(obj), Z_LVAL_P(offset));

        // Create a reference to handle nested array access
        if (value && type != BP_VAR_R) {
            ZVAL_MAKE_REF(value);
        }

        return value;
    }
}

static void ds_vector_write_dimension(zval *obj, zval *offset, zval *value)
{
    ds_vector_t *vector = Z_VECTOR_P(obj);

    /* $v[] = ... */
    if (offset == NULL) {
        ds_vector_push(vector, value);

    } else if (Z_TYPE_P(offset) != IS_LONG) {
        INTEGER_INDEX_REQUIRED(offset);

    } else {
        ds_vector_set(vector, Z_LVAL_P(offset), value);
    }
}

static int ds_vector_has_dimension(zval *obj, zval *offset, int check_empty)
{
    if (Z_TYPE_P(offset) != IS_LONG) {
        return 0;
    }

    return ds_vector_isset(Z_VECTOR_P(obj), Z_LVAL_P(offset), check_empty);
}

static void ds_vector_unset_dimension(zval *obj, zval *offset)
{
    zend_long index;
    ds_vector_t *vector = Z_VECTOR_P(obj);

    if (Z_TYPE_P(offset) == IS_LONG) {
        index = Z_LVAL_P(offset);

    } else {
        if (zend_parse_parameter(ZEND_PARSE_PARAMS_QUIET, 1, offset, "l", &index) == FAILURE) {
            return;
        }
    }

    if (ds_vector_index_exists(vector, index)) { // to avoid OOB
        ds_vector_remove(vector, index, NULL);
    }
}

static int ds_vector_count_elements(zval *obj, zend_long *count)
{
    *count = Z_VECTOR_P(obj)->size;
    return SUCCESS;
}

static void ds_vector_free_object(zend_object *obj)
{
    php_ds_vector_t *intern = (php_ds_vector_t*) obj;
    zend_object_std_dtor(&intern->std);
    ds_vector_destroy(intern->vector);
}

static HashTable *ds_vector_get_debug_info(zval *obj, int *is_temp)
{
    zval arr;
    ds_vector_t *vector = Z_VECTOR_P(obj);

    *is_temp = 1;

    ds_vector_to_array(vector, &arr);
    return Z_ARRVAL(arr);
}

static zend_object *ds_vector_clone_obj(zval *obj)
{
    return php_ds_vector_create_clone(Z_VECTOR_P(obj));
}

static HashTable *ds_vector_get_gc(zval *obj, zval **gc_data, int *gc_count)
{
    ds_vector_t *vector = Z_VECTOR_P(obj);

    *gc_data  = vector->buffer;
    *gc_count = (int) vector->size;

    return NULL;
}

void register_vector_handlers()
{
    memcpy(&ds_vector_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    ds_vector_object_handlers.offset = XtOffsetOf(php_ds_vector_t, std);

    ds_vector_object_handlers.dtor_obj = zend_objects_destroy_object;
    ds_vector_object_handlers.free_obj = ds_vector_free_object;
    ds_vector_object_handlers.get_gc   = ds_vector_get_gc;

    ds_vector_object_handlers.clone_obj        = ds_vector_clone_obj;
    ds_vector_object_handlers.cast_object      = ds_default_cast_object;
    ds_vector_object_handlers.get_debug_info   = ds_vector_get_debug_info;

    ds_vector_object_handlers.count_elements   = ds_vector_count_elements;
    ds_vector_object_handlers.read_dimension   = ds_vector_read_dimension;
    ds_vector_object_handlers.write_dimension  = ds_vector_write_dimension;
    ds_vector_object_handlers.has_dimension    = ds_vector_has_dimension;
    ds_vector_object_handlers.unset_dimension  = ds_vector_unset_dimension;
}
