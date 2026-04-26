#ifndef ALTARR_H
#define ALTARR_H

#ifndef ALTARR_CUSTOM_ALLOCATORS
#     include <stdlib.h>
#     define ALTARR_MALLOC malloc
#     define ALTARR_REALLOC realloc
#     define ALTARR_FREE free
     typedef size_t ialtarr_size;
#else
#    include <stdlib.h>
     typedef size_t ialtarr_size;
#endif


struct ialtarr_handle {
     ialtarr_size length;
     ialtarr_size capacity;
     void* data;
};

#ifndef ALTARR_NO_DIAGNOSTICS
void iAltarrDiagBounds(struct ialtarr_handle* handle, const char* handle_name, ialtarr_size index, const char* file, ialtarr_size line, const char* func);
#define M_ALTARR_CHECKED_INNER(...) __VA_ARGS__)
#define M_ALTARR_CHECKED(...) (__VA_ARGS__, M_ALTARR_CHECKED_INNER
#else
#define M_ALTARR_CHECKED(...)
#endif

#define M_ALTARR_GET_1ST_(x, ...) x
#define M_ALTARR_GET_1ST(x, ...) M_ALTARR_GET_1ST_(x, _)
#define M_ALTARR_GET_REST(x, ...) __VA_ARGS__
#define M_ALTARR_TYPE_SIZE(type) sizeof((union altarr_type_##type){0}.size_[0])
#define M_ALTARR_CONCAT2(a, b) a##b
#define M_ALTARR_CONCAT(a, b) M_ALTARR_CONCAT2(a, b)
#define M_ALTARR_STRINGIFY(x) #x

#if __STDC_VERSION__ <= 201710L
#     include <stdbool.h>
#     include <assert.h>
#     define altarr_typedef(...) union M_ALTARR_CONCAT(altarr_type_, M_ALTARR_GET_1ST(M_ALTARR_GET_REST(__VA_ARGS__, __VA_ARGS__), _)) { \
                                                       struct ialtarr_handle handle;\
                                                       typeof(M_ALTARR_GET_1ST(__VA_ARGS__, _)) *type_; char (*size_)[sizeof(M_ALTARR_GET_1ST(__VA_ARGS__, _))]; }
#     define altarr_t(...) union M_ALTARR_CONCAT(altarr_type_, M_ALTARR_GET_1ST(M_ALTARR_GET_REST(__VA_ARGS__, __VA_ARGS__), _))
#     define typeof(...) __typeof__(__VA_ARGS__)
#else
#     define altarr_typedef(...) [[deprecated("Use altarr_t directly")]] typedef char altarr_typedef; typedef altarr_typedef ALTARR_WARN_DEPRECATED
#     define altarr_t(type, ...) union M_ALTARR_CONCAT(altarr_type_, M_ALTARR_GET_1ST(__VA_ARGS__ __VA_OPT__(,) type)) { struct ialtarr_handle handle; typeof(type) *type_; char (*size_)[sizeof(type)]; }
#endif

#define altarrCreate(type) (union altarr_type_##type){iAltarrCreate(1, M_ALTARR_TYPE_SIZE(type))}
#define altarrClear(arr) iAltarrClear(&arr.handle)
#define altarrResize(arr, amount) iAltarrResize(&arr.handle, amount, altarrElemSize(arr))
#define altarrDestroy(arr) iAltarrDestroy(&arr.handle)

#define altarrValid(arr) (!!arr.handle.data)
#define altarrRaw(arr) ((typeof((arr).type_))(arr).handle.data)
#define altarrLength(arr) (const ialtarr_size)((void)0, (arr).handle.length)
#define altarrCapacity(arr) (const ialtarr_size)((void)0, arr.handle.capacity)
#define altarrElemSize(arr) sizeof((arr).size_[0])

#define altarrAt(arr, index) M_ALTARR_CHECKED(iAltarrDiagBounds(&arr.handle, M_ALTARR_STRINGIFY(arr), index, __FILE__, __LINE__, __func__)) \
                             (altarrRaw(arr))[index]

#define altarrPush(arr, element) (iAltarrGrowMaybe(&(arr).handle, (arr).handle.length + 1, altarrElemSize(arr)) ? \
                                 (void)(altarrRaw(arr)[(arr).handle.length++] = element) : \
                                 (void)0)
#define altarrInsert(arr, at, element) (iAltarrShiftRight(&(arr).handle, at, 1, altarrElemSize(arr)) ? \
                                        (void)(altarrRaw(arr)[at] = element) : \
                                        (void)0)
#define altarrRemove(arr, at) iAltarrShiftLeft(&arr.handle, at, 1, altarrElemSize(arr))

#define altarrPop(arr) M_ALTARR_CHECKED(iAltarrDiagBounds(&arr.handle, M_ALTARR_STRINGIFY(arr), arr.handle.length - 1, __FILE__, __LINE__, __func__)) \
                       (altarrRaw(arr))[--arr.handle.length]
#define altarrJoin(arr_from, arr_to) _Generic(arr_from, typeof(arr_to): iAltarrJoin(&arr_to.handle, &arr_from.handle, altarrElemSize(arr_to)))
#define altarrChop(arr, at) TODO

#define altarr_slice_t(type, ...) static_assert(false, "Sorry, slicing is not supported at the moment.")
#define altarrSlice(arr, at) static_assert(false, "Sorry, slicing is not supported at the moment.")

struct ialtarr_handle iAltarrCreate(ialtarr_size initial_capacity, ialtarr_size element_size);
void iAltarrDestroy(struct ialtarr_handle *handle);
bool iAltarrGrowToCapacity(struct ialtarr_handle *handle, ialtarr_size n, ialtarr_size elem);
void iAltarrJoin(struct ialtarr_handle *to, struct ialtarr_handle *from, ialtarr_size elem);
bool iAltarrShiftRight(struct ialtarr_handle *handle, ialtarr_size at, ialtarr_size n, ialtarr_size elem);
bool iAltarrShiftLeft(struct ialtarr_handle *handle, ialtarr_size at, ialtarr_size n, ialtarr_size elem);

static inline void iAltarrClear(struct ialtarr_handle *handle)
{
    handle->length = 0;
}

static inline bool iAltarrGrowMaybe(struct ialtarr_handle *handle, ialtarr_size n, ialtarr_size elem)
{
    if(handle->length + n < handle->capacity)
        return true;

    return iAltarrGrowToCapacity(handle, handle->capacity * 2, elem);
}

static inline bool iAltarrResize(struct ialtarr_handle *handle, ialtarr_size new_size, ialtarr_size elem)
{
    handle->length = new_size;
    if(new_size < handle->capacity)
        return true;

    new_size = handle->capacity * 2 > new_size ? handle->capacity * 2 : new_size;

    return iAltarrGrowToCapacity(handle, new_size, elem);
}

#ifdef ALTARR_IMPLEMENTATION
#include <string.h>

struct ialtarr_handle iAltarrCreate(ialtarr_size initial_capacity, ialtarr_size element_size)
{
    void* data = ALTARR_MALLOC(initial_capacity * element_size);
    return (data ? (struct ialtarr_handle){.length = 0, .capacity = initial_capacity, data} : (struct ialtarr_handle){0});
}

void iAltarrDestroy(struct ialtarr_handle *handle)
{
    ALTARR_FREE(handle->data);
    handle->data = 0;
    handle->length = 0;
    handle->capacity = 0;
}

bool iAltarrShiftRight(struct ialtarr_handle *handle, ialtarr_size at, ialtarr_size n, ialtarr_size elem)
{
    if(!iAltarrGrowMaybe(handle, handle->length + n, elem))
        return false;
    char *data = (char*)handle->data + at * elem;
    memmove(data + n * elem, data, (handle->length - n) * elem);
    handle->length += n;
    return true;
}

bool iAltarrShiftLeft(struct ialtarr_handle *handle, ialtarr_size at, ialtarr_size n, ialtarr_size elem)
{
    if(at > handle->length - 1 || handle->length - n < 0)
        return false;

    char *data = (char*)handle->data + at * elem;
    memmove(data, data + n * elem, (handle->length - at - 1) * elem);
    handle->length -= n;
    return true;
}

bool iAltarrGrowToCapacity(struct ialtarr_handle *handle, ialtarr_size n, ialtarr_size elem)
{
    void* new_data = ALTARR_REALLOC(handle->data, n * elem);
    if(!new_data) return false;

    handle->data = new_data;
    handle->capacity = n;

    return true;
}

void iAltarrJoin(struct ialtarr_handle * restrict to, struct ialtarr_handle * restrict from, ialtarr_size elem)
{
    if(!to || !from) return;

    iAltarrGrowToCapacity(to, to->length + from->length, elem);
    memcpy((char*)to->data + to->length * elem, from->data, from->length * elem);
    to->length += from->length;
}

#ifndef ALTARR_NO_DIAGNOSTICS
#include <stdio.h>
#define ALTARR_WARN(file, line, text) fprintf(stderr, "[altarr : " "\x1B[33m" "WARNING" "\x1B[0m" "]: " "\x1B[33m" "%s" "\x1B[0m" " (in %s:%lu)\n", text, file, line)
#define ALTARR_ERR(file, line, text) fprintf(stderr, "[altarr : " "\x1B[31m" "ERROR" "\x1B[0m" "]:" "\x1B[33m" " %s " "\x1B[0m" " (in %s:%lu)\n", text, file, line)
#define ALTARR_ERR_(text, ...) fprintf(stderr, "[altarr : " "\x1B[31m" "ERROR" "\x1B[0m" "]: " text "\x1B[0m" "\n", __VA_ARGS__)
#define ALTARR_WARN_(text, ...) fprintf(stderr, "[altarr : " "\x1B[31m" "WARNING" "\x1B[0m" "]: " text "\x1B[0m" "\n", __VA_ARGS__)

void iAltarrDiagBounds(struct ialtarr_handle* handle, const char* handle_name, ialtarr_size index, const char* file, ialtarr_size line, const char* func)
{
    if(index >= handle->capacity) {
        ALTARR_ERR_("\x1B[33m" "Index is out of bounds at " "\x1B[0m" "%s:%lu.\n"
        "    %s(): %s[" "\x1B[31m" "%lu" "\x1B[0m" "] (note: capacity = %lu)", file, line, func, handle_name, index, handle->capacity);
        return;
    }

    if(index >= handle->length)
        ALTARR_WARN_("\x1B[33m" "Index is greater than altarr length " "\x1B[0m" "%s:%lu.\n"
        "    %s(): %s[" "\x1B[31m" "%lu" "\x1B[0m" "] (note: length = %lu)", file, line, func, handle_name, index, handle->length);
}

#endif
#endif
#endif
