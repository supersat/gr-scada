/* -*- c++ -*- */

#define SCADA_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "scada_swig_doc.i"

%{
#include "scada/mdsx710_decode.h"
#include "scada/filters.h"
%}

%include "scada/mdsx710_decode.h"
GR_SWIG_BLOCK_MAGIC2(scada, mdsx710_decode);
%include "scada/filters.h"
