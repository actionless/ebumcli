#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "ebur128.h"


#define have_args(LONG_ARG, SHORT_ARG) ((strcmp(av[arg_offset], LONG_ARG) == 0) || (strcmp(av[arg_offset], SHORT_ARG) == 0))
#define TRUE 1
#define FALSE 0


int measure_loudness(ebur128_state** sts, int i, const char* filename, double loudness) {
  sf_count_t nr_frames_read;
  double* buffer;
  SF_INFO file_info;
  SNDFILE* file;

    memset(&file_info, '\0', sizeof(file_info));
    file = sf_open(filename, SFM_READ, &file_info);
    if (!file) {
      fprintf(stderr, "Could not open file with sf_open!\n");
      return 1;
    }

    sts[i] = ebur128_init((unsigned) file_info.channels,
                          (unsigned) file_info.samplerate, EBUR128_MODE_I);
    if (!sts[i]) {
      fprintf(stderr, "Could not create ebur128_state!\n");
      return 1;
    }

    /* example: set channel map (note: see ebur128.h for the default map) */
    if (file_info.channels == 5) {
      ebur128_set_channel(sts[i], 0, EBUR128_LEFT);
      ebur128_set_channel(sts[i], 1, EBUR128_RIGHT);
      ebur128_set_channel(sts[i], 2, EBUR128_CENTER);
      ebur128_set_channel(sts[i], 3, EBUR128_LEFT_SURROUND);
      ebur128_set_channel(sts[i], 4, EBUR128_RIGHT_SURROUND);
    }

    buffer = (double*) malloc(sts[i]->samplerate * sts[i]->channels *
                              sizeof(double));
    if (!buffer) {
      fprintf(stderr, "malloc failed\n");
      return 1;
    }

    while ((nr_frames_read = sf_readf_double(
                file, buffer, (sf_count_t) sts[i]->samplerate))) {
      ebur128_add_frames_double(sts[i], buffer, (size_t) nr_frames_read);
    }

    ebur128_loudness_global(sts[i], &loudness);
    fprintf(stderr, "%.2f LUFS, %s\n", loudness, filename);

    free(buffer);
    buffer = NULL;
    if (sf_close(file)) {
      fprintf(stderr, "Could not close input file!\n");
    }

	return 0;
}

int measure_true_peak(ebur128_state** sts, int i, const char* filename) {
  double true_peak;
  double max_true_peak = -DBL_MAX;
  int y;
  sf_count_t nr_frames_read;
  double* buffer;
  SF_INFO file_info;
  SNDFILE* file;

    memset(&file_info, '\0', sizeof(file_info));
    file = sf_open(filename, SFM_READ, &file_info);
    if (!file) {
      fprintf(stderr, "Could not open file with sf_open!\n");
      return 1;
    }

  sts[i] = ebur128_init((unsigned) file_info.channels,
  					  (unsigned) file_info.samplerate, EBUR128_MODE_TRUE_PEAK);
  buffer = (double*) malloc(sts[i]->samplerate * sts[i]->channels *
  						  sizeof(double));
  if (!buffer) {
    fprintf(stderr, "malloc failed\n");
    return 1;
  }
  while ((nr_frames_read = sf_readf_double(
  			file, buffer, (sf_count_t) sts[i]->samplerate))) {
    ebur128_add_frames_double(sts[i], buffer, (size_t) nr_frames_read);
  }
  for (y=0; y<=file_info.channels; y++) {
  	ebur128_true_peak(sts[i], y, &true_peak);
	true_peak = 20 * log10(true_peak);
	if (true_peak > max_true_peak) {
		max_true_peak = true_peak;
	}
  }
  fprintf(stderr, "%.2f TruePeak, %s\n", max_true_peak, filename);

  free(buffer);
  buffer = NULL;
    if (sf_close(file)) {
      fprintf(stderr, "Could not close input file!\n");
    }

  return 0;
}


int main(int ac, const char* av[]) {
  ebur128_state** sts = NULL;
  ebur128_state** sts_tp = NULL;
  double loudness = 0.0;
  int i;
	int TRUE_PEAK = 0;
	int arg_offset = 1;

  if ((ac < 2) || have_args("--help", "-h")) {
    fprintf(stderr, "usage: %s [--tp/-t] FILENAME...\n", av[0]);
    exit(1);
  }

  if (have_args("--tp", "-t")) {
	  TRUE_PEAK = TRUE;
	  arg_offset++;
  }

  sts = malloc((size_t) (ac - arg_offset) * sizeof(ebur128_state*));
  if (TRUE_PEAK == TRUE) {
	  sts_tp = malloc((size_t) (ac - arg_offset) * sizeof(ebur128_state*));
	  if (!sts || !sts_tp) {
		fprintf(stderr, "malloc failed\n");
		return 1;
	  }
	}

  for (i = 0; i < ac - arg_offset; ++i) {
		fprintf(stderr, "%i\n", i);

	measure_loudness(sts, i, av[i+arg_offset], loudness);
	  if (TRUE_PEAK == TRUE) {
		measure_true_peak(sts_tp, i, av[i+arg_offset]);
		}
  }

  ebur128_loudness_global_multiple(sts, (size_t) ac - arg_offset, &loudness);
  fprintf(stderr, "-----------\n%.2f LUFS\n", loudness);

  /* clean up */
  for (i = 0; i < ac - arg_offset; ++i) {
    ebur128_destroy(&sts[i]);
  }
  free(sts);
  if (TRUE_PEAK == TRUE) {
	  for (i = 0; i < ac - arg_offset; ++i) {
		ebur128_destroy(&sts_tp[i]);
	  }
	  free(sts_tp);
	}

  return 0;
}
