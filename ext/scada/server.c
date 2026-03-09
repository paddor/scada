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

static VALUE server_run_iterate(VALUE self) {
    GET_SERVER(self, s);
    ServerIterateArgs args = { s->server, UA_FALSE, s->config };
    rb_thread_call_without_gvl(server_iterate_nogvl, &args, RUBY_UBF_IO, NULL);
    return Qnil;
}

static VALUE server_run_shutdown(VALUE self) {
    GET_SERVER(self, s);
    s->running = UA_FALSE;
    UA_StatusCode rc = UA_Server_run_shutdown(s->server);
    scada_check_status(rc);
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
    VALUE input_args;
    VALUE result;
} ScadaMethodCtx;

static void *method_call_with_gvl(void *arg) {
    ScadaMethodCtx *ctx = arg;
    ctx->result = rb_funcall(ctx->proc, rb_intern("call"), 1, ctx->input_args);
    return NULL;
}

static UA_StatusCode method_callback(UA_Server *server,
    const UA_NodeId *sessionId, void *sessionContext,
    const UA_NodeId *methodId, void *methodContext,
    const UA_NodeId *objectId, void *objectContext,
    size_t inputSize, const UA_Variant *input,
    size_t outputSize, UA_Variant *output) {

    (void)server; (void)sessionId; (void)sessionContext;
    (void)methodId; (void)objectId; (void)objectContext;

    ScadaMethodCtx *ctx = (ScadaMethodCtx *)methodContext;
    if (!ctx || NIL_P(ctx->proc))
        return UA_STATUSCODE_BADINTERNALERROR;

    /* Build input array */
    ctx->input_args = rb_ary_new_capa(inputSize);
    for (size_t i = 0; i < inputSize; i++)
        rb_ary_push(ctx->input_args, scada_variant_to_ruby(&input[i]));

    rb_thread_call_with_gvl(method_call_with_gvl, ctx);

    /* Convert result to output */
    if (outputSize > 0 && !NIL_P(ctx->result)) {
        if (TYPE(ctx->result) == T_STRING) {
            UA_String val = UA_STRING_ALLOC(StringValueCStr(ctx->result));
            UA_Variant_setScalarCopy(&output[0], &val, &UA_TYPES[UA_TYPES_STRING]);
            UA_String_clear(&val);
        } else if (TYPE(ctx->result) == T_FLOAT) {
            UA_Double val = NUM2DBL(ctx->result);
            UA_Variant_setScalarCopy(&output[0], &val, &UA_TYPES[UA_TYPES_DOUBLE]);
        } else if (FIXNUM_P(ctx->result) || TYPE(ctx->result) == T_BIGNUM) {
            UA_Int32 val = NUM2INT(ctx->result);
            UA_Variant_setScalarCopy(&output[0], &val, &UA_TYPES[UA_TYPES_INT32]);
        } else if (ctx->result == Qtrue || ctx->result == Qfalse) {
            UA_Boolean val = RTEST(ctx->result);
            UA_Variant_setScalarCopy(&output[0], &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
        }
    }

    return UA_STATUSCODE_GOOD;
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
    ctx->input_args = Qnil;
    ctx->result = Qnil;
    rb_ary_push(s->callback_procs, rb_proc);

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

void Init_scada_server(VALUE rb_mScada) {
    rb_cServer = rb_define_class_under(rb_mScada, "Server", rb_cObject);
    rb_define_alloc_func(rb_cServer, server_alloc);
    rb_define_method(rb_cServer, "initialize", server_initialize, -1);
    rb_define_method(rb_cServer, "_run_startup", server_run_startup, 0);
    rb_define_method(rb_cServer, "_run_iterate", server_run_iterate, 0);
    rb_define_method(rb_cServer, "_run_shutdown", server_run_shutdown, 0);
    rb_define_method(rb_cServer, "_add_variable_node", server_add_variable_node, 6);
    rb_define_method(rb_cServer, "_add_object_node", server_add_object_node, 4);
    rb_define_method(rb_cServer, "_add_data_source_variable", server_add_data_source_variable, 6);
    rb_define_method(rb_cServer, "_add_method_node", server_add_method_node, 6);
    rb_define_method(rb_cServer, "_add_namespace", server_add_namespace, 1);
    rb_define_method(rb_cServer, "_has_node", server_has_node, 1);
}
