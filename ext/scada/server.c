#include "scada.h"
#include <string.h>

typedef struct {
    UA_Server *server;
    UA_ServerConfig *config;
    VALUE callback_procs;
    UA_Boolean running;
} ScadaServer;

static void server_mark(void *ptr) {
    ScadaServer *s = ptr;
    rb_gc_mark(s->callback_procs);
}

static void server_free(void *ptr) {
    ScadaServer *s = ptr;
    if (s->server) {
        if (s->config && s->config->logging)
            scada_deactivate_logger(s->config->logging);
        UA_Server_delete(s->server);
    }
    xfree(s);
}

static size_t server_memsize(const void *ptr) {
    (void)ptr;
    return sizeof(ScadaServer);
}

static const rb_data_type_t server_type = {
    .wrap_struct_name = "Scada::Server",
    .function = {
        .dmark = server_mark,
        .dfree = server_free,
        .dsize = server_memsize,
    },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE server_alloc(VALUE klass) {
    ScadaServer *s = ALLOC(ScadaServer);
    memset(s, 0, sizeof(ScadaServer));
    s->callback_procs = rb_ary_new();
    return TypedData_Wrap_Struct(klass, &server_type, s);
}

#define GET_SERVER(self, svar) \
    ScadaServer *svar; \
    TypedData_Get_Struct(self, ScadaServer, &server_type, svar)

static VALUE server_initialize(int argc, VALUE *argv, VALUE self) {
    VALUE kwargs;
    rb_scan_args(argc, argv, "0:", &kwargs);

    GET_SERVER(self, s);

    if (!NIL_P(kwargs)) {
        VALUE rb_config = Qnil;
        ID id_config = rb_intern("config");
        if (rb_hash_lookup2(kwargs, ID2SYM(id_config), Qundef) != Qundef)
            rb_config = rb_hash_aref(kwargs, ID2SYM(id_config));

        if (!NIL_P(rb_config)) {
            /* Build a zeroed config, set logger first, then call setMinimal.
             * UA_Server_new() would call setDefault internally, creating an
             * event loop with the default stdout logger before we can intervene. */
            UA_ServerConfig config;
            memset(&config, 0, sizeof(UA_ServerConfig));

            VALUE logger = rb_funcall(rb_config, rb_intern("logger"), 0);
            if (!NIL_P(logger)) {
                if (TYPE(logger) == T_SYMBOL && SYM2ID(logger) == rb_intern("silent")) {
                    config.logging = UA_Log_Stdout_new(UA_LOGLEVEL_FATAL + 100);
                } else {
                    rb_ary_push(s->callback_procs, logger);
                    config.logging = scada_create_logger(logger);
                }
            }

            VALUE port = rb_funcall(rb_config, rb_intern("port"), 0);
            UA_UInt16 portNum = NIL_P(port) ? 4840 : (UA_UInt16)NUM2INT(port);

            VALUE rb_cert = rb_funcall(rb_config, rb_intern("certificate"), 0);
            VALUE rb_key = rb_funcall(rb_config, rb_intern("private_key"), 0);
            VALUE rb_trust = rb_funcall(rb_config, rb_intern("trust_list"), 0);

#ifdef UA_ENABLE_ENCRYPTION
            if (!NIL_P(rb_cert) && !NIL_P(rb_key)) {
                UA_ByteString cert = scada_to_bytestring(rb_cert);
                UA_ByteString key = scada_to_bytestring(rb_key);

                /* Build trust list array */
                size_t trustListSize = 0;
                UA_ByteString *trustList = NULL;
                if (!NIL_P(rb_trust) && TYPE(rb_trust) == T_ARRAY) {
                    trustListSize = RARRAY_LEN(rb_trust);
                    if (trustListSize > 0) {
                        trustList = (UA_ByteString *)UA_calloc(
                            trustListSize, sizeof(UA_ByteString));
                        for (size_t i = 0; i < trustListSize; i++)
                            trustList[i] = scada_to_bytestring(
                                rb_ary_entry(rb_trust, (long)i));
                    }
                }

                UA_StatusCode rc = UA_ServerConfig_setDefaultWithSecurityPolicies(
                    &config, portNum, &cert, &key,
                    trustList, trustListSize,
                    NULL, 0,  /* issuer list */
                    NULL, 0); /* revocation list */

                UA_ByteString_clear(&cert);
                UA_ByteString_clear(&key);
                for (size_t i = 0; i < trustListSize; i++)
                    UA_ByteString_clear(&trustList[i]);
                UA_free(trustList);

                scada_check_status(rc);
            } else {
#endif
                UA_ServerConfig_setMinimal(&config, portNum, NULL);
#ifdef UA_ENABLE_ENCRYPTION
            }
#endif

            s->server = UA_Server_newWithConfig(&config);
            if (!s->server) rb_raise(rb_eRuntimeError, "Failed to create UA_Server");
            s->config = UA_Server_getConfig(s->server);
            s->config->tcpReuseAddr = true;

            VALUE app_name = rb_funcall(rb_config, rb_intern("application_name"), 0);
            if (!NIL_P(app_name)) {
                UA_LocalizedText_clear(&s->config->applicationDescription.applicationName);
                s->config->applicationDescription.applicationName =
                    UA_LOCALIZEDTEXT_ALLOC("en-US", StringValueCStr(app_name));
            }

            VALUE app_uri = rb_funcall(rb_config, rb_intern("application_uri"), 0);
            if (!NIL_P(app_uri)) {
                UA_String_clear(&s->config->applicationDescription.applicationUri);
                s->config->applicationDescription.applicationUri =
                    UA_STRING_ALLOC(StringValueCStr(app_uri));
            }

            VALUE product_uri = rb_funcall(rb_config, rb_intern("product_uri"), 0);
            if (!NIL_P(product_uri)) {
                UA_String_clear(&s->config->applicationDescription.productUri);
                s->config->applicationDescription.productUri =
                    UA_STRING_ALLOC(StringValueCStr(product_uri));
            }

            /* Allow password over unencrypted channels (v1.5+ default is false) */
            s->config->allowNonePolicyPassword = true;

            VALUE users = rb_funcall(rb_config, rb_intern("users"), 0);
            VALUE allow_anon = rb_funcall(rb_config, rb_intern("allow_anonymous"), 0);
            if (!NIL_P(users) && RHASH_SIZE(users) > 0) {
                long count = RHASH_SIZE(users);
                UA_UsernamePasswordLogin *logins = (UA_UsernamePasswordLogin *)
                    UA_calloc(count, sizeof(UA_UsernamePasswordLogin));
                VALUE keys = rb_funcall(users, rb_intern("keys"), 0);
                for (long i = 0; i < count; i++) {
                    VALUE k = rb_ary_entry(keys, i);
                    VALUE v = rb_hash_aref(users, k);
                    logins[i].username = UA_STRING_ALLOC(StringValueCStr(k));
                    logins[i].password = UA_STRING_ALLOC(StringValueCStr(v));
                }
                UA_Boolean allowAnon = NIL_P(allow_anon) ? UA_TRUE : RTEST(allow_anon);
                UA_AccessControl_default(s->config, allowAnon,
                    &s->config->securityPolicies[0].policyUri,
                    count, logins);
                for (long i = 0; i < count; i++) {
                    UA_String_clear(&logins[i].username);
                    UA_String_clear(&logins[i].password);
                }
                UA_free(logins);
            }
        } else {
            s->server = UA_Server_new();
            if (!s->server) rb_raise(rb_eRuntimeError, "Failed to create UA_Server");
            s->config = UA_Server_getConfig(s->server);
        }
    } else {
        s->server = UA_Server_new();
        if (!s->server) rb_raise(rb_eRuntimeError, "Failed to create UA_Server");
        s->config = UA_Server_getConfig(s->server);
    }

    return self;
}

static VALUE server_run_startup(VALUE self) {
    GET_SERVER(self, s);
    UA_StatusCode rc = UA_Server_run_startup(s->server);
    scada_check_status(rc);
    s->running = UA_TRUE;
    return Qnil;
}

typedef struct {
    UA_Server *server;
    UA_Boolean waitInternal;
    UA_ServerConfig *config;
} ServerIterateArgs;

static void *server_iterate_nogvl(void *arg) {
    ServerIterateArgs *a = arg;
    if (a->config && a->config->logging)
        scada_logger_set_gvl(a->config->logging, 0);
    UA_Server_run_iterate(a->server, a->waitInternal);
    if (a->config && a->config->logging)
        scada_logger_set_gvl(a->config->logging, 1);
    return NULL;
}

static VALUE server_run_iterate(int argc, VALUE *argv, VALUE self) {
    GET_SERVER(self, s);
    VALUE rb_wait;
    rb_scan_args(argc, argv, "01", &rb_wait);
    UA_Boolean wait = RTEST(rb_wait) ? UA_TRUE : UA_FALSE;
    ServerIterateArgs args = { s->server, wait, s->config };
    rb_thread_call_without_gvl(server_iterate_nogvl, &args,
                               RUBY_UBF_IO, NULL);
    return Qnil;
}

typedef struct {
    UA_Server *server;
    UA_ServerConfig *config;
    UA_StatusCode rc;
} ServerShutdownArgs;

static void *server_shutdown_nogvl(void *arg) {
    ServerShutdownArgs *a = arg;
    if (a->config && a->config->logging)
        scada_logger_set_gvl(a->config->logging, 0);
    a->rc = UA_Server_run_shutdown(a->server);
    if (a->config && a->config->logging)
        scada_logger_set_gvl(a->config->logging, 1);
    return NULL;
}

static VALUE server_run_shutdown(VALUE self) {
    GET_SERVER(self, s);
    s->running = UA_FALSE;
    ServerShutdownArgs args = { s->server, s->config, UA_STATUSCODE_GOOD };
    rb_thread_call_without_gvl(server_shutdown_nogvl, &args,
                               RUBY_UBF_IO, NULL);
    scada_check_status(args.rc);
    return Qnil;
}

static VALUE server_add_variable_node(VALUE self, VALUE rb_nid, VALUE rb_type,
                                       VALUE rb_value, VALUE rb_display_name,
                                       VALUE rb_browse_name, VALUE rb_write_cb) {
    GET_SERVER(self, s);

    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);
    const UA_DataType *type = scada_resolve_type_symbol(rb_type);

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = scada_to_localized_text(rb_display_name);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    attr.dataType = type->typeId;

    if (!NIL_P(rb_value))
        scada_ruby_to_variant(rb_value, rb_type, &attr.value);

    UA_QualifiedName browseName = UA_QUALIFIEDNAME_ALLOC(
        nodeId.namespaceIndex, StringValueCStr(rb_browse_name));

    UA_StatusCode rc = UA_Server_addVariableNode(
        s->server, nodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        browseName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        attr, NULL, NULL);

    UA_QualifiedName_clear(&browseName);
    UA_LocalizedText_clear(&attr.displayName);
    UA_Variant_clear(&attr.value);
    scada_check_status(rc);

    if (!NIL_P(rb_write_cb))
        rb_ary_push(s->callback_procs, rb_write_cb);

    return Qnil;
}

static VALUE server_write_value(VALUE self, VALUE rb_nid, VALUE rb_value, VALUE rb_type_sym) {
    GET_SERVER(self, s);
    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);

    UA_DataValue dv;
    UA_DataValue_init(&dv);
    dv.hasValue = true;
    scada_ruby_to_variant(rb_value, rb_type_sym, &dv.value);
    dv.hasSourceTimestamp = true;
    dv.sourceTimestamp = UA_DateTime_now();
    dv.hasServerTimestamp = true;
    dv.serverTimestamp = UA_DateTime_now();

    UA_StatusCode rc = UA_Server_writeDataValue(s->server, nodeId, dv);
    UA_DataValue_clear(&dv);
    scada_check_status(rc);
    return Qnil;
}

static VALUE server_add_object_node(VALUE self, VALUE rb_nid, VALUE rb_display_name,
                                     VALUE rb_browse_name, VALUE rb_parent) {
    GET_SERVER(self, s);

    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);
    UA_NodeId parentId = scada_node_id_unwrap(rb_parent);

    UA_ObjectAttributes attr = UA_ObjectAttributes_default;
    attr.displayName = scada_to_localized_text(rb_display_name);

    UA_QualifiedName browseName = UA_QUALIFIEDNAME_ALLOC(
        nodeId.namespaceIndex, StringValueCStr(rb_browse_name));

    UA_StatusCode rc = UA_Server_addObjectNode(
        s->server, nodeId, parentId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        browseName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
        attr, NULL, NULL);

    UA_QualifiedName_clear(&browseName);
    UA_LocalizedText_clear(&attr.displayName);
    scada_check_status(rc);
    return Qnil;
}

/* Data source context */
typedef struct {
    VALUE read_proc;
    VALUE write_proc;
    VALUE server_obj;
} ScadaDataSourceCtx;

static void *ds_read_with_gvl(void *arg) {
    ScadaDataSourceCtx *ctx = arg;
    return (void *)rb_funcall(ctx->read_proc, rb_intern("call"), 0);
}

static UA_StatusCode data_source_read_cb(UA_Server *server,
    const UA_NodeId *sessionId, void *sessionContext,
    const UA_NodeId *nodeId, void *nodeContext,
    UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
    UA_DataValue *dataValue) {

    (void)server; (void)sessionId; (void)sessionContext;
    (void)nodeId; (void)range;

    ScadaDataSourceCtx *ctx = (ScadaDataSourceCtx *)nodeContext;
    if (!ctx || NIL_P(ctx->read_proc))
        return UA_STATUSCODE_BADINTERNALERROR;

    VALUE result = (VALUE)rb_thread_call_with_gvl(ds_read_with_gvl, ctx);

    if (TYPE(result) == T_FLOAT) {
        UA_Double val = NUM2DBL(result);
        UA_Variant_setScalarCopy(&dataValue->value, &val, &UA_TYPES[UA_TYPES_DOUBLE]);
    } else if (TYPE(result) == T_STRING) {
        UA_String val = UA_STRING_ALLOC(StringValueCStr(result));
        UA_Variant_setScalarCopy(&dataValue->value, &val, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&val);
    } else if (FIXNUM_P(result) || TYPE(result) == T_BIGNUM) {
        UA_Int32 val = NUM2INT(result);
        UA_Variant_setScalarCopy(&dataValue->value, &val, &UA_TYPES[UA_TYPES_INT32]);
    } else if (result == Qtrue || result == Qfalse) {
        UA_Boolean val = RTEST(result);
        UA_Variant_setScalarCopy(&dataValue->value, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
    }

    dataValue->hasValue = true;
    if (sourceTimeStamp) {
        dataValue->hasSourceTimestamp = true;
        dataValue->sourceTimestamp = UA_DateTime_now();
    }
    return UA_STATUSCODE_GOOD;
}

typedef struct {
    ScadaDataSourceCtx *ctx;
    const UA_DataValue *new_dv;
} DSWriteGVLArgs;

static void *ds_write_with_gvl(void *arg) {
    DSWriteGVLArgs *a = arg;
    VALUE rb_old = Qnil;
    VALUE rb_new = scada_data_value_to_ruby(a->new_dv);
    rb_funcall(a->ctx->write_proc, rb_intern("call"), 2, rb_old, rb_new);
    return NULL;
}

static UA_StatusCode data_source_write_cb(UA_Server *server,
    const UA_NodeId *sessionId, void *sessionContext,
    const UA_NodeId *nodeId, void *nodeContext,
    const UA_NumericRange *range, const UA_DataValue *data) {

    (void)server; (void)sessionId; (void)sessionContext;
    (void)nodeId; (void)range;

    ScadaDataSourceCtx *ctx = (ScadaDataSourceCtx *)nodeContext;
    if (!ctx || NIL_P(ctx->write_proc))
        return UA_STATUSCODE_BADINTERNALERROR;

    DSWriteGVLArgs args = { ctx, data };
    rb_thread_call_with_gvl(ds_write_with_gvl, &args);
    return UA_STATUSCODE_GOOD;
}

static VALUE server_add_data_source_variable(VALUE self, VALUE rb_nid, VALUE rb_type,
    VALUE rb_display_name, VALUE rb_browse_name, VALUE rb_read_proc, VALUE rb_write_proc) {

    GET_SERVER(self, s);

    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);
    const UA_DataType *type = scada_resolve_type_symbol(rb_type);

    ScadaDataSourceCtx *ctx = ALLOC(ScadaDataSourceCtx);
    ctx->read_proc = NIL_P(rb_read_proc) ? Qnil : rb_read_proc;
    ctx->write_proc = NIL_P(rb_write_proc) ? Qnil : rb_write_proc;
    ctx->server_obj = self;

    if (!NIL_P(rb_read_proc))  rb_ary_push(s->callback_procs, rb_read_proc);
    if (!NIL_P(rb_write_proc)) rb_ary_push(s->callback_procs, rb_write_proc);

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = scada_to_localized_text(rb_display_name);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    if (!NIL_P(rb_write_proc))
        attr.accessLevel |= UA_ACCESSLEVELMASK_WRITE;
    attr.dataType = type->typeId;

    UA_DataSource ds;
    ds.read = data_source_read_cb;
    ds.write = NIL_P(rb_write_proc) ? NULL : data_source_write_cb;

    UA_QualifiedName browseName = UA_QUALIFIEDNAME_ALLOC(
        nodeId.namespaceIndex, StringValueCStr(rb_browse_name));

    UA_StatusCode rc = UA_Server_addDataSourceVariableNode(
        s->server, nodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
        browseName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        attr, ds, ctx, NULL);

    UA_QualifiedName_clear(&browseName);
    UA_LocalizedText_clear(&attr.displayName);

    if (rc != UA_STATUSCODE_GOOD) {
        xfree(ctx);
        scada_check_status(rc);
    }

    return Qnil;
}

/* Method callback support */
typedef struct {
    VALUE proc;
    VALUE server_obj;
    VALUE result;
    int is_async;
    /* Per-invocation (set before GVL trampoline) */
    UA_Server *ua_server;
    const UA_Variant *input;
    size_t input_size;
    UA_Variant *output;
    size_t output_size;
    /* Output type symbol, stored at registration */
    VALUE output_type_sym;
} ScadaMethodCtx;

static void *method_call_with_gvl(void *arg) {
    ScadaMethodCtx *ctx = arg;

    /* Build input array (must be inside GVL) */
    VALUE input_args = rb_ary_new_capa(ctx->input_size);
    for (size_t i = 0; i < ctx->input_size; i++)
        rb_ary_push(input_args, scada_variant_to_ruby(&ctx->input[i]));

    /* Call Ruby proc */
    ctx->result = rb_funcall(ctx->proc, rb_intern("call"), 1, input_args);

    /* Check for async result (responds to #wait) */
    ctx->is_async = (!NIL_P(ctx->result) &&
                     rb_respond_to(ctx->result, rb_intern("wait")));

    if (ctx->is_async) {
        /* Schedule async watcher task via Ruby */
        rb_funcall(ctx->server_obj,
                   rb_intern("_schedule_async_completion"), 3,
                   ctx->result,
                   ULL2NUM((uintptr_t)ctx->output),
                   ctx->output_type_sym);
    } else if (ctx->output_size > 0 && !NIL_P(ctx->result)
               && !NIL_P(ctx->output_type_sym)) {
        /* Sync: convert using declared output type */
        scada_ruby_to_variant(ctx->result, ctx->output_type_sym,
                              &ctx->output[0]);
    }

    return NULL;
}

static UA_StatusCode method_callback(UA_Server *server,
    const UA_NodeId *sessionId, void *sessionContext,
    const UA_NodeId *methodId, void *methodContext,
    const UA_NodeId *objectId, void *objectContext,
    size_t inputSize, const UA_Variant *input,
    size_t outputSize, UA_Variant *output) {

    (void)sessionId; (void)sessionContext;
    (void)methodId; (void)objectId; (void)objectContext;

    ScadaMethodCtx *ctx = (ScadaMethodCtx *)methodContext;
    if (!ctx || NIL_P(ctx->proc))
        return UA_STATUSCODE_BADINTERNALERROR;

    /* Set per-invocation fields */
    ctx->ua_server = server;
    ctx->input = input;
    ctx->input_size = inputSize;
    ctx->output = output;
    ctx->output_size = outputSize;
    ctx->is_async = 0;

    rb_thread_call_with_gvl(method_call_with_gvl, ctx);

    return ctx->is_async ? UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY
                         : UA_STATUSCODE_GOOD;
}

static VALUE server_add_method_node(VALUE self, VALUE rb_nid, VALUE rb_display_name,
    VALUE rb_browse_name, VALUE rb_input, VALUE rb_output, VALUE rb_proc) {

    GET_SERVER(self, s);

    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);

    /* Build input/output argument attributes */
    long input_count = RARRAY_LEN(rb_input);
    long output_count = RARRAY_LEN(rb_output);

    UA_Argument *inputArgs = NULL;
    UA_Argument *outputArgs = NULL;

    if (input_count > 0) {
        inputArgs = (UA_Argument *)UA_calloc(input_count, sizeof(UA_Argument));
        for (long i = 0; i < input_count; i++) {
            VALUE arg = rb_ary_entry(rb_input, i);
            VALUE name = rb_hash_aref(arg, ID2SYM(rb_intern("name")));
            VALUE type = rb_hash_aref(arg, ID2SYM(rb_intern("type")));
            UA_Argument_init(&inputArgs[i]);
            inputArgs[i].name = UA_STRING_ALLOC(StringValueCStr(name));
            inputArgs[i].dataType = scada_resolve_type_symbol(type)->typeId;
            inputArgs[i].valueRank = UA_VALUERANK_SCALAR;
        }
    }

    if (output_count > 0) {
        outputArgs = (UA_Argument *)UA_calloc(output_count, sizeof(UA_Argument));
        for (long i = 0; i < output_count; i++) {
            VALUE arg = rb_ary_entry(rb_output, i);
            VALUE name = rb_hash_aref(arg, ID2SYM(rb_intern("name")));
            VALUE type = rb_hash_aref(arg, ID2SYM(rb_intern("type")));
            UA_Argument_init(&outputArgs[i]);
            outputArgs[i].name = UA_STRING_ALLOC(StringValueCStr(name));
            outputArgs[i].dataType = scada_resolve_type_symbol(type)->typeId;
            outputArgs[i].valueRank = UA_VALUERANK_SCALAR;
        }
    }

    /* Create method context */
    ScadaMethodCtx *ctx = ALLOC(ScadaMethodCtx);
    ctx->proc = rb_proc;
    ctx->server_obj = self;
    ctx->result = Qnil;
    ctx->is_async = 0;
    ctx->ua_server = NULL;
    ctx->input = NULL;
    ctx->input_size = 0;
    ctx->output = NULL;
    ctx->output_size = 0;
    ctx->output_type_sym = Qnil;
    rb_ary_push(s->callback_procs, rb_proc);

    /* Store output type symbol for variant conversion */
    if (output_count > 0) {
        VALUE first = rb_ary_entry(rb_output, 0);
        ctx->output_type_sym = rb_hash_aref(first, ID2SYM(rb_intern("type")));
        rb_ary_push(s->callback_procs, ctx->output_type_sym); /* prevent GC */
    }

    UA_MethodAttributes methAttr = UA_MethodAttributes_default;
    methAttr.displayName = scada_to_localized_text(rb_display_name);
    methAttr.executable = true;
    methAttr.userExecutable = true;

    UA_QualifiedName browseName = UA_QUALIFIEDNAME_ALLOC(
        nodeId.namespaceIndex, StringValueCStr(rb_browse_name));

    UA_StatusCode rc = UA_Server_addMethodNode(
        s->server, nodeId,
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        browseName, methAttr,
        method_callback, input_count, inputArgs,
        output_count, outputArgs, ctx, NULL);

    UA_QualifiedName_clear(&browseName);
    UA_LocalizedText_clear(&methAttr.displayName);
    for (long i = 0; i < input_count; i++)
        UA_Argument_clear(&inputArgs[i]);
    for (long i = 0; i < output_count; i++)
        UA_Argument_clear(&outputArgs[i]);
    UA_free(inputArgs);
    UA_free(outputArgs);

    if (rc != UA_STATUSCODE_GOOD) {
        xfree(ctx);
        scada_check_status(rc);
    }

    return Qnil;
}

static VALUE server_add_namespace(VALUE self, VALUE rb_uri) {
    GET_SERVER(self, s);
    UA_UInt16 idx = UA_Server_addNamespace(s->server, StringValueCStr(rb_uri));
    return UINT2NUM(idx);
}

static VALUE server_has_node(VALUE self, VALUE rb_nid) {
    GET_SERVER(self, s);
    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);
    UA_NodeId outId;
    UA_StatusCode rc = UA_Server_readNodeId(s->server, nodeId, &outId);
    if (rc == UA_STATUSCODE_GOOD) {
        UA_NodeId_clear(&outId);
        return Qtrue;
    }
    return Qfalse;
}

static VALUE server_commit_async_method_result(VALUE self,
    VALUE rb_output_ptr, VALUE rb_result, VALUE rb_type) {
    GET_SERVER(self, s);
    UA_Variant *output = (UA_Variant *)(uintptr_t)NUM2ULL(rb_output_ptr);
    scada_ruby_to_variant(rb_result, rb_type, &output[0]);
    UA_Server_setAsyncCallMethodResult(s->server, output,
                                        UA_STATUSCODE_GOOD);
    return Qnil;
}

static VALUE server_fail_async_method_result(VALUE self,
    VALUE rb_output_ptr) {
    GET_SERVER(self, s);
    UA_Variant *output = (UA_Variant *)(uintptr_t)NUM2ULL(rb_output_ptr);
    UA_Server_setAsyncCallMethodResult(s->server, output,
                                        UA_STATUSCODE_BADINTERNALERROR);
    return Qnil;
}

static VALUE server_close(VALUE self) {
    GET_SERVER(self, s);
    if (s->server) {
        if (s->config && s->config->logging)
            scada_deactivate_logger(s->config->logging);
        UA_Server_delete(s->server);
        s->server = NULL;
        s->config = NULL;
    }
    return Qnil;
}

void Init_scada_server(VALUE rb_mScada) {
    rb_cServer = rb_define_class_under(rb_mScada, "Server", rb_cObject);
    rb_define_alloc_func(rb_cServer, server_alloc);
    rb_define_method(rb_cServer, "initialize", server_initialize, -1);
    rb_define_method(rb_cServer, "_run_startup", server_run_startup, 0);
    rb_define_method(rb_cServer, "_run_iterate", server_run_iterate, -1);
    rb_define_method(rb_cServer, "_run_shutdown", server_run_shutdown, 0);
    rb_define_method(rb_cServer, "_add_variable_node", server_add_variable_node, 6);
    rb_define_method(rb_cServer, "_write_value", server_write_value, 3);
    rb_define_method(rb_cServer, "_add_object_node", server_add_object_node, 4);
    rb_define_method(rb_cServer, "_add_data_source_variable", server_add_data_source_variable, 6);
    rb_define_method(rb_cServer, "_add_method_node", server_add_method_node, 6);
    rb_define_method(rb_cServer, "_add_namespace", server_add_namespace, 1);
    rb_define_method(rb_cServer, "_has_node", server_has_node, 1);
    rb_define_method(rb_cServer, "close", server_close, 0);
    rb_define_private_method(rb_cServer, "_commit_async_method_result",
                             server_commit_async_method_result, 3);
    rb_define_private_method(rb_cServer, "_fail_async_method_result",
                             server_fail_async_method_result, 1);
}
