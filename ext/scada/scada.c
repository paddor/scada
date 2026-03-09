#include "scada.h"
#include "generated/ns0_ids.h"
#include "generated/status_codes.h"

VALUE rb_mScada;
VALUE rb_mNS0;
VALUE rb_mDataType;
VALUE rb_cError;
VALUE rb_cNodeId;
VALUE rb_cServer;
VALUE rb_cClient;
VALUE rb_cDataValue;
VALUE rb_cStatus;

void Init_scada(void) {
    rb_mScada = rb_define_module("Scada");
    rb_mNS0 = rb_define_module_under(rb_mScada, "NS0");
    rb_mDataType = rb_define_module_under(rb_mScada, "DataType");
    rb_cError = rb_define_class_under(rb_mScada, "Error", rb_eStandardError);

    rb_define_const(rb_mScada, "OPEN62541_VERSION", rb_str_freeze(rb_str_new_cstr(UA_OPEN62541_VERSION)));

    /* Register generated constants and errors */
    scada_register_ns0(rb_mNS0, rb_mDataType);
    scada_register_errors(rb_cError);

    /* Look up Ruby-defined value objects */
    rb_cDataValue = rb_const_get(rb_mScada, rb_intern("DataValue"));
    rb_cStatus = rb_const_get(rb_mScada, rb_intern("Status"));

    /* Init subsystems */
    Init_scada_node_id(rb_mScada);
    Init_scada_status(rb_mScada);
    Init_scada_server(rb_mScada);
    Init_scada_client(rb_mScada);
}
