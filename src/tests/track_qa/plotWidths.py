
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages



def save_image(filename):
    # PdfPages is a wrapper around pdf file so there is no clash and create files with no error.
    p = PdfPages(filename)
      
    # get_fignums Return list of existing figure numbers
    fig_nums = plt.get_fignums()  
    figs = [plt.figure(n) for n in fig_nums]
      
    # iterating over the numbers in list
    for fig in figs: 
        # and saving the files
        fig.savefig(p, format='pdf') 
          
    # close the object
    p.close() 




########################################## MAIN FUNCTION ##########################################

vollay_identities = ["b disk 5","b disk 4","b disk 3","b disk 2","b disk 1",
    "vertex 1","vertex 2","vertex 3","f disk 1","barrel sagitta 1","f disk 2",
    "barrel sagitta 2","f disk 3","f disk 4","f disk 5",
    "MPGD barrel","TOF barrel","TOF endcap"]

p_bins = [2,5,7,10]

nlayers = 18
npbins = 4


# read dat file
datfilename = "widths.dat"
outputname = "plot_widths_python.pdf"

sigma1s, sigma2s, HMFWs = np.loadtxt(datfilename, unpack=True, usecols=[2,3,4])

# extract data into proper form
# 3 arrays (largewidths_arr, smallwidths_arr, calcwidths_arr), each of size nlayers that holds an array of size npbins
    # so basically a 2D array where first index is the layer
    # and second index is the widths at each momentum bin

rows, cols = (nlayers, npbins)
largewidths_arr = np.empty(shape=(nlayers,npbins)) #[[0]*cols]*rows
smallwidths_arr = np.empty(shape=(nlayers,npbins)) #[[0]*cols]*rows
calcwidths_arr = np.empty(shape=(nlayers,npbins)) #[0]*cols]*rows

ctr = 0
for i in range(nlayers):
    for j in range(npbins):
        largewidths_arr[i][j] = sigma1s[ctr]
        smallwidths_arr[i][j] = sigma2s[ctr]
        calcwidths_arr[i][j] = HMFWs[ctr]
        ctr+=1

print(largewidths_arr)
# plot!
# fig1 = plt.figure()
# # gs = plt.GridSpec(5, 4, height_ratios=[1, 1]) 
# # fig, axes = plt.subplots(5, 4, sharex=True, sharey=True)

# for i in range(nlayers):
#     # if i==0:
#     ax0 = plt.subplot(5, 4, i+1) #gs[0])
#     ax0.set_ylim(bottom=0, top=0.15)
#     # else:
#         # plt.subplot(5,4,i+1, sharex = ax0)
#         # if i%4==0:
#         #     if (int(i/4) == 0):
#         #         plt.subplot(gs[i], sharex = ax0, sharey = ax0)
#         # elif i%4==0:
#         #     plt.subplot(gs[i], sharex = ax0)

#         # if int(i/4) == 0:
            
    
#     plt.plot(p_bins, largewidths_arr[i], 'o', label="Large widths", color='blue')
#     ax0.text(5, 0.12, vollay_identities[i], fontsize=8)
#     # print(largewidths_arr[i])
#     # plt.plot(p_bins, smallwidths_arr[i], 'o', label="Small widths", color='orange')
#     # plt.plot(p_bins, calcwidths_arr[i], 'o', label="Calculated widths", color='green)

# # plt.legend()
# plt.suptitle("Larger Widths from Double Gaussian Fits")
# fig1.supxlabel('p [GeV/c]')
# fig1.supylabel('Width of Fit [mm]')
# fig1.tight_layout(pad=0.75)

######################################################
fig1, axs = plt.subplots(nrows=5, ncols=4, sharex=True, sharey=True, subplot_kw=dict(frameon=True)) # frameon=False removes frames

for i, ax in enumerate(fig1.axes):
    # ax.set_ylabel(str(i))
    if i==18: 
        break
    ax.plot(p_bins, largewidths_arr[i], 'o', label="Large widths", color='blue')
    ax.set_ylim(bottom=0, top=0.24)
    ax.text(3, 0.16, vollay_identities[i], fontsize=8)
    ax.set_xlim(left=0)
    # ax.grid()

# plt.legend()
plt.suptitle("Larger Widths from Double Gaussian Fits")
fig1.supxlabel('p [GeV/c]')
fig1.supylabel('Width of Fit [mm]')
# fig1.tight_layout(pad=0.75)
plt.subplots_adjust(hspace=.0, wspace=.0)



fig2, axs = plt.subplots(nrows=5, ncols=4, sharex=True, sharey=True, subplot_kw=dict(frameon=True)) # frameon=False removes frames

for i, ax in enumerate(fig2.axes):
    # ax.set_ylabel(str(i))
    if i==18: 
        break
    ax.plot(p_bins, smallwidths_arr[i], 'o', label="Small widths", color='orange')
    ax.set_ylim(bottom=0, top=0.06)
    ax.text(3, 0.045, vollay_identities[i], fontsize=8)
    ax.set_xlim(left=0)
    # ax.grid()

# plt.legend()
plt.suptitle("Smaller Widths from Double Gaussian Fits")
fig2.supxlabel('p [GeV/c]')
fig2.supylabel('Width of Fit [mm]')
# fig2.tight_layout(pad=0.75)
plt.subplots_adjust(hspace=.0, wspace=.0)


fig3, axs = plt.subplots(nrows=5, ncols=4, sharex=True, sharey=True, subplot_kw=dict(frameon=True)) # frameon=False removes frames

for i, ax in enumerate(fig3.axes):
    # ax.set_ylabel(str(i))
    if i==18: 
        break
    ax.plot(p_bins, calcwidths_arr[i], 'o', label="Calculated widths", color='green')
    ax.text(3, 0.37, vollay_identities[i], fontsize=8)
    ax.set_ylim(bottom=0, top=0.45)
    ax.set_xlim(left=0)
    # ax.grid()

# plt.legend()
plt.suptitle("Calculated FMHW from Double Gaussian Fits")
fig3.supxlabel('p [GeV/c]')
fig3.supylabel('Calculated FWHM [mm]')
# fig3.tight_layout(pad=0.75)
plt.subplots_adjust(hspace=.0, wspace=.0)
######################################################

# fig2 = plt.figure()
# for i in range(nlayers):
#     ax2 = plt.subplot(5, 4, i+1)
#     ax2.set_ylim(bottom=0, top=0.05)

#     plt.plot(p_bins, smallwidths_arr[i], 'o', label="Small widths", color='orange')
#     ax2.text(5.5, 0.04, vollay_identities[i], fontsize=8)

# # plt.legend()
# plt.suptitle("Smaller Widths from Double Gaussian Fits")
# fig2.supxlabel('p [GeV/c]')
# fig2.supylabel('Width of Fit [mm]')
# # fig2.tight_layout(pad=0.75)



# fig3 = plt.figure()
# for i in range(nlayers):
#     ax3 = plt.subplot(5, 4, i+1)
#     ax3.set_ylim(bottom=0, top=0.4)

#     plt.plot(p_bins, calcwidths_arr[i], 'o', label="Calculated widths", color='green')
#     ax3.text(5, 0.3, vollay_identities[i], fontsize=8)

# # plt.legend()
# plt.suptitle("Calculated FMHW from Double Gaussian Fits")
# fig3.supxlabel('p [GeV/c]')
# fig3.supylabel('Calculated FWHM [mm]')
# # fig3.tight_layout(pad=0.75)



# fig2, axes2 = plt.subplots(5, 4, sharex=True, sharey=True)
# # add a big axes, hide frame
# fig2.add_subplot(111, frameon=False)
# # hide tick and tick label of the big axes
# plt.tick_params(labelcolor='none', top=False, bottom=False, left=False, right=False)
# plt.grid(False)
# plt.xlabel("common X")
# plt.ylabel("common Y")

# plt.savefig(outputname)



save_image(outputname)
