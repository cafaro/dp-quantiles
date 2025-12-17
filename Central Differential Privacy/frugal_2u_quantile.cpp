// Copyright
// Massimo Cafaro
// University of Salento, Lecce, Italy
// June 2024

#include <chrono>
#include <getopt.h>
#include <iostream>
#include <limits>
#include <random>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <boost/random.hpp>
#include <boost/random/laplace_distribution.hpp>


void usage(void) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "-n <number of items to be generated> default: 500 millions of items\n");
  fprintf(stderr, "-q <quantile (0<q<1)> default: 0.99\n");
  fprintf(stderr, "-k <number of chunks (2<= k <= 32)> default: 4\n");
  fprintf(stderr, "-e <epsilon> default: 0.1\n");
  //fprintf(stderr, "-u <upper value of distribution> default: 5000.0\n");
  //fprintf(stderr, "-l <lower value of distribution> default: -5000.0\n");
  fprintf(
      stderr,
      "-d <distribution: 1(normal)|2(cauchy)|3(uniform)|4(exponential)|5(chi "
      "squared)|6(gamma)|7(lognormal)|8(extreme value)>  default: normal\n");
  fprintf(stderr, "-a <parameter> first parameter of the distribution default: "
                  "depends on selected distribution\n");
  fprintf(stderr, "-b <parameter> second parameter of the distribution "
                  "default: depends on selected distribution\n");
  fprintf(stderr, "-s <seed> the seed to be used for pseudo-random number "
                  "generator default: 1234\n");
  fprintf(stderr, "-f <filename>\n");

}

// this function is applied to the step
// to trade off convergence speed for estimation stability,
// we apply a constant factor additive update to the step size
// i.e., f(step) = 1
int f(int x) { return 1; }

int main(int argc, char **argv) {

  long seed = 1234;
  float quantile = 0.99;
  int *items = NULL;
  long len = 500000000;
  long dist = 1;
  char *diststr = NULL;
  float param1, param2;
  char *filename = NULL;
  FILE *fptr = NULL;
  int true_quantile;
  int *estimated_quantile = NULL;
  float elapsed = 0.0;
  bool file_output = false;
  bool param1_default = true;
  bool param2_default = true;
  int *stepsize = NULL;
  int *sign = NULL;
  int chunks = 4;
  float upper = INT_MAX;
  float lower = INT_MIN;
  float epsilon = 0.1;

  int opt;

  while ((opt = getopt(argc, argv, ":n:q:k:e:d:a:b:s:f:h")) != -1) {
    switch (opt) {
    case 'n':
      len = strtol(optarg, NULL, 10);
      break;
    case 'q':
      quantile = strtof(optarg, NULL);
      break;
    case 'k':
      chunks = strtol(optarg, NULL, 10);
      break;
    case 'e':
      epsilon = strtof(optarg, NULL);
      break;
#if 0
    case 'u':
      upper = strtof(optarg, NULL);
      break;
	case 'l':
      lower = strtof(optarg, NULL);
      break;
#endif
    case 'd':
      dist = strtol(optarg, NULL, 10);
      break;
    case 'a':
      param1 = strtof(optarg, NULL);
      param1_default = false;
      break;
    case 'b':
      param2 = strtof(optarg, NULL);
      param2_default = false;
      break;
    case 's':
      seed = strtol(optarg, NULL, 10);
      break;
    case 'f':
      filename = (char *)calloc(strlen(optarg) + 1, sizeof(char));
      if (!filename) {
        fprintf(stderr, "not enough memory\n");
        exit(1);
      }
      memcpy(filename, optarg, strlen(optarg));
      file_output = true;
      break;
    case 'h':
      usage();
      exit(1);
      break;
    case '?':
      fprintf(stderr, "Unknown option: %c\n", optopt);
      usage();
      exit(1);
      break;
    case ':':
      fprintf(stderr, "Missing argument for option -%c\n", optopt);
      usage();
      exit(1);
      break;
    }
  }

  /* allocate items */
  items = (int *)calloc(len, sizeof(int));
  if (!items) {
    fprintf(stderr, "Not enough memory\n");
    exit(1);
  }
  // unsigned int	seed =
  // std::chrono::steady_clock::now().time_since_epoch().count();

  // set default parameter values depending on the distribution
  switch (dist) {

  case 1:
    if (param1_default)
      param1 = 50.0;
    if (param2_default)
      param2 = 2.0;
    break;
  case 2:
    if (param1_default)
      param1 = 10000.0;
    if (param2_default)
      param2 = 1250.0;
    break;
  case 3:
    if (param1_default)
      param1 = 0.0;
    if (param2_default)
      param2 = 1000.0;
    break;
  case 4:
    if (param1_default)
      param1 = 0.5;
    if (param2_default)
      param2 = -1.0; // not used
    break;
  case 5:
    if (param1_default)
      param1 = 5.0;
    if (param2_default)
      param2 = -1.0; // not used
    break;
  case 6:
    if (param1_default)
      param1 = 2.0;
    if (param2_default)
      param2 = 4.0;
    break;
  case 7:
    if (param1_default)
      param1 = 1.0;
    if (param2_default)
      param2 = 1.5;
    break;
  case 8:
    if (param1_default)
      param1 = 20.0;
    if (param2_default)
      param2 = 2.0;
    break;
  default:
    param1 = 50.0;
    param2 = 2.0;
    break;
  }

  std::default_random_engine generator(seed);

  std::normal_distribution<float> normaldistribution(param1, param2);
  std::cauchy_distribution<float> cauchydistribution(param1, param2);
  std::uniform_real_distribution<float> uniformrealdistribution(param1, param2);
  std::exponential_distribution<float> exponentialdistribution(param1);
  std::chi_squared_distribution<float> chisquareddistribution(param1);
  std::gamma_distribution<float> gammadistribution(param1, param2);
  std::lognormal_distribution<float> lognormaldistribution(param1, param2);
  std::extreme_value_distribution<float> extremevaluedistribution(param1,
                                                                  param2);

  switch (dist) {

  case 1:
    for (long i = 0; i < len; i++) {
      items[i] = normaldistribution(generator) * 1000.0;
    }
    diststr = (char *)calloc(16, sizeof(char));
    if (!diststr) {
      fprintf(stderr, "not enough memory\n");
      exit(1);
    }
    memcpy(diststr, "normal", sizeof("normal"));
    break;
  case 2:
    for (long i = 0; i < len; i++) {
      items[i] = cauchydistribution(generator) * 1000.0;
    }
    diststr = (char *)calloc(16, sizeof(char));
    if (!diststr) {
      fprintf(stderr, "not enough memory\n");
      exit(1);
    }
    memcpy(diststr, "cauchy", sizeof("cauchy"));
    break;
  case 3:
    for (long i = 0; i < len; i++) {
      items[i] = uniformrealdistribution(generator) * 1000.0;
      ;
    }
    diststr = (char *)calloc(16, sizeof(char));
    if (!diststr) {
      fprintf(stderr, "not enough memory\n");
      exit(1);
    }
    memcpy(diststr, "uniform", sizeof("uniform"));
    break;
  case 4:
    for (long i = 0; i < len; i++) {
      items[i] = exponentialdistribution(generator) * 1000.0;
    }
    diststr = (char *)calloc(16, sizeof(char));
    if (!diststr) {
      fprintf(stderr, "not enough memory\n");
      exit(1);
    }
    memcpy(diststr, "exponential", sizeof("exponential"));
    break;
  case 5:
    for (long i = 0; i < len; i++) {
      items[i] = chisquareddistribution(generator) * 1000.0;
    }
    diststr = (char *)calloc(16, sizeof(char));
    if (!diststr) {
      fprintf(stderr, "not enough memory\n");
      exit(1);
    }
    memcpy(diststr, "chisquared", sizeof("chisquared"));
    break;
  case 6:
    for (long i = 0; i < len; i++) {
      items[i] = gammadistribution(generator) * 1000.0;
    }
    diststr = (char *)calloc(16, sizeof(char));
    if (!diststr) {
      fprintf(stderr, "not enough memory\n");
      exit(1);
    }
    memcpy(diststr, "gamma", sizeof("gamma"));
    break;
  case 7:
    for (long i = 0; i < len; i++) {
      items[i] = lognormaldistribution(generator) * 1000.0;
    }
    diststr = (char *)calloc(16, sizeof(char));
    if (!diststr) {
      fprintf(stderr, "not enough memory\n");
      exit(1);
    }
    memcpy(diststr, "lognormal", sizeof("lognormal"));
    break;
  case 8:
    for (long i = 0; i < len; i++) {
      items[i] = extremevaluedistribution(generator) * 1000.0;
    }
    diststr = (char *)calloc(16, sizeof(char));
    if (!diststr) {
      fprintf(stderr, "not enough memory\n");
      exit(1);
    }
    memcpy(diststr, "extremevalue", sizeof("extremevalue"));
    break;
  default:
    for (long i = 0; i < len; i++) {
      items[i] = normaldistribution(generator) * 1000.0;
    }
    diststr = (char *)calloc(16, sizeof(char));
    if (!diststr) {
      fprintf(stderr, "not enough memory\n");
      exit(1);
    }
    memcpy(diststr, "normal", sizeof("normal"));
    break;
  }

  fprintf(stderr, "generated random %ld items\n", len);
  if (dist == 1)
    fprintf(stderr,
            "using the normal distribution with parameters mu=%.6f and sigma=%.6f "
            "and seed %ld\n",
            param1, param2, seed);
  if (dist == 2)
    fprintf(stderr,
            "using the cauchy distribution with parameters a=%.6f and b=%.6f and "
            "seed %ld\n",
            param1, param2, seed);
  if (dist == 3)
    fprintf(stderr,
            "using the uniform distribution with parameters a=%.6f and b=%.6f and "
            "seed %ld\n",
            param1, param2, seed);
  if (dist == 4)
    fprintf(
        stderr,
        "using the exponential distribution with parameter a=%.6f and seed %ld\n",
        param1, seed);
  if (dist == 5)
    fprintf(
        stderr,
        "using the chi squared distribution with parameter a=%.6f and seed %ld\n",
        param1, seed);
  if (dist == 6)
    fprintf(stderr,
            "using the gamma distribution with parameters a=%.6f and b=%.6f and "
            "seed %ld\n",
            param1, param2, seed);
  if (dist == 7)
    fprintf(stderr,
            "using the lognormal distribution with parameters a=%.6f and b=%.6f "
            "and seed %ld\n",
            param1, param2, seed);
  if (dist == 8)
    fprintf(stderr,
            "using the extreme value distribution with parameters a=%.6f and "
            "b=%.6f and seed %ld\n",
            param1, param2, seed);

  fprintf(stderr, "Chunks for DP: %d\n", chunks);

  // determine the true quantile, maximum and minimum values
  std::vector<int> vec(items, items + len);
  auto q = vec.begin() + vec.size() * quantile;
  std::nth_element(vec.begin(), q, vec.end());
  true_quantile = vec[vec.size() * quantile];
  upper = (float) (*max_element(vec.begin(), vec.end()) / 1000.0);
  lower = (float) (*min_element(vec.begin(), vec.end()) / 1000.0);
  fprintf(stderr, "the true quantile %.2f is %.6f\n", quantile, (float)true_quantile / 1000.0);
  fprintf(stderr, "maximum value: %.6f minimum value: %.6f\n", upper, lower);

  std::mt19937 gen(seed);
  std::uniform_real_distribution<> dis(0.0, 1.0);

  // set the estimated quantile to the value of the first item
  estimated_quantile = (int *) calloc(chunks, sizeof(int));
  stepsize = (int *) calloc(chunks, sizeof(int));
  sign = (int *) calloc(chunks, sizeof(int));

  if(!estimated_quantile || !stepsize || !sign){
  	fprintf(stderr, "Not enough memory\n");
  	exit(1);
  }

  memset(sign, 1, chunks);


  for(int i = 0; i < chunks; i++)
  	estimated_quantile[i] = items[i];

 clock_t begin_time = clock();

	int idx;

    for (long i = chunks; i < len; ++i) {

      float rnd = dis(gen);
	  idx = (i % chunks);

      if (items[i] > estimated_quantile[idx] && rnd > 1.0 - quantile) {
        stepsize[idx] += (sign[idx] > 0) ? f(stepsize[idx]) : -f(stepsize[idx]);
        estimated_quantile[idx] += (stepsize[idx] > 0) ? stepsize[idx] : 1;
        sign[idx] = 1;

        if (estimated_quantile[idx] > items[i]) {
          stepsize[idx] += items[i] - estimated_quantile[idx];
          estimated_quantile[idx] = items[i];

        }

      } else {
        if (items[i] < estimated_quantile[idx] && rnd > quantile) {

          stepsize[idx] += (sign[idx] < 0) ? f(stepsize[idx]) : -f(stepsize[idx]);
          estimated_quantile[idx] -= (stepsize[idx] > 0) ? stepsize[idx] : 1;
          sign[idx] = -1;

          if (estimated_quantile[idx] < items[i]) {
            stepsize[idx] += estimated_quantile[idx] - items[i];
            estimated_quantile[idx] = items[i];


          }
        }
      }

      if ((estimated_quantile[idx] - items[i]) * sign[idx] < 0 && stepsize[idx] > 1){
        stepsize[idx] = 1;
      }

    }


    clock_t end_time = clock();
    elapsed = (double)(end_time - begin_time) / CLOCKS_PER_SEC;

  free(items), items = NULL;

  float eq = 0;
  for(int i = 0; i < chunks; i++)
  	eq += estimated_quantile[i];

  eq /= chunks;

  //float relative_error = fabs((eq / 1000.0 - (float)true_quantile / 1000.0)) / fabs((float)true_quantile / 1000.0);

  fprintf(stdout, "estimated quantile: %.6f\n", eq / 1000.0);
  //fprintf(stdout, "the relative error is: %.6f\n", relative_error);
  fprintf(stdout, "elapsed time %.6f\n", elapsed);
  fprintf(stdout, "updates/s %ld\n", lround(len / elapsed));

  // differentially private release of the estimated quantile
  // Laplace mechanism
  boost::random::mt19937 rng(seed);
  boost::random::laplace_distribution<float> laplace(0.0, (upper - lower)/(chunks * epsilon));
  boost::variate_generator <boost::random::mt19937&, boost::random::laplace_distribution<float>> laplace_gen(rng, laplace);
  float laplace_noise = laplace_gen();
  float dp_laplace_estimated_quantile = (eq / 1000.0) + laplace_noise;
  fprintf(stdout, "DP epsilon: %.6f\n", epsilon);
  fprintf(stdout, "DP Laplace based estimated sensitivity: %.6f\n", (upper - lower)/ chunks);
  fprintf(stdout, "DP Laplace based estimated quantile: %.6f\n", dp_laplace_estimated_quantile);

  float dp_rel_err = fabs((dp_laplace_estimated_quantile - (float)(true_quantile / 1000.0)) / (float)(true_quantile / 1000.0));
  fprintf(stdout, "the relative error for the DP estimated quantile is: %.6f\n", dp_rel_err);




  if (file_output) {

    fptr = fopen(filename, "w");

    if (!fptr) {
      fprintf(stderr, "Error opening file %s\n", filename);
      free(filename), filename = NULL;
      exit(1);
    }

    // writing to csv file the following information:

    //<n>, <quantile>, <distribution>, <param1>, <param2>, <seed>, <estimated
      //quantile>, <true quantile>, <elapsed time>,
      //<updates/s>, <epsilon>, <estimated sensitivity>, <chunks>, <laplace dp estimate>, <DP relative error>
      fprintf(fptr, "%ld, %.2f, %s, %.6f, %.6f, %ld, %.6f, %.6f, %.6f, %ld, %.6f, %.6f, %d, %.6f, %.6f\n",
              len, quantile, diststr, param1, param2, seed,
              eq / 1000.0, (float)true_quantile / 1000.0,
              elapsed, lround(len / elapsed), epsilon, (upper - lower)/ chunks, chunks, dp_laplace_estimated_quantile, dp_rel_err);
      fclose(fptr);
    free(filename), filename = NULL;

  }

  free(diststr), diststr = NULL;

  return 0;
}
