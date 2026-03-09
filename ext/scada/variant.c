#include "scada.h"
#include <ruby/encoding.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

const UA_DataType *scada_resolve_type_symbol(VALUE sym) {
    if (TYPE(sym) != T_SYMBOL)
        rb_raise(rb_eArgError, "type must be a Symbol");

    ID id = SYM2ID(sym);

    if (id == rb_intern("boolean"))     return &UA_TYPES[UA_TYPES_BOOLEAN];
    if (id == rb_intern("sbyte"))       return &UA_TYPES[UA_TYPES_SBYTE];
    if (id == rb_intern("byte"))        return &UA_TYPES[UA_TYPES_BYTE];
    if (id == rb_intern("int16"))       return &UA_TYPES[UA_TYPES_INT16];
    if (id == rb_intern("uint16"))      return &UA_TYPES[UA_TYPES_UINT16];
    if (id == rb_intern("int32"))       return &UA_TYPES[UA_TYPES_INT32];
    if (id == rb_intern("uint32"))      return &UA_TYPES[UA_TYPES_UINT32];
    if (id == rb_intern("int64"))       return &UA_TYPES[UA_TYPES_INT64];
    if (id == rb_intern("uint64"))      return &UA_TYPES[UA_TYPES_UINT64];
    if (id == rb_intern("float"))       return &UA_TYPES[UA_TYPES_FLOAT];
    if (id == rb_intern("double"))      return &UA_TYPES[UA_TYPES_DOUBLE];
    if (id == rb_intern("string"))      return &UA_TYPES[UA_TYPES_STRING];
    if (id == rb_intern("datetime"))    return &UA_TYPES[UA_TYPES_DATETIME];
    if (id == rb_intern("byte_string")) return &UA_TYPES[UA_TYPES_BYTESTRING];
    if (id == rb_intern("node_id"))     return &UA_TYPES[UA_TYPES_NODEID];

    rb_raise(rb_eArgError, "Unknown OPC UA type: %s", rb_id2name(id));
    return NULL;
}

VALUE scada_variant_to_ruby(const UA_Variant *var) {
    if (UA_Variant_isEmpty(var)) return Qnil;

    /* Handle arrays */
    if (!UA_Variant_isScalar(var)) {
        size_t len = var->arrayLength;
        VALUE ary = rb_ary_new_capa(len);
        for (size_t i = 0; i < len; i++) {
            UA_Variant elem;
            UA_Variant_init(&elem);
            elem.type = var->type;
            elem.data = (void *)((char *)var->data + i * var->type->memSize);
            elem.storageType = UA_VARIANT_DATA_NODELETE;
            rb_ary_push(ary, scada_variant_to_ruby(&elem));
        }
        return ary;
    }

    if (var->type == &UA_TYPES[UA_TYPES_BOOLEAN])
        return *(UA_Boolean *)var->data ? Qtrue : Qfalse;
    if (var->type == &UA_TYPES[UA_TYPES_SBYTE])
        return INT2FIX(*(UA_SByte *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_BYTE])
        return INT2FIX(*(UA_Byte *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_INT16])
        return INT2FIX(*(UA_Int16 *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_UINT16])
        return UINT2NUM(*(UA_UInt16 *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_INT32])
        return INT2NUM(*(UA_Int32 *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_UINT32])
        return UINT2NUM(*(UA_UInt32 *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_INT64])
        return LL2NUM(*(UA_Int64 *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_UINT64])
        return ULL2NUM(*(UA_UInt64 *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_FLOAT])
        return DBL2NUM((double)*(UA_Float *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_DOUBLE])
        return DBL2NUM(*(UA_Double *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_STRING]) {
        UA_String *s = (UA_String *)var->data;
        if (s->data == NULL) return Qnil;
        return rb_utf8_str_new((const char *)s->data, s->length);
    }
    if (var->type == &UA_TYPES[UA_TYPES_DATETIME]) {
        UA_DateTime dt = *(UA_DateTime *)var->data;
        UA_DateTimeStruct dts = UA_DateTime_toStruct(dt);
        return rb_funcall(rb_cTime, rb_intern("utc"), 7,
            INT2NUM(dts.year), INT2NUM(dts.month), INT2NUM(dts.day),
            INT2NUM(dts.hour), INT2NUM(dts.min), INT2NUM(dts.sec),
            INT2NUM(dts.milliSec * 1000 + dts.microSec));
    }
    if (var->type == &UA_TYPES[UA_TYPES_BYTESTRING]) {
        UA_ByteString *bs = (UA_ByteString *)var->data;
        if (bs->data == NULL) return Qnil;
        VALUE str = rb_str_new((const char *)bs->data, bs->length);
        rb_enc_associate(str, rb_ascii8bit_encoding());
        return str;
    }
    if (var->type == &UA_TYPES[UA_TYPES_NODEID])
        return scada_node_id_wrap(*(UA_NodeId *)var->data);
    if (var->type == &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]) {
        UA_LocalizedText *lt = (UA_LocalizedText *)var->data;
        if (lt->text.data == NULL) return Qnil;
        return rb_utf8_str_new((const char *)lt->text.data, lt->text.length);
    }
    if (var->type == &UA_TYPES[UA_TYPES_QUALIFIEDNAME]) {
        UA_QualifiedName *qn = (UA_QualifiedName *)var->data;
        if (qn->name.data == NULL) return Qnil;
        return rb_utf8_str_new((const char *)qn->name.data, qn->name.length);
    }

    return Qnil;
}

void scada_ruby_to_variant(VALUE rb_val, VALUE rb_type_sym, UA_Variant *var) {
    const UA_DataType *type = scada_resolve_type_symbol(rb_type_sym);

    /* Handle arrays */
    if (TYPE(rb_val) == T_ARRAY) {
        long len = RARRAY_LEN(rb_val);
        void *arr = UA_Array_new(len, type);
        for (long i = 0; i < len; i++) {
            UA_Variant tmp;
            UA_Variant_init(&tmp);
            scada_ruby_to_variant(rb_ary_entry(rb_val, i), rb_type_sym, &tmp);
            memcpy((char *)arr + i * type->memSize, tmp.data, type->memSize);
            UA_free(tmp.data);
        }
        UA_Variant_setArray(var, arr, len, type);
        return;
    }

    if (type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
        UA_Boolean val = RTEST(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_SBYTE]) {
        UA_SByte val = (UA_SByte)NUM2INT(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_BYTE]) {
        UA_Byte val = (UA_Byte)NUM2UINT(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_INT16]) {
        UA_Int16 val = (UA_Int16)NUM2INT(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_UINT16]) {
        UA_UInt16 val = (UA_UInt16)NUM2UINT(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_INT32]) {
        UA_Int32 val = NUM2INT(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_UINT32]) {
        UA_UInt32 val = NUM2UINT(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_INT64]) {
        UA_Int64 val = NUM2LL(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_UINT64]) {
        UA_UInt64 val = NUM2ULL(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_FLOAT]) {
        UA_Float val = (UA_Float)NUM2DBL(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_DOUBLE]) {
        UA_Double val = NUM2DBL(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_STRING]) {
        UA_String val = UA_STRING_ALLOC(StringValueCStr(rb_val));
        UA_Variant_setScalarCopy(var, &val, type);
        UA_String_clear(&val);
    } else if (type == &UA_TYPES[UA_TYPES_DATETIME]) {
        double epoch = NUM2DBL(rb_funcall(rb_val, rb_intern("to_f"), 0));
        UA_DateTime val = UA_DateTime_fromUnixTime((UA_Int64)epoch);
        UA_Variant_setScalarCopy(var, &val, type);
    } else if (type == &UA_TYPES[UA_TYPES_BYTESTRING]) {
        StringValue(rb_val);
        UA_ByteString val;
        val.length = RSTRING_LEN(rb_val);
        val.data = (UA_Byte *)UA_malloc(val.length);
        memcpy(val.data, RSTRING_PTR(rb_val), val.length);
        UA_Variant_setScalarCopy(var, &val, type);
        UA_ByteString_clear(&val);
    } else if (type == &UA_TYPES[UA_TYPES_NODEID]) {
        UA_NodeId val = scada_node_id_unwrap(rb_val);
        UA_Variant_setScalarCopy(var, &val, type);
    } else {
        rb_raise(rb_eArgError, "Unsupported type for variant conversion");
    }
}

UA_LocalizedText scada_to_localized_text(VALUE rb_text) {
    if (TYPE(rb_text) == T_STRING) {
        return UA_LOCALIZEDTEXT_ALLOC("en-US", StringValueCStr(rb_text));
    } else if (TYPE(rb_text) == T_HASH) {
        VALUE keys = rb_funcall(rb_text, rb_intern("keys"), 0);
        VALUE first_key = rb_ary_entry(keys, 0);
        VALUE first_val = rb_hash_aref(rb_text, first_key);
        return UA_LOCALIZEDTEXT_ALLOC(StringValueCStr(first_key),
                                       StringValueCStr(first_val));
    }
    return UA_LOCALIZEDTEXT_ALLOC("en-US", "");
}

UA_ByteString scada_to_bytestring(VALUE rb_val) {
    if (NIL_P(rb_val)) return UA_BYTESTRING_NULL;

    /* If it responds to #to_der (OpenSSL::X509::Certificate, OpenSSL::PKey), use that */
    if (rb_respond_to(rb_val, rb_intern("to_der")))
        rb_val = rb_funcall(rb_val, rb_intern("to_der"), 0);

    /* If it responds to #read (Pathname, IO), read contents */
    if (rb_respond_to(rb_val, rb_intern("read")))
        rb_val = rb_funcall(rb_val, rb_intern("read"), 0);

    StringValue(rb_val);
    UA_ByteString bs;
    bs.length = RSTRING_LEN(rb_val);
    bs.data = (UA_Byte *)UA_malloc(bs.length);
    memcpy(bs.data, RSTRING_PTR(rb_val), bs.length);
    return bs;
}

VALUE scada_data_value_to_ruby(const UA_DataValue *dv) {
    VALUE val = Qnil, status = Qnil, source_ts = Qnil, server_ts = Qnil;

    if (dv->hasValue)
        val = scada_variant_to_ruby(&dv->value);

    if (dv->hasStatus)
        status = rb_funcall(rb_cStatus, rb_intern("new"), 1, UINT2NUM(dv->status));
    else
        status = rb_funcall(rb_cStatus, rb_intern("new"), 1, UINT2NUM(UA_STATUSCODE_GOOD));

    if (dv->hasSourceTimestamp) {
        double epoch = (double)(dv->sourceTimestamp - UA_DATETIME_UNIX_EPOCH) / 1e7;
        source_ts = rb_funcall(rb_cTime, rb_intern("at"), 1, DBL2NUM(epoch));
    }
    if (dv->hasServerTimestamp) {
        double epoch = (double)(dv->serverTimestamp - UA_DATETIME_UNIX_EPOCH) / 1e7;
        server_ts = rb_funcall(rb_cTime, rb_intern("at"), 1, DBL2NUM(epoch));
    }

    return rb_funcall(rb_cDataValue, rb_intern("new"), 4,
                      val, status, source_ts, server_ts);
}

/* --- Custom logger --- */

typedef struct {
    VALUE proc;
    int active;     /* 0 = suppress logging (e.g. during shutdown) */
    int has_gvl;    /* 1 = currently holding GVL (safe for direct rb_funcall) */
} ScadaLoggerCtx;

static const char *log_category_name(UA_LogCategory cat) {
    switch (cat) {
        case UA_LOGCATEGORY_NETWORK:        return "network";
        case UA_LOGCATEGORY_SECURECHANNEL:  return "securechannel";
        case UA_LOGCATEGORY_SESSION:        return "session";
        case UA_LOGCATEGORY_SERVER:         return "server";
        case UA_LOGCATEGORY_CLIENT:         return "client";
        case UA_LOGCATEGORY_USERLAND:       return "userland";
        case UA_LOGCATEGORY_SECURITYPOLICY: return "securitypolicy";
        case UA_LOGCATEGORY_EVENTLOOP:      return "eventloop";
        case UA_LOGCATEGORY_PUBSUB:         return "pubsub";
        case UA_LOGCATEGORY_DISCOVERY:      return "discovery";
        default:                            return "unknown";
    }
}

static const char *log_level_name(UA_LogLevel level) {
    if (level <= UA_LOGLEVEL_TRACE)   return "trace";
    if (level <= UA_LOGLEVEL_DEBUG)   return "debug";
    if (level <= UA_LOGLEVEL_INFO)    return "info";
    if (level <= UA_LOGLEVEL_WARNING) return "warn";
    if (level <= UA_LOGLEVEL_ERROR)   return "error";
    return "fatal";
}

typedef struct {
    ScadaLoggerCtx *ctx;
    UA_LogLevel level;
    UA_LogCategory category;
    char message[1024];
} LogGVLArgs;

static void *logger_call_with_gvl(void *arg) {
    LogGVLArgs *a = arg;
    VALUE rb_cat = ID2SYM(rb_intern(log_category_name(a->category)));
    VALUE rb_level = ID2SYM(rb_intern(log_level_name(a->level)));
    VALUE rb_msg = rb_utf8_str_new_cstr(a->message);
    rb_funcall(a->ctx->proc, rb_intern("call"), 3, rb_cat, rb_level, rb_msg);
    return NULL;
}

static void scada_logger_log(void *logContext, UA_LogLevel level,
                              UA_LogCategory category, const char *msg, va_list args) {
    ScadaLoggerCtx *ctx = (ScadaLoggerCtx *)logContext;
    if (!ctx || NIL_P(ctx->proc) || !ctx->active) return;

    LogGVLArgs gvl_args;
    gvl_args.ctx = ctx;
    gvl_args.level = level;
    gvl_args.category = category;
    vsnprintf(gvl_args.message, sizeof(gvl_args.message), msg, args);

    if (ctx->has_gvl) {
        logger_call_with_gvl(&gvl_args);
    } else {
        rb_thread_call_with_gvl(logger_call_with_gvl, &gvl_args);
    }
}

static void scada_logger_clear(UA_Logger *logger) {
    if (logger->context) {
        xfree(logger->context);
        logger->context = NULL;
    }
}

/* Silent logger — pure C no-op, safe during shutdown */
static void scada_null_logger_log(void *logContext, UA_LogLevel level,
                                   UA_LogCategory category, const char *msg, va_list args) {
    (void)logContext; (void)level; (void)category; (void)msg; (void)args;
}

static void scada_null_logger_clear(UA_Logger *logger) {
    (void)logger;
}

void scada_deactivate_logger(UA_Logger *logger) {
    if (logger && logger->context && logger->log == scada_logger_log) {
        ScadaLoggerCtx *ctx = (ScadaLoggerCtx *)logger->context;
        ctx->active = 0;
    }
}

void scada_logger_set_gvl(UA_Logger *logger, int has_gvl) {
    if (logger && logger->context && logger->log == scada_logger_log) {
        ScadaLoggerCtx *ctx = (ScadaLoggerCtx *)logger->context;
        ctx->has_gvl = has_gvl;
    }
}

UA_Logger *scada_create_logger(VALUE rb_proc) {
    UA_Logger *logger = (UA_Logger *)UA_calloc(1, sizeof(UA_Logger));

    if (rb_proc == Qfalse || rb_proc == Qnil) {
        /* Silent logger — no Ruby calls, safe everywhere */
        logger->log = scada_null_logger_log;
        logger->context = NULL;
        logger->clear = scada_null_logger_clear;
    } else {
        ScadaLoggerCtx *ctx = ALLOC(ScadaLoggerCtx);
        ctx->proc = rb_proc;
        ctx->active = 1;
        ctx->has_gvl = 1; /* default: assume GVL is held */
        logger->log = scada_logger_log;
        logger->context = ctx;
        logger->clear = scada_logger_clear;
    }
    return logger;
}
