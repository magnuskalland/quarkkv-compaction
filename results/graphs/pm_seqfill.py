import numpy as np
import matplotlib.pyplot as plt

# Data
groups = ['8 GiB', '16 GiB', '32 GiB', '64 GiB'] 
categories = ['File system baseline', 'QuarkStore baseline', 'QuarkStore+Append'] 
data = [list(map(lambda x: x / 60.0, group)) for group in [
    [174.316, 142.586, 61.093],  # Data for Group 1
    [398.812, 315.829, 130.609],  # Data for Group 2
    [826.795, 814.191, 318.341],  # Data for Group 3
    [2021.058, 1700.623, 697.789],  # Data for Group 4
]]

# Colors for each category
colors = ['#ffd966', '#8faadc', '#a9d18e']  # Specify custom colors for bars

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
plt.xlabel('Workload size')
plt.ylabel('Time (minutes)')
# plt.title('Runtime of a sequential fill of QuarkKV.')
plt.legend(title="Configurations")
plt.tight_layout()

# Show the plot
plt.show()
