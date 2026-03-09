#include "scada.h"

void Init_scada_status(VALUE rb_mScada) {
    /* rb_cStatus and rb_cDataValue are already set in Init_scada
       by looking up the Ruby-defined Data.define classes. */
    (void)rb_mScada;
}
