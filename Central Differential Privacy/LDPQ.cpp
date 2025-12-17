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
#include <boost/random.hpp>


int locally_randomized_compare(float q, float r, float x, std::mt19937 gen);

void usage(void) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "-n <number of items to be generated> default: 100 millions of items\n");
  fprintf(stderr, "-q <quantile (0<q<1)> default: 0.99\n");
  fprintf(stderr, "-r <response rate> default: 0.25\n");
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

int main(int argc, char **argv) {

  long seed = 1234;
  float quantile = 0.99;
  float *items = NULL;
  long len = 100000000;
  long dist = 1;
  char *diststr = NULL;
  float param1, param2;
  char *filename = NULL;
  FILE *fptr = NULL;
  float true_quantile;
  float estimated_quantile = 0.0;
  float elapsed = 0.0;
  bool file_output = false;
  bool param1_default = true;
  bool param2_default = true;
  float r = 0.75; // response rate
  
  

  int opt;

  while ((opt = getopt(argc, argv, ":n:q:r:d:a:b:s:f:h")) != -1) {
    switch (opt) {
    case 'n':
      len = strtol(optarg, NULL, 10);
      break;
    case 'q':
      quantile = strtof(optarg, NULL);
      break;
    case 'r':
      r = strtof(optarg, NULL);
      break;
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
  items = (float *)calloc(len, sizeof(float));
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

  std::mt19937 generator(seed);

  std::normal_distribution<float> normaldistribution(param1, param2);
  std::cauchy_distribution<float> cauchydistribution(param1, param2);
  std::uniform_real_distribution<float> uniformrealdistribution(param1, param2);
  std::exponential_distribution<float> exponentialdistribution(param1);
  std::chi_squared_distribution<float> chisquareddistribution(param1);
  std::gamma_distribution<float> gammadistribution(param1, param2);
  std::lognormal_distribution<float> lognormaldistribution(param1, param2);
  std::extreme_value_distribution<float> extremevaluedistribution(param1, param2);
  

  switch (dist) {

  case 1:
    for (long i = 0; i < len; i++) {
      items[i] = normaldistribution(generator);
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
      items[i] = cauchydistribution(generator);
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
      items[i] = uniformrealdistribution(generator);
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
      items[i] = exponentialdistribution(generator);
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
      items[i] = chisquareddistribution(generator);
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
      items[i] = gammadistribution(generator);
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
      items[i] = lognormaldistribution(generator);
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
      items[i] = extremevaluedistribution(generator);
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
      items[i] = normaldistribution(generator);
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

  // determine the true quantile
  std::vector<float> vec(items, items + len);
  auto q = vec.begin() + vec.size() * quantile;
  std::nth_element(vec.begin(), q, vec.end());
  true_quantile = vec[vec.size() * quantile];
  fprintf(stderr, "the true quantile %.2f is %.6f\n", quantile, (float)true_quantile);


  clock_t begin_time = clock();
  
  for (long i = 0; i < len; i++) {
  
  	  float stepsize = 2/(pow((float)i,0.51)+100.0);
  
      int s = locally_randomized_compare(estimated_quantile, r, items[i], generator);
      if (s == 1)
      	estimated_quantile = estimated_quantile + ((1.0-r+2.0*quantile*r)/2.0) * stepsize; 
      else 
      	estimated_quantile = estimated_quantile - ((1.0+r-2.0*quantile*r)/2.0) * stepsize; 
      	
  }
  
  clock_t end_time = clock();
  elapsed = (float)(end_time - begin_time) / CLOCKS_PER_SEC;

  free(items), items = NULL;

  float relative_error = fabs((estimated_quantile - true_quantile) /  true_quantile);
  float eps = log((1+r)/(1-r));
 
  fprintf(stdout, "response rate: %.2f\n", r);
  fprintf(stdout, "epsilon corresponding to r: %.6f\n", eps);
  fprintf(stdout, "private estimated quantile: %.6f\n", estimated_quantile);
  fprintf(stdout, "the relative error is: %.6f\n", relative_error);
  fprintf(stdout, "elapsed time %.6f\n", elapsed);
  fprintf(stdout, "updates/s %ld\n", lround(len / elapsed));

  
  if (file_output) {

    fptr = fopen(filename, "w");

    if (!fptr) {
      fprintf(stderr, "Error opening file %s\n", filename);
      free(filename), filename = NULL;
      exit(1);
    }

    // writing to csv file the following information:

   //<n>, <quantile>, <distribution>, <param1>, <param2>, <seed>, <estimated
      //quantile>, <true quantile>, <relative error>, <elapsed time>,
      //<updates/s>, <response rate><corresponding epsilon>
      fprintf(fptr, "%ld, %.2f, %s, %.6f, %.6f, %ld, %.6f, %.6f, %.6f, %.6f, %.ld, %.2f, %.6f\n", 
      len, quantile, diststr, param1, param2, seed,        
      estimated_quantile, true_quantile,
      relative_error, elapsed, lround(len / elapsed), r, eps);
      
      fclose(fptr);
    
    free(filename), filename = NULL;
  }

  free(diststr), diststr = NULL;

  return 0;
}


int locally_randomized_compare(float q, float r, float x, std::mt19937 gen)
{

	std::bernoulli_distribution bernoulli_r(r);
	std::bernoulli_distribution bernoulli_one_half(0.5);
	int u;
	int v;
	
	//std::cout << br(gen) << " " << boh(gen) << std::endl;
	
	if(bernoulli_r(gen))
		u = 1;
	else
		u = 0;
	
	if(bernoulli_one_half(gen))
		v = 1;
	else
		v = 0;
	
	if(u == 1){
		if(x > q)
			return 1;
		else
			return 0;
	}
	else{
		return v;
	}
		
}
