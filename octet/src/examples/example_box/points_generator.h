#pragma once

namespace octet {
  class points_generator {
    std::vector<vec3> waypoints;
    std::vector<vec3> sorted_waypoints;

    float random_float(float a, float b) {
      float random = ((float)rand()) / (float)RAND_MAX;
      float diff = b - a;
      float r = random * diff;
      return a + r;
    }

    float get_distance(vec3 a, vec3 b) {
      return sqrt(((b[0] - a[0]) * (b[0] - a[0])) // x
        + ((b[1] - a[1]) * (b[1] - a[1])) // y
        + ((b[2] - a[2]) * (b[2] - a[2]))); // z
    }

    void sort_waypoints() {
      // sorting points by closest distance to each other
      sorted_waypoints.push_back(waypoints.back());
      waypoints.pop_back();
      float shortest_dist, distance2;
      int point_index;

      while (!waypoints.empty()) {
        shortest_dist = 0.0f;
        distance2 = 0.0f;
        point_index = 0;

        for (int i = 0; i < waypoints.size(); i++) {
          distance2 = get_distance(sorted_waypoints.back(), waypoints[i]);
          if (distance2 < shortest_dist || shortest_dist <= 0) {
            shortest_dist = distance2;
            point_index = i;
          }
        }
        sorted_waypoints.push_back(waypoints[point_index]);
        waypoints.erase(waypoints.begin() + point_index);
      }
    }

    void average_waypoints() {
      // Averaging points back into original vector.
      for (int i = 0; i < sorted_waypoints.size(); i++) {
        if ((i + 2) == sorted_waypoints.size()) {
          waypoints.push_back((sorted_waypoints[i] + sorted_waypoints[i + 1] + sorted_waypoints[0]) / 3);
        }
        else if ((i + 1) == sorted_waypoints.size()) {
          waypoints.push_back((sorted_waypoints[i] + sorted_waypoints[0] + sorted_waypoints[1]) / 3);
        }
        else {
          waypoints.push_back((sorted_waypoints[i] + sorted_waypoints[i + 1] + sorted_waypoints[i + 2]) / 3);
        }
      }
    }
  public:
    points_generator() {}

    std::vector<vec3> generate_random_points(int num_points) {
      std::srand(std::time(nullptr));
      //std::srand(0);
      waypoints = std::vector<vec3>();
      sorted_waypoints = std::vector<vec3>();
      // set the number of points you want generated
      for (int i = 0; i < num_points; i++) {
        waypoints.push_back(vec3(random_float(-1, 1), random_float(-1, 1), 0));
      }
      sort_waypoints();
      average_waypoints();
      return waypoints;
    }
  };
}