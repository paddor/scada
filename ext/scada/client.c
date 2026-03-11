#include "scada.h"
#include <string.h>

typedef struct {
    UA_Client *client;
    VALUE callback_procs;
    char *endpoint_url;
    VALUE pending;  /* Hash { request_id => [condition, result] } */
    int freeing;    /* set during GC to suppress callbacks */
} ScadaClient;

static void client_mark(void *ptr) {
    ScadaClient *c = ptr;
    rb_gc_mark(c->callback_procs);
    rb_gc_mark(c->pending);
}

static void client_free(void *ptr) {
    ScadaClient *c = ptr;
    if (c->client) {
        UA_ClientConfig *config = UA_Client_getConfig(c->client);
        if (config && config->logging)
            scada_deactivate_logger(config->logging);
        /* Stop the event loop before delete. UA_Client_delete calls
         * UA_Client_disconnect which spins on el->run() until the
         * secure channel closes. With a stopped event loop that spin
         * is skipped, so cleanup is instant instead of ~5 s. */
        if (config && config->eventLoop &&
            !config->externalEventLoop)
            config->eventLoop->stop(config->eventLoop);
        /* Suppress async callbacks during UA_Client_delete —
         * they would try to allocate Ruby objects during GC. */
        c->freeing = 1;
        UA_Client_delete(c->client);
    }
    if (c->endpoint_url) xfree(c->endpoint_url);
    xfree(c);
}

static size_t client_memsize(const void *ptr) {
    (void)ptr;
    return sizeof(ScadaClient);
}

static const rb_data_type_t client_type = {
    .wrap_struct_name = "Scada::Client",
    .function = {
        .dmark = client_mark,
        .dfree = client_free,
        .dsize = client_memsize,
    },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE client_alloc(VALUE klass) {
    ScadaClient *c = ALLOC(ScadaClient);
    memset(c, 0, sizeof(ScadaClient));
    c->callback_procs = rb_ary_new();
    c->pending = rb_hash_new();
    return TypedData_Wrap_Struct(klass, &client_type, c);
}

#define GET_CLIENT(self, cvar) \
    ScadaClient *cvar; \
    TypedData_Get_Struct(self, ScadaClient, &client_type, cvar)

static VALUE client_initialize(int argc, VALUE *argv, VALUE self) {
    VALUE rb_url, kwargs;
    rb_scan_args(argc, argv, "1:", &rb_url, &kwargs);

    GET_CLIENT(self, c);

    const char *url = StringValueCStr(rb_url);
    c->endpoint_url = (char *)xmalloc(strlen(url) + 1);
    strcpy(c->endpoint_url, url);

    if (!NIL_P(kwargs)) {
        VALUE rb_config = Qnil;
        ID id_config = rb_intern("config");
        if (rb_hash_lookup2(kwargs, ID2SYM(id_config), Qundef) != Qundef)
            rb_config = rb_hash_aref(kwargs, ID2SYM(id_config));

        if (!NIL_P(rb_config)) {
            /* Build a zeroed config, set logger first, then call setDefault.
             * UA_Client_new() would call setDefault internally, creating an
             * event loop with the default stdout logger before we can intervene. */
            UA_ClientConfig config;
            memset(&config, 0, sizeof(UA_ClientConfig));

            VALUE logger = rb_funcall(rb_config, rb_intern("logger"), 0);
            if (!NIL_P(logger)) {
                if (TYPE(logger) == T_SYMBOL && SYM2ID(logger) == rb_intern("silent")) {
                    config.logging = UA_Log_Stdout_new(UA_LOGLEVEL_FATAL + 100);
                } else {
                    rb_ary_push(c->callback_procs, logger);
                    config.logging = scada_create_logger(logger);
                }
            }

            int encrypted = 0;
#ifdef UA_ENABLE_ENCRYPTION
            VALUE rb_cert = rb_funcall(rb_config, rb_intern("certificate"), 0);
            VALUE rb_key = rb_funcall(rb_config, rb_intern("private_key"), 0);
            VALUE rb_trust = rb_funcall(rb_config, rb_intern("trust_list"), 0);

            if (!NIL_P(rb_cert) && !NIL_P(rb_key)) {
                UA_ByteString cert = scada_to_bytestring(rb_cert);
                UA_ByteString key = scada_to_bytestring(rb_key);

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

                UA_StatusCode rc = UA_ClientConfig_setDefaultEncryption(
                    &config, cert, key,
                    trustList, trustListSize,
                    NULL, 0); /* revocation list */

                UA_ByteString_clear(&cert);
                UA_ByteString_clear(&key);
                for (size_t i = 0; i < trustListSize; i++)
                    UA_ByteString_clear(&trustList[i]);
                UA_free(trustList);

                scada_check_status(rc);
                encrypted = 1;
            }
#endif
            if (!encrypted)
                UA_ClientConfig_setDefault(&config);

            c->client = UA_Client_newWithConfig(&config);
            if (!c->client) rb_raise(rb_eRuntimeError, "Failed to create UA_Client");

            UA_ClientConfig *cc = UA_Client_getConfig(c->client);

            VALUE app_name = rb_funcall(rb_config, rb_intern("application_name"), 0);
            if (!NIL_P(app_name)) {
                UA_LocalizedText_clear(&cc->clientDescription.applicationName);
                cc->clientDescription.applicationName =
                    UA_LOCALIZEDTEXT_ALLOC("en-US", StringValueCStr(app_name));
            }

            VALUE app_uri = rb_funcall(rb_config, rb_intern("application_uri"), 0);
            if (!NIL_P(app_uri)) {
                UA_String_clear(&cc->clientDescription.applicationUri);
                cc->clientDescription.applicationUri =
                    UA_STRING_ALLOC(StringValueCStr(app_uri));
            }

            /* Security mode */
            VALUE rb_sec_mode = rb_funcall(rb_config, rb_intern("security_mode"), 0);
            if (!NIL_P(rb_sec_mode) && TYPE(rb_sec_mode) == T_SYMBOL) {
                ID mode_id = SYM2ID(rb_sec_mode);
                if (mode_id == rb_intern("sign"))
                    cc->securityMode = UA_MESSAGESECURITYMODE_SIGN;
                else if (mode_id == rb_intern("sign_and_encrypt"))
                    cc->securityMode = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT;
            }

            /* Username/password auth */
            VALUE username = rb_funcall(rb_config, rb_intern("username"), 0);
            VALUE password = rb_funcall(rb_config, rb_intern("password"), 0);
            if (!NIL_P(username) && !NIL_P(password)) {
                rb_iv_set(self, "@_username", username);
                rb_iv_set(self, "@_password", password);
                /* Allow password over unencrypted channels (v1.5+ default is false) */
                if (!encrypted)
                    cc->allowNonePolicyPassword = true;
            }

            /* Connectivity check interval */
            VALUE rb_cci = rb_funcall(rb_config, rb_intern("connectivity_check_interval"), 0);
            if (!NIL_P(rb_cci))
                cc->connectivityCheckInterval = (UA_UInt32)(NUM2DBL(rb_cci) * 1000.0);
        } else {
            c->client = UA_Client_new();
            if (!c->client) rb_raise(rb_eRuntimeError, "Failed to create UA_Client");
        }
    } else {
        c->client = UA_Client_new();
        if (!c->client) rb_raise(rb_eRuntimeError, "Failed to create UA_Client");
    }

    /* Store back-pointer so async callbacks can check the freeing flag */
    UA_Client_getConfig(c->client)->clientContext = c;

    return self;
}

/* --- Async connect --- */

static VALUE client_connect_async(VALUE self) {
    GET_CLIENT(self, c);

    VALUE username = rb_iv_get(self, "@_username");
    VALUE password = rb_iv_get(self, "@_password");
    if (!NIL_P(username) && !NIL_P(password)) {
        UA_ClientConfig *config = UA_Client_getConfig(c->client);
        UA_StatusCode rc = UA_ClientConfig_setAuthenticationUsername(
            config, StringValueCStr(username), StringValueCStr(password));
        scada_check_status(rc);
    }

    UA_StatusCode rc = UA_Client_connectAsync(c->client, c->endpoint_url);
    scada_check_status(rc);
    return self;
}

static VALUE client_get_state(VALUE self) {
    GET_CLIENT(self, c);
    UA_SecureChannelState channelState;
    UA_SessionState sessionState;
    UA_StatusCode connectStatus;
    UA_Client_getState(c->client, &channelState, &sessionState, &connectStatus);

    VALUE ary = rb_ary_new_capa(3);
    rb_ary_push(ary, INT2NUM((int)sessionState));
    rb_ary_push(ary, UINT2NUM(connectStatus));
    rb_ary_push(ary, INT2NUM((int)channelState));
    return ary;
}

/* Check if the namespace array has been fetched from the server.
 * Before the async read completes, ns=1 is UA_STRING_NULL.
 * After, it contains the server's application URI. */
static VALUE client_have_namespaces(VALUE self) {
    GET_CLIENT(self, c);
    UA_String nsUri;
    UA_StatusCode rc = UA_Client_getNamespaceUri(c->client, 1, &nsUri);
    if (rc != UA_STATUSCODE_GOOD)
        return Qfalse;
    UA_Boolean loaded = nsUri.length > 0;
    UA_String_clear(&nsUri);
    return loaded ? Qtrue : Qfalse;
}

/* --- Run iterate (GVL released) --- */

typedef struct {
    UA_Client *client;
    UA_UInt32 timeout;
} ClientIterateArgs;

static void *client_iterate_nogvl(void *arg) {
    ClientIterateArgs *a = arg;
    UA_ClientConfig *config = UA_Client_getConfig(a->client);
    if (config && config->logging)
        scada_logger_set_gvl(config->logging, 0);
    UA_Client_run_iterate(a->client, a->timeout);
    if (config && config->logging)
        scada_logger_set_gvl(config->logging, 1);
    return NULL;
}

static VALUE client_run_iterate(int argc, VALUE *argv, VALUE self) {
    GET_CLIENT(self, c);
    VALUE rb_timeout;
    rb_scan_args(argc, argv, "01", &rb_timeout);
    UA_UInt32 timeout = NIL_P(rb_timeout) ? 0
                                          : (UA_UInt32)NUM2UINT(rb_timeout);
    ClientIterateArgs args = { c->client, timeout };
    rb_thread_call_without_gvl(client_iterate_nogvl, &args,
                               RUBY_UBF_IO, NULL);
    return Qnil;
}

/* --- Async service callback infrastructure ---
 *
 * Async callbacks fire during UA_Client_run_iterate (GVL released).
 * We use rb_thread_call_with_gvl in the callback to re-enter Ruby,
 * convert the response, and resolve an Async::Promise.
 */

typedef struct {
    VALUE condition;
    VALUE result;
    UA_StatusCode status;
    int completed;
} AsyncPending;

/* GVL trampoline: called from within the async callback (which fires
 * during run_iterate without GVL). Converts response and stores result. */

typedef struct {
    AsyncPending *pending;
    const UA_ReadResponse *read_response;
    const UA_WriteResponse *write_response;
    const UA_CallResponse *call_response;
    int kind; /* 0=read, 1=write, 2=call */
} GVLSignalArgs;

static void *signal_pending_with_gvl(void *arg) {
    GVLSignalArgs *a = arg;
    AsyncPending *p = a->pending;

    if (a->kind == 0 && a->read_response) {
        const UA_ReadResponse *rr = a->read_response;
        p->status = rr->responseHeader.serviceResult;
        if (p->status == UA_STATUSCODE_GOOD && rr->resultsSize > 0) {
            /* Check per-result status */
            if (rr->results[0].hasStatus && rr->results[0].status != UA_STATUSCODE_GOOD) {
                p->status = rr->results[0].status;
                p->result = Qnil;
            } else {
                p->result = scada_data_value_to_ruby(&rr->results[0]);
            }
        } else {
            p->result = Qnil;
        }
    } else if (a->kind == 1 && a->write_response) {
        const UA_WriteResponse *wr = a->write_response;
        p->status = wr->responseHeader.serviceResult;
        if (p->status == UA_STATUSCODE_GOOD && wr->resultsSize > 0)
            p->status = wr->results[0];
        p->result = Qnil;
    } else if (a->kind == 2 && a->call_response) {
        const UA_CallResponse *cr = a->call_response;
        p->status = cr->responseHeader.serviceResult;
        if (p->status == UA_STATUSCODE_GOOD && cr->resultsSize > 0) {
            UA_CallMethodResult *mr = &cr->results[0];
            p->status = mr->statusCode;
            if (p->status == UA_STATUSCODE_GOOD) {
                if (mr->outputArgumentsSize == 0) {
                    p->result = Qnil;
                } else if (mr->outputArgumentsSize == 1) {
                    p->result = scada_variant_to_ruby(&mr->outputArguments[0]);
                } else {
                    p->result = rb_ary_new_capa(mr->outputArgumentsSize);
                    for (size_t i = 0; i < mr->outputArgumentsSize; i++)
                        rb_ary_push(p->result, scada_variant_to_ruby(&mr->outputArguments[i]));
                }
            } else {
                p->result = Qnil;
            }
        } else {
            p->result = Qnil;
        }
    }

    p->completed = 1;

    /* Signal the condition with [status_code, result] */
    VALUE signal_val = rb_ary_new_capa(2);
    rb_ary_push(signal_val, UINT2NUM(p->status));
    rb_ary_push(signal_val, p->result);
    rb_funcall(p->condition, rb_intern("resolve"), 1, signal_val);

    return NULL;
}

/* C callbacks that fire during run_iterate (without GVL) */

/* Check if the client is being freed (GC phase) — if so, skip Ruby calls */
static int client_is_freeing(UA_Client *client) {
    UA_ClientConfig *config = UA_Client_getConfig(client);
    if (config && config->clientContext) {
        ScadaClient *c = (ScadaClient *)config->clientContext;
        return c->freeing;
    }
    return 0;
}

static void read_async_cb(UA_Client *client, void *userdata,
                           UA_UInt32 requestId, UA_ReadResponse *response) {
    (void)requestId;
    if (client_is_freeing(client)) return;
    AsyncPending *p = (AsyncPending *)userdata;
    GVLSignalArgs args = { p, response, NULL, NULL, 0 };
    rb_thread_call_with_gvl(signal_pending_with_gvl, &args);
}

static void write_async_cb(UA_Client *client, void *userdata,
                            UA_UInt32 requestId, UA_WriteResponse *response) {
    (void)requestId;
    if (client_is_freeing(client)) return;
    AsyncPending *p = (AsyncPending *)userdata;
    GVLSignalArgs args = { p, NULL, response, NULL, 1 };
    rb_thread_call_with_gvl(signal_pending_with_gvl, &args);
}

static void call_async_cb(UA_Client *client, void *userdata,
                           UA_UInt32 requestId, UA_CallResponse *response) {
    (void)requestId;
    if (client_is_freeing(client)) return;
    AsyncPending *p = (AsyncPending *)userdata;
    GVLSignalArgs args = { p, NULL, NULL, response, 2 };
    rb_thread_call_with_gvl(signal_pending_with_gvl, &args);
}

/* --- Async read --- */

static VALUE client_read_async(VALUE self, VALUE rb_nid, VALUE rb_attribute, VALUE rb_condition) {
    GET_CLIENT(self, c);
    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);
    ID attr_id = SYM2ID(rb_attribute);

    AsyncPending *pending = ALLOC(AsyncPending);
    pending->condition = rb_condition;
    pending->result = Qnil;
    pending->status = UA_STATUSCODE_GOOD;
    pending->completed = 0;

    /* Keep condition alive */
    rb_ary_push(c->callback_procs, rb_condition);

    if (attr_id == rb_intern("value")) {
        UA_ReadValueId item;
        UA_ReadValueId_init(&item);
        item.nodeId = nodeId;
        item.attributeId = UA_ATTRIBUTEID_VALUE;

        UA_ReadRequest request;
        UA_ReadRequest_init(&request);
        request.nodesToRead = &item;
        request.nodesToReadSize = 1;

        UA_UInt32 reqId;
        UA_StatusCode rc = UA_Client_sendAsyncReadRequest(
            c->client, &request,
            (UA_ClientAsyncReadCallback)read_async_cb,
            pending, &reqId);

        if (rc != UA_STATUSCODE_GOOD) {
            xfree(pending);
            scada_check_status(rc);
        }
    } else if (attr_id == rb_intern("display_name")) {
        UA_ReadValueId item;
        UA_ReadValueId_init(&item);
        item.nodeId = nodeId;
        item.attributeId = UA_ATTRIBUTEID_DISPLAYNAME;

        UA_ReadRequest request;
        UA_ReadRequest_init(&request);
        request.nodesToRead = &item;
        request.nodesToReadSize = 1;

        UA_UInt32 reqId;
        UA_StatusCode rc = UA_Client_sendAsyncReadRequest(
            c->client, &request,
            (UA_ClientAsyncReadCallback)read_async_cb,
            pending, &reqId);

        if (rc != UA_STATUSCODE_GOOD) {
            xfree(pending);
            scada_check_status(rc);
        }
    } else if (attr_id == rb_intern("browse_name")) {
        UA_ReadValueId item;
        UA_ReadValueId_init(&item);
        item.nodeId = nodeId;
        item.attributeId = UA_ATTRIBUTEID_BROWSENAME;

        UA_ReadRequest request;
        UA_ReadRequest_init(&request);
        request.nodesToRead = &item;
        request.nodesToReadSize = 1;

        UA_UInt32 reqId;
        UA_StatusCode rc = UA_Client_sendAsyncReadRequest(
            c->client, &request,
            (UA_ClientAsyncReadCallback)read_async_cb,
            pending, &reqId);

        if (rc != UA_STATUSCODE_GOOD) {
            xfree(pending);
            scada_check_status(rc);
        }
    } else {
        xfree(pending);
        rb_raise(rb_eArgError, "Unknown attribute");
    }

    return Qnil;
}

/* --- Async write --- */

static VALUE client_write_async(VALUE self, VALUE rb_nid, VALUE rb_value, VALUE rb_type_sym, VALUE rb_condition) {
    GET_CLIENT(self, c);
    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);

    AsyncPending *pending = ALLOC(AsyncPending);
    pending->condition = rb_condition;
    pending->result = Qnil;
    pending->status = UA_STATUSCODE_GOOD;
    pending->completed = 0;

    rb_ary_push(c->callback_procs, rb_condition);

    UA_Variant value;
    UA_Variant_init(&value);

    if (!NIL_P(rb_type_sym)) {
        /* Explicit type provided — use full variant conversion */
        scada_ruby_to_variant(rb_value, rb_type_sym, &value);
    } else if (TYPE(rb_value) == T_FLOAT) {
        UA_Double val = NUM2DBL(rb_value);
        UA_Variant_setScalarCopy(&value, &val, &UA_TYPES[UA_TYPES_DOUBLE]);
    } else if (TYPE(rb_value) == T_STRING) {
        UA_String val = UA_STRING_ALLOC(StringValueCStr(rb_value));
        UA_Variant_setScalarCopy(&value, &val, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&val);
    } else if (FIXNUM_P(rb_value) || TYPE(rb_value) == T_BIGNUM) {
        UA_Int32 val = NUM2INT(rb_value);
        UA_Variant_setScalarCopy(&value, &val, &UA_TYPES[UA_TYPES_INT32]);
    } else if (rb_value == Qtrue || rb_value == Qfalse) {
        UA_Boolean val = RTEST(rb_value);
        UA_Variant_setScalarCopy(&value, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
    } else {
        xfree(pending);
        rb_raise(rb_eArgError, "Cannot infer OPC UA type from Ruby value");
    }

    UA_WriteValue wv;
    UA_WriteValue_init(&wv);
    wv.nodeId = nodeId;
    wv.attributeId = UA_ATTRIBUTEID_VALUE;
    wv.value.hasValue = true;
    wv.value.value = value;

    UA_WriteRequest request;
    UA_WriteRequest_init(&request);
    request.nodesToWrite = &wv;
    request.nodesToWriteSize = 1;

    UA_UInt32 reqId;
    UA_StatusCode rc = UA_Client_sendAsyncWriteRequest(
        c->client, &request,
        (UA_ClientAsyncWriteCallback)write_async_cb,
        pending, &reqId);

    UA_Variant_clear(&value);

    if (rc != UA_STATUSCODE_GOOD) {
        xfree(pending);
        scada_check_status(rc);
    }

    return Qnil;
}

/* --- Async call --- */

static VALUE client_call_async(VALUE self, VALUE rb_nid, VALUE rb_args, VALUE rb_condition) {
    GET_CLIENT(self, c);
    UA_NodeId objectId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId methodId = scada_node_id_unwrap(rb_nid);

    AsyncPending *pending = ALLOC(AsyncPending);
    pending->condition = rb_condition;
    pending->result = Qnil;
    pending->status = UA_STATUSCODE_GOOD;
    pending->completed = 0;

    rb_ary_push(c->callback_procs, rb_condition);

    size_t inputSize = 0;
    UA_Variant *inputs = NULL;

    if (TYPE(rb_args) == T_ARRAY) {
        inputSize = RARRAY_LEN(rb_args);
        if (inputSize > 0) {
            inputs = (UA_Variant *)UA_calloc(inputSize, sizeof(UA_Variant));
            for (size_t i = 0; i < inputSize; i++) {
                VALUE arg = rb_ary_entry(rb_args, i);
                if (TYPE(arg) == T_STRING) {
                    UA_String val = UA_STRING_ALLOC(StringValueCStr(arg));
                    UA_Variant_setScalarCopy(&inputs[i], &val, &UA_TYPES[UA_TYPES_STRING]);
                    UA_String_clear(&val);
                } else if (TYPE(arg) == T_FLOAT) {
                    UA_Double val = NUM2DBL(arg);
                    UA_Variant_setScalarCopy(&inputs[i], &val, &UA_TYPES[UA_TYPES_DOUBLE]);
                } else if (FIXNUM_P(arg) || TYPE(arg) == T_BIGNUM) {
                    UA_Int32 val = NUM2INT(arg);
                    UA_Variant_setScalarCopy(&inputs[i], &val, &UA_TYPES[UA_TYPES_INT32]);
                } else if (arg == Qtrue || arg == Qfalse) {
                    UA_Boolean val = RTEST(arg);
                    UA_Variant_setScalarCopy(&inputs[i], &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
                }
            }
        }
    }

    UA_UInt32 reqId;
    UA_StatusCode rc = UA_Client_call_async(
        c->client, objectId, methodId,
        inputSize, inputs,
        (UA_ClientAsyncCallCallback)call_async_cb,
        pending, &reqId);

    for (size_t i = 0; i < inputSize; i++)
        UA_Variant_clear(&inputs[i]);
    UA_free(inputs);

    if (rc != UA_STATUSCODE_GOOD) {
        xfree(pending);
        scada_check_status(rc);
    }

    return Qnil;
}

/* --- Namespace --- */

static VALUE client_namespace_get_index(VALUE self, VALUE rb_uri) {
    GET_CLIENT(self, c);
    UA_UInt16 nsIndex;
    UA_String nsUri = UA_STRING_ALLOC(StringValueCStr(rb_uri));
    UA_StatusCode rc = UA_Client_NamespaceGetIndex(c->client, &nsUri, &nsIndex);
    UA_String_clear(&nsUri);
    scada_check_status(rc);
    return UINT2NUM(nsIndex);
}

/* Local namespace index lookup from the client's mapping table
 * (populated automatically after connect). */
static VALUE client_get_namespace_index(VALUE self, VALUE rb_uri) {
    GET_CLIENT(self, c);
    UA_UInt16 nsIndex;
    UA_String nsUri = UA_STRING_ALLOC(StringValueCStr(rb_uri));
    UA_StatusCode rc = UA_Client_getNamespaceIndex(c->client, nsUri, &nsIndex);
    UA_String_clear(&nsUri);
    scada_check_status(rc);
    return UINT2NUM(nsIndex);
}

/* --- Subscriptions --- */

typedef struct {
    AsyncPending *pending;
    UA_CreateSubscriptionResponse *response;
} CreateSubGVLArgs;

static void *create_sub_signal_gvl(void *arg) {
    CreateSubGVLArgs *a = arg;
    AsyncPending *p = a->pending;
    UA_CreateSubscriptionResponse *resp = a->response;

    p->status = resp->responseHeader.serviceResult;
    p->completed = 1;

    VALUE signal_val = rb_ary_new_capa(2);
    rb_ary_push(signal_val, UINT2NUM(p->status));
    rb_ary_push(signal_val, UINT2NUM(
        p->status == UA_STATUSCODE_GOOD ? resp->subscriptionId : 0));
    rb_funcall(p->condition, rb_intern("resolve"), 1, signal_val);
    return NULL;
}

static void create_sub_cb_nogvl(UA_Client *client, void *userdata,
                                 UA_UInt32 requestId,
                                 UA_CreateSubscriptionResponse *response) {
    (void)requestId;
    if (client_is_freeing(client)) return;
    AsyncPending *p = (AsyncPending *)userdata;
    CreateSubGVLArgs args = { p, response };
    rb_thread_call_with_gvl(create_sub_signal_gvl, &args);
}

static VALUE client_create_subscription_async(VALUE self, VALUE rb_interval, VALUE rb_condition) {
    GET_CLIENT(self, c);

    AsyncPending *pending = ALLOC(AsyncPending);
    pending->condition = rb_condition;
    pending->result = Qnil;
    pending->status = UA_STATUSCODE_GOOD;
    pending->completed = 0;

    rb_ary_push(c->callback_procs, rb_condition);

    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    request.requestedPublishingInterval = NUM2DBL(rb_interval) * 1000.0;

    UA_UInt32 reqId;
    UA_StatusCode rc = UA_Client_Subscriptions_create_async(
        c->client, request, NULL, NULL, NULL,
        create_sub_cb_nogvl, pending, &reqId);

    if (rc != UA_STATUSCODE_GOOD) {
        xfree(pending);
        scada_check_status(rc);
    }

    return Qnil;
}

/* --- Monitored Items --- */

typedef struct {
    VALUE proc;
} MonitorCtx;

typedef struct {
    MonitorCtx *ctx;
    UA_DataValue *value;
} DataChangeGVLArgs;

static void *data_change_with_gvl(void *arg) {
    DataChangeGVLArgs *a = arg;
    VALUE dv = scada_data_value_to_ruby(a->value);
    rb_funcall(a->ctx->proc, rb_intern("call"), 1, dv);
    return NULL;
}

static void data_change_notification_cb(UA_Client *client, UA_UInt32 subId,
    void *subContext, UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    (void)subId; (void)subContext; (void)monId;
    if (client_is_freeing(client)) return;
    MonitorCtx *ctx = (MonitorCtx *)monContext;
    if (!ctx || NIL_P(ctx->proc)) return;
    DataChangeGVLArgs args = { ctx, value };
    rb_thread_call_with_gvl(data_change_with_gvl, &args);
}

typedef struct {
    AsyncPending *pending;
    UA_CreateMonitoredItemsResponse *response;
} CreateMonGVLArgs;

static void *create_mon_signal_gvl(void *arg) {
    CreateMonGVLArgs *a = arg;
    AsyncPending *p = a->pending;
    UA_CreateMonitoredItemsResponse *resp = a->response;

    p->status = resp->responseHeader.serviceResult;
    UA_UInt32 monId = 0;
    if (p->status == UA_STATUSCODE_GOOD && resp->resultsSize > 0) {
        p->status = resp->results[0].statusCode;
        monId = resp->results[0].monitoredItemId;
    }
    p->completed = 1;

    VALUE signal_val = rb_ary_new_capa(2);
    rb_ary_push(signal_val, UINT2NUM(p->status));
    rb_ary_push(signal_val, UINT2NUM(monId));
    rb_funcall(p->condition, rb_intern("resolve"), 1, signal_val);
    return NULL;
}

static void create_mon_cb(UA_Client *client, void *userdata,
                           UA_UInt32 requestId,
                           UA_CreateMonitoredItemsResponse *response) {
    (void)requestId;
    if (client_is_freeing(client)) return;
    AsyncPending *p = (AsyncPending *)userdata;
    CreateMonGVLArgs args = { p, response };
    rb_thread_call_with_gvl(create_mon_signal_gvl, &args);
}

static VALUE client_add_monitored_data_change(VALUE self, VALUE rb_sub_id,
    VALUE rb_nid, VALUE rb_proc, VALUE rb_trigger, VALUE rb_condition) {
    GET_CLIENT(self, c);

    UA_UInt32 subId = NUM2UINT(rb_sub_id);
    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);

    MonitorCtx *ctx = ALLOC(MonitorCtx);
    ctx->proc = rb_proc;
    rb_ary_push(c->callback_procs, rb_proc);

    AsyncPending *pending = ALLOC(AsyncPending);
    pending->condition = rb_condition;
    pending->result = Qnil;
    pending->status = UA_STATUSCODE_GOOD;
    pending->completed = 0;
    rb_ary_push(c->callback_procs, rb_condition);

    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(nodeId);

    /* Override data change trigger if specified */
    if (!NIL_P(rb_trigger)) {
        ID tid = SYM2ID(rb_trigger);
        UA_DataChangeTrigger trigger;
        if (tid == rb_intern("status"))
            trigger = UA_DATACHANGETRIGGER_STATUS;
        else if (tid == rb_intern("status_value"))
            trigger = UA_DATACHANGETRIGGER_STATUSVALUE;
        else if (tid == rb_intern("status_value_timestamp"))
            trigger = UA_DATACHANGETRIGGER_STATUSVALUETIMESTAMP;
        else
            rb_raise(rb_eArgError, "Unknown trigger: %s", rb_id2name(tid));

        UA_DataChangeFilter *filter = UA_DataChangeFilter_new();
        filter->trigger = trigger;
        filter->deadbandType = UA_DEADBANDTYPE_NONE;
        monRequest.requestedParameters.filter.encoding = UA_EXTENSIONOBJECT_DECODED;
        monRequest.requestedParameters.filter.content.decoded.type = &UA_TYPES[UA_TYPES_DATACHANGEFILTER];
        monRequest.requestedParameters.filter.content.decoded.data = filter;
    }

    UA_CreateMonitoredItemsRequest createRequest;
    UA_CreateMonitoredItemsRequest_init(&createRequest);
    createRequest.subscriptionId = subId;
    createRequest.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
    createRequest.itemsToCreateSize = 1;
    createRequest.itemsToCreate = &monRequest;

    void *contexts[] = { ctx };
    UA_Client_DataChangeNotificationCallback callbacks[] = { data_change_notification_cb };

    UA_Client_DeleteMonitoredItemCallback deleteCallbacks[] = { NULL };
    UA_UInt32 reqId;
    UA_StatusCode rc = UA_Client_MonitoredItems_createDataChanges_async(
        c->client, createRequest, contexts, callbacks, deleteCallbacks,
        create_mon_cb, pending, &reqId);

    if (rc != UA_STATUSCODE_GOOD) {
        xfree(ctx);
        xfree(pending);
        scada_check_status(rc);
    }

    return Qnil;
}

/* Event monitoring */

typedef struct {
    MonitorCtx *ctx;
    UA_KeyValueMap eventFields;
    VALUE rb_select;
} EventGVLArgs;

static void *event_with_gvl(void *arg) {
    EventGVLArgs *a = arg;

    /* Build event hash from select fields and values */
    VALUE event_type = Qnil, message = Qnil, severity = Qnil;
    VALUE source_name = Qnil, time_val = Qnil;

    long select_len = RARRAY_LEN(a->rb_select);
    for (size_t i = 0; i < a->eventFields.mapSize && (long)i < select_len; i++) {
        VALUE field_sym = rb_ary_entry(a->rb_select, i);
        VALUE val = scada_variant_to_ruby(&a->eventFields.map[i].value);
        ID fid = SYM2ID(field_sym);

        if (fid == rb_intern("event_type"))       event_type = val;
        else if (fid == rb_intern("message"))     message = val;
        else if (fid == rb_intern("severity"))    severity = val;
        else if (fid == rb_intern("source_name")) source_name = val;
        else if (fid == rb_intern("time"))        time_val = val;
    }

    VALUE rb_cEvent = rb_const_get(rb_mScada, rb_intern("Event"));
    VALUE event = rb_funcall(rb_cEvent, rb_intern("new"), 5,
        event_type, message, severity, source_name, time_val);

    rb_funcall(a->ctx->proc, rb_intern("call"), 1, event);
    return NULL;
}

static void event_notification_cb(UA_Client *client, UA_UInt32 subId,
    void *subContext, UA_UInt32 monId, void *monContext,
    const UA_KeyValueMap eventFields) {
    (void)subId; (void)subContext; (void)monId;
    if (client_is_freeing(client)) return;
    MonitorCtx *ctx = (MonitorCtx *)monContext;
    if (!ctx || NIL_P(ctx->proc)) return;

    /* Retrieve the select array from the Ruby side.
     * We stored it as an ivar on the proc. */
    VALUE rb_select = rb_funcall(ctx->proc, rb_intern("instance_variable_get"),
        1, ID2SYM(rb_intern("@_scada_select")));
    if (NIL_P(rb_select)) rb_select = rb_ary_new();

    EventGVLArgs args = { ctx, eventFields, rb_select };
    rb_thread_call_with_gvl(event_with_gvl, &args);
}

static VALUE client_add_monitored_event(VALUE self, VALUE rb_sub_id,
    VALUE rb_nid, VALUE rb_proc, VALUE rb_select, VALUE rb_condition) {
    GET_CLIENT(self, c);

    UA_UInt32 subId = NUM2UINT(rb_sub_id);
    UA_NodeId nodeId = scada_node_id_unwrap(rb_nid);

    MonitorCtx *ctx = ALLOC(MonitorCtx);
    ctx->proc = rb_proc;
    rb_ary_push(c->callback_procs, rb_proc);

    AsyncPending *pending = ALLOC(AsyncPending);
    pending->condition = rb_condition;
    pending->result = Qnil;
    pending->status = UA_STATUSCODE_GOOD;
    pending->completed = 0;
    rb_ary_push(c->callback_procs, rb_condition);

    /* Store select array on the proc for the callback */
    rb_funcall(rb_proc, rb_intern("instance_variable_set"),
        2, ID2SYM(rb_intern("@_scada_select")), rb_select);

    /* Build event filter with select clauses */
    long select_len = RARRAY_LEN(rb_select);

    UA_SimpleAttributeOperand *selectClauses = NULL;
    if (select_len > 0) {
        selectClauses = (UA_SimpleAttributeOperand *)
            UA_calloc(select_len, sizeof(UA_SimpleAttributeOperand));
        for (long i = 0; i < select_len; i++) {
            UA_SimpleAttributeOperand_init(&selectClauses[i]);
            selectClauses[i].typeDefinitionId =
                UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
            selectClauses[i].attributeId = UA_ATTRIBUTEID_VALUE;

            VALUE sym = rb_ary_entry(rb_select, i);
            const char *name = rb_id2name(SYM2ID(sym));

            const char *browse_name = name;
            if (strcmp(name, "event_type") == 0) browse_name = "EventType";
            else if (strcmp(name, "message") == 0) browse_name = "Message";
            else if (strcmp(name, "severity") == 0) browse_name = "Severity";
            else if (strcmp(name, "source_name") == 0) browse_name = "SourceName";
            else if (strcmp(name, "time") == 0) browse_name = "Time";

            selectClauses[i].browsePathSize = 1;
            selectClauses[i].browsePath = UA_QualifiedName_new();
            *selectClauses[i].browsePath =
                UA_QUALIFIEDNAME_ALLOC(0, browse_name);
        }
    }

    UA_EventFilter filter;
    UA_EventFilter_init(&filter);
    filter.selectClauses = selectClauses;
    filter.selectClausesSize = select_len;

    UA_MonitoredItemCreateRequest monRequest;
    UA_MonitoredItemCreateRequest_init(&monRequest);
    monRequest.itemToMonitor.nodeId = nodeId;
    monRequest.itemToMonitor.attributeId = UA_ATTRIBUTEID_EVENTNOTIFIER;
    monRequest.monitoringMode = UA_MONITORINGMODE_REPORTING;
    monRequest.requestedParameters.samplingInterval = 250;
    monRequest.requestedParameters.discardOldest = true;
    monRequest.requestedParameters.queueSize = 10;
    monRequest.requestedParameters.filter.encoding = UA_EXTENSIONOBJECT_DECODED;
    monRequest.requestedParameters.filter.content.decoded.type =
        &UA_TYPES[UA_TYPES_EVENTFILTER];
    monRequest.requestedParameters.filter.content.decoded.data = &filter;

    UA_CreateMonitoredItemsRequest createRequest;
    UA_CreateMonitoredItemsRequest_init(&createRequest);
    createRequest.subscriptionId = subId;
    createRequest.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
    createRequest.itemsToCreateSize = 1;
    createRequest.itemsToCreate = &monRequest;

    void *contexts[] = { ctx };
    UA_Client_EventNotificationCallback callbacks[] = { event_notification_cb };
    UA_Client_DeleteMonitoredItemCallback evDeleteCallbacks[] = { NULL };

    UA_UInt32 reqId;
    UA_StatusCode rc = UA_Client_MonitoredItems_createEvents_async(
        c->client, createRequest, contexts, callbacks, evDeleteCallbacks,
        create_mon_cb, pending, &reqId);

    for (long i = 0; i < select_len; i++)
        UA_SimpleAttributeOperand_clear(&selectClauses[i]);
    UA_free(selectClauses);

    if (rc != UA_STATUSCODE_GOOD) {
        xfree(ctx);
        xfree(pending);
        scada_check_status(rc);
    }

    return Qnil;
}

/* --- Init --- */

void Init_scada_client(VALUE rb_mScada) {
    rb_cClient = rb_define_class_under(rb_mScada, "Client", rb_cObject);
    rb_define_alloc_func(rb_cClient, client_alloc);
    rb_define_method(rb_cClient, "initialize", client_initialize, -1);
    rb_define_method(rb_cClient, "_connect_async", client_connect_async, 0);
    rb_define_method(rb_cClient, "_get_state", client_get_state, 0);
    rb_define_method(rb_cClient, "_have_namespaces?", client_have_namespaces, 0);
    rb_define_method(rb_cClient, "_run_iterate", client_run_iterate, -1);
    rb_define_method(rb_cClient, "_read_async", client_read_async, 3);
    rb_define_method(rb_cClient, "_write_async", client_write_async, 4);
    rb_define_method(rb_cClient, "_call_async", client_call_async, 3);
    rb_define_method(rb_cClient, "_namespace_get_index", client_namespace_get_index, 1);
    rb_define_method(rb_cClient, "_get_namespace_index", client_get_namespace_index, 1);
    rb_define_method(rb_cClient, "_create_subscription_async", client_create_subscription_async, 2);
    rb_define_method(rb_cClient, "_add_monitored_data_change", client_add_monitored_data_change, 5);
    rb_define_method(rb_cClient, "_add_monitored_event", client_add_monitored_event, 5);
}
