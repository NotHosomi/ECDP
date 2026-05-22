import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
import argparse

def plotFromCSV(filepath, outputpath="", dpi=100, xlabel='X', ylabel='Y', ylabel2='Y2', 
                 ylim=None, ylim2=None, xscale='linear', yscale='linear', y2scale='linear',
                 label_fontsize=12, tick_fontsize=10, 
                 title='', title_fontsize=14):
    """
    Plot a dual-axis error bar graph from a CSV file.
    
    Required CSV columns: x, y
    Optional columns: yerr, y2, y2err
    
    Parameters:
    -----------
    filepath : str
        Path to the CSV file containing the data
    outputpath : str
        Optional output file path to save the plot. If empty, displays the plot.
    dpi : int
        Image resolution in dots per inch (default: 100)
    xlabel : str
        Label for x-axis (default: 'X')
    ylabel : str
        Label for left y-axis (default: 'Y')
    ylabel2 : str
        Label for right y-axis (default: 'Y2')
    ylim : tuple
        Y-axis limits as (ymin, ymax) for left axis. If None, auto-calculates.
    ylim2 : tuple
        Y-axis limits as (ymin, ymax) for right axis. If None, auto-calculates.
    xscale : str
        Scale for x-axis: 'linear' or 'log' (default: 'linear')
    yscale : str
        Scale for left y-axis: 'linear' or 'log' (default: 'linear')
    y2scale : str
        Scale for right y-axis: 'linear' or 'log' (default: 'linear')
    label_fontsize : ints
        Font size for axis labels (default: 12)
    tick_fontsize : int
        Font size for tick labels (default: 10)
    title : str
        Title for the figure (default: '')
    title_fontsize : int
        Font size for the title (default: 14)
    """
    
    # Read the CSV file
    try:
        df = pd.read_csv(filepath)
    except Exception as e:
        print(f"Error: failed to read plot file - {e}")
        exit(1)

    # x values
    if 'x' not in df.columns:
        print("Error: plot file does not contain an 'x' column")
        exit(1)
    x = df['x'].values

    # Support multiple y-series. Column naming convention:
    # - Left-axis series: base names starting with 'y' but not 'y2' (e.g. 'y', 'y1', 'y_1')
    # - Right-axis series: base names starting with 'y2' (e.g. 'y2', 'y2_1')
    # Error columns are the base name with 'err' appended (e.g. 'yerr', 'y1err', 'y2err')
    cols = list(df.columns)
    cols.remove('x')

    # Build set of bases (without trailing 'err')
    bases = {}
    for c in cols:
        if c.endswith('err'):
            base = c[:-3]
            bases.setdefault(base, {})['err'] = c
        else:
            bases.setdefault(c, {})['val'] = c

    left_bases = [b for b in bases.keys() if b.startswith('y') and not b.startswith('y2') and 'val' in bases[b]]
    right_bases = [b for b in bases.keys() if b.startswith('y2') and 'val' in bases[b]]

    # Prepare series lists
    left_series = []
    for base in left_bases:
        val_col = bases[base].get('val')
        err_col = bases[base].get('err')
        y_vals = df[val_col].values
        y_errs = df[err_col].values if err_col is not None else None
        left_series.append((base, y_vals, y_errs))

    right_series = []
    for base in right_bases:
        val_col = bases[base].get('val')
        err_col = bases[base].get('err')
        y2_vals = df[val_col].values
        y2_errs = df[err_col].values if err_col is not None else None
        right_series.append((base, y2_vals, y2_errs))

    if len(left_series) == 0:
        print("Error: plot file has no valid y")
        exit(1)
    
    # Create figure and first axis
    fig, ax1 = plt.subplots(dpi=dpi)
    
    # Plot left-axis series (support multiple)
    ax1.set_xlabel(xlabel, fontsize=label_fontsize)
    ax1.set_ylabel(ylabel, color='tab:blue', fontsize=label_fontsize)
    ax1.tick_params(axis='y', labelcolor='tab:blue', labelsize=tick_fontsize)
    ax1.tick_params(axis='x', labelsize=tick_fontsize)
    ax1.set_xscale(xscale)
    ax1.set_yscale(yscale)

    # color/marker cycles
    colors = plt.rcParams['axes.prop_cycle'].by_key().get('color', ['b', 'r', 'g', 'c', 'm', 'y', 'k'])
    markers = ['o', 's', '^', 'D', 'v', 'P', 'X']

    paired = (len(left_series) > 1 and len(right_series) > 1)
    single_pair = (len(left_series) == 1 and len(right_series) == 1)

    for i, (base, y_vals, y_errs) in enumerate(left_series):
        if single_pair:
            color = 'tab:blue'
        elif paired:
            color = colors[i % len(colors)]
        else:
            color = colors[i % len(colors)]
        marker = markers[i % len(markers)]
        if y_errs is not None:
            ax1.errorbar(x=x, y=y_vals, yerr=y_errs, fmt=f'{marker}-', color=color, capsize=2, elinewidth=1.1, label=base)
        else:
            ax1.plot(x, y_vals, marker + '-', color=color, label=base)
    if len(left_series) > 1:
        ax1.legend(loc='upper left')
    
    # Validate log scales
    if xscale == 'log' and np.any(x <= 0):
        raise ValueError("x contains non-positive values but xscale='log'")

    if yscale == 'log':
        for base, y_vals, y_errs in left_series:
            if np.any(y_vals <= 0):
                raise ValueError(f"Left series '{base}' contains non-positive values but yscale='log'")
            if y_errs is not None and np.any(y_vals - y_errs <= 0):
                raise ValueError(f"Left series '{base}' has errors that reach non-positive values for yscale='log'")

    # Set y1 limits (auto across all left series if not provided)
    if ylim is None:
        mins = []
        maxs = []
        for _, y_vals, y_errs in left_series:
            if y_errs is not None:
                mins.append(np.min(y_vals - y_errs))
                maxs.append(np.max(y_vals + y_errs))
            else:
                mins.append(np.min(y_vals))
                maxs.append(np.max(y_vals))
        ymin = np.min(mins)
        ymax = np.max(maxs)
        if yscale == 'log':
            ax1.set_ylim(ymin, ymax * 1.1)
        else:
            pad = (ymax - ymin) * 0.1 if ymax != ymin else ymax * 0.1
            ax1.set_ylim(ymin - pad, ymax + pad)
    else:
        ax1.set_ylim(ylim)
    
    ax2 = None
    if len(right_series) > 0:
        # Create second axis and plot multiple series
        ax2 = ax1.twinx()
        for i, (base, y2_vals, y2_errs) in enumerate(right_series):
            # Determine color: paired -> match left by index; single_pair -> red; else follow cycle offset
            if single_pair:
                color = 'tab:red'
            elif paired:
                color = colors[i % len(colors)]
            else:
                color = colors[(i + len(left_series)) % len(colors)]
            # use triangle marker for right-axis series
            marker = '^'
            if y2_errs is not None:
                ax2.errorbar(x=x, y=y2_vals, yerr=y2_errs, fmt=f'{marker}--', color=color, capsize=2, elinewidth=0.9, label=base)
            else:
                ax2.plot(x, y2_vals, marker + '--', color=color, label=base)
        ax2.set_ylabel(ylabel2, color='tab:red', fontsize=label_fontsize)
        ax2.tick_params(axis='y', labelcolor='tab:red', labelsize=tick_fontsize)
        ax2.set_yscale(y2scale)
        if len(right_series) > 1:
            ax2.legend(loc='upper right')
    
    # Set y2 limits
    if len(right_series) > 0 and ylim2 is not None:
        ax2.set_ylim(ylim2)
    
    # Make right-axis y-ticks appear every 10 units
    if len(right_series) > 0:
        y2_min, y2_max = ax2.get_ylim()
        start = np.floor(y2_min / 10.0) * 10
        end = np.ceil(y2_max / 10.0) * 10
        yticks = np.arange(start, end + 0.1, 10)
        ax2.set_yticks(yticks)
        ax2.tick_params(axis='y', labelsize=tick_fontsize)
    
    # Add title if provided
    if len(title) > 0:
        fig.suptitle(title, fontsize=title_fontsize)
    
    fig.tight_layout()
    
    # Save or show
    if len(outputpath) == 0:
        plt.show()
    else:
        plt.savefig(outputpath, dpi=dpi)


# Main execution
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Plot dual-axis error bar graph from CSV file')
    parser.add_argument('filepath', help='Path to the CSV file containing the data')
    parser.add_argument('-o', '--output', default='', help='Output file path to save the plot')
    parser.add_argument('--dpi', type=int, default=100, help='Image resolution in dpi (default: 100)')
    parser.add_argument('--xlabel', default='X', help='Label for x-axis (default: X)')
    parser.add_argument('--ylabel', default='Y', help='Label for left y-axis (default: Y)')
    parser.add_argument('--ylabel2', default='Y2', help='Label for right y-axis (default: Y2)')
    parser.add_argument('--ylim', nargs='+', help='Left y-axis limits as space-separated min max (e.g., 1 10000 or --ylim=1,10000)')
    parser.add_argument('--ylim2', nargs='+', help='Right y-axis limits as space-separated min max (e.g., -90 0 or --ylim2=-90,0)')
    parser.add_argument('--xscale', choices=['linear', 'log'], default='linear', help='Scale for x-axis (linear or log, default: linear)')
    parser.add_argument('--yscale', choices=['linear', 'log'], default='linear', help='Scale for left y-axis (linear or log, default: linear)')
    parser.add_argument('--y2scale', choices=['linear', 'log'], default='linear', help='Scale for right y-axis (linear or log, default: linear)')
    parser.add_argument('--label-fontsize', type=int, default=12, help='Font size for axis labels (default: 12)')
    parser.add_argument('--tick-fontsize', type=int, default=10, help='Font size for tick labels (default: 10)')
    parser.add_argument('--title', default='', help='Figure title (default: empty)')
    parser.add_argument('--title-fontsize', type=int, default=14, help='Font size for title (default: 14)')
    
    args = parser.parse_args()
    
    if not os.path.isfile(args.filepath):
        print(f"Error: File '{args.filepath}' not found.")
        exit(1)
    
    # Parse ylim if provided
    ylim = None
    if args.ylim:
        try:
            # Handle both space-separated (e.g., -90 0) and comma-separated (e.g., -90,0 or --ylim=-90,0)
            if len(args.ylim) == 1:
                # Single argument, try splitting by comma
                ylim = tuple(map(float, args.ylim[0].split(',')))
            else:
                # Multiple arguments, treat as space-separated
                ylim = tuple(map(float, args.ylim))
            if len(ylim) != 2:
                raise ValueError("Expected exactly 2 values")
        except ValueError as e:
            print(f"Error: Invalid format for ylim, expected 'min max' or 'min,max' - {e}")
            exit(1)
    
    # Parse ylim2 if provided
    ylim2 = None
    if args.ylim2:
        try:
            if len(args.ylim2) == 1:
                ylim2 = tuple(map(float, args.ylim2[0].split(',')))
            else:
                ylim2 = tuple(map(float, args.ylim2))
            if len(ylim2) != 2:
                raise ValueError("Expected exactly 2 values")
        except ValueError as e:
            print(f"Error: Invalid format for ylim2, expected 'min max' or 'min,max' - {e}")
            exit(1)
    
    plotFromCSV(args.filepath, args.output, dpi=args.dpi, xlabel=args.xlabel, 
                ylabel=args.ylabel, ylabel2=args.ylabel2, ylim=ylim, ylim2=ylim2, 
                xscale=args.xscale, yscale=args.yscale, y2scale=args.y2scale,
                label_fontsize=args.label_fontsize, tick_fontsize=args.tick_fontsize, 
                title=args.title, title_fontsize=args.title_fontsize)
