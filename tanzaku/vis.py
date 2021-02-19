#!/usr/bin/env python3

import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np

# df_latency = pd.read_table('second_adventure.tsv')
# df_latency = pd.read_table('third_adventure.tsv')
# df_latency = pd.read_table('third_adventure_latency.tsv')

df_latency = pd.read_table('fourth_adventure_latency.tsv')
df_latency_pivot = pd.pivot_table(data=df_latency, values='latency',
                                  columns='region', index='country', aggfunc=np.mean)
sns.heatmap(df_latency_pivot)
plt.show()

df_unit = pd.read_table('fourth_adventure_unit.tsv')
df_unit_pivot = pd.pivot_table(data=df_unit, values='units',
                               columns='project', index='service', aggfunc=np.mean)

# print(df_latency_pivot.head(5))
sns.heatmap(df_unit_pivot)
plt.show()
