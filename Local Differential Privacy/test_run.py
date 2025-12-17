#!/usr/bin/env python3

import subprocess as sbc
import sys
import argparse
import math
import os

############################################################

q_values = ["0.01", "0.25",  "0.5", "0.75", "0.90", "0.95", "0.99"]
q_default = "0.99"

n_values = ["1000000", "5000000", "7500000", "10000000"]
n_default = "5000000"

d_values = ["1", "2", "3", "4", "5", "6", "7", "8"]
d_values_restrict = ["1","3","4","7"]
d_default = "1"

e_values = ["0.5", "1", "1.5", "2", "2.5", "3", "3.5", "4", "4.5", "5"]
e_default = ["1.0", "2.0", "3.0", "4.0", "5.0"]

s_base = 16033099
s_step = 127
reps = 100
############################################################


parser = argparse.ArgumentParser()

parser.add_argument("cmd", help="executable name")

options = parser.parse_args()

exec_name = "./" + options.cmd


def print_to_stderr(msg):
    sys.stderr.write(msg)
    sys.stderr.flush()
    return

def test_on_q(outdir):
    for q in q_values:
        outputdir = outdir + "/test_q_" + str(q)
        os.makedirs(outputdir, exist_ok=True)
        print_to_stderr('Test quantile = ' + q + '\n')

        for d in d_values_restrict:
            for e in e_default:
                rep = 1
                for seed in range(s_base, s_base + (reps * s_step), s_step):
                    outputfile = f"{outputdir}/test_q_{q}_e_{e}_d_{d}_{rep}.csv"
                    sbc.run([exec_name, "-n", n_default, "-q" , q, "-d", d, "-e", e, "-f", outputfile, "-s", str(seed)])

                    rep = rep + 1
                    print_to_stderr("#")

                print_to_stderr("\n")

    return


def test_on_n(outdir):
    for n in n_values:
        outputdir = outdir + "/test_n_" + str(n)
        os.makedirs(outputdir, exist_ok=True)

        print_to_stderr('Test stream length = ' + n + '\n')

        rep = 1
        for seed in range(s_base, s_base + (reps * s_step), s_step):
            for e in e_default:
                outputfile = f"{outputdir}/test_n_{n}_e_{e}_{rep}.csv"
                sbc.run([exec_name, "-n", n, "-q" , q_default, "-d", d_default, "-e", e, "-f", outputfile, "-s", str(seed)])
            
            rep = rep + 1
            print_to_stderr("#")

        print_to_stderr("\n")

    return


def test_on_d(outdir):
    for d in d_values:
        outputdir = outdir + "/test_d_" + str(d)
        os.makedirs(outputdir, exist_ok=True)

        print_to_stderr('Test stream distribution = ' + d + '\n')

        rep = 1
        for seed in range(s_base, s_base + (reps * s_step), s_step):
            for e in e_default:
                outputfile = f"{outputdir}/test_d_{d}_e_{e}_{rep}.csv"
                sbc.run([exec_name, "-n", n_default, "-q" , q_default, "-d", d, "-e", e, "-f", outputfile, "-s", str(seed)])
            
            rep = rep + 1
            print_to_stderr("#")

        print_to_stderr("\n")

    return


def test_on_e(outdir):
    for e in e_values:
        outputdir = outdir + "/test_e_" + str(e)
        os.makedirs(outputdir, exist_ok=True)
        print_to_stderr('Test e = ' + e + '\n')

        rep = 1
        for seed in range(s_base, s_base + (reps * s_step), s_step):
            outputfile = outputdir + "/test_e_" + str(e) + "_" + str(rep) + ".csv"
            sbc.run([exec_name, "-n", n_default, "-q" , q_default, "-d", d_default, "-e", e, "-f", outputfile, "-s", str(seed)])
            
            rep = rep + 1
            print_to_stderr("#")

        print_to_stderr("\n")

    return


outdir = "Test_of_" + options.cmd

test_on_q(outdir)
test_on_d(outdir)
test_on_n(outdir)
test_on_e(outdir)

print_to_stderr("\nTest completed!!\n")
