# -*- coding: utf-8 -*-
import importlib

import matplotlib.pyplot as plt
import erg
import numpy as np

importlib.reload(erg)

#plt.close()

wf = r'C:\Users\danpo\Documents\BYB\BYB_Recording_2019-07-01_18.53.08.wav'# old: r'C:\Users\danpo\Documents\BYB\BYB_Recording_2019-06-17_18.12.54.wav'
e = erg.ERG(wf)

messages = e.get_signals()

#%% 
plt.figure()

plt.gca().set_prop_cycle(plt.cycler('color',
       plt.cm.prism(np.linspace(0, 1, len(messages["signal_ts"])))))

for i in range(len(messages["timestamps"])):
    plt.plot(messages["timestamps"][i], np.zeros((len(messages["timestamps"][i]),1))+.25, '*')                    


#%%
plt.figure()
plt.plot(e.wavedata)
#for i in range(len(messages["signals"])):
#    plt.plot(np.array(messages["signals"][i]) * e.samplerate,
#             np.zeros((len(messages["signals"][i]),1))+.25, '*')

for i in range(len(messages["timestamps"])):
    plt.plot(np.array(messages["timestamps"][i]) * e.samplerate,
             np.zeros((len(messages["timestamps"][i]),1))+1000, '*')

# initialize the something dict
freqs = e.frequencies()
freq_times = {freq:[] for freq in freqs} # dict comprehension!

# fill it in
sigs = messages["signal_ts"]
onoff = messages["timestamps"]
curr_freq = None
sigs_i = 0
onoff_i = 0
 
for i in range(len(messages["timestamps"])):
    freq_ind = len(messages["signal_ts"][2*i]) - 1 # just because it's one too high
    color_ind = len(messages["signal_ts"][2*i+1])
    
    freq = freqs[freq_ind]
    
    if len(freq_times[freq])==0:
        freq_times[freq] = messages["timestamps"][i]
    else:
        freq_times[freq].append(messages["timestamps"][i])
    
    