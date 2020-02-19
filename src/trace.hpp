#ifndef EO_TRACE_HPP
#define EO_TRACE_HPP

//#ifdef ENABLE_TRACE
#include "cio/cio.hpp"
#define trace_log(printer) cio::out << '[' << TRACE_CTX << ':' << __func__ << ']' << ' ' << printer << cio::endl
//#else
//#define trace_log(printer)
//#endif

#endif // EO_TRACE_HPP
