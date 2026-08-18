"""
CSV -> SVG plotter (two separate plots)

Expected CSV format (first line = header):
    <X-axis column>, <Ni-field-1>, <Ni-field-2>, ...
where the X-axis column is in VALID_X_AXIS (a list of valid names for the column),
and each subsequent column follows the pattern "<Ni>-<suffix>".
The suffix is looked up in ``Y_AXIS_MAP`` to determine which
y-field the column belongs to (e.g. "ttft", "speed", "acceptance").

Only the columns that match ``Y_AXIS_MAP`` are used.
The last line of the file is ignored (usually a reason for stopping the test).

Two (or more) SVG files are produced for each y-field:
    <basename>-<graph_name>.svg
where ``graph_name`` comes from ``GRAPH_NAME_MAP``.
"""

import csv
import os
import sys
import argparse
from numbers import Number
from typing import Iterable

# Color constants for terminal output
RED = "\033[31m"
YELLOW = "\033[33m"
RESET = "\033[0m"

# ----------------------------------------------------------------------
# Configuration constants
# ----------------------------------------------------------------------
COLOR_LIST = ['red', 'blue', 'green', 'purple', 'orange']

# X-axis column names that are accepted
VALID_X_AXIS = {"Batch-Size", "Max Draft Tokens"}

# Mapping from canonical y-field name to the name used for the SVG file
GRAPH_NAME_MAP = {
    "TTFT": "ttft",
    "tok/sec": "speed",
    "acceptance": "acceptance",
    # extend here for additional y-fields
}
VALID_Y_AXIS = list(GRAPH_NAME_MAP.keys())

# SVG canvas size
CANVAS_WIDTH = 800
CANVAS_HEIGHT = 800

# Padding around the plot area (pixels)
PADDING_LEFT = 80
PADDING_RIGHT = 40
PADDING_TOP = 60
PADDING_BOTTOM = 60

# Axis & grid appearance
NUM_TICKS = 15
TICK_SIZE = 4
GRID_STROKE_WIDTH = 1
GRID_COLOR = "#ddd"
AXIS_STROKE_WIDTH = 2
AXIS_COLOR = "#777"
AXIS_MARGIN_PERCENTAGE = .03

# Point appearance
POINT_RADIUS = 5

# Text appearance
LABEL_FONT_SIZE = 16
TICK_LABEL_FONT_SIZE = 12
LABEL_SPACING_VERT = 20
LABEL_OFFSET_FROM_TOP = PADDING_TOP // 2
LABEL_OFFSET_FROM_RIGHT = 10
TITLE_FONT_SIZE = 24
TITLE_OFFSET_FROM_TOP = 30
TITLE_COLOR = "blue"
POINT_COORDINATES_LABEL_FONT_SIZE = 12
POINT_COORDINATES_LABEL_X_OFFSET = -30
POINT_COORDINATES_LABEL_Y_OFFSET = 4
POINT_COORDINATES_LABEL_Y_INDEX_OFFSET = 14

# SVG overall layout
SVG_BACKGROUND = None
SVG_HEADER_STR = f'<svg width="{CANVAS_WIDTH}" height="{CANVAS_HEIGHT}" xmlns="http://www.w3.org/2000/svg" version="1.1">'
SVG_TAIL_STR = "</svg>"

# ----------------------------------------------------------------------
# Helper: format numbers with at least 4 digit precision
# ----------------------------------------------------------------------
def format_val(v: float) -> str:
    """Format a number with at least 4 digit precision.
    - If the absolute value has 4+ digits (>=1000), render as integer.
    - Otherwise, format with the appropriate number of decimal places to keep at least 4 significant digits, removing trailing zeros.
    """
    # Handle zero explicitly to avoid issues with digit counting
    if v == 0:
        return "0"
    # Count digits before the decimal point in the absolute value
    digits_before = len(str(int(abs(v))))
    # Determine how many decimal places we need to reach at least 4 significant digits
    decimals_needed = max(0, 4 - digits_before)
    # Round to that many decimal places
    rounded = round(v, decimals_needed)
    # Format with fixed-point
    s = f"{rounded:.{decimals_needed}f}"
    # Remove trailing zeros and a trailing decimal point if any
    return s.rstrip('0').rstrip('.')

# ----------------------------------------------------------------------
# CSV reading & preprocessing
# ----------------------------------------------------------------------
def read_csv(filepath: str) -> \
    tuple[
        tuple[str],                     # ni_values
        tuple[int],                     # x_values
        dict[str, dict[(str, int), float]],  # data: {y_field: {(ni, x) -> y}}
    ]:
    """
    Read the CSV, discard ignored columns and the final empty line.
    Returns:
        - ni_values: tuple of Ni strings in order of appearance
        - x_values: tuple of x values (int) in order of appearance
        - data: dict mapping each y-field name to a dict {(ni, x) -> y}
    """
    # open and read the CSV file
    with open(filepath, newline='', encoding='utf-8') as f:
        reader = csv.reader(f)
        try:
            header = [h.strip() for h in next(reader)]
            # Keep everything except the final line
            rows = list(reader)[:-1]
        except StopIteration:
            raise ValueError("CSV file is empty.")

    # Basic header validation
    if not header:
        raise ValueError("CSV header is empty.")
    if header[0] not in VALID_X_AXIS:
        raise ValueError(f"First column must be one of {VALID_X_AXIS}, got '{header[0]}'.")

    # ---- discover Ni-field columns -----------------------------------------
    ni_values: list[str] = []
    # Mapping: y_field -> {ni -> column index}
    yfield_to_ni_indices: dict[str, dict[str, int]] = {}

    for idx in range(1, len(header)):  # skip the first column (x-axis)
        col = header[idx]
        parts = col.split('-')
        if len(parts) < 2:
            continue  # not in "<Ni>-<suffix>" form
        ni = '-'.join(parts[:-1])
        suffix = parts[-1]

        if suffix not in VALID_Y_AXIS:
            # ignore columns we don't have a mapping for
            continue

        # Ensure we have a mapping for this y-field
        if suffix not in yfield_to_ni_indices:
            yfield_to_ni_indices[suffix] = {}

        if ni in yfield_to_ni_indices[suffix]:
            raise ValueError(f"Duplicate column for ni='{ni}' in y-field='{suffix}'.")
        yfield_to_ni_indices[suffix][ni] = idx

        # Collect ni values once
        if ni not in ni_values:
            ni_values.append(ni)

    if not ni_values:
        raise ValueError("No recognized Ni-field columns found (expected suffixes: "
                         + ", ".join(VALID_Y_AXIS) + ").")

    if not yfield_to_ni_indices:
        raise ValueError("No columns matched known suffixes in Y_AXIS_MAP.")

    # ---- parse rows ---------------------------------------------------------
    # Prepare storage: for each y-field, a dict {(ni, x) -> float}
    data: dict[str, dict[(str, int), float]] = {
        y_field: {} for y_field in yfield_to_ni_indices.keys()
    }

    x_values: list[int] = []

    for row_num, row in enumerate(rows, start=1):
        # Skip empty/malformed lines
        if (not row or all(cell.strip() == '' for cell in row)) or len(row) < len(header):
            print(f"{YELLOW}Warning: line {row_num} has fewer columns than header - skipping.{RESET}")
            continue

        # Parse x value (first column)
        try:
            x = int(row[0])
        except ValueError:
            print(f"{YELLOW}Warning: cannot parse X value on line {row_num} - skipping.{RESET}")
            continue
        x_values.append(x)

        # Parse each y-field's columns
        for y_field, ni_to_idx in yfield_to_ni_indices.items():
            ni_data = data[y_field]
            for ni, col_idx in ni_to_idx.items():
                try:
                    val = float(row[col_idx])
                except (ValueError, IndexError):
                    print(f"{YELLOW}Warning: bad numeric value for {ni} on line {row_num} - skipping this row.{RESET}")
                    # Abort this row entirely – we don't have complete data for it
                    break
                ni_data[(ni, x)] = val

    # Convert to immutable tuples for the caller
    return tuple(ni_values), tuple(x_values), data


# ----------------------------------------------------------------------
# Scaling helpers
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
    
    def sx(x: Number) -> Number:
        return map_value(x, x_min, x_max, PADDING_LEFT, CANVAS_WIDTH - PADDING_RIGHT)

    def sy(y: Number) -> Number:
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
        raise ValueError("SVG limit computation: No data to compute limits.")

    min_val = min(axis_vals)
    max_val = max(axis_vals)

    # margin
    axis_range = max_val - min_val
    margin = AXIS_MARGIN_PERCENTAGE * axis_range

    return min_val - margin, max_val + margin

# ----------------------------------------------------------------------
# SVG drawing helpers
# ----------------------------------------------------------------------

def draw_grid_and_ticks(svg_parts: list[str], x_min: Number, x_max: Number, y_min: Number, y_max: Number, sx, sy) -> None:
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
        svg_parts.append(
            f'  <text x="{x_svg:.2f}" y="{sy(y_min) + TICK_SIZE + 12:.2f}" '
            f'text-anchor="middle" font-family="sans-serif" '
            f'font-size="{TICK_LABEL_FONT_SIZE}" fill="{AXIS_COLOR}">{format_val(x_val)}</text>'
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
        svg_parts.append(
            f'  <text x="{sx(x_min) - TICK_SIZE - 4:.2f}" y="{y_svg + 4:.2f}" '
            f'text-anchor="end" font-family="sans-serif" '
            f'font-size="{TICK_LABEL_FONT_SIZE}" fill="{AXIS_COLOR}">{format_val(y_val)}</text>'
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

def draw_series(svg_parts: list[str], ni_values: tuple[str], x_values: tuple[int], data: dict[(str, int), float], sx, sy) -> None:
    """
    Plot the field's values for all the series (each series gets a unique color
    according to COLOR_LIST).  
    Each value is drawn as a point (draw small circle).  
    Lines are drawn connecting consecutive points for better visibility.
    """
    for idx, ni in enumerate(ni_values):
        color = COLOR_LIST[idx]
        points = []  # Store (x, y) coordinates for this series

        # Collect points for this series in order of x values
        for i, x in enumerate(x_values):
            cx, cy = sx(x), sy(data[ni, x])
            points.append((cx, cy))
            # Draw the point
            svg_parts.append(
                f'  <circle cx="{cx:.2f}" cy="{cy:.2f}" r="{POINT_RADIUS}" '
                f'fill="{color}"/>'
            )
            # Add coordinate label above the point
            label_cx = cx + POINT_COORDINATES_LABEL_X_OFFSET
            label_cy = cy + POINT_COORDINATES_LABEL_Y_OFFSET + POINT_COORDINATES_LABEL_Y_INDEX_OFFSET * (i % 2 * 2 - 1)
            svg_parts.append(
                f'  <text x="{label_cx}" y="{label_cy}" '
                f'font-family="sans-serif" font-size="{POINT_COORDINATES_LABEL_FONT_SIZE}" fill="{color}">({x}, {format_val(data[ni, x])})</text>'
            )

        # Draw lines connecting consecutive points
        if len(points) > 1:
            for i in range(len(points) - 1):
                x1, y1 = points[i]
                x2, y2 = points[i + 1]
                svg_parts.append(
                    f'  <line x1="{x1:.2f}" y1="{y1:.2f}" '
                    f'x2="{x2:.2f}" y2="{y2:.2f}" '
                    f'stroke="{color}" stroke-width="1"/>'
                )

def draw_labels(svg_parts: list[str], labels, label_x, label_y_start, label_spacing):
    """Add series labels (Ni) in the top-right corner."""
    for idx, ni in enumerate(labels):
        label_y = label_y_start + idx * label_spacing
        svg_parts.append(
            f'  <text x="{label_x}" y="{label_y}" '
            f'font-family="sans-serif" font-size="{LABEL_FONT_SIZE}" '
            f'fill="{COLOR_LIST[idx]}">{ni}</text>')

def draw_rectangle(svg_parts: list[str], width, height, color) -> None:
    svg_parts.append(f'<rect width="{width}" height="{height}" fill="{color}"/>')

def draw_title(svg_parts: list[str], title: str, title_x, title_y, color) -> None:
    svg_parts.append(
        f'  <text x="{title_x}" y="{title_y}" '
        f'text-anchor="middle" font-family="sans-serif" '
        f'font-size="{TITLE_FONT_SIZE}" fill="{color}">{title}</text>')


# ----------------------------------------------------------------------
# SVG creation functions
# ----------------------------------------------------------------------

def make_svg_parts(ni_values: tuple[str], x_values: tuple[int], data: dict[(str, int), float], title: str) -> list[str]:
    # Produce plot's parts list
    x_min, x_max = compute_limits(x_values)
    y_min, y_max = compute_limits(data.values())
    scaling_x_func, scaling_y_func = make_scaling(x_min, x_max, y_min, y_max)

    svg_parts: list[str] = []
    svg_parts.append(SVG_HEADER_STR)

    if SVG_BACKGROUND:
        draw_rectangle(svg_parts, CANVAS_WIDTH, CANVAS_HEIGHT, SVG_BACKGROUND) # Draw background

    draw_title(svg_parts, title, CANVAS_WIDTH / 2, TITLE_OFFSET_FROM_TOP, TITLE_COLOR)

    draw_grid_and_ticks(svg_parts, x_min, x_max, y_min, y_max, scaling_x_func, scaling_y_func)
    draw_axes(svg_parts, x_min, x_max, y_min, y_max, scaling_x_func, scaling_y_func)
    draw_series(svg_parts, ni_values, x_values, data, scaling_x_func, scaling_y_func)

    # Labels for Ni series (top-right)
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


def csv2svg(csv_filepath: str) -> None:
    """
    Create an SVG file for each y-field defined in ``data``.
    """
    # Parse CSV once
    ni_values, x_values, data = read_csv(csv_filepath)

    base, _ = os.path.splitext(csv_filepath)

    for y_field, y_data in data.items():
        graph_name = GRAPH_NAME_MAP[y_field]
        out_path = base + '-' + graph_name + '.svg'
        svg_parts = make_svg_parts(ni_values, x_values, y_data, y_field)
        write_svg(out_path, svg_parts)


# ----------------------------------------------------------------------
# Main routine
# ----------------------------------------------------------------------


def main():
    parser = argparse.ArgumentParser(description='Convert CSV files to SVG plots (multiple y-fields).')
    parser.add_argument('root_dir', nargs='?', default='.', help='Root directory to search for CSV files (default: current directory)')
    args = parser.parse_args()

    root_dir = os.path.abspath(args.root_dir)

    # Walk the directory tree to find all .csv files
    csv_files = []
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.lower().endswith('.csv'):
                full_path = os.path.join(dirpath, filename)
                csv_files.append(full_path)

    # Sort for deterministic output
    csv_files.sort()

    for csv_file in csv_files:
        rel_path = os.path.relpath(csv_file, root_dir)
        print(rel_path)

        try:
            # Generate SVGs
            csv2svg(csv_file)
            print("--- SVG images were created")
        except ValueError as e:
            print(f"{RED}--- ERROR: couldn't create SVG images because {e}{RESET}")
        except Exception as e:
            print(f"{RED}--- ERROR: couldn't create SVG images because {e}{RESET}")
        print()  # blank line after each file

if __name__ == "__main__":
    main()