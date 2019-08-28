#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt

# data to plot
n_groups = 6
means_task = ( 1.4281962308657103, 1.9581784323812381, 1.6896272868449282, 1.4221719389030594, 1.9745772854819725, 1.7002851478927532 )
means_subpixel = ( 1.4069885980965595, 1.6679112519564054, 1.6335733688674048, 1.437190014471677, 1.7601560801847551, 1.5575636987997532 )
means_sample = ( 1.4220945901834257, 1.816524483941993, 1.6181160195102278, 1.3952034632324108, 1.8192685189390885, 1.625039985943767 )

# create plot
fig, ax = plt.subplots()
index = np.arange(n_groups)
bar_width = 0.2
opacity = 0.8

rects1 = plt.bar(index, means_task, bar_width,
alpha=opacity,
color='b',
label='Pixels')

rects2 = plt.bar(index + bar_width, means_subpixel, bar_width,
alpha=opacity,
color='g',
label='Sous-pixels')

rects3 = plt.bar(index + 2 * bar_width, means_sample, bar_width,
alpha=opacity,
color='r',
label='Sample')

plt.xlabel('Executions')
plt.ylabel('temps single/multi thread')
plt.xticks(index + bar_width, ('small 2p', 'small 4p', 'small 8p', 'medium 2p', 'medium 4p', 'medium 8p'))
plt.legend()

plt.tight_layout()
plt.show()

means_task = ( 1.421257257734867, 2.9367189738278, 2.615416539108114, 1.4110643846938884, 3.1940396899969237, 2.8461796234036516 )
means_subpixel = ( 1.403621368660069, 1.484383396868927, 1.491915751330674, 1.3522897879961997, 0.7012591553260386, 0.9478735783202865 )
means_sample = ( 1.4188052780225138, 2.611844445513464, 1.233853536030235, 1.3790583193862824, 2.693704654153557, 0.6955535589540045 )

fig, ax = plt.subplots()
index = np.arange(n_groups)
bar_width = 0.2
opacity = 0.8

rects1 = plt.bar(index, means_task, bar_width,
alpha=opacity,
color='b',
label='Pixels')

rects2 = plt.bar(index + bar_width, means_subpixel, bar_width,
alpha=opacity,
color='g',
label='Sous-pixels')

rects3 = plt.bar(index + 2 * bar_width, means_sample, bar_width,
alpha=opacity,
color='r',
label='Sample')

plt.xlabel('Executions')
plt.ylabel('temps single/multi thread')
plt.xticks(index + bar_width, ('small 2p', 'small 4p', 'small 8p', 'medium 2p', 'medium 4p', 'medium 8p'))
plt.legend()

plt.tight_layout()
plt.show()

