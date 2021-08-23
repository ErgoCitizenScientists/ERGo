# -*- coding: utf-8 -*-
"""
Created on Sun Dec 15 16:04:10 2019

@author: danpo

This script compares the CFF assay's waveform alignment when I use two strategies
for finding on/off windows: with markers for on, and without markers for on.
"""
import importlib

import erg
import erg.plotting as plotting

importlib.reload(erg)
importlib.reload(erg.plotting)

import numpy as np
import matplotlib.pyplot as plt
wf_w_markers = r'C:\Users\danpo\Documents\BYB\BYB_Recording_2019-12-09_23.50.47.wav'
text_w_markers = r'C:\Users\danpo\Documents\BYB\BYB_Recording_2019-12-09_23.50.47-events.txt'

ergram = erg.ERG(wf_w_markers)
p = plotting.Plotting(ergram)

