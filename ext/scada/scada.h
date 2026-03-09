#ifndef SCADA_H
#define SCADA_H

#include <ruby.h>
#include <ruby/thread.h>
#include "deps/open62541.h"

/* Module/class VALUE references */
extern VALUE rb_mScada;
extern VALUE rb_mNS0;
extern VALUE rb_mDataType;
extern VALUE rb_cError;
extern VALUE rb_cNodeId;
extern VALUE rb_cServer;
extern VALUE rb_cClient;
extern VALUE rb_cDataValue;
extern VALUE rb_cStatus;

/* NodeId helpers */
VALUE scada_node_id_numeric(uint16_t ns, uint32_t id);
VALUE scada_node_id_wrap(UA_NodeId nid);
UA_NodeId scada_node_id_unwrap(VALUE self);
VALUE scada_node_id_parse(VALUE str);

/* Variant helpers */
VALUE scada_variant_to_ruby(const UA_Variant *var);
void scada_ruby_to_variant(VALUE rb_val, VALUE rb_type, UA_Variant *var);
const UA_DataType *scada_resolve_type_symbol(VALUE sym);

/* DataValue helpers */
VALUE scada_data_value_to_ruby(const UA_DataValue *dv);

/* Status check - raises on error */
void scada_check_status(UA_StatusCode code);

/* Init functions */
void Init_scada_node_id(VALUE rb_mScada);
void Init_scada_status(VALUE rb_mScada);
void Init_scada_server(VALUE rb_mScada);
void Init_scada_client(VALUE rb_mScada);

/* Localized text helper */
UA_LocalizedText scada_to_localized_text(VALUE rb_text);

/* Certificate/key helper: converts Ruby String/Pathname/OpenSSL to UA_ByteString */
UA_ByteString scada_to_bytestring(VALUE rb_val);

/* Custom logger: bridges open62541 logging to a Ruby proc.
 * Pass Qfalse for a silent (null) logger. */
UA_Logger *scada_create_logger(VALUE rb_proc);

/* Deactivate a custom logger (safe to call before UA_Server/Client_delete) */
void scada_deactivate_logger(UA_Logger *logger);

/* Toggle GVL flag on logger (0 = no GVL, use trampoline; 1 = has GVL, direct call) */
void scada_logger_set_gvl(UA_Logger *logger, int has_gvl);

#endif
