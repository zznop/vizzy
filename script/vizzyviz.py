"""Script for analyzing and visualizing vizzy heap trace files
"""

import argparse
from collections import OrderedDict
from bokeh.plotting import figure, show

TOOLS = 'pan,wheel_zoom,box_zoom,reset,save'

def heap_summary(dataset: OrderedDict):
    """Analyzes the dataset and prints a heap summary

    Iterates through the ordered dictionary and keeps track of allocations and frees in order.
    Prints report on number of allocations, number of frees, total bytes allocated, and memory
    that hasn't been freed on exit.

    Args:
      dataset: Ordered dictionary containing data parsed from vizzy trace file
    """

    allocnum = 0
    freenum = 0
    allocs = OrderedDict()
    total_bytes_allocated = 0
    for _, data in dataset.items():
        if data['function'] == 'free':
            freenum += 1
            if data['address'] in allocs:
                del allocs[data['address']]
            else:
                print('Double free @ {:08x}?'.format(data['address']))
        else:
            allocnum += 1
            total_bytes_allocated += data['size']
            allocs[data['address']] = data['size']

    # Compute blocks in use at exit
    in_use_bytes = 0
    in_use_blocks = 0
    for _, size in allocs.items():
        in_use_blocks += 1
        in_use_bytes += size

    print('''HEAP SUMMARY:
    in use at exit   : {} bytes in {} blocks
    total heap usage : {} allocs, {} frees, {} bytes allocated'''.format(
        in_use_bytes, in_use_blocks, allocnum, freenum, total_bytes_allocated))

def viz_heap_usage_over_time(dataset: OrderedDict):
    """Generate a line graph SVG showing heap memory consumption over time

    Processes dataset from the vizzy trace file and creates a SVG line graph that shows heap memory
    consumption over time. Displays the SVG in the user's default browser.

    Args:
      dataset: Ordered dictionary containing data parsed from vizzy trace file
    """

    curr_allocs = OrderedDict()
    curr_memusage = 0
    times = []
    memusage = []
    for time, data in dataset.items():
        if data['function'] == 'free':
            if data['address'] in curr_allocs:
                curr_memusage -= curr_allocs[data['address']]
                del curr_allocs[data['address']]
        else:
            curr_allocs[data['address']] = data['size']
            curr_memusage += data['size']

        times.append(time)
        memusage.append(curr_memusage/1024.0)
    fig = figure(title="Memory Usage over Time", x_axis_label='Time (seconds)',
                 y_axis_label='Memory (kB)', tools=TOOLS)
    fig.line(times, memusage, line_width=0.5)
    show(fig)

def parse_vizzy_file(tracefile: str) -> OrderedDict:
    """Iterate through trace file lines and parse data into an ordered dictionary

    Converts the nanoseconds to seconds and adds to seconds field to get absolute time of traced
    event in seconds (as a float)

    Args:
      tracefile: Path to vizzy trace file

    Returns:
      Ordered dictionary of parsed data
    """

    dataset = OrderedDict()
    with open(tracefile, 'r') as _file:
        first_line = True
        starttime = 0
        for line in _file.readlines():
            line = line.strip()
            elements = line.split(',')

            seconds = float(elements[0])
            nanoseconds = float(elements[1]) / 1000000.0
            abstime = seconds+nanoseconds

            function = elements[2]
            address = int(elements[3], 16)
            size = None
            if elements[4]:
                size = int(elements[4])

            reltime = 0
            if first_line:
                starttime = abstime
                first_line = False
            else:
                reltime = abstime-starttime

            dataset[reltime] = dict(
                function=function,
                address=address,
                size=size,
            )

    return dataset

def parse_args() -> argparse.Namespace:
    """Parse command line arguments
    """

    parser = argparse.ArgumentParser()
    parser.add_argument('tracefile', help='File path to vizzy heap trace file')
    parser.add_argument('--mem-usage', action='store_true',
                        help='Visualize heap memory usage over time')
    return parser.parse_args()

def main():
    """Parse args, load the heap trace, and generate visualizations
    """

    args = parse_args()
    dataset = parse_vizzy_file(args.tracefile)
    if args.mem_usage:
        viz_heap_usage_over_time(dataset)
    else:
        heap_summary(dataset)


if __name__ == '__main__':
    main()
