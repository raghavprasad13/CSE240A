import os
import pandas as pd
from collections import defaultdict


def parse_output(output):
    lines = output.strip().split("\n")
    mispred_line = lines[-1]
    mispred_rate = mispred_line.split(":")[-1].strip()
    return float(mispred_rate)


traces = ["fp_1.bz2", "fp_2.bz2", "int_1.bz2", "int_2.bz2", "mm_1.bz2", "mm_2.bz2"]
data = defaultdict(list)

# gshare
ghistorybits_inputs = [5, 10, 13, 15, 20]
for ghistorybits in ghistorybits_inputs:
    for trace in traces:
        stream = os.popen(
            f"bunzip2 -kc ../traces/{trace} | ./predictor --gshare:{ghistorybits}"
        )
        output = stream.read()
        mispred_rate = parse_output(output)
        data["ghistoryBits"].append(ghistorybits)
        data["trace"].append(trace.split(".")[0])
        data["misprediction rate"].append(mispred_rate)

df = pd.DataFrame(data=data)
df.to_csv("gshare_data.csv", index=False)

# tournament
tournament_inputs = [
    (5, 10, 10),
    (9, 10, 10),
    (15, 5, 5),
    (15, 10, 10),
    (15, 20, 20),
    (15, 40, 40),
    (25, 10, 10),
    (25, 20, 20),
    (15, 5, 15),
    (15, 15, 5),
    (15, 20, 10),
]

data = defaultdict(list)
for ghistoryBits, lhistoryBits, pcIndexBits in tournament_inputs:
    for trace in traces:
        stream = os.popen(
            f"bunzip2 -kc ../traces/{trace} | ./predictor --tournament:{ghistoryBits}:{lhistoryBits}:{pcIndexBits}"
        )
        output = stream.read()
        mispred_rate = parse_output(output)
        data["tournament input"].append(
            str(ghistoryBits) + ":" + str(lhistoryBits) + ":" + str(pcIndexBits)
        )
        data["trace"].append(trace.split(".")[0])
        data["misprediction rate"].append(mispred_rate)

df = pd.DataFrame(data=data)
df.to_csv("tournament_data.csv", index=False)

perceptron_inputs = [
    (64, 7),
    (128, 7),
    (256, 7),
    (64, 15),
    (128, 15),
    (256, 15),
    (64, 31),
    (128, 31),
    (256, 31),
]
data = defaultdict(list)

for num_perceptrons, hist_len in perceptron_inputs:
    for trace in traces:
        stream = os.popen(
            f"bunzip2 -kc ../traces/{trace} | ./predictor --custom:{num_perceptrons}:{hist_len}"
        )
        output = stream.read()
        mispred_rate = parse_output(output)
        data["num_perceptrons:perceptron_history_length"].append(
            str(num_perceptrons) + ":" + str(hist_len)
        )
        data["trace"].append(trace.split(".")[0])
        data["misprediction rate"].append(mispred_rate)

df = pd.DataFrame(data=data)
df.to_csv("perceptron_data.csv", index=False)
