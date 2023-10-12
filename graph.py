import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv('prod1_30_timing.csv')

plt.plot(df['iteration'], df['time'], marker='o', linestyle='-')
plt.grid()
plt.xlabel('Iteration')
plt.ylabel('Time (μs)')
plt.yscale('log')
plt.show()

print("hello")