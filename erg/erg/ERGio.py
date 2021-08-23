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
from scipy.signal import iirnotch, filtfilt
import matplotlib.pyplot as plt


class ERG:    
    """
    A class for loading erg data saved as a wavefile with some metadata on the side
    Resamples data to a resamplerate
    """

    def __init__(
            self,
            wav_file,
            text_file=None,
            resamplerate=None,
            subtract_baseline=True
    ):
        '''
        Parameters
        ----------
        wav_file: str
            Path to wav data file.
            
        text_file: str
            Path to text file 
        
        resamplerate: int
            Target sample rate
            
        subtract_baseline: bool
            subtracts baseline from recording

        Returns
        -------
            ERG object.

        '''

        # Read in wave data
        samplerate, data = wavfile.read(wav_file)
        
        # Data is read-only
        wavedata = data.copy() 
        
        # Subtract baseline if necessary
        if subtract_baseline:
            wavedata -= np.median(
                wavedata
            ).astype(wavedata.dtype)


        # Resample to a fixed sample rate
        # Have wavedata at samplerate, and want it at resamplerate
        sample_resample_ratio = 1
        
        # Replace samplerate if necessary
        if resamplerate == None:
            resamplerate = samplerate
        
        # Set resampling ratio
        sample_resample_ratio = resamplerate/samplerate

        # Resample time    
        sample_times = np.arange(len(wavedata)) / samplerate # in s
        
        # Generate sample times
        new_sample_times = np.linspace(
            0,
            sample_times[-1] * sample_resample_ratio,
            int(len(sample_times) * sample_resample_ratio)
        )
        
        # Set sample times and sample rate
        self.sample_times = new_sample_times
        self.sr = resamplerate
        
        # resample wavedata (interpolate)        
        wavedata_resamp = np.zeros((len(new_sample_times),3))
        
        # For each channel,
        for channel in range(3):
            
            # Interpolate if necessary
            if sample_resample_ratio != 1:
                wavedata_resamp[:,channel] = np.interp(
                    new_sample_times, 
                    sample_times,
                    wavedata[:,channel]
                )
        
        # If resampling, set wavedata to resampled data
        if sample_resample_ratio != 1:
            wavedata = wavedata_resamp
        
        # Remove wavedata variable from workspace
        del wavedata_resamp
        
        #%% Filter
        # Remove 60 Hz data from first two channels. 
        # Create notch filter data (60 or 50 Hz)
        f0 = 60 # Hz
        Q = 30.0 # dimensionless
        fs = resamplerate
        
        # Create notch filter
        b_notch, a_notch = iirnotch(f0, Q, fs)
        
        # Filter first two channels are analogs (?)
        wavedata[:,0] = filtfilt(b_notch, a_notch, wavedata[:,0])
        wavedata[:,1] = filtfilt(b_notch, a_notch, wavedata[:,1])
        
        # Third channel is TTL, must be unfiltered.
        
        #%% Data cleaning
        
        # There's this weird glitch where it goes really low. Conveniently, we 
        # can fix that by setting those values to zero.
        wavedata[:,2][wavedata[:,2] < 0] = 0

        #%% Misc metadata
                
        # Extract date        
        # Reference -  http://strftime.org/        
        yearstr = os.path.splitext(os.path.split(wav_file)[1])[0]
        
        # Get datetime
        self.datetime = datetime.strptime(
            yearstr[len('BYB_Recording_'):len('BYB_Recording_') + 19],
            '%Y-%m-%d_%H.%M.%S'
        )  
        
        # Get text file
        if text_file==None:
            text_file = wav_file[:-4] + '-events.txt'
        
        # Open text file
        with open (text_file, "r") as tf:
            rawtextdata=tf.readlines()
            
        # Remove first two lines, then remove commas and the last character (\n)
        textdata = [e.replace(',','')[:-1] for e in rawtextdata[2:]]
        
        # make a list of lists to put into dataframe
        textdata = [e.split('\t') for e in textdata]
        
        # Make text file into a dataframe
        self.stim_df = pd.DataFrame(
            textdata,
            columns=['markers','timestamps'],
            dtype='float' # cast as float
        )
        #%%
        # Add wavedata
        self.signal = wavedata 
        
        # Dict to eventually be put into a DataFrame
        df_dict = {
            'trial':[], # This is just the index
            'color':[],
            'frequency (Hz)':[],
            'channel 1':[],
            'channel 2':[],
            'TTL':[],
            'time (s)':[], # take from self.sample_times, which is in seconds
        }
        
        # For indexing colors from markers
        color_dict = {
            0:'R',
            2:'G',
            4:'B',
            6:'IR',
            8:'UV'
        }
        
        # List of all possible frequencies
        freq_list = [1, 2, 4, 8, 16, 32, 64, 128, 254, 512]
        
        # For each stimulation timestamp
        for stim_ind in range(len(self.stim_df)):
            
            # Get current row of data
            curr_row = self.stim_df.iloc[stim_ind,:]
            
            # Color
            marker = int(curr_row.markers) 
            color = color_dict[marker]
            
            # Frequency
            timestamp = curr_row.timestamps
                       
            # Buffer of 0.1 s because I don't trust the system not to miss the 
            # first onset
            # Convert timestamp in samples to seconds and cast as int
            # I call it a sill because windows comprise sills.
            rough_win = [
                int(sill*self.sr) for sill in (timestamp-.1, timestamp+2.1)
            ] 
            
            # Get the time slice
            rough_slice = self.signal[rough_win[0]:rough_win[1],:]
            
            # May be subject to change with subsequent iterations
            THRES = 1500 
            
            # average periods, convert to frequencies
            onsets = np.where(np.diff(rough_slice[:,2]) > THRES)[0]
            
            # Finally get frequencies
            empirical_freq = len(onsets)/2 # Note this 2 may change for some reason
            
            # Reset window to start with the first onset
            precise_win = (rough_win[0] + onsets[0], rough_win[0] + onsets[0] + 2 * self.sr)
            curr_slice = self.signal[precise_win[0]:precise_win[1],:]

            # Get times
            curr_times = self.sample_times[precise_win[0]:precise_win[1]]
            
            # Put everything into the df_dict
            df_dict['trial'].append(stim_ind)
            df_dict['color'].append(color)
            df_dict['frequency (Hz)'].append(empirical_freq)
            df_dict['channel 1'].append(curr_slice[:,0])
            df_dict['channel 2'].append(curr_slice[:,1])
            df_dict['TTL'].append(curr_slice[:,2])
            df_dict['time (s)'].append(curr_times)
    
        unwrangled_df = pd.DataFrame(df_dict)    
        
        # Include metadata in the first
        c1df = unwrangled_df.explode('channel 1')[
            [ 'trial',
             'color',
             'frequency (Hz)',
             'channel 1'
            ]
        ]
        
        # Just include exploded data types for the last three
        c2df = unwrangled_df.explode('channel 2')['channel 2']
        TTLdf = unwrangled_df.explode('TTL')['TTL']
        timedf = unwrangled_df.explode('time (s)')['time (s)']
        
        self.df = pd.concat([c1df, c2df, TTLdf, timedf], axis=1)
        
        # Get a shifted time column so they can be plotted next to each other
        # Defensive programming: instead of taking the first elem, take lowest
        # For some reason, I need to put .values, otherwise it bugs out.
        # Important to note.
        self.df['shifted time (s)'] = self.df.groupby(
            'trial'
        )['time (s)'].apply(
            lambda x: x - x.min()
        ).reset_index()['time (s)'].values
        
        
        # Convert empirical frequency into a real frequency for easier viz
        self.df['theoretical frequency'] = [
            self._closest_frequency(emp_freq) 
            for emp_freq in self.df['frequency (Hz)'].values
        ]
    
    #%%
    def _closest_frequency(self, empirical_freq):
        '''
        Measured frequency, while more accurate, is difficult to plot. So this 
        converts the measured frequency into what the program says it should be

        Parameters
        ----------
        empirical_freq : float
            measured frequency as number of offsets divided by number of seconds.

        Returns
        -------
        int
        '''
        real_freqs = np.array([1,2,4,8,16,32,64,128,256,512])
        return real_freqs[np.argmin(np.abs(empirical_freq-real_freqs))]
        

#%%
if __name__ == '__main__':
    
    # ergram = ERG(r'C:\Users\danpo\Documents\BYB\BYB_Recording_2020-03-22_18.00.44.wav')
    # ergram.vizCFF()
    
    # ergram = ERG(r'C:\Users\danpo\Documents\BYB\BYB_Recording_2020-03-22_17.52.20.wav')
    # ergram.vizCFF()
    
    # ergram = ERG(r'C:\Users\danpo\Documents\BYB\BYB_Recording_2020-03-22_16.50.13.wav')
    # ergram.vizCFF()
    
    # ergram = ERG(r'C:\Users\danpo\Documents\BYB\BYB_Recording_2020-08-14_11.31.22.wav')
    # ergram.vizCFF()
    # ergram = ERG(r'C:\Users\danpo\Documents\BYB\BYB_Recording_2020-08-14_16.10.46.wav')
    ergram = ERG(r'../../data/BYB_Recording_2020-12-23_13.47.39.wav')
    def show_alignment(sr):
        plt.plot(ergram.signal[:,2])
        for i in range(len(ergram.stim_df)):
            plt.plot(ergram.stim_df.timestamps[i] * sr, 4e3, '*r')
    # show_alignment(3e3)
        