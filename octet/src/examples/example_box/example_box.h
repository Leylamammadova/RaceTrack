////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include <stdlib.h>
#include <ctime>


namespace octet {
  /// Scene containing a box with octet.
  class example_box : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    std::vector<std::tuple<vec3, vec3>> input;
    std::vector<float> vertBuff;
    GLuint vertex_buffer;
    shader road_shader;

    std::string load_file(const char* file_name) {
      std::ifstream is(file_name);
      if (is.bad() || !is.is_open()) return nullptr;
      char buffer[2048];
      // loop over lines
      std::string out;
      while (!is.eof()) {
        is.getline(buffer, sizeof(buffer));
        out += buffer;
        out += "\n";
      }
      //printf("%s", out.c_str());
      return out;
    }

    float RandomFloat(float a, float b) {
      float random = ((float)rand()) / (float)RAND_MAX;
      float diff = b - a;
      float r = random * diff;
      return a + r;
    }

    /// Vector of points
    int num_points;
    std::vector<vec3> waypoints;
    std::vector<vec3> sorted_waypoints;


  public:
    /// this is called when we construct the class before everything is initialised.
    example_box(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();

      glGenBuffers(1, &vertex_buffer); // Sets up our vertex array buffer for rendering
      road_shader.init(load_file("shaders/tree.vert").c_str(), load_file("shaders/tree.frag").c_str()); // loads, compiles and links our shader programs

      //random points
      std::srand(std::time(0));

      // set the number of points you want generated
      num_points = 13;
      for (int i = 0; i < num_points; i++) {
        waypoints.push_back(vec3(RandomFloat(-1, 1), RandomFloat(-1, 1), 0));
      }

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

      float TRACK_WIDTH = 0.1f;
      float DETAIL_STEP = 0.00001f;

      //input = std::vector<std::tuple<vec3, vec3>>(5);
      //input[0] = std::tuple<vec3, vec3>(vec3(-0.75f, -0.5f, 0), vec3(-1, 0, 0));
      //input[1] = std::tuple<vec3, vec3>(vec3(-0.5f, 0, 0), vec3(-1, 1, 0));
      //input[2] = std::tuple<vec3, vec3>(vec3(0, 0.5, 0), vec3(0, 1, 0));
      //input[3] = std::tuple<vec3, vec3>(vec3(0.5f, 0, 0), vec3(1, 1, 0));
      //input[4] = std::tuple<vec3, vec3>(vec3(0.75f, -0.5f, 0), vec3(1, 0, 0));

      vertBuff = std::vector<float>();

      for (float t = 0.0f; t <= 1.0f; t += DETAIL_STEP) {
        vec3 pos = get_bezier_point(t);
        vec3 tan = get_bezier_tangent(t);
        vec3 norm = tan.cross(vec3(0, 0, 1)); // Get normal from tangent.
        /*vec3 pos = std::get<0>(point_norm);
        vec3 norm = std::get<1>(point_norm);*/
        norm = norm.normalize() * TRACK_WIDTH * 0.5f; // Create track radius
        vec3 p1 = pos - norm; // Calculate border vertex locations
        vec3 p2 = pos + norm;
        vertBuff.push_back(p1[0]); // Add vertex data (3 Floats (x, y and y)) to the buffer
        vertBuff.push_back(p1[1]);
        vertBuff.push_back(p1[2]);
        vertBuff.push_back(p2[0]);
        vertBuff.push_back(p2[1]);
        vertBuff.push_back(p2[2]);
      }

      /*for (float &f : vertBuff) {
        printf("%f \n", f);
      }*/



      /*material *red = new material(vec4(1, 0, 0, 1));
      mesh *box = new mesh_box(vec3(4));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, red));*/
    }

    float get_distance(vec3 a, vec3 b) {
      return sqrt(((b[0] - a[0]) * (b[0] - a[0]))
        + ((b[1] - a[1]) * (b[1] - a[1]))
        + ((b[2] - a[2]) * (b[2] - a[2])));
    }

    vec3 get_bezier_point(float t) {
      vec3 point(0, 0, 0);
      point[0] = (1 - t) * (1 - t) * waypoints[0][0] + 2 * (1 - t) * t * waypoints[1][0] + t * t * waypoints[2][0];
      point[1] = (1 - t) * (1 - t) * waypoints[0][1] + 2 * (1 - t) * t * waypoints[1][1] + t * t * waypoints[2][1];
      return point;
    }
    vec3 get_bezier_tangent(float t) {
      //P(1)1 = (1 − t)P0 + tP1   (= P0 + t(P1 − P0))
      //P(1)2 = (1 − t)P1 + tP2   (= P1 + t(P2 - P1))

      vec3 P11 = waypoints[0] + t * (waypoints[1] - waypoints[2]);
      vec3 P12 = waypoints[1] + t * (waypoints[2] - waypoints[1]);

      vec3 tan = P12 - P11;
      return tan;
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);



      // update matrices. assume 30 fps.
      app_scene->update(1.0f / 30);

      // draw the scene
      app_scene->render((float)vx / vy);

      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER, vertBuff.size() * sizeof(GLfloat), &vertBuff[0], GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(attribute_pos);

      glUseProgram(road_shader.get_program());
      glDrawArrays(GL_TRIANGLE_STRIP, 0, vertBuff.size() / 3);
      glBindVertexArray(attribute_pos);
    }
  };
}
