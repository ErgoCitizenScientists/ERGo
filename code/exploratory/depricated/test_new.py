# -*- coding: utf-8 -*-
import importlib

import erg
import erg.plotting as plotting

importlib.reload(erg)
importlib.reload(erg.plotting)
''' good source for colormaps
https://matplotlib.org/3.1.0/tutorials/colors/colormaps.html
'''
# new: r'C:\Users\danpo\Documents\BYB\BYB_Recording_2019-07-01_18.53.08.wav'
# old: r'C:\Users\danpo\Documents\BYB\BYB_Recording_2019-06-17_18.12.54.wav'
# fake data:
# r'C:\Users\danpo\Documents\BYB\fake_ergs\BYB_Recording_2019-07-01_18.28.58.wav'
# real flawed data to do funky stuff with the dict with data:
# C:\Users\danpo\Documents\BYB\BYB_Recording_2019-07-08_12.06.15.wav
# C:\Users\danpo\Documents\BYB\BYB_Recording_2019-07-08_11.56.51.wav
wf = r'C:\Users\danpo\Documents\BYB\BYB_Recording_2019-07-08_12.06.15.wav'#r'C:\Users\danpo\Documents\BYB\BYB_Recording_2019-07-08_12.06.15.wav' # r'C:\Users\danpo\Desktop\good ergs\BYB_Recording_2019-06-17_18.12.54.wav' # 
ergram = erg.ERG(wf)
p = plotting.Plotting(ergram)
#p.quick(whole_trial=True)
#p.bode(jitter=True)
p.get_2Hz()

'''TODO List
'''

# TODO: Why does my arduino code like to crap out on me and send the same frequency twice?
#  look into inferring missing markers
# TODO: repopulate missed message entries by inferring the proper frequency and
#  color based on the fact that if they have the same number of entries (plus minus one)
#  then it's the same freq diff color, and if round(math.log2(len(key[0])) doesn't match
#  key[0], then it's the same color different frequency. It relies on a "parity bit"
#  that surely, SURELY it wouldn't fuck up two consecutive messages. If it does, 
#  the code gets longer :(
#  https://stackoverflow.com/questions/22932904/in-python-with-matplotlib-how-to-check-if-a-subplot-is-empty-in-the-figure
#  Now it looks like Stan is going to be able to fix the spikerecorder
#
# soon:
# TODO: make the line graph a seaborn line plot
# TODO: possibly test with inferred frequencies instead of taking the markers
# TODO: For color adaptation assay, you actually need to vary the off time, not the on time.
#  It's because the time constant, which is usually around 500 to
#  1000 ms, will only activate adaptation mechanisms in the retina with feedback
#  when it's fast enough for the system to not go past the time constant. So
#  consider ways to vary dark time  (duty cycle variation?)
#  See the models Aljoscha sent to ERGo

# TODO: Contrast response function for color decay assay
#
# Long term:
# TODO: in paper, make a little inset thing that shows a sine wave and then P(1 Hz)
#  to indicate this abstract idea of power at freq. Think about how to talk about fourier transform.
#  Also, maybe just stick with a simpler measure, like the convolution
# TODO: Look into a way of showing (mathematically?) how the Light evoked
#  response potential starts to approximate the 1 Hz as you approach fusion.
