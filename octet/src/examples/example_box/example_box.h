////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include <stdlib.h>
#include <ctime>
#include "points_generator.h"


namespace octet {
  /// Scene containing a box with octet.
  class example_box : public app {

    enum curve_mode {
      QUADRATIC_BEZIER = 0,
      CUBIC_BEZIER,
      CATMULL_ROM
    };

    curve_mode current_curve;

    bool debug_mode = true;

    //quadratic = 2, cubic = 3
    int curve_step = 2;


    // scene for drawing box
    ref<visual_scene> app_scene;

    std::vector<std::tuple<vec3, vec3>> input;
    std::vector<float> vertBuff;
    std::vector<vec3> debugBezBuff; // Used to show the actual bezier path with debug lines
    GLuint vertex_buffer;
    shader road_shader;

    // Used to load shader files into a string varaible
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

    points_generator pg;
    int num_points;
    std::vector<vec3> waypoints;

  public:
    /// this is called when we construct the class before everything is initialised.
    example_box(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();

      current_curve = QUADRATIC_BEZIER;

      glGenBuffers(1, &vertex_buffer); // Sets up our vertex array buffer for rendering
      road_shader.init(load_file("shaders/road.vert").c_str(), load_file("shaders/road.frag").c_str()); // loads, compiles and links our shader programs

      switch (current_curve) {
        case QUADRATIC_BEZIER:
          curve_step = 2;
          break;
        case CUBIC_BEZIER:
          curve_step = 3;
          break;
        case CATMULL_ROM:
          curve_step = 1;
          break;
      }

      // initialise random
      std::srand(std::time(0));

      // create points for curves
      num_points = curve_step * 10 + 1;
      waypoints = pg.generate_random_points(num_points);

      float TRACK_WIDTH = 0.1f;
      float DETAIL_STEP = 0.01f;

      debugBezBuff = std::vector<vec3>();
      vertBuff = std::vector<float>();

      for (int i = 0; i < waypoints.size(); i += curve_step) {
        for (float t = 0.0f; t <= 1.0f; t += DETAIL_STEP) {
          vec3 pos = get_bezier_point(t, i);
          vec3 segment_pos = get_bezier_point(t + DETAIL_STEP * 0.1f, i);
          vec3 tan = segment_pos - pos;
          vec3 norm = tan.cross(vec3(0, 0, 1)); // Get normal from tangent.

          norm = norm.normalize() * TRACK_WIDTH * 0.5f; // Create track radius

          vec3 p1 = pos - norm; // Calculate border vertex locations
          vec3 p2 = pos + norm;

          vertBuff.push_back(p1[0]); // Add vertex data (3 Floats (x, y and y)) to the buffer
          vertBuff.push_back(p1[1]); // The buffer is used by opengl to render the triangles
          vertBuff.push_back(p1[2]);
          vertBuff.push_back(p2[0]);
          vertBuff.push_back(p2[1]);
          vertBuff.push_back(p2[2]);

          debugBezBuff.push_back(pos);
        }
      }

      printf("Created curve with %d points", num_points);
    }

    vec3 get_bezier_point(float t, int iter) {

      vec3 point(0, 0, 0);

      //sorted some variables 
      float tt = t * t;
      float ttt = t * t * t;
      float u = (1 - t);
      float uu = u * (1 - t);
      float uuu = uu *(1 - t);

      int idx = iter;
      int idx1 = iter + 1;
      int idx2 = iter + 2; 
      int idx3 = iter + 3;
      if ((waypoints.size() <= (iter))) {
        idx = 0;
        idx1 = 1;
        idx2 = 2;
        idx3 = 3;
      }
      else if ((waypoints.size() <= (iter+1))) {
        idx1 = 0;
        idx2 = 1;
        idx3 = 2;
      }
      else if ((waypoints.size() <= (iter+2))) {
        idx2 = 0;
        idx3 = 1;
      }
      else if ((waypoints.size() <= (iter + 3))) {
        idx3 = 0;
      }

      if ((waypoints.size() <= (iter))) {
        printf("Lookup vertex outside of waypoints range\n");
      }
      else if ((waypoints.size() <= (iter + 1))) {
        printf("Lookup vertex outside of waypoints range + 1\n");
      }
      else if ((waypoints.size() <= (iter + 2))) {
        printf("Lookup vertex outside of waypoints range + 2\n");
      }
      else if ((waypoints.size() <= (iter + 3))) {
        printf("Lookup vertex outside of waypoints range + 3\n");
      }

      switch (current_curve) {

        case QUADRATIC_BEZIER:

          point[0] = uu * waypoints[idx][0] + 2 * u * t * waypoints[idx1][0] + tt * 0.5f *(waypoints[idx1][0] + waypoints[idx2][0]);
          point[1] = uu * waypoints[idx][1] + 2 * u * t * waypoints[idx1][1] + tt * 0.5f *(waypoints[idx1][1] + waypoints[idx2][1]);

          break;

        case CUBIC_BEZIER:

          point[0] = uuu * waypoints[idx][0] + 3 * uu * t * waypoints[idx1][0] + 3 * u * tt* waypoints[idx2][0] + ttt * 0.5f*(waypoints[idx2][0] + waypoints[idx3][0]);
          point[1] = uuu * waypoints[idx][1] + 3 * uu * t * waypoints[idx1][1] + 3 * u * tt* waypoints[idx2][1] + ttt * 0.5f*(waypoints[idx2][1] + waypoints[idx3][1]);

          break;

        case CATMULL_ROM:
          
          point[0] = 0.5f * ((-t) * uu * waypoints[idx][0] + (2 - 5 * tt + 3 * ttt) * waypoints[idx1][0] + t * (1 + 4 * t - 3 * tt) * waypoints[idx2][0] - tt * u * waypoints[idx3][0]);
          point[1] = 0.5f * ((-t) * uu * waypoints[idx][1] + (2 - 5 * tt + 3 * ttt) * waypoints[idx1][1] + t * (1 + 4 * t - 3 * tt) * waypoints[idx2][1] - tt * u * waypoints[idx3][1]);

          break;
      }

      return point;
    }


    //  if (iter == (waypoints.size() - curve_step)) {
    //    if (curve_step == 2) {
    //      vec3 endpoint((waypoints[iter] + waypoints[0]) / 2);
    //      //Quadratic Bezier
    //      point[0] = uu * waypoints[iter][0] + 2 * u * t * endpoint[0] + tt * waypoints[0][0];
    //      point[1] = uu * waypoints[iter][1] + 2 * u * t * endpoint[1] + tt * waypoints[0][1];

    //       //Catmull-Rom
    //      point[0] = half * ((-t) * uu * waypoints[iter][0] + (2 - 5 * tt +3 * ttt) * waypoints[0][0] + t * (1 + 4 * t - 3 * tt ) * waypoints[1][0] - tt * u * waypoints[2][0]);
    //      point[1] = half * ((-t) * uu * waypoints[iter][1] + (2 - 5 * tt + 3 * ttt) * waypoints[0][1] + t * (1 + 4 * t - 3 * tt) * waypoints[1][1] - tt * u * waypoints[2][1]);
    //    }
    //    else if (curve_step == 3) {
    //      //formula of Cubic Bezier 
    //      point[0] = uuu * waypoints[iter][0] + 3 * uu * t * waypoints[0][0] + 3 * u * tt* waypoints[1][0] + ttt*half*(waypoints[1][0] + waypoints[iter][0]);
    //      point[1] = uuu * waypoints[iter][1] + 3 * uu * t * waypoints[0][1] + 3 * u * tt* waypoints[1][1] + ttt*half*(waypoints[1][1] + waypoints[iter][1]);
    //      //point[2] = uuu * waypoints[iter][2] + 3 * uu * t * waypoints[0][2] + 3 * u * tt* waypoints[1][2] + ttt* waypoints[2][2];
    //    }
    //  }
    //  else {
    //    if (curve_step == 2) {
    //      //Quadratic Bezier
    //    // point[0] = uu * waypoints[iter][0] + 2 * u * t * waypoints[iter + 1][0] + tt * half *(waypoints[iter + 1][0] + waypoints[iter + 2][0]);
    //    // point[1] = uu * waypoints[iter][1] + 2 * u * t * waypoints[iter + 1][1] + tt * half *(waypoints[iter + 1][1] + waypoints[iter + 2][1]);

    //   // point[0] = uu * waypoints[iter][0] + 2 * u * t * waypoints[iter + 1][0] + tt * waypoints[iter + 2][0];
    //  //  point[1] = uu * waypoints[iter][1] + 2 * u * t * waypoints[iter + 1][1] + tt * waypoints[iter + 2][1];

    //    //Catmul-Rom
    //      point[0] = half * ((-t) * uu * waypoints[iter][0] + (2 - 5 * tt + 3 * ttt) * waypoints[iter + 1][0] + t * (1 + 4 * t - 3 * tt) * waypoints[iter + 2][0] - tt * u * waypoints[iter + 3][0]);
    //      point[1] = half * ((-t) * uu * waypoints[iter][1] + (2 - 5 * tt + 3 * ttt) * waypoints[iter + 1][1] + t * (1 + 4 * t - 3 * tt) * waypoints[iter + 2][1] - tt * u * waypoints[iter + 3][1]);
    //    }
    //    else if (curve_step == 3) {
    //      //formula of Cubic Bezier 
    //      point[0] = uuu * waypoints[iter][0] + 3 * uu * t * waypoints[iter + 1][0] + 3 * u * tt* waypoints[iter + 2][0] + ttt * half*(waypoints[iter + 2][0] + waypoints[iter+3][0]);
    //      point[1] = uuu * waypoints[iter][1] + 3 * uu * t * waypoints[iter + 1][1] + 3 * u * tt* waypoints[iter + 2][1] + ttt* half*(waypoints[iter + 2][1] + waypoints[iter+3][1]);
    //      //point[2] = uuu * waypoints[iter][2] + 3 * uu * t * waypoints[iter + 1][2] + 3 * u * tt* waypoints[iter + 2][2] + ttt* waypoints[iter + 3][2];
    //    }
    //  }
    //  return point;

    //}


    // DEPRECATED USING    get_bezier_point(t + 0.01f, i) - get_bezier_point(t, i)  TO CALCULATE TANGENT

    //vec3 get_bezier_tangent(float t, int iter) {
    //  //P(1)1 = (1 − t)P0 + tP1   (= P0 + t(P1 − P0))
    //  //P(1)2 = (1 − t)P1 + tP2   (= P1 + t(P2 - P1))
    //  vec3 P11, P12;
    //  if (iter == (waypoints.size() - curve_step)) {
    //    vec3 endpoint((waypoints[iter] + waypoints[0]) / 2);
    //    P11 = waypoints[iter] + t * (waypoints[0] - endpoint);
    //    P12 = waypoints[iter + 1] + t * (waypoints[0] - endpoint);
    //    //vec3 P13 = waypoints[2] + t * (waypoints[3] - waypoints[2]);
    //  }
    //  else {
    //    P11 = waypoints[iter] + t * (waypoints[iter + 1] - waypoints[iter]);
    //    P12 = waypoints[iter + 1] + t * (waypoints[iter + 2] - waypoints[iter + 1]);
    //    //vec3 P13 = waypoints[2] + t * (waypoints[3] - waypoints[2]);
    //  }
    //  vec3 tan = P12 - P11;
    //  return tan;
    //}



    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);


      // update matrices. assume 30 fps.
      app_scene->update(1.0f / 30);

      // draw the scene
      app_scene->render((float)vx / vy);

      if (debug_mode) {
        draw_debug();
      }else{
        // openGL takes in an array of floats. Every 3 floats represents one vertex. 
        // Bellow is code telling opengl what float vertex data to use.
        // openGL reads the raw bytes in memory, so we need to tell it how many bytes per value (in this case float 4 bytes) 
        // and we also need to tell it how many values per vertex (in this case 3 for x, y and z)
        // We then tell openGL what shader program to use to render the mesh 
        // and we specify the render mode, here, GL_TRIANGLE_STRIP tells opengl to make the vertex data connect up into a mesh like this:
        //  The numbers represent the vertices, each vertex is three floats wide (z,y,z)
        //
        //   0-----2-----4
        //   |    /|    /|
        //   |   / |   / |
        //   |  /  |  /  |
        //   | /   | /   |
        //   |/    |/    |
        //   1-----3-----5

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertBuff.size() * sizeof(GLfloat), &vertBuff[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(attribute_pos);
        glUseProgram(road_shader.get_program());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertBuff.size() / 3);
        glBindVertexArray(attribute_pos);
      }
    }

    void draw_debug() {
      /* https://en.wikibooks.org/wiki/OpenGL_Programming/GLStart/Tut3 */

      // Draw the waypoints
      glUseProgram(0);
      glColor3f(1.0f, 0.0f, 0.0f); //red colour
      glPointSize(5.0f);//set point size to 10 pixels
      glBegin(GL_POINTS); //starts drawing of points
      for (vec3 &point : waypoints) {
        glVertex3f(point[0], point[1], point[2]);
      }
      glEnd();//end drawing of points

      // Draw the bezzier line.
      glColor3f(0.0f, 1.0f, 0.0f); //green colour
      glBegin(GL_LINE_STRIP); //starts drawing of line_strip
      for (int i = 0; i < debugBezBuff.size() - 1; i++) {
        glVertex3f(debugBezBuff[i][0], debugBezBuff[i][1], debugBezBuff[i][2]);
        glVertex3f(debugBezBuff[i+1][0], debugBezBuff[i+1][1], debugBezBuff[i+1][2]);
      }
      glEnd();//end drawing of Line_strip
      
    }
  };
}
