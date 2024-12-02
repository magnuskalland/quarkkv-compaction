import numpy as np
import matplotlib.pyplot as plt

# Data
groups = ['A', 'B', 'C', 'D', 'E', 'F'] 
categories = ['File system baseline', 'QuarkStore baseline', 'QuarkStore+Append'] 
data = list(map(lambda x : x / 60.0, [
    [2387.848, 2201.000, 1580.055],  # A
    [429.009, 428.949, 346.637],  # B
    [219.537, 217.031, 198.051],  # C
    [415.644, 395.173, 328.790],  # D
    [6641.622, 6599.133, 6672.000],  # E
    [2485.990, 2260.875, 1674.255],  # F
]))

# Colors for each category
colors = ['#EC6B56', '#FFC154', '#47B39C']  # Specify custom colors for bars

# Number of groups and categories
n_groups = len(groups)
n_categories = len(categories)

# Bar width and positions
bar_width = 0.2
spacing_scale = 0.7  # Scale to reduce space between groups
x = np.arange(n_groups) * spacing_scale  # Group positions on the x-axis
offsets = np.arange(-(n_categories // 2), n_categories // 2 + 1) * bar_width

# Plot
plt.figure(figsize=(10, 6))
for i, category in enumerate(categories):
    plt.bar(
        x + offsets[i], 
        [group[i] for group in data], 
        bar_width, 
        label=category, 
        color=colors[i], 
        edgecolor='black'  # Add black border to bars
    )

# Labels and customization
plt.xticks(x, groups)  # Set group labels on x-axis
plt.xlabel('YCSB workload')
plt.ylabel('Time (minutes)')
# plt.title('Runtime of a sequential fill of QuarkKV.')
plt.legend(title="Configurations")
plt.tight_layout()

# Show the plot
plt.show()
