#!/usr/bin/env python3

import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np

# df_latency = pd.read_table('second_adventure.tsv')
# df_latency = pd.read_table('third_adventure.tsv')
# df_latency = pd.read_table('fourth_adventure.tsv')

# df_latency_pivot = pd.pivot_table(data=df_latency, values='latency',
#                                   columns='region', index='country', aggfunc=np.mean)

df_latency = pd.read_table('fourth_adventure_service.tsv')
df_latency_pivot = pd.pivot_table(data=df_latency, values='units',
                                  columns='project', index='service', aggfunc=np.mean)

print(df_latency_pivot.head(5))

sns.heatmap(df_latency_pivot)

plt.show()
