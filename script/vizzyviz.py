import argparse
import csv
from collections import OrderedDict
from bokeh.plotting import figure, show

TOOLS = "pan,wheel_zoom,box_zoom,reset,save"

def heap_summary(dataset):
    allocnum = 0
    freenum = 0
    allocs = OrderedDict()
    total_bytes_allocated = 0
    for time, data in dataset.items():
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
        in_use_bytes, in_use_blocks, allocnum, freenum, total_bytes_allocated)
    )

def user_address_range():
    user_range = []
    for i in range(0, 0x800000000000, 0x20000000000):
        user_range.append(i)

    return user_range

def viz_heap_usage_over_time(dataset):
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
    p = figure(title="Memory Usage over Time", x_axis_label='Time (seconds)', y_axis_label='Memory (kB)', tools=TOOLS)
    p.line(times, memusage, line_width=0.5)
    show(p)

def parse_vizzy_file(tracefile):
    """Iterate through CSV rows
    """

    dataset = OrderedDict()
    with open(tracefile, 'r') as f:
        linenum = 0
        starttime = 0
        for line in f.readlines():
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
            if linenum == 0:
                starttime = abstime
            else:
                reltime = abstime-starttime

            dataset[reltime] = dict(
                function=function,
                address=address,
                size=size,
            )

            linenum += 1

    return dataset

def parse_args():
    """Parse command line arguments
    """

    parser = argparse.ArgumentParser()
    parser.add_argument('tracefile', help='File path to vizzy heap trace file')
    return parser.parse_args()

def main():
    """Parse args, load the heap trace, and generate visualizations
    """

    args = parse_args()
    dataset = parse_vizzy_file(args.tracefile)
    heap_summary(dataset)
    viz_heap_usage_over_time(dataset)

if __name__ == '__main__':
    main()
