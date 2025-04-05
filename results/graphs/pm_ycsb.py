import numpy as np
import matplotlib.pyplot as plt

# Data
groups = ['A', 'B', 'C', 'D', 'F'] 
categories = ['File system baseline', 'QuarkStore baseline', 'QuarkStore+Append'] 
data = [
    [194 / 60.0, 153 / 60.0, 80 / 60.0],  # A
    [56 / 60.0, 53 / 60.0, 45 / 60.0],  # B
    [36 / 60.0, 42 / 60.0, 46 / 60.0],  # C
    [30 / 60.0, 30 / 60.0, 21 / 60.0],  # D
    # list(map(lambda x: x / 2.0, [151 / 60.0, 149 / 60.0, 159 / 60.0])),  # E
    [216 / 60.0, 205 / 60.0, 99 / 60.0],  # F
]

print(f"FS baseline A: {194}")
print(f"FS baseline F: {80}")
print(f"QuarkStore+Append A: {216}")
print(f"QuarkStore+Append F: {99}")

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
plt.xlabel('YCSB workload')
plt.ylabel('Time (minutes)')
# plt.title('Runtime of a sequential fill of QuarkKV.')
plt.legend(title="Configurations")
plt.tight_layout()

# Show the plot
plt.show()
