# Network Flow Clustering

## Description
This project implements a hierarchical agglomerative clustering algorithm for network traffic flows. It reads a dataset of network flows from a text file and iteratively groups them into a specified number of clusters based on their similarity. Similarity is determined by calculating the weighted Euclidean distance between flow characteristics.

## Features
* **Hierarchical Clustering**: Automatically merges the closest flows into clusters until the target number of clusters (N) is reached.
* **Weighted Euclidean Distance**: Calculates similarity based on four customizable weights (Bytes, Time, Delay, and Size).
* **Dynamic Memory Management**: Utilizes `malloc`, `realloc`, and `free` to efficiently handle variable amounts of flow data and IP addresses.
* **Organized Output**: Sorts the contents of each cluster internally by `flow_id`, and then sorts the clusters themselves before printing.

## Compilation
The program is written in standard C and utilizes the `<math.h>` library for square root calculations. Therefore, you must link the math library using the `-lm` flag during compilation:

```bash
gcc -std=c11 -Wall -Wextra flows.c -o flows -lm
```

## Usage
The program requires exactly 6 command-line arguments (excluding the program name itself):

```bash
./flows [FILE] [N] [WB] [WT] [WD] [WS]
```

### Arguments:
* `[FILE]`: Path to the input text file containing the flow data.
* `[N]`: The target number of clusters to form (Integer).
* `[WB]`: Weight for the total bytes difference (Float).
* `[WT]`: Weight for the flow duration difference (Float).
* `[WD]`: Weight for the average interarrival time difference (Float).
* `[WS]`: Weight for the average packet size difference (Float).

*Note: All weight arguments must be non-negative values.*

## Input File Format
The program expects a specific format for the input file. The first line must declare the total count of flows, followed by the flow data:

```text
count=[NUMBER_OF_FLOWS]
[ID] [SRC_IP] [DEST_IP] [TOTAL_BYTES] [DURATION] [PACKET_COUNT] [INTERARRIVAL_TIME]
```
*(Example IPs must be in standard IPv4 format: X.X.X.X)*

## Distance Calculation Algorithm
To determine which clusters to merge, the program calculates the Euclidean distance between flows using the following formula:

Distance = sqrt( W_B*(Delta B)^2 + W_T*(Delta T)^2 + W_D*(Delta D)^2 + W_S*(Delta S)^2 )

Where:
* **Delta B**: Difference in `total_bytes`.
* **Delta T**: Difference in `flow_duration`.
* **Delta D**: Difference in `avg_interarrival_time`.
* **Delta S**: Difference in average packet size (calculated as `total_bytes / packet_count`).

## Example Output
Upon successful execution and clustering, the program prints the resulting clusters and the IDs of the flows within them:

```text
Clusters: 
cluster 0: 1 4 5 
cluster 1: 2 
cluster 2: 3 6 
```
