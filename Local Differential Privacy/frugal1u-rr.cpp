// Copyright
// Massimo Cafaro
// University of Salento, Lecce, Italy
// June 2024

#include "QuickSelect.h"
#include <cstring>
#include <getopt.h>
#include <math.h>
#include <random>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void log(char active, const char *fmt, ...)
{
    va_list args;
    if (active) {
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
}

int randomized_response(int q, double p, int x, std::mt19937 &gen1)
{
    std::bernoulli_distribution bernoulli_p(p);
    int u = bernoulli_p(gen1);

    if (u) {
        return (x > q)? 1 : 0;
    } else {
        return (x > q)? 0 : 1;
    }
}

void usage(void)
{
    fprintf(stderr, "Usage:\n");
    fprintf(
                stderr,
                "-n <number of items to be generated> default: 10 millions of items\n");
    fprintf(stderr, "-q <quantile (0<q<1)> default: 0.99\n");
    fprintf(stderr, "-e <privacy budget> default: 2\n");
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

int main(int argc, char **argv)
{
    long seed       = 1234;
    double quantile = 0.99;
    double *items   = NULL;
    long len        = 10000000;
    long dist       = 1;
    char *diststr   = NULL;
    double param1, param2;
    char *filename = NULL;
    FILE *fptr     = NULL;
    double true_quantile;
    double elapsed      = 0.0;
    bool file_output    = false;
    bool param1_default = true;
    bool param2_default = true;
    double eps          = 2.0;      // privacy budget
    double prec         = 1000000.0;

    int opt;

    while ((opt = getopt(argc, argv, ":n:q:e:d:a:b:s:f:p:h")) != -1) {
        switch (opt) {
            case 'n':
                len = strtol(optarg, NULL, 10);
                break;
            case 'q':
                quantile = strtof(optarg, NULL);
                break;
            case 'e':
                eps = strtof(optarg, NULL);
                break;
            case 'd':
                dist = strtol(optarg, NULL, 10);
                break;
            case 'p':
                prec = strtol(optarg, NULL, 10);
                break;
            case 'a':
                param1         = strtof(optarg, NULL);
                param1_default = false;
                break;
            case 'b':
                param2         = strtof(optarg, NULL);
                param2_default = false;
                break;
            case 's':
                seed = strtol(optarg, NULL, 10);
                break;
            case 'f':
                filename = (char *) calloc(strlen(optarg) + 1, sizeof(char));
                if (! filename) {
                    log(! file_output, "not enough memory\n");
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
                log(! file_output, "Unknown option: %c\n", optopt);
                usage();
                exit(1);
                break;
            case ':':
                log(! file_output, "Missing argument for option -%c\n", optopt);
                usage();
                exit(1);
                break;
        }
    }

    /* allocate items */
    items = (double *) calloc(len, sizeof(double));
    if (! items) {
        log(! file_output, "Not enough memory\n");
        exit(1);
    }

    // unsigned int	seed =
    // std::chrono::steady_clock::now().time_since_epoch().count();

    // set default parameter values depending on the distribution
    switch (dist) {

        case 1:
            if (param1_default)
                param1 = 0.0;
            if (param2_default)
                param2 = 1.0;
            break;
        case 2:
            if (param1_default)
                param1 = 0.0;
            if (param2_default)
                param2 = 1.0;
            break;
        case 3:
            if (param1_default)
                param1 = -1.0;
            if (param2_default)
                param2 = 1.0;
            break;
        case 4:
            if (param1_default)
                param1 = 0.5;
            if (param2_default)
                param2 = -1.0;      // not used
            break;
        case 5:
            if (param1_default)
                param1 = 5.0;
            if (param2_default)
                param2 = -1.0;      // not used
            break;
        case 6:
            if (param1_default)
                param1 = 3.0;
            if (param2_default)
                param2 = 2.0;
            break;
        case 7:
            if (param1_default)
                param1 = 0.0;
            if (param2_default)
                param2 = 0.5;
            break;
        case 8:
            if (param1_default)
                param1 = 1.5;
            if (param2_default)
                param2 = 3.0;
            break;
        default:
            param1 = 0.0;
            param2 = 1.0;
            break;
    }

    std::srand(seed);
    long seed1 = std::rand();
    long seed2 = std::rand();
    long seed3 = std::rand();
    long seed4 = std::rand();

    log(! file_output, "Seeds generated: %ld, %ld, %ld\n", seed1, seed2, seed3);

    std::mt19937 mtgenerator(seed1);
    std::mt19937 mtgenerator1(seed2);
    std::mt19937 mtgenerator2(seed3);
    std::mt19937 mtgenerator3(seed4);

    std::normal_distribution<double> normaldistribution(param1, param2);
    std::cauchy_distribution<double> cauchydistribution(param1, param2);
    std::uniform_real_distribution<double> uniformrealdistribution(param1, param2);
    std::exponential_distribution<double> exponentialdistribution(param1);
    std::chi_squared_distribution<double> chisquareddistribution(param1);
    std::gamma_distribution<double> gammadistribution(param1, param2);
    std::lognormal_distribution<double> lognormaldistribution(param1, param2);
    std::extreme_value_distribution<double> extremevaluedistribution(param1, param2);

    switch (dist) {
        case 1:
            for (long i = 0; i < len; i++) {
                items[i] = normaldistribution(mtgenerator);
            }
            diststr = (char *) calloc(16, sizeof(char));
            if (! diststr) {
                log(! file_output, "not enough memory\n");
                exit(1);
            }
            memcpy(diststr, "normal", sizeof("normal"));
            break;
        case 2:
            for (long i = 0; i < len; i++) {
                items[i] = cauchydistribution(mtgenerator);
            }
            diststr = (char *) calloc(16, sizeof(char));
            if (! diststr) {
                log(! file_output, "not enough memory\n");
                exit(1);
            }
            memcpy(diststr, "cauchy", sizeof("cauchy"));
            break;
        case 3:
            for (long i = 0; i < len; i++) {
                items[i] = uniformrealdistribution(mtgenerator);
                ;
            }
            diststr = (char *) calloc(16, sizeof(char));
            if (! diststr) {
                log(! file_output, "not enough memory\n");
                exit(1);
            }
            memcpy(diststr, "uniform", sizeof("uniform"));
            break;
        case 4:
            for (long i = 0; i < len; i++) {
                items[i] = exponentialdistribution(mtgenerator);
            }
            diststr = (char *) calloc(16, sizeof(char));
            if (! diststr) {
                log(! file_output, "not enough memory\n");
                exit(1);
            }
            memcpy(diststr, "exponential", sizeof("exponential"));
            break;
        case 5:
            for (long i = 0; i < len; i++) {
                items[i] = chisquareddistribution(mtgenerator);
            }
            diststr = (char *) calloc(16, sizeof(char));
            if (! diststr) {
                log(! file_output, "not enough memory\n");
                exit(1);
            }
            memcpy(diststr, "chisquared", sizeof("chisquared"));
            break;
        case 6:
            for (long i = 0; i < len; i++) {
                items[i] = gammadistribution(mtgenerator);
            }
            diststr = (char *) calloc(16, sizeof(char));
            if (! diststr) {
                log(! file_output, "not enough memory\n");
                exit(1);
            }
            memcpy(diststr, "gamma", sizeof("gamma"));
            break;
        case 7:
            for (long i = 0; i < len; i++) {
                items[i] = lognormaldistribution(mtgenerator);
            }
            diststr = (char *) calloc(16, sizeof(char));
            if (! diststr) {
                log(! file_output, "not enough memory\n");
                exit(1);
            }
            memcpy(diststr, "lognormal", sizeof("lognormal"));
            break;
        case 8:
            for (long i = 0; i < len; i++) {
                items[i] = extremevaluedistribution(mtgenerator);
            }
            diststr = (char *) calloc(16, sizeof(char));
            if (! diststr) {
                log(! file_output, "not enough memory\n");
                exit(1);
            }
            memcpy(diststr, "extremevalue", sizeof("extremevalue"));
            break;
        default:
            for (long i = 0; i < len; i++) {
                items[i] = normaldistribution(mtgenerator);
            }
            diststr = (char *) calloc(16, sizeof(char));
            if (! diststr) {
                log(! file_output, "not enough memory\n");
                exit(1);
            }
            memcpy(diststr, "normal", sizeof("normal"));
            break;
    }

    log(! file_output, "generated random %ld items\n", len);
    if (dist == 1)
        log(! file_output,
                    "using the normal distribution with parameters mu=%.6f and sigma=%.6f "
                    "and seed %ld\n",
                    param1, param2, seed1);
    if (dist == 2)
        log(! file_output,
                    "using the cauchy distribution with parameters a=%.6f and b=%.6f and "
                    "seed %ld\n",
                    param1, param2, seed1);
    if (dist == 3)
        log(! file_output,
                    "using the uniform distribution with parameters a=%.6f and b=%.6f and "
                    "seed %ld\n",
                    param1, param2, seed1);
    if (dist == 4)
        log(! file_output,
                    "using the exponential distribution with parameter a=%.6f and seed "
                    "%ld\n",
                    param1, seed1);
    if (dist == 5)
        log(! file_output,
                    "using the chi squared distribution with parameter a=%.6f and seed "
                    "%ld\n",
                    param1, seed1);
    if (dist == 6)
        log(! file_output,
                    "using the gamma distribution with parameters a=%.6f and b=%.6f and "
                    "seed %ld\n",
                    param1, param2, seed1);
    if (dist == 7)
        log(! file_output,
                    "using the lognormal distribution with parameters a=%.6f and b=%.6f "
                    "and seed %ld\n",
                    param1, param2, seed1);
    if (dist == 8)
        log(! file_output,
                    "using the extreme value distribution with parameters a=%.6f and "
                    "b=%.6f and seed %ld\n",
                    param1, param2, seed1);

    // stream min and max
    double smax = std::numeric_limits<double>::min();
    double smin = std::numeric_limits<double>::max();
    for (int i = 0; i < len; i++) {
        if (items[i] < smin)
            smin = items[i];
        if (items[i] > smax)
            smax = items[i];
    }
    // stream range
    double range = smax - smin;

    log(! file_output,
                "stream min = %.3f; stream max = %.3f; stream range = %.3f \n", smin,
                smax, range);

    true_quantile = quickselect(items, len, (long) (len * quantile));
    log(! file_output, "the true quantile %.2f is %.3f\n", quantile,
                true_quantile);

    std::uniform_real_distribution<> dis(0.0, 1.0);

    // set the estimated quantile to the value of the first item
    int integer_norm_quantile = (items[0]-smin)/range * prec;

    double p = exp(eps) / (exp(eps) + 1);

    clock_t begin_time = clock();
    for (long i = 1; i < len; ++i) {

        double norm_item      = (items[i] - smin) / range;
        int integer_norm_item = norm_item * prec;
        int s  = randomized_response(integer_norm_quantile, p, integer_norm_item, mtgenerator1);

        float rnd = dis(mtgenerator3);
        if (s && rnd > 1.0 - quantile)
            integer_norm_quantile += 1;
        else if (!s && rnd > quantile)
            integer_norm_quantile -= 1;
    }

    clock_t end_time = clock();
    elapsed          = (double) (end_time - begin_time) / CLOCKS_PER_SEC;

    free(items), items = NULL;
    double estimated_quantile = (double)integer_norm_quantile / prec * range + smin;
    double abs_error          = fabs(estimated_quantile - true_quantile);
    double norm_abs_error     = abs_error / range;
    double relative_error     = abs_error / true_quantile;

    log(! file_output, "Epsilon: %.2f\n", eps);
    log(! file_output, "Private estimated quantile: %.6f\n", estimated_quantile);
    log(! file_output, "Elapsed time %.6f\n", elapsed);
    log(! file_output, "Updates/s %ld\n", lround(len / elapsed));
    log(! file_output, "The relative error is: %.6f\n", relative_error);
    log(! file_output, "Absolute error: %.6f\n", abs_error);

    if (file_output) {
        fptr = fopen(filename, "w");
        if (! fptr) {
            log(1, "Error opening file %s\n", filename);
            free(filename), filename = NULL;
            exit(1);
        }

        // writing to csv file the following information:
        //<n>, <quantile>, <eps>, <distribution>, <param1>, <param2>, <seed>,
        //<estimated
        // quantile>, <true quantile>, <relative error>, <absolute error>, <input
        // range>, <stream min>, <stream max>, <elapsed time>, <updates/s>
        fprintf(fptr, "n,q,e,d,a,b,s,qv,tqv,re,ae,nae,rg,min,max,time,upd\n");
        fprintf(fptr,
                    "%ld,%.2f,%.3f,%s,%.6f,%.6f,%ld,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,"
                    "%.6f,%.6f,%ld\n",
                    len, quantile, eps, diststr, param1, param2, seed, estimated_quantile,
                    true_quantile, relative_error, abs_error, norm_abs_error, range,
                    smin, smax, elapsed, lround(len / elapsed));

        fclose(fptr);
        free(filename), filename = NULL;
    }

    free(diststr), diststr = NULL;
    return 0;
}
