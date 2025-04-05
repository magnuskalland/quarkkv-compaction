import numpy as np
import matplotlib.pyplot as plt

# Data for the segments
categories = ['File system baseline', 'QuarkStore baseline', 'QuarkStore+Append']

fs = 1984.92
qs = 1953.671
app = 1346.366

data = {
    'Writing or appending during compaction':   list(map(lambda x : x /60, [fs * 0.5894, qs * 0.4975, app * 0.1321])),
    'Reading during compaction':                list(map(lambda x : x /60, [fs * 0.2845, qs * 0.3728, app * 0.6255])),
    'Flushing':                                 list(map(lambda x : x /60, [fs * 0.047, qs * 0.0373, app * 0.0852])),
    'MemTable insertion':                       list(map(lambda x : x /60, [fs * 0.0053, qs * 0.0058, app * 0.0156])),
    'Other':                                    list(map(lambda x : x / 60, [
        fs - (fs * (0.5894 + 0.2845 + 0.047 + 0.0053)),
        qs - (qs * (0.4975 + 0.3728 + 0.0373 + 0.0058)),
        app - (app * (0.1321 + 0.6255 + 0.0852 + 0.0156)),
    ])),
}

print(f"FS compaction time: {fs * (0.5122 + 0.2088)}")
print(f"QuarkStore compaction time: {qs * (0.3933 + 0.2978)}")
print(f"QuarkStore+Append compaction time: {app * (0.0787 + 0.525)}")

print(f"FS read time: {fs * (0.2845)}")
print(f"QuarkStore read time: {qs * (0.3728)}")
print(f"QuarkStore+Append read time: {app * (0.6255)}")

# print(f"FS write time: {fs * (0.5122)}")
# print(f"QuarkStore write time: {qs * (0.3933)}")
# print(f"QuarkStore+Append write time: {app * (0.0787)}")

print(f"QuarkStore write time: {qs * (0.4975)}")
print(f"QuarkStore+Append write time: {app * (0.1321)}")


# Colors for each segment
segment_colors = ['#ffd966', '#8faadc', '#a9d18e', '#EC6B56', '#6A4C93']

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
plt.ylabel('Time (minutes)')

# Reverse the legend order
handles, labels = plt.gca().get_legend_handles_labels()
plt.legend(handles[::-1], labels[::-1], title="Subprocesses", loc='upper right')  # Reverse legend order

plt.tight_layout()

# Show the plot
plt.show()
