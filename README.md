# Challenge: Shortest Path

A C++17 implementation of the **funnel algorithm** for finding a shortest path through an ordered sequence of gateways in a geometric environment.

The program takes a start point, a destination, and a series of gateways defined by pairs of points. It computes a path by tightening a funnel between the gateway boundaries and adding intermediate waypoints when necessary.

## Features

- **Shortest-path computation:** Calculates a path through an ordered sequence of gateways.
- **Funnel algorithm:** Uses geometric orientation tests and cross products to update the funnel boundaries.
- **JSON input:** Reads start and end coordinates, gateway points, and expected results from a JSON file.
- **Automated validation:** Compares the calculated path against an expected path and reports whether the test passes.
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

## Usage

Run the program with the default test input:

```bash
./build/marine-routing
```

By default, the program reads:

```text
tests/challenge_shortest_path.json
```

To use a different JSON input file, pass its path as an argument:

```bash
./build/marine-routing path/to/input.json
```

The program prints `PASS` if the calculated path matches the expected path, or `FAIL` if it does not. On failure, the expected and actual coordinates are printed for comparison.

The process exits with status code `0` on success and `1` when the path comparison fails.

## Input Format

The input is a JSON object with four fields:

| Field | Description |
|---|---|
| `from` | Starting point, represented by `x` and `y` coordinates |
| `to` | Destination point, represented by `x` and `y` coordinates |
| `gatePoints` | Ordered array of gateway boundary points, supplied in pairs |
| `expected` | Array of points representing the expected path |

Each point contains numeric `x` and `y` properties. Each gateway point also contains a `label` string.

Consecutive gateway points form a gateway pair. The points must be supplied in the correct order for the intended route.

### Example

```json
{
  "from": {
    "x": 0,
    "y": 0
  },
  "to": {
    "x": 10,
    "y": 10
  },
  "gatePoints": [
    {
      "label": "A",
      "x": 2,
      "y": 1
    },
    {
      "label": "B",
      "x": 2,
      "y": 3
    },
    {
      "label": "C",
      "x": 6,
      "y": 5
    },
    {
      "label": "D",
      "x": 6,
      "y": 7
    }
  ],
  "expected": [
    {
      "x": 0,
      "y": 0
    },
    {
      "x": 10,
      "y": 10
    }
  ]
}
```

This example illustrates the input structure. Replace the `expected` coordinates with the correct path for your geometry to create a passing test.

**Note:** `gatePoints` must contain an even number of points because the implementation processes them in pairs.

## How It Works

The algorithm maintains a funnel consisting of an apex and two boundaries: the left and right sides.

1. **Initialize the funnel:** The start point is used as the initial apex and both boundaries.
2. **Process gateways:** Each gateway supplies new left and right boundary points.
3. **Tighten the funnel:** Cross products determine whether the new boundaries can narrow the existing funnel.
4. **Add waypoints:** When a boundary crosses the opposite side, the algorithm adds the appropriate boundary as a waypoint and resets the funnel.
5. **Complete the path:** The destination is appended if it is not already the final point.

The result is an ordered list of coordinates representing the calculated path.

## Project Structure

```text
Challenge---shortest-path/
├── src/
│   └── main.cpp
├── tests/
│   └── challenge_shortest_path.json
├── CMakeLists.txt
└── .gitignore
```

- `src/main.cpp` — JSON parsing, geometric calculations, funnel algorithm, and test execution.
- `tests/` — Test input data containing gateway definitions and expected paths.
- `CMakeLists.txt` — CMake configuration for compiling the executable.
- `.gitignore` — Specifies files excluded from version control.

## Testing

Run the default test:

```bash
./build/marine-routing
```

Run a specific test file:

```bash
./build/marine-routing tests/challenge_shortest_path.json
```

The test runner compares the number of points in the calculated and expected paths and checks corresponding coordinates using a tolerance of `1e-6`.

## Limitations

- Gateway points must be provided in the correct order and in left/right pairs.
- Input files must follow the expected JSON structure.
- The program processes one input file per execution.

