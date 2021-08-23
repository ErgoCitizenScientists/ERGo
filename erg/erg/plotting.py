# -*- coding: utf-8 -*-
from math import floor
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1.anchored_artists import AnchoredSizeBar
import numpy as np
import seaborn as sns
from scipy.signal import butter, filtfilt


from matplotlib.ticker import ScalarFormatter





class Plotting:
    def __init__(self, ergram):
        self.ergram = ergram


    def periodogram(self,win,freq):
        plt.figure()
        N=len(win)
        T=1/10000
        xf = np.linspace(0.0, 1.0/(2.0*T), N/2)
        transform = np.fft.fft(np.squeeze(win))
        plt.subplot(2,1,1)
        plt.plot(xf, 2.0/N * np.abs(transform[:N//2]))
        plt.subplot(2,1,2)
        plt.plot(win)      
        freq_indices = np.fft.fftfreq(N,T)
        plt.title(freq_indices[1:5])
        
        
        
    def bode(self, jitter=False):
        bode_dict={}
        messages = self.ergram.messages
        for key in sorted(messages):

            trials_list = messages[key]
            for timestamps_list in trials_list:
                
                freq = self.ergram.get_frequency(key)
                                
                period = 1/freq
                beg = timestamps_list[0]
                end = timestamps_list[-1] + period
                trial_window = self.ergram.get_window(beg, end)
                
                bode_dict[key]=trial_window
        
        plt.figure()
        for pin in range(7,12):
            # for each color:
            bode_arr = np.zeros((10,1))
            bode_arr.fill(np.nan)
            bode_arr=np.squeeze(bode_arr)
            
            
            for freq_i in range(2,12):
                # for each frequency:
                
                key = (freq_i, pin)
                freq = self.ergram.get_frequency(key)
                if key in bode_dict:
                    win = bode_dict[key]
                    N=len(win)
                    T=1/10000
                    transform = np.fft.fft(np.squeeze(win))
                    freq_indices = np.fft.fftfreq(N,T)
                    # freq_indices is an array of floats that are too precise 
                    # and not accurate enough -- so where is it closest to the frequency?
                    freq_index = np.argmin(np.abs(freq_indices-freq)) 
                    power = transform[freq_index]
                    bode_arr[freq_i-2] = np.abs(power)


            color = self.ergram.get_color(key)
            if color == 'ultraviolet':
                color='darkviolet'
            elif color == 'infrared':
                color='deeppink'
                
            # Now, change bode_arr to decibel scale
            bode_arr = 20 * np.log10(bode_arr/bode_arr[0])
            bode_arr = bode_arr + np.array(np.random.normal(0, 0.7, len(bode_arr)))
            
            # Now, plot everything
            plt.hlines(-3,xmin=0,xmax=512, color='lightgrey')
            plt.semilogx(self.ergram.frequencies(), bode_arr, color=color,
                       basex=2)
            plt.title("Bode plot for square wave input")
            plt.xlabel("Frequency (Hz)")
            plt.ylabel("Power (dB)")
            plt.gca().xaxis.set_major_formatter(ScalarFormatter())
                
    
    def quick(self, normalize=False, show_all=False, whole_trial=False):
        ''' This function spits out a subplot of all the combinations of color
        and frequency present, with colors in columns and frequencies in rows.
        
        Useful article about subplotting efficiently:
        https://matplotlib.org/3.1.0/gallery/subplots_axes_and_figures/subplots_demo.html
        
        Useful article about tight_layout
        https://matplotlib.org/users/tight_layout_guide.html
        '''
        # Setting up and labeling the subplot figure so I can 
        # fill it in in the loops below
        
    
        subfig, subax = self.get_subaxes()
        
        if whole_trial:
            subfig_whole, subax_whole = self.get_subaxes()


        # Looping through the stimuli
        messages = self.ergram.messages
        for key in sorted(messages):

            trials_list = messages[key]
            for timestamps_list in trials_list:
                
                freq = self.ergram.get_frequency(key)
                
                color = self.ergram.get_color(key)
                if color == 'ultraviolet':
                    color='darkviolet'
                elif color == 'infrared':
                    color='deeppink'

                x_ind = key[0] - 2
                y_ind = key[1] - 7
                
                y_ind = y_ind - 1 if not 0 else 4 # I need to shift it circularly so that IR is at the bottom.

                period = 1 / freq
                AHP_period = 0.2 # afterhyperpolarization period

                # First the whole trial is plotted.
                if whole_trial:

                    beg = timestamps_list[0]
                    end = timestamps_list[-1] + period + AHP_period
                    trial_window = self.ergram.get_window(beg, end)


                    # add to dict for bode here too
                    curr_sub_whole = subax_whole[y_ind][x_ind]
                    curr_sub_whole.plot(trial_window, 'k')
                    self.plot_stim_times(timestamps_list, max(trial_window),
                                         color, freq, curr_sub_whole)

                # Second, the rest of the trials are plotted
                curr_sub = subax[y_ind][x_ind]
                winsize= int(period*self.ergram.samplerate)
                
                stim_matrix = np.zeros((winsize,1))
                stim_matrix.fill(np.nan)
                # put together the matrix
                for stim in timestamps_list:                
                    beg = stim
                    
                    stim_win = self.ergram.get_window_precise(
                            beg*self.ergram.samplerate, winsize)
                    
                    stim_matrix = np.hstack((stim_matrix, stim_win))
                
                # iterate through the matrix depending on optional arguments
                if show_all and freq < 32:
                    for i in range(np.shape(stim_matrix)[0]):
                        curr_sub.plot(stim_win, color=color, linewidth=0.75)
                else:
                    avg_stim_win = np.nanmean(stim_matrix, axis=1)
                    curr_sub.plot(avg_stim_win, color=color)
                
            # now this will show the full waveform for everything
            if not normalize:
                curr_sub.set_ylim(self.get_max_ylim(subax))

    def get_2Hz(self, normalize=False, whole_trial=False, filter_=True, interval=.1):

        subfig, subax = self.get_2Hz_subaxes()
        
        # Looping through the stimuli
        messages = self.ergram.messages
        for key in sorted(messages):

            trials_list = messages[key]
            if key[1] != 6: # if color code isn't 6 (which is an error somewhere)
                for timestamps_list in trials_list:
                    
                    freq = self.ergram.get_frequency(key)
                    
                    color = self.ergram.get_color(key)
                    if color == 'ultraviolet':
                        color='darkviolet'
                    elif color == 'infrared':
                        color='deeppink'
    
                    x_ind = key[0] - 2
                    y_ind = key[1] - 7
                    
                    # I need to shift it circularly so that IR is at the bottom.
                    if y_ind is 7:
                        y_ind = 0
                    elif y_ind is 8:
                        y_ind = 4
                    elif y_ind > 8:
                        y_ind = y_ind - 8
                    
                    
                    
                    period = 1 / freq
                    AHP_period = 0.2 # afterhyperpolarization period
    
                    # Only plot if x_ind is 1, (2Hz)
                    if x_ind == 1:
                        sr = self.ergram.samplerate
                        buf = 0.035
                        
                        curr_sub = subax[y_ind]
                        winsize= int(period*self.ergram.samplerate)
                        
                        stim_matrix = np.zeros((winsize+int(buf*sr),1))
                        stim_matrix.fill(np.nan)
                        # put together the matrix
                        for stim in timestamps_list:                
                            beg = stim * sr
                            stim_win = self.ergram.get_window_precise(
                                    beg-int(buf*sr), winsize+int(sr*buf))
                            
                                
                            stim_matrix = np.hstack((stim_matrix, stim_win))
                        
                        # iterate through the matrix depending on optional arguments
                        avg_stim_win = np.nanmean(stim_matrix, axis=1)
                        
                        #plot pres
                        curr_sub.plot(range(int(buf*sr)),
                                      avg_stim_win[0:int(buf*sr)],
                                      color='k',
                                      linewidth=3)
                        # plot the illuminated part
                        curr_sub.plot(range(int(buf*sr),int(winsize/2+buf*sr)),
                                      avg_stim_win[int(buf*sr):int(winsize/2+buf*sr)],
                                      color=color,
                                      linewidth=3)
                        
                        # plot the unilluminated part
                        curr_sub.plot(range(int(winsize/2+buf*sr), int(winsize+buf*sr)),
                                      avg_stim_win[int(winsize/2+buf*sr):winsize+int(buf*sr)],
                                      color='k',
                                      linewidth=3)
                        
                        
        for axis in subax:
            axis.set_ylim(self.get_max_ylim1D(subax))
            
        # add size bar
        bar = AnchoredSizeBar(plt.gca().transData, self.ergram.samplerate * interval,
                              str(int(interval * 1000)) + ' ms', loc=4, frameon=False)
        plt.gca().add_artist(bar)

    def get_2Hz_subaxes(self):
        subfig, subax = plt.subplots(5,1)
        self.full_despine(subfig)
        subax[0].set_title('2 Hz')
        
        rows = ['IR','R','G','B','UV']
        for i in range(len(subax)):
            subax[i].set_ylabel(rows[i], rotation=0, size='large')
            
        return(subfig, subax)


    def get_subaxes(self):
        subfig, subax = plt.subplots(5, 10)
        self.full_despine(subfig)
        for ax, col in zip(subax[0], [1, 2, 4, 8, 16, 32, 64, 128, 256, 512]):
            ax.set_title(col)

        for ax, row in zip(subax[:, 0], ['UV        ', 'R        ',
                           'G        ', 'B        ', 'IR        ']):
            ax.set_ylabel(row, rotation=0, size='large')
        return (subfig, subax)
    
    def get_max_ylim1D(self, axes):
        tup_list = []
        dif_list = []
        for i in range(np.shape(axes)[0]):
            ylim_ = axes[i].get_ylim()
            tup_list.append(ylim_)
            dif_list.append(ylim_[1]-ylim_[0])
        return tup_list[np.argmax(dif_list)]
                
    def get_max_ylim(self, axes):
        tup_list = []
        dif_list = []
        for i in range(np.shape(axes)[0]):
            for j in range(np.shape(axes)[1]):
                ylim_ = axes[i][j].get_ylim()
                tup_list.append(ylim_)
                dif_list.append(ylim_[1]-ylim_[0])
        
        return tup_list[np.argmax(dif_list)]
                
    
    def ca(self):
        plt.close("all")


    def full_despine(self, fig):
        sns.despine(top=True, left=True, bottom=True, right=True)
        for ax in fig.axes:
            ax.set_xticks([])
            ax.set_yticks([])

    def pretty_up_CFF(self, beg, end, key, fig, interval=0.250):

        '''
        Parameters
        ----------
        beg : float
              time in seconds on left of window
        end : float
              time in seconds on right of window
        key : tuple(int, int)
              contains information about the frequency and color being sent
        '''
        fig.canvas.set_window_title(
            "freq: {}, color: {}; {}:{} to {}:{}".format(
                self.ergram.get_frequency(key), self.ergram.get_color(key),
                floor(beg / 60), floor(beg % 60), floor(end / 60), floor(end % 60)))

        self.full_despine(fig)

        bar = AnchoredSizeBar(plt.gca().transData, self.ergram.samplerate * interval,
                              str(int(interval * 1000)) + ' ms', loc=4, frameon=False)
        plt.gca().add_artist(bar)


    def plot_stim_times(self, timestamps_list, max_, color, freq, ax):
        '''
        Parameters
        ----------
        timestamps_list : list(float)
            Contains times of timestamps in seconds
        max_ : float
            maximum value of sampled data
        '''

        duration = 1 / freq / 2 * self.ergram.samplerate
        if freq < 16:
            on = []
            off = []
            for stimulation in self.ergram.to_window(timestamps_list):
                on.append(stimulation)
                off.append(stimulation+duration)
                
            levels = np.ones((len(on),1)) * max_ * 1.1
            ax.hlines(y=levels, xmin=on, xmax=off, linewidth=2, color=color)
