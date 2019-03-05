import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
plt.style.use('seaborn-whitegrid')

data = pd.read_csv("measures.csv", sep=';')
df = pd.DataFrame(data=data)
df['err'] = 2500-df['position']
dff['err'] = df['err']/1250
df['t'] = df['t']/1000


# ------------
df.loc[df['num_samples_avg'] == 1].plot(x='t', y='err', label="size_sample=1, f=100Hz")
df.loc[df['num_samples_avg'] == 6].plot(x='t', y='err', label="size_sample=6, f=100Hz")
df.loc[df['num_samples_avg'] == 12].plot(x='t', y='err', label="size_sample=12, f=100Hz")
plt.xlabel('Time [seconds]')
plt.ylabel('Error line to center [cm]')
plt.savefig('qtr-flow.png')
plt.show()


# ------------
# params = {
#     'num_samples_avg': 10,
#     'f': 50,
#     'mode': "manual"
# }
# sub_sample = df
# for key, value in params.items():
#     sub_sample = sub_sample.loc[sub_sample[key] == value]
# sub_sample.plot(x='t', y='position')
# plt.show()


# ------------
# sub_sample.boxplot(column=['v0', 'v1', 'v2', 'v3', 'v4', 'v5'])
# plt.show()









