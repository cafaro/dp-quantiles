#!/usr/bin/env python3

import subprocess as sbc
import sys
import time
import argparse
import math
import os
from testfrugal1_params import *
#import pandas as pd
import glob

parser = argparse.ArgumentParser()

parser.add_argument("cmd", help="executable name")

options = parser.parse_args()

exec_name = "./" + options.cmd


def print_to_stderr(msg):
    sys.stderr.write(msg)
    sys.stderr.flush()
    return

def test_on_quantiles(outdir):
    test_count = 1
    for q in q_values:
        outputdir = outdir + "/test_stream_quantiles_" + str(test_count)
        os.makedirs(outputdir, exist_ok=True)
        print_to_stderr('Test quantile = ' + q + '\n')

        seed_count = 1
        proc = []
        for seed in range(seed_base, seed_base + (step_num * seed_step), seed_step):
            outputfile = outputdir + "/test_stream_quantile_" + str(test_count) + "_" + str(seed_count) + ".csv"
            proc.append(sbc.run([exec_name, "-n", ni_default, "-q" , q, "-d", d_default, "-e", eps_default, "-p", delta_default, "-r", rho_default, "-f", outputfile, "-s", str(seed)]))
            seed_count = seed_count + 1
            print_to_stderr("#")

        print_to_stderr("\n")

        test_count = test_count + 1

    return


def test_on_stream_lengths(outdir):
    test_count = 1
    for n in ni_values:
        outputdir = outdir + "/test_stream_length_" + str(test_count)
        os.makedirs(outputdir, exist_ok=True)

        print_to_stderr('Test stream length = ' + n + '\n')

        seed_count = 1
        proc = []
        for seed in range(seed_base, seed_base + (step_num * seed_step), seed_step):
            outputfile = outputdir + "/test_stream_length_" + str(test_count) + "_" + str(seed_count) + ".csv"
            proc.append(sbc.run([exec_name, "-n", n, "-q" , q_default, "-d", d_default, "-e", eps_default, "-p", delta_default, "-r", rho_default, "-f", outputfile, "-s", str(seed)]))
            seed_count = seed_count + 1
            print_to_stderr("#")

        # done_list = [False for i in range(seed_count-1)]
        # while not all(done_list):
        #     for i in range(seed_count-1):
        #         if proc[i].poll() is not None:
        #             done_list[i] = True

        print_to_stderr("\n")

        test_count = test_count + 1

    return


def test_on_distributions(outdir):
    test_count = 1
    for d in d_values:
        outputdir = outdir + "/test_stream_distribution_" + str(test_count)
        os.makedirs(outputdir, exist_ok=True)

        print_to_stderr('Test stream distribution = ' + d + '\n')

        seed_count = 1
        proc = []
        for seed in range(seed_base, seed_base + (step_num * seed_step), seed_step):
            outputfile = outputdir + "/test_stream_distribution_" + str(test_count) + "_" + str(seed_count) + ".csv"
            proc.append(sbc.run([exec_name, "-n", ni_default, "-q" , q_default, "-d", d, "-e", eps_default, "-p", delta_default, "-r", rho_default, "-f", outputfile, "-s", str(seed)]))
            seed_count = seed_count + 1
            print_to_stderr("#")

        # done_list = [False for i in range(seed_count-1)]
        # while not all(done_list):
        #     for i in range(seed_count-1):
        #         if proc[i].poll() is not None:
        #             done_list[i] = True

        print_to_stderr("\n")

        test_count = test_count + 1

    return


def test_on_epsilon(outdir):
    test_count = 1
    for eps in eps_values:
        outputdir = outdir + "/test_stream_epsilon" + str(test_count)
        os.makedirs(outputdir, exist_ok=True)
        print_to_stderr('Test epsilon = ' + eps + '\n')

        seed_count = 1
        proc = []
        for seed in range(seed_base, seed_base + (step_num * seed_step), seed_step):
            outputfile = outputdir + "/test_stream_epsilon_" + str(test_count) + "_" + str(seed_count) + ".csv"
            proc.append(sbc.run([exec_name, "-n", ni_default, "-q" , q_default, "-d", d_default, "-e", eps, "-p", delta_default, "-r", rho_default, "-f", outputfile, "-s", str(seed)]))
            seed_count = seed_count + 1
            print_to_stderr("#")

        print_to_stderr("\n")

        test_count = test_count + 1

    return


def test_on_delta(outdir):
    test_count = 1
    for delta in delta_values:
        outputdir = outdir + "/test_stream_delta" + str(test_count)
        os.makedirs(outputdir, exist_ok=True)
        print_to_stderr('Test delta = ' + delta + '\n')

        seed_count = 1
        proc = []
        for seed in range(seed_base, seed_base + (step_num * seed_step), seed_step):
            outputfile = outputdir + "/test_stream_delta_" + str(test_count) + "_" + str(seed_count) + ".csv"
            proc.append(sbc.run([exec_name, "-n", ni_default, "-q" , q_default, "-d", d_default, "-e", eps_default, "-p", delta, "-r", rho_default, "-f", outputfile, "-s", str(seed)]))
            seed_count = seed_count + 1
            print_to_stderr("#")

        print_to_stderr("\n")

        test_count = test_count + 1

    return


def test_on_rho(outdir):
    test_count = 1
    for rho in rho_values:
        outputdir = outdir + "/test_stream_rho" + str(test_count)
        os.makedirs(outputdir, exist_ok=True)
        print_to_stderr('Test rho = ' + rho + '\n')

        seed_count = 1
        proc = []
        for seed in range(seed_base, seed_base + (step_num * seed_step), seed_step):
            outputfile = outputdir + "/test_stream_rho_" + str(test_count) + "_" + str(seed_count) + ".csv"
            proc.append(sbc.run([exec_name, "-n", ni_default, "-q" , q_default, "-d", d_default, "-e", eps_default, "-p", delta_default, "-r", rho, "-f", outputfile, "-s", str(seed)]))
            seed_count = seed_count + 1
            print_to_stderr("#")

        print_to_stderr("\n")

        test_count = test_count + 1

    return

outdir = "Test_on_" + options.cmd
test_on_distributions(outdir)
test_on_quantiles(outdir)
test_on_stream_lengths(outdir)
test_on_epsilon(outdir)
test_on_delta(outdir)
test_on_rho(outdir)

print_to_stderr("\nTest completed!!\n")
