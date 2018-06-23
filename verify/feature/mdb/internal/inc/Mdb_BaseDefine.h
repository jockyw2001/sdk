#ifndef _MDB_BASE_DEFINE_
#define _MDB_BASE_DEFINE_

#define TYPE_NUMBER(x)  "["<<(unsigned int)(x)<<"]"
#define DO_PRINT_ENUM(sstr, value, name) do  {  \
    sstr << #name << " " << (unsigned)value << " | ";  \
}while (0);

#endif
