# -*- coding: utf-8 -*-
"""
Cribbed from Joe Donovan's EEG class by DJP
"""

from datetime import datetime
from math import floor
import numpy as np
import os
import pandas as pd
from scipy.io import wavfile
from scipy.signal import butter, filtfilt


class ERG:
    """
    A class for loading erg data saved as a wavefile with some metadata on the side
    Resamples data to a resamplerate
    """
    # https://docs.python.org/2/tutorial/modules.html
    def __init__(self, wav_file, text_file=None, resamplerate=None, subtract_baseline=True):
        self.new_format_dt = datetime(2019, 7, 1, 18, 10, 51)
        
        # Read in wave data
        samplerate, data = wavfile.read(wav_file)
        
        # if not resampling
        if resamplerate == None:
            resamplerate = samplerate
            
        # Extract date
        yearstr = os.path.splitext(os.path.split(wav_file)[1])[0]
        dt = datetime.strptime(yearstr[len('BYB_Recording_'):len('BYB_Recording_') + 19], '%Y-%m-%d_%H.%M.%S')  # Reference -  http://strftime.org/
        self.datetime = dt
       
        wavedata = data.copy()
        
        if subtract_baseline:
            wavedata -= np.median(wavedata).astype(wavedata.dtype)

        # resample to a fixed sample rate
        # have wavedata at samplerate, and want it at resamplerate
        sample_times = np.arange(len(wavedata)) / samplerate
        # new_sample_times = np.linspace(0, sample_times.max(), sample_times.max() * resamplerate)
        # wavedata_resamp = np.interp(new_sample_times, sample_times, wavedata)

        # self.sample_times = new_sample_times
        self.wavedata = wavedata#_resamp
        self.samplerate = samplerate #resamplerate
        
        sr = self.samplerate
        b,a=butter(1,[1 / (.5*sr),20/(.5*sr)],btype='band')
        self.wavedata = filtfilt(b,a,self.wavedata)
        
        
        # Extract dataframe of stim id's
        if text_file==None:
            text_file = wav_file[:-4] + '-events.txt'
         
        with open (text_file, "r") as tf:
            rawtextdata=tf.readlines()
            
        # remove first two lines and then remove commas, then the last character (a newline)
        textdata = [e.replace(',','')[:-1] for e in rawtextdata[2:]]
        # make a list of lists to put into dataframe
        textdata = [e.split('\t') for e in textdata]
        
        # cast as float
        self.markers = pd.DataFrame(
            textdata, columns=['markers','timestamps'],dtype='float')
        
        # self.messages = self.get_signals()
    
    
    def frequencies(self):
        if self.datetime < self.new_format_dt:
            return [1, 8, 16, 32, 64, 125, 150, 175, 190, 200, 210]
        else:
            return [1, 2, 4, 8, 16, 32, 64, 128, 256, 512]
    
    def to_window(self, stimulations):
        '''
        Parameters
        ----------
        stimulations : list
                       a list of stim times in seconds. Convert to numpy array,
                       shift down to zero start, and convert to samples
        '''
        return (np.array(stimulations) - stimulations[0]) * self.samplerate
    
    def get_frequency(self, key):
        freq_index = key[0] - 2 
        
        if freq_index < 0:
            raise Exception("freq_index is below 0")
            
        freqs = self.frequencies()
        return freqs[freq_index]
    
    def get_color(self, key):
        color_index = key[1]
        
        
        if color_index == 7:
            return 'infrared'
        elif color_index == 8:
            return 'ultraviolet'
        elif color_index == 9:
            return 'red'
        elif color_index == 10:
            return 'green'
        elif color_index == 11:
            return 'blue'
        elif color_index not in [9, 10, 11, 12, 13]:
            raise Exception("invalid color_index {}".format(color_index))
            
    
    @property
    def duration(self):
        """Duration in seconds"""
        return self.sample_times.max() - self.sample_times.min()

    
    '''
    -----------------------------------
    Signals
    -----------------------------------
    '''
    def clean_signals(self, dictionary):
        '''uses clever inference techniques to figure out which dictionary entries
        are mismatched
          Infer the proper frequency and color based on the following
              if same number of entries (plus minus one):
                  then it's the same frequency but diff color,
              and if round(math.log2(len(key[0])) doesn't match key[0]:
                  then it's the same color but different frequency.
          It relies on a "parity bit" that surely, SURELY it wouldn't fuck both.
          If it does, the code gets longer :(
        '''
        all_single = lambda d : np.array([len(entry)==0 for _, entry in d.items()]).all()
        while(not all_single(dictionary)):
            pass
            
    
    def get_signals(self, tol=0.01, tally_dif=0.2, mess_dif=1,
                    ontime=5.4):

        assert (tally_dif + tol < mess_dif and tally_dif + tol < ontime), "tol too high"

        if self.datetime <= self.new_format_dt:
            # accepts a dataframe from erg.ERG having read the text file as a CSV
            return self.get_old_signals()
        else:
            df = self.markers
            
            # note that the [2:] is to ignore the first two, which are noise
            a1 = df.index.values
            for elem in np.unique(a1):
                try:
                    # try to cast as float, if it doesn't like that, convert all elems of that type to 3
                    float(elem)
                except ValueError:
                    a1[a1==elem] = 3

            a1 = a1.astype(float)
            a2 = df['# Marker IDs can be arbitrary strings.'].values.astype(float)
                    
            CFF = {} # {(freq, color):[[timestamps],[timestamps]]}
            nonCFF = {} # {(assay, color):[[timestamps],[timestamps]]}
            
            is_CFF = False # to know if current trial is CFF
            key = None # current key for plugging in
            holder = []

            split = lambda arr: (         np.argmax(np.diff(arr)) + 1,
                                 len(arr)-np.argmax(np.diff(arr)) - 1) if len(arr) > 1 else 1
            
            for i, (marker, ts) in enumerate(zip(a1, a2)):
                
                prev_marker = a1[i-1] if i is not 0 else marker # so it equals
                new_marker = marker != prev_marker
                
                if not new_marker:
                    holder.append(ts)
                else: # if is new marker,
                    if prev_marker == 2 or prev_marker == 4:

                        key = split(holder) # returns a tuple
                        
                        # then wait to plug sth into the key
                        is_CFF = True if prev_marker == 2 else False
                        
                    elif prev_marker == 3:
                        if key == None:
                            raise Exception("key is None")
                         
                        if is_CFF:
                            if not key in CFF:
                                CFF[key]=[]
                            CFF[key].append(holder)
                        else:
                            if not key in nonCFF:
                                nonCFF[key]=[]
                            nonCFF[key].append(holder)
                        # clean up
                        key = None 

                    else:
                        raise Exception("Unknown marker {}".format(prev_marker))                    
                    
                    holder = [ts]
                    
#            self.clean_signals(CFF)       
            return CFF

    def get_old_signals(self):
        # id of markers doesn't matter for old version
        old_markers = self.markers["# Marker IDs can be arbitrary strings."].values[1:].astype(float)
        not_end = lambda i: i < len(self.markers)-1
        next_dif = lambda i: self.old_markers[i+1]-curr
        
        tally = 0
        t_l = []
        signal_ts = []
        signals = []
        
        onoff = [] # list of trials
        trial= [] # goes into onoff
        
        curr = 0
        for i in range(len(old_markers)):
            prev = curr
            curr = old_markers[i]
            dif = curr-prev
            
            ## tally handling
            if self.is_same_interval(dif, tally_dif, tol=0.015):
                
                if tally==0:
                    t_l.append(prev)
                
                tally = tally + 1
                t_l.append(curr)
            
            # it was skipping single elements at the beginning of the message, counting as onoff
            elif (len(onoff) is not 0
                  and onoff[-1][-1] == old_markers[i-1]
                  and self.is_same_interval(next_dif(i), mess_dif)):
                tally = tally+1
                signals.append(tally)
                tally = 0
                signal_ts.append([curr])
            # DELETE THIS TO REVERT TO OLD BEHAVIOR
            # it was skipping single elements at the end of the message?
            elif (self.is_same_interval(dif, mess_dif,tol=0.015) and 
                  self.is_same_interval(next_dif(i), 0.5, tol=0.050)):
                signals.append(tally)
                tally = 0
                
                signal_ts.append(t_l)
                t_l=[]
                
                tally = tally+1
                t_l.append(curr)    
            
#                    
            elif tally > 0: 
                # If there was a tally going up and now you're done, do this
                # With dif, you lose one tally;
                # same reason as for next block's nested ifs.
                signals.append(tally + 1)
                tally = 0 # reset tally
                
                signal_ts.append(t_l)
                t_l=[]
                
            # for a weird situation where theres a single tally at the beginning of the signal
            # NOTE: for my data from 6-17, this will skip the first led on time , so go back and grab that 
            # Otherwise just act normally
            elif not_end(i) and self.is_same_interval(mess_dif, next_dif(i)) and signal==[]:
                signals.append(1)
                signal_ts.append([prev])
                
            
            ## onoff signal handling for CFF trials
            # last condition to make sure you don't get the intro beeps
            elif (self.is_light(dif) and not_end(i) 
                  and self.is_same_interval(dif, next_dif(i)) 
                  and not len(signals)==0):
                
                if len(trial) == 0:
                    trial.append(prev)
                trial.append(curr)
                
            # if it is light and is not the same interval
            # It's not the same interval twice: before and after the interval
            # Therefore, check that dif is smaller, meaning (hopefully) that it
            # is before the interval
            elif (self.is_light(dif) and i < len(old_markers)-1 
                  and not self.is_same_interval(dif, old_markers[i+1]-curr) 
                  and dif < old_markers[i+1]-curr 
                  and not len(signals)==0):
                trial.append(curr)
                onoff.append(trial)
                trial=[]

            # If at end
            elif self.is_light(dif) and not not_end(i):
                trial.append(curr)
                onoff.append(trial) 
                trial = []
                
            else: 
                pass # do nothing if you get noise, which there is
        
        # b/c it doesn't come to this line before it goes through the entire set, so I have it again.
#        return (signals, onoff, signal_ts)
        return {"signal_ts":signal_ts, "timestamps":onoff}
    
    def is_light(self, dif, frequencies=[1, 8, 16, 32, 64, 125, 150, 175, 190,
                                         200, 210], tol_scale=0.999):
        '''
        This function is to be used with the old data, which used this wierd
        and ugly set of frequencies for CFF.
        '''
        
        ints = 1 / np.array(frequencies)
        fits = [dif > elem[0] and dif < elem[1] for elem in list(zip(ints-ints*tol_scale, ints+ints*tol_scale))]
        
        return any(fits)
    
    def is_same_interval(self, dif, curr_interval, tol=0.15):
        # to make sure you don't group different trails as the same
        return abs(dif-curr_interval) < tol
    
    def to_time(self, secs):
        print((floor(secs/60), floor(secs%60)))
    
    def get_window(self, s1, s2):
        '''
        Parameters
        ----------
        
        s1 : float
             left side of window in seconds
        s2 : float
             right side of window in seconds
        '''
        i1 = int(round(s1,3) * self.samplerate)
        i2 = int(round(s2,3) * self.samplerate)
        output = self.wavedata[i1:i2]
        return np.reshape(output, (np.shape(output)[0],1))
    
    def get_window_precise(self, i1, winsize):
        '''
        Parameters
        ----------
        
        i1      : int
                  left side of window in samples
        winsize : float
                  length of window in samples
        '''
        output = self.wavedata[int(i1):int(i1+winsize)]
        return np.reshape(output, (np.shape(output)[0],1))

if __name__ == '__main__':
    # ergram = ERG(r'C:\Users\danpo\Documents\BYB\BYB_Recording_2020-03-22_18.00.44.wav')
    # ergram.vizCFF()
    
    ergram_old = ERG(r'C:\Users\danpo\Documents\BYB\BYB_Recording_2020-03-22_17.52.20.wav')
    # ergram.vizCFF()
    