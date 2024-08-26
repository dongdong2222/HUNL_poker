from graphviz import Source

dot_file_path = "visualization.dot"
png_fime_path = "result"

with open(dot_file_path, "r") as f:
    dot_data = f.read()
    

source = Source(dot_data)
source.render(png_fime_path, format="png")

print("Graph successfully saved as {output_path}")