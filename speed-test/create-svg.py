"""
CSV -> SVG plotter (two separate plots)

Expected CSV format (first line = header):
    Batch-Size, Exports on CPU, RAM, VRAM, N1-TTFT, N1-tok/sec, N2-TTFT, N2-tok/sec, ...

Only the columns Batch-Size and the Ni-TTFT / Ni-tok/sec pairs are used.
The last line of the file is ignored (usually a line with a Batch-Size value, and why can't the model run with it).

Two SVG files are produced:
    <basename>-ttft.svg   : TTFT values only
    <basename>-speed.svg  : tok/sec (speed) values only
"""

import csv
import os
import sys

from numbers import Number
from typing import Iterable

# ----------------------------------------------------------------------
# Configuration constants
# ----------------------------------------------------------------------
COLOR_LIST = ['red', 'blue', 'green', 'purple', 'orange']

# CSV layout
DATA_START_IDX = 4                     # first Ni‑TTFT column index (0‑based)

# SVG canvas size
CANVAS_WIDTH = 900
CANVAS_HEIGHT = 600

# Padding around the plot area (pixels)
PADDING_LEFT = 80
PADDING_RIGHT = 40
PADDING_TOP = 60
PADDING_BOTTOM = 60

# Axis & grid appearance
NUM_TICKS = 15                         # how many tick marks per axis
TICK_SIZE = 4                          # length of tick marks (pixels)
GRID_STROKE_WIDTH = 1
GRID_COLOR = "#ddd"
AXIS_STROKE_WIDTH = 2
AXIS_COLOR = "#777"
AXIS_MARGIN_PERCENTAGE = .03 # 3%

# Point appearance
POINT_RADIUS = 3

# Text appearance
LABEL_FONT_SIZE = 14                   # series labels (top‑right)
TICK_LABEL_FONT_SIZE = 12              # axis numbers
LABEL_SPACING_VERT = 20                # vertical space between series labels
LABEL_OFFSET_FROM_TOP = PADDING_TOP // 2
LABEL_OFFSET_FROM_RIGHT = 10           # distance of label block from right edge

# SVG overall layout
SVG_BACKGROUND = None
SVG_HEADER_STR = f'<svg width="{CANVAS_WIDTH}" height="{CANVAS_HEIGHT}" xmlns="http://www.w3.org/2000/svg" version="1.1">'
SVG_TAIL_STR = "</svg>"

# ----------------------------------------------------------------------
# CSV reading & preprocessing
# ----------------------------------------------------------------------

def read_csv(filepath: str) -> \
    tuple[
        tuple[str],
        tuple[int],
        dict[(str, int), float],
        dict[(str, int), float],
    ]:
    """
    Read the CSV, discard the ignored columns and the final empty line.  
    return:  
        - ni_values: list of Ni strings in the order they appear  
        - batch_values: list of the batch values in the order they appear  
        - ttft_data: dict { (Ni, batch) -> ttft }  
        - tok_data: dict { (Ni, batch) -> tok/sec }  
    """

    # open and read the CSV file
    with open(filepath, newline='', encoding='utf-8') as f:
        reader = csv.reader(f)
        try:
            header = [h.strip() for h in next(reader)]
            # Keep everything except the final line
            rows = list(reader)[:-1]
        except StopIteration:
            print("CSV file is empty.")
            sys.exit(1)

    # Basic header validation
    if len(header) < 5:
        print("Unexpected CSV header - the CSV must contains at least 5 columns.")
        sys.exit(1)
    if header[0] != "Batch-Size":
        print("Unexpected CSV header - first column must be 'Batch-Size'.")
        sys.exit(1)

    # Scan header for Ni-TTFT / Ni-tok/sec pairs
    ni_values = []  # keeps Ni strings in the order they appear (e.g. ['N1','N2',...])
    idx = DATA_START_IDX
    while idx < len(header):
        ttft_col = header[idx]
        if not ttft_col.endswith("-TTFT"):
            # If we meet something that is not a TTFT column we stop parsing
            print(f"Error: Got unknown column: {ttft_col}, expected a TTFT values column")
            sys.exit(1)
        ni = "-".join(ttft_col.split("-")[:-1])  # get the 'N1' string
        # Verify the following column matches the tok/sec counterpart
        if idx + 1 >= len(header):
            print(f"Missing tok/sec column for {ni}")
            sys.exit(1)
        tok_col = header[idx + 1]
        expected_tok = f"{ni}-tok/sec"
        if tok_col != expected_tok:
            print(f"Expected column '{expected_tok}' but found '{tok_col}'")
            sys.exit(1)

        ni_values.append(ni)
        idx += 2 # skip the pair we just processed

    if not ni_values:
        print("No Ni-TTFT / Ni-tok/sec column pairs found.")
        sys.exit(1)

    if len(ni_values) > len(COLOR_LIST):
        print(f"Error: found {len(ni_values)} different Ni values, but only up to {len(COLOR_LIST)} are supported.")
        sys.exit(1)

    # Prepare storage for each series
    batch_values = []
    ttft_data: dict[(str, int), float] = {}
    tok_data: dict[(str, int), float] = {}

    for row_num, row in enumerate(rows, start=1):
        # empty or malformed line - skip with a warning
        if (not row or all(cell.strip() == '' for cell in row)) or len(row) < len(header):
            print(f"Warning: line {row_num} has fewer columns than header - skipping.")
            continue

        try:
            batch = int(row[0])
        except ValueError:
            print(f"Warning: cannot parse Batch-Size on line {row_num} - skipping.")
            continue
        batch_values.append(batch)
        
        # Walk through the Ni pairs and store the numbers
        idx = DATA_START_IDX
        for ni in ni_values:
            try:
                ttft = float(row[idx])
                tok = float(row[idx + 1])
            except (ValueError, IndexError):
                print(f"Warning: bad numeric value for {ni} on line {row_num} - skipping this row.")
                break # abort this row entirely
            ttft_data[ni, batch] = ttft
            tok_data[ni, batch] = tok
            idx += 2

    return tuple(ni_values), tuple(batch_values), ttft_data, tok_data


# ----------------------------------------------------------------------
# Prepare SVG scaling helpers
# ----------------------------------------------------------------------

def make_scaling(x_min: Number, x_max: Number, y_min: Number, y_max: Number):
    """Return scaling-x, scaling-y functions for the given limits.
    The scaling functions map a value in [min, max] to the SVG canvas's axis.
    """

    def map_value(v, v_min, v_max, out_min, out_max):
        """Map v from [v_min, v_max] -> [out_min, out_max] (linear)."""
        if v_max == v_min:          # avoid division by zero
            return (out_min + out_max) / 2.0
        return out_min + (v - v_min) * (out_max - out_min) / (v_max - v_min)
    
    def sx(x):
        return map_value(x, x_min, x_max, PADDING_LEFT, CANVAS_WIDTH - PADDING_RIGHT)

    def sy(y):
        return map_value(y, y_min, y_max, CANVAS_HEIGHT - PADDING_BOTTOM, PADDING_LEFT)
    
    return sx, sy

def nice_ticks(v_min: Number, v_max: Number, steps: int = NUM_TICKS) -> list[Number]:
    """Return `steps` evenly spaced values from v_min to v_max (inclusive)."""
    if steps < 2:
        steps = 2
    step = (v_max - v_min) / (steps - 1)
    return [v_min + i * step for i in range(steps)]

def compute_limits(axis_vals: Iterable[Number]) -> tuple[Number, Number]:
    """
    Return (min, max) with a margin.
    `axis_vals` are iterables of numbers.
    """
    if not axis_vals:
        print("ERROR: SVG limit computation: No data to compute limits.")
        sys.exit(1)

    min_val = min(axis_vals)
    max_val = max(axis_vals)

    # 10% margin
    axis_range = max_val - min_val
    margin = AXIS_MARGIN_PERCENTAGE * axis_range

    return min_val - margin, max_val + margin

# ----------------------------------------------------------------------
# SVG drawing helpers
# ----------------------------------------------------------------------

def draw_grid_and_ticks(svg_parts: list[str], x_min: Number, x_max: Number, y_min: Number, y_max: Number, sx, sy) -> None:

    def fmt_val(val: Number) -> str:
        """Format a number for tick labels - drop trailing .0 when possible.  
        Round up to 1 digit after the point.
        """
        s = f"{val:.1f}"
        return s.rstrip('0').rstrip('.')

    """Add grid lines, tick marks and numeric axis labels."""
    xticks = nice_ticks(x_min, x_max, NUM_TICKS)
    yticks = nice_ticks(y_min, y_max, NUM_TICKS)

    # Vertical grid lines & x‑axis ticks / labels
    for x_val in xticks:
        x_svg = sx(x_val)
        # grid line
        svg_parts.append(
            f'  <line x1="{x_svg:.2f}" y1="{sy(y_min):.2f}" '
            f'x2="{x_svg:.2f}" y2="{sy(y_max):.2f}" '
            f'stroke="{GRID_COLOR}" stroke-width="{GRID_STROKE_WIDTH}"/>'
        )
        # tick mark (below axis)
        svg_parts.append(
            f'  <line x1="{x_svg:.2f}" y1="{sy(y_min):.2f}" '
            f'x2="{x_svg:.2f}" y2="{sy(y_min) + TICK_SIZE:.2f}" '
            f'stroke="{AXIS_COLOR}" stroke-width="1"/>'
        )
        # x‑axis label
        label = fmt_val(x_val)
        svg_parts.append(
            f'  <text x="{x_svg:.2f}" y="{sy(y_min) + TICK_SIZE + 12:.2f}" '
            f'text-anchor="middle" font-family="sans-serif" '
            f'font-size="{TICK_LABEL_FONT_SIZE}" fill="{AXIS_COLOR}">{label}</text>'
        )

    # Horizontal grid lines & y‑axis ticks / labels
    for y_val in yticks:
        y_svg = sy(y_val)
        # grid line
        svg_parts.append(
            f'  <line x1="{sx(x_min):.2f}" y1="{y_svg:.2f}" '
            f'x2="{sx(x_max):.2f}" y2="{y_svg:.2f}" '
            f'stroke="{GRID_COLOR}" stroke-width="{GRID_STROKE_WIDTH}"/>'
        )
        # tick mark (left of axis)
        svg_parts.append(
            f'  <line x1="{sx(x_min) - TICK_SIZE:.2f}" y1="{y_svg:.2f}" '
            f'x2="{sx(x_min):.2f}" y2="{y_svg:.2f}" '
            f'stroke="{AXIS_COLOR}" stroke-width="1"/>'
        )
        # y‑axis label
        label = fmt_val(y_val)
        svg_parts.append(
            f'  <text x="{sx(x_min) - TICK_SIZE - 4:.2f}" y="{y_svg + 4:.2f}" '
            f'text-anchor="end" font-family="sans-serif" '
            f'font-size="{TICK_LABEL_FONT_SIZE}" fill="{AXIS_COLOR}">{label}</text>'
        )

def draw_axes(svg_parts: list[str], x_min: Number, x_max: Number, y_min: Number, y_max: Number, sx, sy) -> None:
    """Draw the main X and Y axes (over the grid/ticks)."""
    svg_parts.append(
        f'  <line x1="{sx(x_min)}" y1="{sy(y_min)}" '
        f'x2="{sx(x_max)}" y2="{sy(y_min)}" '
        f'stroke="{AXIS_COLOR}" stroke-width="{AXIS_STROKE_WIDTH}"/>'
    )  # X‑axis
    svg_parts.append(
        f'  <line x1="{sx(x_min)}" y1="{sy(y_min)}" '
        f'x2="{sx(x_min)}" y2="{sy(y_max)}" '
        f'stroke="{AXIS_COLOR}" stroke-width="{AXIS_STROKE_WIDTH}"/>'
    )  # Y‑axis

def draw_series(svg_parts: list[str], ni_values: tuple[str], batch_values: tuple[int], data: dict[(str, int), float], sx, sy) -> None:
    """
    Plot the field's values for all the series (each series gets a unique color
    according to COLOR_LIST).  
    Each value is drawn as a point (draw small circle).
    """
    for idx, ni in enumerate(ni_values):
        color = COLOR_LIST[idx]
        for batch in batch_values:
            cx, cy = sx(batch), sy(data[ni, batch])
            svg_parts.append(
                f'  <circle cx="{cx:.2f}" cy="{cy:.2f}" r="{POINT_RADIUS}" '
                f'fill="{color}"/>'
            )

def draw_labels(svg_parts: list[str], labels, label_x, label_y_start, label_spacing):
    """Add the series labels (Ni) in the top-right corner."""
    for idx, ni in enumerate(labels):
        label_text = f"{ni}"
        color = COLOR_LIST[idx]
        label_y = label_y_start + idx * label_spacing
        svg_parts.append(
            f'  <text x="{label_x}" y="{label_y}" '
            f'font-family="sans-serif" font-size="{LABEL_FONT_SIZE}" '
            f'fill="{color}">{label_text}</text>'
        )

def draw_rectangle(svg_parts: list[str], width, height, color) -> None:
    svg_parts.append(f'<rect width="{width}" height="{height}" fill="{color}"/>')


# ----------------------------------------------------------------------
# SVG creation functions
# ----------------------------------------------------------------------

def make_svg_parts(ni_values: tuple[str], batch_values: tuple[int], data: dict[(str, int), float]) -> list[str]:
    x_min, x_max = compute_limits(batch_values)
    y_min, y_max = compute_limits(data.values())
    
    # ------------------------------------------------------------------
    # Produce plot's parts list
    # ------------------------------------------------------------------
    scaling_x_func, scaling_y_func = make_scaling(x_min, x_max, y_min, y_max)

    svg_parts = []
    svg_parts.append(SVG_HEADER_STR)

    if SVG_BACKGROUND:
        draw_rectangle(svg_parts, CANVAS_WIDTH, CANVAS_HEIGHT, SVG_BACKGROUND) # Draw background

    draw_grid_and_ticks(svg_parts, x_min, x_max, y_min, y_max, scaling_x_func, scaling_y_func)
    draw_axes(svg_parts, x_min, x_max, y_min, y_max, scaling_x_func, scaling_y_func)
    draw_series(svg_parts, ni_values, batch_values, data, scaling_x_func, scaling_y_func)

    label_x = CANVAS_WIDTH - PADDING_RIGHT - LABEL_OFFSET_FROM_RIGHT
    label_y_start = LABEL_OFFSET_FROM_TOP
    draw_labels(svg_parts, ni_values, label_x, label_y_start, LABEL_SPACING_VERT)
    svg_parts.append(SVG_TAIL_STR)

    return svg_parts

def write_svg(filepath: str, svg_parts: list[str]) -> None:
    """Write the accumulated SVG lines to disk."""
    try:
        with open(filepath, 'w', encoding='utf-8') as out_f:
            out_f.write('\n'.join(svg_parts))
        print(f"SVG written to: {filepath}")
    except OSError as e:
        print(f"Failed to write SVG file: {e}")
        sys.exit(1)

def csv2svg(csv_filepath: str, ttft_svg_filepath: str, tok_svg_filepath: str) -> None:
    # Load CSV and extract series / data
    ni_values, batch_values, ttft_data, tok_data = read_csv(csv_filepath)

    # ------------------------------------------------------------------
    # Produce TTFT plot
    # ------------------------------------------------------------------
    svg_parts_ttft = make_svg_parts(ni_values, batch_values, ttft_data)
    write_svg(ttft_svg_filepath, svg_parts_ttft)

    # ------------------------------------------------------------------
    # Produce speed (tok/sec) plot
    # ------------------------------------------------------------------
    svg_parts_tok = make_svg_parts(ni_values, batch_values, tok_data)
    write_svg(tok_svg_filepath, svg_parts_tok)

# ----------------------------------------------------------------------
# Main routine
# ----------------------------------------------------------------------
def main():
    # # Ask user for CSV path
    # csv_path = input("Enter path to CSV file: ")
    # if not csv_path:
    #     print("No file name supplied - exiting.")
    #     sys.exit(1)
    # if not os.path.isfile(csv_path):
    #     print(f"File not found: {csv_path}")
    #     sys.exit(1)

    # Test file
    csv_path = "speed-test/NVIDIA-Nemotron-3-Nano-30B-A3B-Q4_K_M/KV-Cache-Q4_0/512k.csv"

    # Base name for output files (same directory, same stem as CSV)
    base, _ = os.path.splitext(csv_path)
    csv2svg(csv_path, f"{base}-ttft.svg", f"{base}-speed.svg")

if __name__ == "__main__":
    main()
