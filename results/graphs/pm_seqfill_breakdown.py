import numpy as np
import matplotlib.pyplot as plt

# Data for the segments
categories = ['File system baseline', 'QuarkStore baseline', 'QuarkStore+Append']

fs = 174
qs = 142
app = 61

data = {
    'Writing or appending during compaction': [fs * 0.5122, qs * 0.3933, app * 0.0787],  # Segment for compaction time
    'Reading during compaction': [fs * 0.2088, qs * 0.2978, app * 0.525],  # Segment for compaction time
    'Flushing': [fs * 0.0755, qs * 0.0742, app * 0.1207],   # Segment for flushing time
    'MemTable insertion': [fs * 0.071, qs * 0.093, app * 0.228],   # Segment for memtable time
    'Other': [fs - (fs * 0.893), qs - (qs * 0.8896), app - (app * 0.8376)],  # Segment for other operations
}

print(f"FS: {fs * 0.5122}")
print(f"QuarkStore+Append: {app * 0.0787}")

# Colors for each segment
segment_colors = ['#EC6B56', '#FFC154', '#47B39C', '#8E4162', '#6A4C93']

# Bar width and positions
n_categories = len(categories)
bar_width = 0.6
x = np.arange(n_categories)

# Plot
plt.figure(figsize=(8, 5))
bottom = np.zeros(n_categories)  # Initialize bottom for stacking

# Stack each segment
bars = []
for segment, color in zip(data.keys(), segment_colors):
    bars.append(plt.bar(
        x,
        data[segment],       # Segment values for each category
        bar_width,
        label=segment,       # Label for legend
        color=color,
        bottom=bottom,       # Stack on top of the previous segment
        edgecolor='black'
    ))
    bottom += data[segment]  # Update bottom for the next segment

# Labels and customization
plt.xticks(x, categories)  # Category names on x-axis
plt.ylabel('Time (seconds)')

# Reverse the legend order
handles, labels = plt.gca().get_legend_handles_labels()
plt.legend(handles[::-1], labels[::-1], title="Subprocesses", loc='upper right')  # Reverse legend order

plt.tight_layout()

# Show the plot
plt.show()
