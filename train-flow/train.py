# -*- coding:utf-8 -*-

import sys
import os
import argparse
import logging
logging.basicConfig(level=logging.DEBUG)
from common import find_mxnet, data, fit
import mxnet as mx

def get_nir_flow_data():
    data_dir="./data/"
    fnames = (os.path.join(data_dir, "train_48x64_2w.rec"),
              os.path.join(data_dir, "test_48x64_2w.rec"))
    return fnames

if __name__ == '__main__':
    # download data
    (train_fname, val_fname) = get_nir_flow_data()

    # parse args
    parser = argparse.ArgumentParser(description="train nirFlow",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    fit.add_fit_args(parser)
    data.add_data_args(parser)
    data.add_data_aug_args(parser)
    data.set_data_aug_level(parser, 2)
    parser.set_defaults(
        # network
        network        = 'shufflenet_v2',
        #num_layers     = 110,
        # data
        data_train     = train_fname,
        data_val       = val_fname,
        num_classes    = 2,
        num_examples   = 20000,
        image_shape    = '3,64,48', # channel,height,width
        pad_size       = 0,
        # data aug
        max_random_rotate_angle = 45,
        max_random_scale = 0,
        min_random_scale = 0,
        # train
        batch_size     = 128,
        num_epochs     = 1001,
        lr             = 1e-5,
        lr_step_epochs = '800,900',
        model_prefix   = 'checkpoint_48x64_2w',
        checkpoint_period = 100, # How many epochs to wait before checkpointing. Defaults to 1.
#	    load_epoch     = 1000,
	    gpus           = '0,1,2,3'
    )

    
    args = parser.parse_args()

    # load network
    from importlib import import_module
    net = import_module('symbols.'+args.network)
    sym = net.get_symbol(**vars(args))

    # train
    fit.fit(args, sym, data.get_rec_iter)

