
#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <vector>

using json = nlohmann::json;

struct Point
{
    double x;
    double y;
};

struct Gateway
{
    std::string label;
    Point left;
    Point right;
};

Point readPoint(const nlohmann::json &j)
{
    return {
        j.at("x").get<double>(),
        j.at("y").get<double>()};
}

bool samePoint(const Point &a, const Point &b)
{
    constexpr double tolerance = 1e-6;

    return std::abs(a.x - b.x) < tolerance && std::abs(a.y - b.y) < tolerance;
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
    std::vector<Point> expectedPath;
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

    for (std::size_t i = 0; i < gatePoints.size(); i += 2)
    {
        Gateway gateway{
            gatePoints.at(i).at("label").get<std::string>() + "-" + gatePoints.at(i + 1).at("label").get<std::string>(),
            readPoint(gatePoints.at(i)),
            readPoint(gatePoints.at(i + 1))};

        gateways.push_back(gateway);
    }

    gateways.push_back(Gateway{"to", end, end});

    std::vector<Point> expectedPath;

    for (const auto &point : data.at("expected"))
    {
        expectedPath.push_back(readPoint(point));
    }

    return InputData{start, end, gateways, expectedPath};
}

std::vector<Point> findPath(const InputData &data)
{
    const auto &start = data.start;
    const auto &end = data.end;
    const auto &gateways = data.gateways;

    std::vector<Point> path;
    path.push_back(start);

    auto apex = start;
    auto left = start;
    auto right = start;

    std::size_t apexIndex = 0;
    std::size_t leftIndex = 0;
    std::size_t rightIndex = 0;
    std::size_t i = 0;

    while (i < gateways.size())
    {
        bool apexNeedsUpdate = false;
        const auto &newLeft = gateways[i].left;
        const auto &newRight = gateways[i].right;

        // newLeft lies to the right of apex->left, tightening the funnel.
        if (crossProduct(apex, left, newLeft) <= 0)
        {
            if (samePoint(apex, left) || crossProduct(apex, right, newLeft) > 0)
            {
                left = newLeft;
                leftIndex = i;
            }
            else
            {
                // newLeft even lies to the right of apex->right, crossing the right leg. Make a new funnel.
                apex = right;
                apexIndex = rightIndex;
                apexNeedsUpdate = true;
            }
        }
        // newRight lies to the left of apex->right, tightening the funnel.
        if (!apexNeedsUpdate && crossProduct(apex, right, newRight) >= 0)
        {
            if (samePoint(apex, right) || crossProduct(apex, left, newRight) < 0)
            {
                right = newRight;
                rightIndex = i;
            }
            else
            {
                // newRight even lies to the right of apex->left, crossing the left leg. Make a new funnel.
                apex = left;
                apexIndex = leftIndex;
                apexNeedsUpdate = true;
            }
        }
        if (apexNeedsUpdate)
        {
            left = apex;
            right = apex;
            leftIndex = apexIndex;
            rightIndex = apexIndex;

            path.push_back(apex);
            i = apexIndex + 1;
        }
        else
        {
            i++;
        }
    }

    if (!samePoint(end, path.back()))
    {
        path.push_back(end);
    }

    return path;
}

bool runTest(const std::string &filename, const std::vector<Point> &path, const std::vector<Point> &expectedPath)
{
    bool passed = path.size() == expectedPath.size();

    if (passed)
    {
        for (std::size_t i = 0; i < path.size(); ++i)
        {
            if (!samePoint(path[i], expectedPath[i]))
            {
                passed = false;
                break;
            }
        }
    }

    if (passed)
    {
        std::cout << "PASS: " << filename << '\n';
    }
    else
    {
        std::cout << "FAIL: " << filename << '\n';
        std::cout << "Expected path (" << expectedPath.size() << " points):\n";

        for (const auto &point : expectedPath)
        {
            std::cout << "(" << point.x << ", " << point.y << ")\n";
        }

        std::cout << "Actual path (" << path.size() << " points):\n";

        for (const auto &point : path)
        {
            std::cout << "(" << point.x << ", " << point.y << ")\n";
        }
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
