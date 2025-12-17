#include "QuickSelect.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <limits>
#include <random>
#include <stdarg.h>

void log(char active, const char *fmt, ...)
{
    va_list args;
    if (active) {
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
}

double square_wave_randomizer(double q, double l, double v,
            std::mt19937 &gen1)
{
    std::uniform_real_distribution<double> unif(0, 1.0);
    double u = unif(gen1);

    double v_tilde = 0.0;
    if (u >= 0 && u < v * q) {
        v_tilde = u / q - l;
    } else if (u >= v * q && v < (1 + (v - 1) * q)) {
        v_tilde = 2 * l * (u - v * q) / (1 - q) + v - l;
    } else if (u >= (1 + (v - 1) * q) && u <= 1) {
        v_tilde = (u - 1 - (v - 1) * q) / q + v + l;
    }

    return v_tilde;
}

void usage(void)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "-n <number of items to be generated> default: 1000000\n");
    fprintf(stderr, "-q <quantile (0<q<1)> default: 0.99\n");
    fprintf(stderr, "-e <privacy budget for Laplace mechanism (0 < epsilon < "
                    "20)> default: 2.0\n");
    fprintf(stderr, "-g <privacy budget for local randomizer (0 < gamma < 20)> "
                    "default: 2.0\n");
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

int f(int x)
{
    return 1;
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
    FILE *fptr     = NULL;
    char *filename = NULL;
    double true_quantile;
    double eps  = 2.0;
    double prec = 1000000.0;

    double estimated_quantile = 0.0;
    double elapsed            = 0.0;
    bool file_output          = false;
    bool param1_default       = true;
    bool param2_default       = true;
    double l                  = 0.0;
    double q                  = 0.0;

    int opt;

    while ((opt = getopt(argc, argv, ":n:q:e:d:a:b:s:t:f:h:g:l:p:")) != -1) {
        switch (opt) {
            case 'n':
                len = strtol(optarg, NULL, 10);
                break;
            case 'q':
                quantile = strtod(optarg, NULL);
                break;
            case 'e':
                eps = strtod(optarg, NULL);
                break;
            case 'd':
                dist = strtol(optarg, NULL, 10);
                break;
            case 'p':
                prec = strtod(optarg, NULL);
                break;
            case 'l':
                l = strtod(optarg, NULL);
                break;
            case 'a':
                param1         = strtod(optarg, NULL);
                param1_default = false;
                break;
            case 'b':
                param2         = strtod(optarg, NULL);
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

    // set default parameter values depending on the distribution
    switch (dist) {

        case 1:      // normal (0,1)
            if (param1_default)
                param1 = 0.0;
            if (param2_default)
                param2 = 1.0;
            break;
        case 2:      // cauchy (0,1)
            if (param1_default)
                param1 = 0.0;
            if (param2_default)
                param2 = 1.0;
            break;
        case 3:      // uniform (-1,1)
            if (param1_default)
                param1 = -1.0;
            if (param2_default)
                param2 = 1.0;
            break;
        case 4:      // exponential (0.5)
            if (param1_default)
                param1 = 0.5;
            if (param2_default)
                param2 = -1.0;      // not used
            break;
        case 5:      // chisquared (5)
            if (param1_default)
                param1 = 5.0;
            if (param2_default)
                param2 = -1.0;      // not used
            break;
        case 6:      // gamma (3,2)
            if (param1_default)
                param1 = 3.0;
            if (param2_default)
                param2 = 2.0;
            break;
        case 7:      // lognormal (0,0.5)
            if (param1_default)
                param1 = 0.0;
            if (param2_default)
                param2 = 0.5;
            break;
        case 8:      // extremevalue (1.5,3)
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

    if (! l) {
        l = (eps * exp(eps) - exp(eps) + 1) /
            (2.0 * exp(eps) * (exp(eps) - 1 - eps));
    }
    q = 1 / (2 * l * exp(eps) + 1);
    log(! file_output, "Local Diffential Privacy: Frugal 2U algorithm with "
                       "Square Wave mechanism\n");
    log(! file_output, "Privacy budget epsilon = %.3f\n", eps);
    log(! file_output, "q probability = %.5f\n", q);
    log(! file_output, "l value = %.5f\n", l);

    std::srand(seed);      // use current time as seed for random generator

    long seed1 = std::rand();
    long seed2 = std::rand();
    long seed3 = std::rand();
    long seed4 = std::rand();

    log(! file_output, "Seeds generated: %ld, %ld, %ld, %ld\n", seed1, seed2,
                seed3, seed4);
    std::mt19937 mtgenerator(seed1);
    std::mt19937 mtgenerator1(seed2);
    std::mt19937 mtgenerator2(seed3);

    std::normal_distribution<double> normaldistribution(param1, param2);
    std::cauchy_distribution<double> cauchydistribution(param1, param2);
    std::uniform_real_distribution<double> uniformrealdistribution(param1,
                param2);
    std::exponential_distribution<double> exponentialdistribution(param1);
    std::chi_squared_distribution<double> chisquareddistribution(param1);
    std::gamma_distribution<double> gammadistribution(param1, param2);
    std::lognormal_distribution<double> lognormaldistribution(param1, param2);
    std::extreme_value_distribution<double> extremevaluedistribution(param1,
                param2);

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
            param1 = 0.0;
            param2 = 1.0;
            break;
    }

    log(! file_output, "generated random %ld items\n", len);
    log(! file_output,
                "using the %s distribution with parameters %f and %f "
                "and seed %ld\n",
                diststr, param1, param2, seed1);

    double smax = std::numeric_limits<double>::min();
    double smin = std::numeric_limits<double>::max();
    for (int i = 0; i < len; i++) {
        if (items[i] < smin)
            smin = items[i];
        if (items[i] > smax)
            smax = items[i];
    }

    double range = smax - smin;

    log(! file_output,
                "stream min = %.3f; stream max = %.3f; stream range = %.3f; seed = %ld\n",
                smin, smax, range, seed2);
    true_quantile = quickselect(items, len, (long) (len * quantile));
    log(! file_output, "the true quantile %.2f is %.3f\n", quantile,
                true_quantile);

    // std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis(0.0, 1.0);

    // set the estimated quantile to the value of the first item
    int sign           = 1;
    int stepsize       = 1;
    int integer_norm_quantile = (items[0] - smin) / range * prec;

    clock_t begin_time = clock();
    int min            = std::numeric_limits<int>::max();
    int max            = std::numeric_limits<int>::min();

    for (long i = 1; i < len; ++i) {

        float rnd = dis(mtgenerator2);

        // 1. normalize item, 2. randomize, 3. make randomized item an integer
        double norm_item     = (items[i] - smin) / range;
        double number        = square_wave_randomizer(q, l, norm_item, mtgenerator1);
        int integer_norm_item  = number * prec;
        min                  = (integer_norm_item < min) ? integer_norm_item : min;
        max                  = (integer_norm_item > max) ? integer_norm_item : max;

        if (integer_norm_item > integer_norm_quantile && rnd > 1.0 - quantile) {
            stepsize += (sign > 0) ? f(stepsize) : -f(stepsize);
            integer_norm_quantile += (stepsize > 0) ? stepsize : 1;
            sign = 1;

            if (integer_norm_quantile > integer_norm_item) {
                stepsize += integer_norm_item - integer_norm_quantile;
                integer_norm_quantile = integer_norm_item;
            }
        } else {
            if (integer_norm_item < integer_norm_quantile && rnd > quantile) {

                stepsize += (sign < 0) ? f(stepsize) : -f(stepsize);
                integer_norm_quantile -= (stepsize > 0) ? stepsize : 1;
                sign = -1;

                if (integer_norm_quantile < integer_norm_item) {
                    stepsize += integer_norm_quantile - integer_norm_item;
                    integer_norm_quantile = integer_norm_item;
                }
            }
        }

        if ((integer_norm_quantile - integer_norm_item) * sign < 0 && stepsize > 1) {
            stepsize = 1;
        }
    }

    clock_t end_time = clock();
    elapsed          = (double) (end_time - begin_time) / CLOCKS_PER_SEC;

    free(items), items = NULL;

    estimated_quantile = (double)integer_norm_quantile / prec * range + smin;

    double relative_error =
                fabs(estimated_quantile - true_quantile) / fabs(true_quantile);
    double abs_error      = fabs(estimated_quantile - true_quantile);
    double norm_abs_error = abs_error / range;

    log(! file_output, "Perturbed stream min = %d; perturbed stream max %d\n", min,
                max);
    log(! file_output, "estimated quantile: %.3f\n", estimated_quantile);
    log(! file_output, "elapsed time %f\n", elapsed);
    log(! file_output, "updates/s %ld\n", lround(len / elapsed));
    log(! file_output, "the relative error is: %f\n", relative_error);
    log(! file_output, "the absolute error is: %f\n", abs_error);

    if (file_output) {

        fptr = fopen(filename, "w");

        if (! fptr) {
            log(! file_output, "Error opening file %s\n", filename);
            free(filename), filename = NULL;
            exit(1);
        }

        // writing to csv file the following information:

        //<n>, <quantile>, <epsilon>,<distribution>,
        //<param1>, <param2>, <seed>, <estimated quantile>, <true quantile>,
        // <relative error>, <absoute error>, <normalized absolute error>, <input
        // range>, <stream min>, <stream max>, <elapsed time>, <updates/s>
        fprintf(fptr, "n,q,e,d,a,b,s,qv,tqv,re,ae,nae,rg,min,max,time,upd\n");
        fprintf(fptr,
                    "%ld,%.2f,%.3f,%s,%.6f,%.6f,%ld,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,"
                    "%.6f,%.6f,%ld\n",
                    len, quantile, eps, diststr, param1, param2, seed,
                    estimated_quantile, true_quantile, relative_error, abs_error,
                    norm_abs_error, range, smin, smax, elapsed, lround(len / elapsed));
        fclose(fptr);

        free(filename), filename = NULL;
    }

    free(diststr), diststr = NULL;

    return 0;
}
