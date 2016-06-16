#ifndef SAMPLER_H_
# define SAMPLER_H_

# include "precomp.h"

typedef struct sampler_ctx *sampler_t;

HANDLE start_sampling(HANDLE logfile, int sampling, int threshold, sampler_t *sampler);
void shutdown_sampling(sampler_t sampler);

#endif /* !SAMPLER_H_ */
