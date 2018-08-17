# updateSamples.py
# Author: Caleb Smith
# Date: 16-Aug-2018

# update samples config files
# - input samples config file with paths already updated
# - input file that is output of nEvts script with updated positive and negative weights
# - output new samples config file with updated weights
import argparse
import re

# regex
# regular expression to get sample name and positive/negative weights
#
# Processing file(s): SMS-T2tt_fastsim_mStop-150to250 /eos/uscms/store/user/lpcsusyhad/Stop_production/Summer16_80X_Jan_2017_Ntp_v12X/SMS-T2tt_mStop-150to250_TuneCUETP8M1_13TeV-madgraphMLM-pythia8.txt  Neg weigths = 0, Pos weights = 32283695
# (.*): (.*)\t/eos.*Neg weigths = (.*), Pos weights = (.*)
# Groups:
#  1: 'Processing file(s)'                  - beginning
#  2: 'SMS-T2tt_fastsim_mStop-150to250'     - dataset name   
#  3: '0'                                   - number of negative weights
#  4: '32283695'                            - number of positive weights

regex = re.compile('(.*): (.*)\t/eos.*Neg weigths = (.*), Pos weights = (.*)')

# from samples.cc
#                                                       dataset, path,   file,   tree,   cross section, nevents+, nevents-, kfactor 
# int nMatches = sscanf(buf, "%s %s %s %s %f %f %f %f", cDSname, cFPath, cfName, cTPath, &f1,           &f2,      &f3,      &f4);
#
# string,  string, string, string, float,         float,    float,    float
# dataset, path,   file,   tree,   cross section, nevents+, nevents-, kfactor 
# cDSname, cFPath, cfName, cTPath, &f1,           &f2,      &f3,      &f4
#

def getNewSample(sample, nevents_file):
    # be careful: strip() removes spaces and endlines
    sample_list = list(x.strip() for x in sample.split(','))
    name = sample_list[0] 
    nevents = open(nevents_file, 'r')
    num_matches = 0
    for nevent in nevents:
        match = regex.match(nevent)
        if not match:
            continue
        if name == match.group(2):
            num_matches += 1
            neg_weights = int(match.group(3))
            pos_weights = int(match.group(4))
            print "original: ({0}, {1}) new: ({2}, {3})".format(sample_list[-3], sample_list[-2], pos_weights, neg_weights)
            if sample_list[-2] == neg_weights and sample_list[-3] == pos_weights:
                print "weights have not changed"
            else:
                print "weights have changed"
            sample_list[-2] = str(neg_weights)
            sample_list[-3] = str(pos_weights)
    print "{0} has {1} match(es) in nevents file".format(name, num_matches)
    if num_matches > 1:
        print "WARNING: more than one match found; {0} has {1} matches".format(name, num_matches)
    nevents.close()
    # replace commas and a space removed by split()
    # replace endline that was removed by strip()
    newSample = ", ".join(sample_list) + "\n"
    return newSample

def main():
    print "producing new samples config files"

    # options
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--samples_file", "-s", default="sampleSets.cfg", help="file containing sample sets")
    parser.add_argument("--nevents_file", "-e", default="nEvents.txt",    help="file containing number of events with weights")
    options = parser.parse_args()
    samples_file = options.samples_file
    nevents_file = options.nevents_file

    print "samples file: {0}".format(samples_file)
    print "nevents file: {0}".format(nevents_file)

    samples = open(samples_file, 'r')
    newSamples = open("sampleCfg_v3/sampleSets.cfg", 'w')

    # for each sample, find the correct weights
    for sample in samples:
        newSample = sample
        # be careful: strip() removes spaces and endlines
        sample = sample.strip()
        #print "sample: {0}".format(sample)
        if not sample:
            newSamples.write(newSample)
            continue
        if sample[0] == "#":
            newSamples.write(newSample)
            continue
        ###   # be careful: strip() removes spaces and endlines
        ###   sample_list = list(x.strip() for x in sample.split(','))
        ###   getNewSample(sample_list[0], nevents_file)
        ###   #print sample_list
        ###   # replace commas removed by split()
        ###   # replace endline that was removed by strip()
        ###   newSample = ", ".join(sample_list) + "\n"
        newSample = getNewSample(sample, nevents_file)
        newSamples.write(newSample)

    samples.close()
    newSamples.close()


if __name__ == "__main__":
    main()

