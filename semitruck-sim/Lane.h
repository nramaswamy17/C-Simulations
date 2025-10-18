#ifndef LANE_H
#define LANE_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include "SemiTruck.h"

// Represents a point on the road centerline
struct RoadPoint {
    float x, y;
    float angle;  // Direction at this point
    
    RoadPoint(float px, float py, float a) : x(px), y(py), angle(a) {}
};

class Lane {
public:
    std::vector<RoadPoint> centerline;  // Points defining the lane center
    float width;                         // Lane width in pixels
    int laneNumber;                      // Which lane (0=inner, 1=middle, 2=outer)
    
    sf::Color laneColor;
    sf::Color lineColor;
    
    Lane(float laneWidth, int laneNum) {
        this->width = laneWidth;
        this->laneNumber = laneNum;
        
        laneColor = sf::Color(80, 80, 80);      // Dark gray road
        lineColor = sf::Color(255, 255, 255);   // White lane markings
    }
    
    void generateOvalPath(float centerX, float centerY, float radiusX, float radiusY, 
                         float laneOffset, int numPoints = 200) {
        centerline.clear();
        
        for (int i = 0; i < numPoints; i++) {
            float theta = (2.0f * M_PI * i) / numPoints;
            
            // Base oval position
            float baseX = centerX + radiusX * std::cos(theta);
            float baseY = centerY + radiusY * std::sin(theta);
            
            // Calculate tangent angle for CLOCKWISE motion (in screen coordinates)
            float dx = -radiusX * std::sin(theta);
            float dy = radiusY * std::cos(theta);
            float angle = std::atan2(dy, dx) * 180.0f / M_PI;
            
            // Offset for this lane (perpendicular to direction)
            float perpAngle = (angle + 90.0f) * M_PI / 180.0f;
            float offsetX = laneOffset * std::cos(perpAngle);
            float offsetY = laneOffset * std::sin(perpAngle);
            
            centerline.emplace_back(baseX + offsetX, baseY + offsetY, angle);
        }
    }
    
    // Find the closest point on the centerline to the truck
    int findClosestPointIndex(const SemiTruck& truck) const {
        int closestIdx = 0;
        float minDist = std::numeric_limits<float>::max();
        
        for (size_t i = 0; i < centerline.size(); i++) {
            float dx = truck.cab_x - centerline[i].x;
            float dy = truck.cab_y - centerline[i].y;
            float dist = dx * dx + dy * dy;
            
            if (dist < minDist) {
                minDist = dist;
                closestIdx = i;
            }
        }
        
        return closestIdx;
    }
    
    // Calculate lateral error (distance from truck to lane center)
    float getLateralError(const SemiTruck& truck) const {
        if (centerline.empty()) return 0.0f;
        
        int closestIdx = findClosestPointIndex(truck);
        const RoadPoint& closest = centerline[closestIdx];
        
        // Vector from closest point to truck
        float dx = truck.cab_x - closest.x;
        float dy = truck.cab_y - closest.y;
        
        // Perpendicular direction (90 degrees from road direction)
        float perpAngle = (closest.angle + 90.0f) * M_PI / 180.0f;
        float perpX = std::cos(perpAngle);
        float perpY = std::sin(perpAngle);
        
        // Project distance onto perpendicular (positive = right of center, negative = left)
        float lateralError = dx * perpX + dy * perpY;
        
        return lateralError;
    }
    
    // Calculate heading error (angle difference from road direction)
    float getHeadingError(const SemiTruck& truck) const {
        if (centerline.empty()) return 0.0f;
        
        int closestIdx = findClosestPointIndex(truck);
        const RoadPoint& closest = centerline[closestIdx];
        
        float error = truck.cab_angle - closest.angle;
        
        // Normalize to -180 to +180
        while (error > 180.0f) error -= 360.0f;
        while (error < -180.0f) error += 360.0f;
        
        return error;
    }
    
    // Check if truck is within lane boundaries
    bool isInLane(const SemiTruck& truck) const {
        float lateralError = std::abs(getLateralError(truck));
        return lateralError < (width / 2 - 15.0f); // 15px safety margin
    }
    
    // Get distance to left lane edge
    float getDistanceToLeftEdge(const SemiTruck& truck) const {
        float lateralError = getLateralError(truck);
        return (width / 2) + lateralError;
    }
    
    // Get distance to right lane edge
    float getDistanceToRightEdge(const SemiTruck& truck) const {
        float lateralError = getLateralError(truck);
        return (width / 2) - lateralError;
    }
    
    void draw(sf::RenderWindow& window, bool isInnerEdge, bool isOuterEdge) {
        if (centerline.empty()) return;
        
        // Draw lane boundaries with thicker lines for visibility
        for (size_t i = 0; i < centerline.size(); i++) {
            size_t nextIdx = (i + 1) % centerline.size();
            
            const RoadPoint& p1 = centerline[i];
            const RoadPoint& p2 = centerline[nextIdx];
            
            // Calculate perpendicular offset for lane edges
            float perpAngle1 = (p1.angle + 90.0f) * M_PI / 180.0f;
            float perpAngle2 = (p2.angle + 90.0f) * M_PI / 180.0f;
            
            float halfWidth = width / 2;
            
            // Left edge of this lane
            if (isInnerEdge) {
                // Solid white line (innermost edge of track)
                for (int offset = -2; offset <= 2; offset++) {
                    float x1 = p1.x - std::cos(perpAngle1) * halfWidth + offset * std::cos(p1.angle * M_PI / 180.0f);
                    float y1 = p1.y - std::sin(perpAngle1) * halfWidth + offset * std::sin(p1.angle * M_PI / 180.0f);
                    float x2 = p2.x - std::cos(perpAngle2) * halfWidth + offset * std::cos(p2.angle * M_PI / 180.0f);
                    float y2 = p2.y - std::sin(perpAngle2) * halfWidth + offset * std::sin(p2.angle * M_PI / 180.0f);
                    
                    sf::Vertex line[] = {
                        sf::Vertex(sf::Vector2f(x1, y1), sf::Color::White),
                        sf::Vertex(sf::Vector2f(x2, y2), sf::Color::White)
                    };
                    window.draw(line, 2, sf::Lines);
                }
            }
            
            // Right edge of this lane
            if (isOuterEdge) {
                // Solid white line (outermost edge of track)
                for (int offset = -2; offset <= 2; offset++) {
                    float x1 = p1.x + std::cos(perpAngle1) * halfWidth + offset * std::cos(p1.angle * M_PI / 180.0f);
                    float y1 = p1.y + std::sin(perpAngle1) * halfWidth + offset * std::sin(p1.angle * M_PI / 180.0f);
                    float x2 = p2.x + std::cos(perpAngle2) * halfWidth + offset * std::cos(p2.angle * M_PI / 180.0f);
                    float y2 = p2.y + std::sin(perpAngle2) * halfWidth + offset * std::sin(p2.angle * M_PI / 180.0f);
                    
                    sf::Vertex line[] = {
                        sf::Vertex(sf::Vector2f(x1, y1), sf::Color::White),
                        sf::Vertex(sf::Vector2f(x2, y2), sf::Color::White)
                    };
                    window.draw(line, 2, sf::Lines);
                }
            } else {
                // Dashed yellow line (lane divider between lanes)
                // Draw on the right edge of non-outer lanes
                if (i % 10 < 5) {  // Dashed pattern
                    for (int offset = -1; offset <= 1; offset++) {
                        float x1 = p1.x + std::cos(perpAngle1) * halfWidth + offset * std::cos(p1.angle * M_PI / 180.0f);
                        float y1 = p1.y + std::sin(perpAngle1) * halfWidth + offset * std::sin(p1.angle * M_PI / 180.0f);
                        float x2 = p2.x + std::cos(perpAngle2) * halfWidth + offset * std::cos(p2.angle * M_PI / 180.0f);
                        float y2 = p2.y + std::sin(perpAngle2) * halfWidth + offset * std::sin(p2.angle * M_PI / 180.0f);
                        
                        sf::Vertex centerLine[] = {
                            sf::Vertex(sf::Vector2f(x1, y1), sf::Color::Yellow),
                            sf::Vertex(sf::Vector2f(x2, y2), sf::Color::Yellow)
                        };
                        window.draw(centerLine, 2, sf::Lines);
                    }
                }
            }
        }
    }
};

// Road with multiple lanes in an oval configuration
class Road {
public:
    std::vector<Lane> lanes;
    sf::Color roadColor;
    sf::Color grassColor;
    
    float centerX, centerY;
    float radiusX, radiusY;
    
    Road(float windowWidth, float windowHeight, float wallThickness) {
        roadColor = sf::Color(60, 60, 60);      // Dark gray
        grassColor = sf::Color(34, 139, 34);    // Grass green
        
        // Create an oval track around the perimeter
        centerX = windowWidth / 2;
        centerY = windowHeight / 2;
        
        // Oval dimensions (leave comfortable margin for walls and barriers)
        float margin = wallThickness + 120;  // Increased margin
        radiusX = (windowWidth / 2) - margin;
        radiusY = (windowHeight / 2) - margin;
        
        std::cout << "Creating OVAL track with center (" << centerX << ", " << centerY 
                  << "), radiusX=" << radiusX << ", radiusY=" << radiusY << std::endl;
        
        float laneWidth = 80.0f;
        
        // Create three lanes (inner, middle, outer)
        for (int i = 0; i < 3; i++) {
            Lane lane(laneWidth, i);
            
            // Offset from center: inner lane is closer to center, outer is farther
            float laneOffset = (i - 1) * laneWidth; // -1, 0, +1 * laneWidth
            
            lane.generateOvalPath(centerX, centerY, radiusX, radiusY, laneOffset);
            lanes.push_back(lane);
            
            std::cout << "  Lane " << i << " created with " << lane.centerline.size() 
                      << " points, offset=" << laneOffset << std::endl;
        }
    }
    
    void draw(sf::RenderWindow& window) {
        // Draw road background (wider oval to cover all lanes)
        sf::ConvexShape roadShape;
        int numPoints = 100;
        roadShape.setPointCount(numPoints);
        
        float totalWidth = lanes.size() * lanes[0].width;
        
        for (int i = 0; i < numPoints; i++) {
            float theta = (2.0f * M_PI * i) / numPoints;
            float expandedRadiusX = radiusX + totalWidth / 2;
            float expandedRadiusY = radiusY + totalWidth / 2;
            
            float x = centerX + expandedRadiusX * std::cos(theta);
            float y = centerY + expandedRadiusY * std::sin(theta);
            roadShape.setPoint(i, sf::Vector2f(x, y));
        }
        roadShape.setFillColor(roadColor);
        window.draw(roadShape);
        
        // Draw inner grass (center of oval)
        sf::ConvexShape grassShape;
        grassShape.setPointCount(numPoints);
        
        for (int i = 0; i < numPoints; i++) {
            float theta = (2.0f * M_PI * i) / numPoints;
            float innerRadiusX = radiusX - totalWidth / 2;
            float innerRadiusY = radiusY - totalWidth / 2;
            
            float x = centerX + innerRadiusX * std::cos(theta);
            float y = centerY + innerRadiusY * std::sin(theta);
            grassShape.setPoint(i, sf::Vector2f(x, y));
        }
        grassShape.setFillColor(grassColor);
        window.draw(grassShape);
        
        // Draw lane markings
        for (size_t i = 0; i < lanes.size(); i++) {
            bool isInnerEdge = (i == 0);
            bool isOuterEdge = (i == lanes.size() - 1);
            lanes[i].draw(window, isInnerEdge, isOuterEdge);
        }
    }
    
    // Find which lane the truck is closest to
    int getClosestLaneIndex(const SemiTruck& truck) const {
        int closestIdx = 0;
        float minError = std::abs(lanes[0].getLateralError(truck));
        
        for (size_t i = 1; i < lanes.size(); i++) {
            float error = std::abs(lanes[i].getLateralError(truck));
            if (error < minError) {
                minError = error;
                closestIdx = i;
            }
        }
        
        return closestIdx;
    }
    
    const Lane& getClosestLane(const SemiTruck& truck) const {
        return lanes[getClosestLaneIndex(truck)];
    }
};

#endif // LANE_H