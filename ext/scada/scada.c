#include "scada.h"

VALUE rb_mScada;
VALUE rb_mNS0;
VALUE rb_mDataType;
VALUE rb_cError;
VALUE rb_cNodeId;
VALUE rb_cServer;
VALUE rb_cClient;
VALUE rb_cDataValue;
VALUE rb_cStatusCode;

static ID id_by_code;

void scada_check_status(UA_StatusCode code) {
    if (code == UA_STATUSCODE_GOOD) return;
    VALUE by_code = rb_ivar_get(rb_cError, id_by_code);
    if (!NIL_P(by_code)) {
        VALUE entry = rb_hash_aref(by_code, UINT2NUM(code));
        if (!NIL_P(entry)) {
            VALUE klass = rb_ary_entry(entry, 0);
            VALUE desc  = rb_ary_entry(entry, 1);
            rb_raise(klass, "%s", StringValueCStr(desc));
        }
    }
    rb_raise(rb_cError, "OPC UA error: 0x%08x", code);
}

void Init_scada(void) {
    id_by_code = rb_intern("@by_code");

    rb_mScada = rb_define_module("Scada");
    rb_mNS0 = rb_define_module_under(rb_mScada, "NS0");
    rb_mDataType = rb_define_module_under(rb_mScada, "DataType");
    rb_cError = rb_define_class_under(rb_mScada, "Error", rb_eStandardError);

    rb_define_const(rb_mScada, "OPEN62541_VERSION", rb_str_freeze(rb_str_new_cstr(UA_OPEN62541_VERSION)));

    /* Look up Ruby-defined value objects */
    rb_cDataValue = rb_const_get(rb_mScada, rb_intern("DataValue"));
    rb_cStatusCode = rb_const_get(rb_mScada, rb_intern("StatusCode"));

    /* Init subsystems */
    Init_scada_node_id(rb_mScada);
    Init_scada_status(rb_mScada);
    Init_scada_server(rb_mScada);
    Init_scada_client(rb_mScada);
}
