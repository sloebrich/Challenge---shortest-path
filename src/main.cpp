
#include <nlohmann/json.hpp>

#include <deque>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using json = nlohmann::json;

struct Point
{
    double x;
    double y;
};

struct LabeledPoint : Point
{
    std::string label;
};

struct Gateway
{
    LabeledPoint left;
    LabeledPoint right;
};

Point readPoint(const json &j)
{
    return {
        j.at("x").get<double>(),
        j.at("y").get<double>()};
}

LabeledPoint readLabeledPoint(const json &j)
{
    return {
        readPoint(j),
        j.at("label")
            .get<std::string>()};
}

// positive iff p lies to the left when going from a to b
double crossProduct(const Point &a, const Point &b, const Point &p)
{
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

struct InputData
{
    Point start;
    Point end;
    std::vector<Gateway> gateways;
    std::vector<std::string> expectedPath;
};

InputData readInput(const std::string &filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        throw std::runtime_error("Could not open file: " + filename);
    }

    json data;
    file >> data;

    const auto start = readPoint(data.at("from"));
    const auto end = readPoint(data.at("to"));
    const auto &gatePoints = data.at("gatePoints");

    std::vector<Gateway> gateways;
    gateways.reserve(gatePoints.size() / 2 + 1);

    for (std::size_t i = 0; i < gatePoints.size(); i += 2)
    {
        Gateway gateway{
            readLabeledPoint(gatePoints.at(i)),
            readLabeledPoint(gatePoints.at(i + 1)),
        };
        gateways.push_back(gateway);
    }

    gateways.push_back(Gateway{{end, "to"}, {end, "to"}});

    std::vector<std::string>
        expectedPath;

    for (const auto &pointLabel : data.at("expected"))
    {
        expectedPath.push_back(pointLabel);
    }

    return InputData{start, end, std::move(gateways), std::move(expectedPath)};
}

// The point a chain vertex is reached from: the vertex before it, or the apex for the first one.
const Point &predecessor(const std::deque<LabeledPoint> &queue, const LabeledPoint &apex)
{
    return queue.size() >= 2 ? queue[queue.size() - 2] : apex;
}

std::vector<std::string> findPath(const InputData &data)
{
    const auto &gateways = data.gateways;
    std::vector<std::string> path;

    // Funnel algorithm: the apex plus a chain of gateway points on each side (leftQueue, rightQueue).
    // A chain holds the points the shortest path to the newest left/right gateway point bends around.
    // Each point enters and leaves a chain at most once, so the run time is linear.
    // The last gateway is the destination (added in readInput).
    LabeledPoint apex = {data.start, "from"};
    std::deque<LabeledPoint> leftQueue;
    std::deque<LabeledPoint> rightQueue;

    for (const auto &gateway : gateways)
    {

        // Keep the left chain convex (turning left only).
        while (!leftQueue.empty() && crossProduct(predecessor(leftQueue, apex), leftQueue.back(), gateway.left) <= 0)
        {
            leftQueue.pop_back();
        }

        // The new left point is beyond apex -> first_right_point: the path must bend here.
        // It becomes the new apex and the left chain starts over.
        while (!rightQueue.empty() && crossProduct(apex, rightQueue.front(), gateway.left) <= 0)
        {
            apex = rightQueue.front();
            rightQueue.pop_front();
            path.push_back(apex.label);
            leftQueue.clear();
        }

        // Keep the right chain convex (turning right only).
        while (!rightQueue.empty() && crossProduct(predecessor(rightQueue, apex), rightQueue.back(), gateway.right) >= 0)
        {
            rightQueue.pop_back();
        }

        // The new right point is beyond apex -> first_left_point: the path must bend here.
        // It becomes the new apex and the right chain starts over.
        while (!leftQueue.empty() && crossProduct(apex, leftQueue.front(), gateway.right) >= 0)
        {
            apex = leftQueue.front();
            leftQueue.pop_front();
            path.push_back(apex.label);
            rightQueue.clear();
        }

        leftQueue.emplace_back(gateway.left);
        rightQueue.emplace_back(gateway.right);
    }
    return path;
}

void printPath(const std::vector<std::string> &path)
{
    for (std::size_t i = 0; i < path.size(); ++i)
    {
        if (i > 0)
            std::cout << '-';
        std::cout << path[i];
    }
    std::cout << '\n';
}

bool runTest(const std::string &filename, const std::vector<std::string> &path, const std::vector<std::string> &expectedPath)
{
    bool passed = path == expectedPath;

    if (passed)
    {
        std::cout << "PASS: " << filename << '\n';
    }
    else
    {
        std::cout << "FAIL: " << filename << '\n';
        std::cout << "Expected path (" << expectedPath.size() << " points):\n";

        printPath(expectedPath);

        std::cout << "Actual path (" << path.size() << " points):\n";

        printPath(path);
    }

    return passed;
}

int main(int argc, char *argv[])
{
    std::string filename = "tests/challenge_shortest_path.json";

    if (argc > 1)
    {
        filename = argv[1];
    }

    auto data = readInput(filename);
    auto path = findPath(data);
    auto passed = runTest(filename, path, data.expectedPath);

    return passed ? 0 : 1;
}
