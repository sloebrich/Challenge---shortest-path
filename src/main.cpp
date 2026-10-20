
#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <deque>

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

    return InputData{start, end, gateways, expectedPath};
}

std::vector<std::string> findPath(const InputData &data)
{
    const LabeledPoint &start = {data.start, "from"};
    const LabeledPoint &end = {data.end, "to"};
    const auto &gateways = data.gateways;

    std::vector<std::string> path;

    const LabeledPoint *apex = &start;
    std::deque<const LabeledPoint *> leftQueue;
    std::deque<const LabeledPoint *> rightQueue;

    for (auto const &gateway : gateways)
    {

        // gateway.left lies to the right of apex->lastLeft, tightening the funnel.
        while (!leftQueue.empty() && crossProduct(leftQueue.size() >= 2 ? *leftQueue[leftQueue.size() - 2] : *apex, *leftQueue.back(), gateway.left) <= 0)
        {
            leftQueue.pop_back();
        }

        // gateway.left even lies to the right of apex->lastRight, crossing the right leg. Make a new funnel.
        while (!rightQueue.empty() && crossProduct(*apex, *rightQueue.front(), gateway.left) <= 0)
        {
            auto lastRight = rightQueue.front();
            rightQueue.pop_front();
            apex = lastRight;
            path.push_back(apex->label);
            leftQueue.clear();
        }

        // gateway.right lies to the left of apex->lastRight, tightening the funnel.
        while (!rightQueue.empty() && crossProduct(rightQueue.size() >= 2 ? *rightQueue[rightQueue.size() - 2] : *apex, *rightQueue.back(), gateway.right) >= 0)
        {
            rightQueue.pop_back();
        }

        // gateway.right even lies to the left of apex->lastLeft, crossing the left leg. Make a new funnel.
        while (!leftQueue.empty() && crossProduct(*apex, *leftQueue.front(), gateway.right) >= 0)
        {
            auto lastLeft = leftQueue.front();
            leftQueue.pop_front();
            apex = lastLeft;
            path.push_back(apex->label);
            rightQueue.clear();
        }

        leftQueue.emplace_back(&gateway.left);
        rightQueue.emplace_back(&gateway.right);
    }

    // auto left = start;
    // auto right = start;

    // std::size_t apexIndex = 0;
    // std::size_t leftIndex = 0;
    // std::size_t rightIndex = 0;
    // std::size_t i = 0;

    // while (i <= gateways.size())
    // {
    //     std::cout << i << '\n';
    //     bool apexNeedsUpdate = false;
    //     const auto &newLeft = i == gateways.size() ? end : gateways[i].left;
    //     const auto &newRight = i == gateways.size() ? end : gateways[i].right;

    //     // newLeft lies to the right of apex->left, tightening the funnel.
    //     if (crossProduct(apex, left, newLeft) <= 0)
    //     {
    //         if (apex.label == left.label || crossProduct(apex, right, newLeft) > 0)
    //         {
    //             left = newLeft;
    //             leftIndex = i;
    //         }
    //         else
    //         {
    //             // newLeft even lies to the right of apex->right, crossing the right leg. Make a new funnel.
    //             apex = right;
    //             apexIndex = rightIndex;
    //             apexNeedsUpdate = true;
    //         }
    //     }
    //     // newRight lies to the left of apex->right, tightening the funnel.
    //     if (!apexNeedsUpdate && crossProduct(apex, right, newRight) >= 0)
    //     {
    //         if (apex.label == right.label || crossProduct(apex, left, newRight) < 0)
    //         {
    //             right = newRight;
    //             rightIndex = i;
    //         }
    //         else
    //         {
    //             // newRight even lies to the right of apex->left, crossing the left leg. Make a new funnel.
    //             apex = left;
    //             apexIndex = leftIndex;
    //             apexNeedsUpdate = true;
    //         }
    //     }
    //     if (apexNeedsUpdate)
    //     {
    //         left = apex;
    //         right = apex;
    //         leftIndex = apexIndex;
    //         rightIndex = apexIndex;

    //         if (apex.label != end.label)
    //         {
    //             path.push_back(apex.label);
    //         }
    //         i = apexIndex + 1;
    //     }
    //     else
    //     {
    //         i++;
    //     }
    // }

    return path;
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

        for (const auto &s : expectedPath)
        {
            if (&s != &expectedPath.front())
                std::cout << '-';
            std::cout << s;
        }
        std::cout << '\n';

        std::cout << "Actual path (" << path.size() << " points):\n";

        for (const auto &s : path)
        {
            if (&s != &path.front())
                std::cout << '-';
            std::cout << s;
        }
        std::cout << '\n';
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
