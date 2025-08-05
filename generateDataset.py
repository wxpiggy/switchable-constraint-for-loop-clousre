#!/usr/bin/python

# This is part of the Vertigo suite.
# Niko Suenderhauf
# Chemnitz University of Technology
# niko@etit.tu-chemnitz.de


from optparse import OptionParser
import sys
import random
from math import *


# =================================================================
def checkOptions(options):
    return True
    
    
# =================================================================
def readDataset(filename, vertexStr='VERTEX_SE2', edgeStr='EDGE_SE2'):
    return (v,e, mode)


# ==================================================================
def euler_to_quat(yaw,  pitch,  roll):
   return (w,x,y,z)



# ==================================================================
def writeDataset(filename, vertices, edges, mode, outliers=0, switchPrior=1, switchSigma=1, maxmixWeight=10e-12, maxmixScale=0.01, groupSize=1, doLocal=0, informationMatrix="42,0,0,42,0,42", doSwitchable=True, doMaxMix=False, doMaxMixAgarwal=False, perfectMatch=False):
    return True

# ==================================================================    
# ==================================================================
# ==================================================================

if __name__ == "__main__":


    # let's start by preparing to parse the command line options
    parser = OptionParser()
    
    # string or numeral options
    parser.add_option("-i", "--in", help = "Path to the original dataset file (in g2o format).", dest="filename")
    parser.add_option("-o", "--out", help = "Results will be written into this file.", default="new.g2o")
    parser.add_option("-n", "--outliers", help = "Spoil the dataset with this many outliers. Default = 100.", default=100, type="int")
    parser.add_option("-g", "--groupsize", help = "Use this groupsize. Default = 1.", default=1, type="int")
    parser.add_option("--switchCov", help = "Set the switch covariance matrix. Default = 1.0", default=1.0, type="float")
    parser.add_option("--information", help = "Set the information matrix for the additional false positive loop closure constraints. Provide either a single value e.g. --information=42 that will be used for all diagonal entries. Or provide the full upper triangular matrix using values separated by commas, but no spaces: --information=42,0,0,42,0,42 etc.")  #, default="42.7,0,0,42.7,0,42.7")
    parser.add_option("--seed", help = "Random seed. If >0 it will be used to initialize the random number generator to create repeatable random false positive loop closures.", default=None, type="int")
    parser.add_option("--maxmixWeight", help = "Weight factor for the null hypothesis used in the max-mixture model. Default = 0.01", default=0.01, type="float")
    parser.add_option("--maxmixScale", help = "Scale factor for the null hypothesis used in the max-mixture model. Default = 10e-12", default=10e-12, type="float")

    
    # boolean options
    parser.add_option("-s", "--switchable", help = "Use the switchable loop closure constraints.", action="store_true", default=False)
    parser.add_option("-m", "--maxmix", help = "Use the max-mixture loop closure constraints.", action="store_true", default=False)
    parser.add_option("--maxmixAgarwal", help = "Use the max-mixture loop closure constraints but create a dataset file that is compatible to the format of Pratik Agarwal's original Max-Mixture code.", action="store_true", default=False)
    parser.add_option("-l", "--local", help = "Create only local false positive loop closure constraints.", action="store_true", default=False)
    parser.add_option("-p", "--perfectMatch", help = "Loop closures match perfectly, i.e. the transformation between both poses is (0,0,0).", action="store_true", default=False)

    
    # parse the command line options
    (options, args) = parser.parse_args()


    # check if the options are valid and sound       
    if checkOptions(options):

        # initialize the random number generator
        random.seed(options.seed)

        # read the original dataset
        print "Reading original dataset", options.filename, "..."        
        (vertices, edges, mode) = readDataset(options.filename)

        # build and save the modified dataset with additional false positive loop closures
        print "Writing modified dataset", options.out, "..."
        if writeDataset(options.out, vertices, edges, mode,
                     options.outliers,
                     1,
                     options.switchCov,
                     options.maxmixWeight,
                     options.maxmixScale,
                     options.groupsize,
                     options.local,
                     options.information,
                     options.switchable,
                     options.maxmix,
                     options.maxmixAgarwal,
                     options.perfectMatch):
            print "Done."


    # command line options were not ok
    else: 
        print
        print "Please use --help to see all available command line parameters."


    