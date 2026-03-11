#include "scada.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    UA_NodeId node_id;
} ScadaNodeId;

static void node_id_free(void *ptr) {
    ScadaNodeId *nid = ptr;
    UA_NodeId_clear(&nid->node_id);
    xfree(nid);
}

static size_t node_id_memsize(const void *ptr) {
    (void)ptr;
    return sizeof(ScadaNodeId);
}

static const rb_data_type_t node_id_type = {
    .wrap_struct_name = "Scada::NodeId",
    .function = {
        .dmark = NULL,
        .dfree = node_id_free,
        .dsize = node_id_memsize,
    },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE node_id_alloc(VALUE klass) {
    ScadaNodeId *nid = ALLOC(ScadaNodeId);
    UA_NodeId_init(&nid->node_id);
    return TypedData_Wrap_Struct(klass, &node_id_type, nid);
}

VALUE scada_node_id_numeric(uint16_t ns, uint32_t id) {
    VALUE obj = node_id_alloc(rb_cNodeId);
    ScadaNodeId *nid;
    TypedData_Get_Struct(obj, ScadaNodeId, &node_id_type, nid);
    nid->node_id = UA_NODEID_NUMERIC(ns, id);
    return obj;
}

VALUE scada_node_id_wrap(UA_NodeId ua_nid) {
    VALUE obj = node_id_alloc(rb_cNodeId);
    ScadaNodeId *nid;
    TypedData_Get_Struct(obj, ScadaNodeId, &node_id_type, nid);
    UA_NodeId_copy(&ua_nid, &nid->node_id);
    return obj;
}

UA_NodeId scada_node_id_unwrap(VALUE self) {
    ScadaNodeId *nid;
    TypedData_Get_Struct(self, ScadaNodeId, &node_id_type, nid);
    return nid->node_id;
}

static VALUE node_id_s_parse(VALUE klass, VALUE str) {
    const char *s = StringValueCStr(str);
    uint16_t ns = 0;

    if (strncmp(s, "ns=", 3) == 0) {
        ns = (uint16_t)atoi(s + 3);
        const char *semi = strchr(s, ';');
        if (!semi) rb_raise(rb_eArgError, "Invalid NodeId format: %s", s);
        s = semi + 1;
    }

    VALUE obj = node_id_alloc(klass);
    ScadaNodeId *nid;
    TypedData_Get_Struct(obj, ScadaNodeId, &node_id_type, nid);

    if (strncmp(s, "i=", 2) == 0) {
        nid->node_id = UA_NODEID_NUMERIC(ns, (uint32_t)atol(s + 2));
    } else if (strncmp(s, "s=", 2) == 0) {
        nid->node_id = UA_NODEID_STRING_ALLOC(ns, s + 2);
    } else if (strncmp(s, "g=", 2) == 0) {
        UA_Guid guid;
        if (UA_Guid_parse(&guid, UA_STRING((char *)(s + 2))) != UA_STATUSCODE_GOOD) {
            rb_raise(rb_eArgError, "Invalid GUID in NodeId: %s", s + 2);
        }
        nid->node_id = UA_NODEID_GUID(ns, guid);
    } else if (strncmp(s, "b=", 2) == 0) {
        size_t len = strlen(s + 2);
        UA_ByteString bs;
        bs.length = len;
        bs.data = (UA_Byte *)UA_malloc(len);
        memcpy(bs.data, s + 2, len);
        nid->node_id.namespaceIndex = ns;
        nid->node_id.identifierType = UA_NODEIDTYPE_BYTESTRING;
        nid->node_id.identifier.byteString = bs;
    } else {
        rb_raise(rb_eArgError, "Invalid NodeId format: %s", StringValueCStr(str));
    }

    return obj;
}

VALUE scada_node_id_parse(VALUE str) {
    return node_id_s_parse(rb_cNodeId, str);
}

static VALUE node_id_namespace(VALUE self) {
    ScadaNodeId *nid;
    TypedData_Get_Struct(self, ScadaNodeId, &node_id_type, nid);
    return UINT2NUM(nid->node_id.namespaceIndex);
}

static VALUE node_id_to_s(VALUE self) {
    ScadaNodeId *nid;
    TypedData_Get_Struct(self, ScadaNodeId, &node_id_type, nid);

    UA_String out = UA_STRING_NULL;
    UA_NodeId_print(&nid->node_id, &out);
    VALUE str = rb_str_new((const char *)out.data, out.length);
    UA_String_clear(&out);
    return str;
}

static VALUE node_id_inspect(VALUE self) {
    VALUE s = node_id_to_s(self);
    VALUE result = rb_sprintf("#<Scada::NodeId %s>", StringValueCStr(s));
    return result;
}

static VALUE node_id_eq(VALUE self, VALUE other) {
    if (!rb_obj_is_kind_of(other, rb_cNodeId)) return Qfalse;
    ScadaNodeId *a, *b;
    TypedData_Get_Struct(self, ScadaNodeId, &node_id_type, a);
    TypedData_Get_Struct(other, ScadaNodeId, &node_id_type, b);
    return UA_NodeId_equal(&a->node_id, &b->node_id) ? Qtrue : Qfalse;
}

static VALUE node_id_hash(VALUE self) {
    ScadaNodeId *nid;
    TypedData_Get_Struct(self, ScadaNodeId, &node_id_type, nid);
    return UINT2NUM(UA_NodeId_hash(&nid->node_id));
}

void Init_scada_node_id(VALUE rb_mScada) {
    rb_cNodeId = rb_define_class_under(rb_mScada, "NodeId", rb_cObject);
    rb_define_alloc_func(rb_cNodeId, node_id_alloc);
    rb_define_singleton_method(rb_cNodeId, "parse", node_id_s_parse, 1);
    rb_define_method(rb_cNodeId, "namespace", node_id_namespace, 0);
    rb_define_method(rb_cNodeId, "to_s", node_id_to_s, 0);
    rb_define_method(rb_cNodeId, "inspect", node_id_inspect, 0);
    rb_define_method(rb_cNodeId, "==", node_id_eq, 1);
    rb_define_method(rb_cNodeId, "eql?", node_id_eq, 1);
    rb_define_method(rb_cNodeId, "hash", node_id_hash, 0);
}
