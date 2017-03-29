#include "../include/pvr.h"

#include <kos.h>

void initiatePVR() {
  pvr_init_params_t pvrParams;
  pvrParams.opb_sizes[0] = 16;
  pvrParams.opb_sizes[1] = 0;
  pvrParams.opb_sizes[2] = 16;
  pvrParams.opb_sizes[3] = 0;
  pvrParams.opb_sizes[4] = 16;
  pvrParams.vertex_buf_size = 512 * 1024;
  pvrParams.dma_enabled = 0;
  pvrParams.fsaa_enabled = 0;

  pvr_init(&pvrParams);
}
