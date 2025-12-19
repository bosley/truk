#ifndef ENUMHEADER_H
#define ENUMHEADER_H

typedef enum { STATUS_OK = 0, STATUS_ERROR = 1, STATUS_PENDING = 42 } Status;

static inline int get_status_value(Status s) { return (int)s; }

#endif
