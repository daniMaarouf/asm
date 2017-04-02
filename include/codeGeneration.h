#ifndef __DMAAROUF_CODEGENERATION__
#define __DMAAROUF_CODEGENERATION__

#include <stdbool.h>
#include "tokenData.h"

bool fillInstructionFields(struct LinkedToken * tokens);

bool generateCode(struct LinkedToken * tokens, const char * fileLoc);

#endif
/* defined __DMAAROUF_CODEGENERATION__ */
