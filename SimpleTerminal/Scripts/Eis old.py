import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
import argparse

def plotFromCSV(filepath, outputpath="", dpi=100, xlabel='X', ylabel='Y', ylabel2='Y2', 
                 ylim=None, ylim2=None, label_fontsize=12, tick_fontsize=10, 
                 title='', title_fontsize=14):
    """
    Plot a dual-axis error bar graph from a CSV file.
    
    Expected CSV columns: x, y, yerr, y2, y2err
    
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
    label_fontsize : int
        Font size for axis labels (default: 12)
    tick_fontsize : int
        Font size for tick labels (default: 10)
    title : str
        Title for the figure (default: '')
    title_fontsize : int
        Font size for the title (default: 14)
    """
    
    # Read the CSV file
    df = pd.read_csv(filepath)
    
    # Extract columns
    x = df['x'].values
    y = df['y'].values
    yerr = df['yerr'].values
    y2 = df['y2'].values
    y2err = df['y2err'].values
    
    # Create figure and first axis
    fig, ax1 = plt.subplots(dpi=dpi)
    
    # Plot first dataset on left y-axis
    ax1.errorbar(x=x, y=y, yerr=yerr, fmt='bo-', capsize=2, elinewidth=1.1)
    ax1.set_xlabel(xlabel, fontsize=label_fontsize)
    ax1.set_ylabel(ylabel, color='tab:blue', fontsize=label_fontsize)
    ax1.tick_params(axis='y', labelcolor='tab:blue', labelsize=tick_fontsize)
    ax1.tick_params(axis='x', labelsize=tick_fontsize)
    ax1.set_xscale('log')
    ax1.set_yscale('log')
    
    # Set y1 limits
    if ylim is None:
        y_with_err = y + yerr
        ylim_max = np.max(y_with_err)
        ax1.set_ylim(1, ylim_max * 1.1)
    else:
        ax1.set_ylim(ylim)
    
    # Create second axis
    ax2 = ax1.twinx()
    
    # Plot second dataset on right y-axis
    ax2.errorbar(x=x, y=y2, yerr=y2err, fmt='r^--', capsize=2, elinewidth=0.9)
    ax2.set_ylabel(ylabel2, color='tab:red', fontsize=label_fontsize)
    ax2.tick_params(axis='y', labelcolor='tab:red', labelsize=tick_fontsize)
    
    # Set y2 limits
    if ylim2 is not None:
        ax2.set_ylim(ylim2)
    
    # Make right-axis y-ticks appear every 10 units
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
        print(f"Saved plot to {outputpath}")


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
            # Handle both space-separated (e.g., -90 0) and comma-separated (e.g., -90,0 or --ylim2=-90,0)
            if len(args.ylim2) == 1:
                # Single argument, try splitting by comma
                ylim2 = tuple(map(float, args.ylim2[0].split(',')))
            else:
                # Multiple arguments, treat as space-separated
                ylim2 = tuple(map(float, args.ylim2))
            if len(ylim2) != 2:
                raise ValueError("Expected exactly 2 values")
        except ValueError as e:
            print(f"Error: Invalid format for ylim2, expected 'min max' or 'min,max' - {e}")
            exit(1)
    
    plotFromCSV(args.filepath, args.output, dpi=args.dpi, xlabel=args.xlabel, 
                ylabel=args.ylabel, ylabel2=args.ylabel2, ylim=ylim, ylim2=ylim2, 
                label_fontsize=args.label_fontsize, tick_fontsize=args.tick_fontsize, 
                title=args.title, title_fontsize=args.title_fontsize)
