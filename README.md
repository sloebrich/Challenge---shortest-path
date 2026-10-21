# Challenge: Shortest Path

A C++17 implementation of the **funnel algorithm** for finding a shortest path through an ordered sequence of gateways in a geometric environment.

The program takes a start point, a destination, and a series of gateways defined by pairs of points. It tightens a funnel between the gateway boundaries and reports the **gateway points the shortest path bends around**, in travel order.

## Features

- **Shortest-path computation:** Finds the corners of the shortest path through an ordered sequence of gateways.
- **Funnel algorithm:** Uses cross products and two double-ended queues to maintain the funnel boundaries, and runs in linear time (see [Performance](#performance)).
- **JSON input:** Reads start and destination coordinates, labelled gateway points, and the expected result from a JSON file.
- **Label-based results:** The path is reported as the labels of the gateway points where it bends. Start and destination are not part of it.
- **Automated validation:** Compares the calculated path against the expected one, reports `PASS` or `FAIL`, and sets the exit status accordingly.
- **Test suite:** A collection of JSON test cases in `tests/`, from a single corner up to corridors with more than 200 gateways.
- **Visualizer:** A standalone HTML page, `visualizer.html`, that draws a test file: start, destination, gateways, and the expected route (see [Visualizer](#visualizer)).
- **Configurable input:** Accepts a custom JSON file as a command-line argument.

## Requirements

- A C++ compiler with C++17 support
- [CMake](https://cmake.org/) 3.16 or later
- [nlohmann/json](https://github.com/nlohmann/json)

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/sloebrich/Challenge---shortest-path.git
cd Challenge---shortest-path
```

### 2. Install dependencies

On Ubuntu or Debian:

```bash
sudo apt update
sudo apt install build-essential cmake nlohmann-json3-dev
```

On macOS with [Homebrew](https://brew.sh/) (the Xcode command line tools provide the compiler):

```bash
brew install cmake nlohmann-json
```

### 3. Build the project

Configure the project:

```bash
cmake -S . -B build
```

Compile the executable:

```bash
cmake --build build
```

The executable is generated at `build/marine-routing`.

This default configuration compiles without optimizations. For large inputs, configure with a release build instead:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

## Usage

Run the program from the repository root with the default test input:

```bash
./build/marine-routing
```

By default, the program reads:

```text
tests/challenge_shortest_path.json
```

The path is relative to the working directory, so the default only works when the program is started from the repository root.

To use a different JSON input file, pass its path as an argument:

```bash
./build/marine-routing path/to/input.json
```

The program is a test runner. It prints `PASS` if the calculated path matches the expected path, or `FAIL` if it does not:

```text
PASS: tests/challenge_shortest_path.json
```

On failure, the expected and the actual path are printed for comparison, with the labels joined by `-` and the number of corners in parentheses:

```text
FAIL: path/to/input.json
Expected path (1 points):
D
Actual path (2 points):
D-E
```

The process exits with status code `0` on success and `1` when the path comparison fails. If the input file cannot be opened, the program aborts with an error message.

## Input Format

The input is a JSON object with four fields:

| Field | Description |
|---|---|
| `from` | Starting point, represented by `x` and `y` coordinates |
| `to` | Destination point, represented by `x` and `y` coordinates |
| `gatePoints` | Ordered array of labelled gateway boundary points, supplied in pairs |
| `expected` | Array of strings: the labels of the gateway points the shortest path bends around, in travel order |

Each point contains numeric `x` and `y` properties. Each gateway point also contains a `label` string.

`expected` lists only the corners. It contains neither `from` nor `to`, and it is an empty array (`[]`) when the straight line from `from` to `to` passes through every gateway.

### Conventions

- **Pairs:** Consecutive gateway points form a gateway. The first point of a pair is the **left** end and the second is the **right** end, as seen when travelling from `from` towards `to`.
- **Orientation:** Coordinates use the usual mathematical orientation (x to the right, y up). Travelling in the positive x direction, left is the positive y direction.
- **Order:** Gateways are processed in the order they appear in `gatePoints`, which must be the order of the intended route.
- **Distinct points:** No point may occur twice among `from`, `to`, and the gateway points. A gateway therefore cannot have zero width, and the start or destination cannot lie on a gateway point. Use a very narrow gateway, or place the point next to the gateway, instead. 
- **Unique labels:** Labels should be unique, so that `expected` identifies each gateway point unambiguously.
- **Even count:** `gatePoints` must contain an even number of points because the implementation processes them in pairs.

### Example

This is equivalent to `tests/one_corner.json`:

```json
{
  "from": { "x": 0, "y": 0 },
  "to": { "x": 10, "y": 0 },
  "gatePoints": [
    { "label": "A", "x": 2, "y": 2 },
    { "label": "B", "x": 2, "y": -2 },
    { "label": "C", "x": 4, "y": 5 },
    { "label": "D", "x": 4, "y": 3 }
  ],
  "expected": ["D"]
}
```

Travelling east from `(0, 0)`, the first gateway A–B at `x = 2` is wide enough for a straight line. The second gateway C–D at `x = 4` only spans `y = 3` to `y = 5`, so the straight line along `y = 0` misses it. The shortest path runs from the start to D and on to the destination, bending around the right end D of the second gateway. The expected result is therefore `["D"]`.

## How It Works

The algorithm maintains a funnel consisting of an apex and two chains of gateway points, one for the left side and one for the right side. Each chain is stored in a double-ended queue (`std::deque`) and, seen from the apex, is a convex line that hugs its side of the corridor.

1. **Initialize the funnel:** The start point is the initial apex, and both chains are empty. The destination is appended to the gateways as one more gateway whose left and right points are both the destination.
2. **Process gateways:** The gateways are processed once, in order. Each one supplies a new left and a new right point.
3. **Tighten a chain:** While the new left point lies on or to the right of the last edge of the left chain, which runs from the second-to-last chain point (or the apex) to the last chain point, the last point is removed from the chain. The right chain is tightened the same way, mirrored.
4. **Add corners:** While the new left point lies on or to the right of the line from the apex to the first point of the right chain, the funnel is pinched shut on that side, and that right point is a corner. Its label is added to the result, it becomes the new apex, and it is removed from the right chain. The left chain is cleared, because its points are now behind the apex. The same check, mirrored, runs for the new right point against the left chain.
5. **Extend the chains:** The new left and right points are appended to the ends of their chains.

Because the destination is the last gateway, the same steps decide whether it is visible from the apex or whether further corners are needed first. The destination itself is never added to the result.

The result is an ordered list of the labels of the corner gateway points. The start and the destination are not part of it.

## Performance

The algorithm runs in linear time, O(n) in the number of gateways. Every gateway point is appended to a chain once and removed from it at most once, and the funnel never goes back to gateways it has already processed after finding a corner.

As a stress test, an auto-generated input with 1 million gateways that make a 180° turn around a circular left wall (about 235,000 corners) took 11 s to run with the default build. Almost all of that is spent parsing the roughly 150 MB JSON file, not computing the path:

| Build | Total | JSON parsing | Funnel computation |
|---|---|---|---|
| Default (no optimizations) | about 11 s | about 10.8 s | about 0.14 s |
| Release (`-O3 -DNDEBUG`, see [Build the project](#3-build-the-project)) | about 1.3 s | about 1.1 s | about 0.015 s |

The parsing time is high because the JSON library is header-only and heavily templated, so without optimizations its many small functions are not inlined. The input is parsed completely into memory first, which took about 1 GB for this file.

## Project Structure

```text
Challenge---shortest-path/
├── src/
│   └── main.cpp
├── tests/
│   ├── challenge_shortest_path.json
│   └── ...
├── CMakeLists.txt
├── visualizer.html
├── README.md
└── .gitignore
```

- `src/main.cpp` — JSON parsing, geometric calculations, funnel algorithm, and test execution.
- `tests/` — Test input data containing gateway definitions and expected paths (see [Testing](#testing)).
- `CMakeLists.txt` — CMake configuration for compiling the executable.
- `visualizer.html` — Browser-based viewer for the JSON test files (see [Visualizer](#visualizer)).
- `README.md` — This documentation.
- `.gitignore` — Specifies files excluded from version control.

## Testing

Run the default test:

```bash
./build/marine-routing
```

Run a specific test file:

```bash
./build/marine-routing tests/one_corner.json
```

Run all tests. Each prints its own `PASS` or `FAIL` line, and a `failed:` line is added for every file that fails:

```bash
for f in tests/*.json; do ./build/marine-routing "$f" || echo "failed: $f"; done
```

The test runner compares the labels of the calculated path with `expected`. The two lists must have the same length, and the labels must match exactly, in order.

### Test cases

| Group | Files | What they cover |
|---|---|---|
| Basic | `straight`, `no_gates`, `one_corner`, `two_corners`, `alternating_turns`, `single_corner_left`, `single_corner_right` | No corner, no gateways at all, single corners on either side, and bends that alternate between left and right |
| Gateway geometry | `fan_left`, `fan_right`, `pinch_points`, `pinch_chain_12`, `narrowing`, `widening` | Gateways that rotate around one corner, very narrow gateways (0.1 wide) the path has to squeeze through, and 60 gateways that shrink or grow around a straight path |
| Start or destination next to a gateway point | `start_near_left_vertex`, `start_near_right_vertex`, `end_near_left_vertex`, `end_near_right_vertex` | The start or destination close to, but not on, a gateway point, which forces a corner there |
| Many gateways | `slalom_12`, `hairpin`, `spiral`, `serpentine`, `serpentine_utm`, `zigzag`, `walk_200` | 12 gateways with a corner at each one, a 180° U-turn (29 gateways), an inward spiral (118 gateways, 115 corners), an S-shaped corridor (79 gateways), the same corridor at UTM-scale coordinates, and two random walks (61 and 201 gateways) |
| Reference example | `challenge_shortest_path` | The default input: 12 gateways and 5 corners |

Cases that exist in a mirrored version, such as `spiral` and `spiral_mirror`, are the same corridor flipped along the x axis (`y` becomes `-y`) with left and right swapped, so that a mistake on only one side shows up. Other left/right pairs are named `_left` and `_right`.

The expected paths of the large cases were generated with a separate brute-force shortest-path computation that is independent of the funnel implementation.

## Visualizer

`visualizer.html` is a single-file web page that draws a test file in 2D, which makes it easier to see what a test case looks like and why its expected path bends where it does. It needs no build step, no server, and no dependencies. Open it in a browser:

```bash
open visualizer.html        # macOS
xdg-open visualizer.html    # Linux
```

Then click **Choose JSON file** and select a file from `tests/`, or click **Load sample** to see a built-in example. The file is read locally in the browser and is not uploaded anywhere.

The scene shows:

- the start point (`from`, green) and the destination (`to`, red)
- every gateway as a grey line between its two labelled points
- the expected route (blue), drawn as a line from the start through the gateway points listed in `expected` to the destination

**Fit view** resets the view to the whole scene. The side panel shows the number of gateway points and corners, and the detected JSON structure. If a label in `expected` does not match any gateway point, the visualizer reports the missing labels.

The visualizer only draws the route stored in `expected`. It does not run the funnel algorithm, so it does not show the calculated path and cannot tell whether a test passes. Use the program for that (see [Testing](#testing)).

## Limitations

- Gateway points must be provided in the correct order and in left/right pairs.
- `from`, `to`, and all gateway points must be pairwise distinct, and labels should be unique.
- Left and right assume y-up coordinates. With y-down coordinates, such as screen space, the two sides are swapped.
- Exactly collinear situations are not handled symmetrically. If a gateway point lies exactly on the straight line between its neighbouring path points, it may or may not be reported as a corner, depending on whether it is a left or a right point. The test cases avoid such inputs.
- Input files must follow the expected JSON structure. A malformed file aborts the program with an exception.
- The program processes one input file per execution and parses it completely into memory before computing the path.
