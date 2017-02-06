////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include <stdlib.h>
#include <ctime>
#include "points_generator.h"
#include "perlin.h"


namespace octet {
  /// Scene containing a box with octet.
  class example_box : public app {
  private:

    enum curve_mode {
      QUADRATIC_BEZIER = 0,
      CUBIC_BEZIER,
      CATMULL_ROM
    };
    curve_mode current_curve;
    curve_mode prev_curve;

    bool debug_mode = true;

    //// scene for drawing box
    //ref<visual_scene> app_scene;

    std::vector<std::tuple<vec3, vec3>> input;
    std::vector<float> vertBuff;
    std::vector<int> faceBuff;
    std::vector<vec3> debugBezBuff; // Used to show the actual bezier path with debug lines
    GLuint vertex_buffer;
    shader road_shader;

    perlin perlin_noise;
    points_generator pg;
    std::vector<vec3> waypoints;

    float TRACK_WIDTH = 0.1f;
    float DETAIL_STEP = 0.01f;
    float height_scale = 0.5f;
    int track_length = 10;
    int curve_step;


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

    void refresh_curve() {
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

      perlin_noise = perlin();

      // create points for curves
      int num_points = curve_step * track_length + 1;
      waypoints = std::vector<vec3>();
      waypoints = pg.generate_random_points(num_points);


      debugBezBuff = std::vector<vec3>();
      vertBuff = std::vector<float>();
      faceBuff = std::vector<int>();
      int vertPair = 0;

      // This is to avoid an infinite loop when adding points...
      // ...to the vector during the loop.
      int track_size = waypoints.size();

      for (int i = 0; i < track_size; i += curve_step) {
        // Connecting the start and the end points of the track...
        // ...by generating points.
        if ((i == track_size - 1) 
          && (current_curve == QUADRATIC_BEZIER || CUBIC_BEZIER))
        {
          vec3 v_point = (waypoints[i] + waypoints[0]) / 2;
          waypoints.push_back(v_point);
          if (current_curve == CUBIC_BEZIER) {
            v_point = (waypoints[i+1] + waypoints[0]) / 2;
            waypoints.push_back(v_point);
          }
          v_point = waypoints[0];
          waypoints.push_back(v_point);
        }

        // Getting rid of end points when changing to CATMULL_ROM
        if ((i == track_size - 1) 
          && (prev_curve == QUADRATIC_BEZIER || CUBIC_BEZIER) 
          && (current_curve == CATMULL_ROM))
        {  
          waypoints.pop_back();
          waypoints.pop_back();
          if (prev_curve == CUBIC_BEZIER) {
            waypoints.pop_back();
          }
        }

        for (float t = 0.0f; t <= 1.0f; t += DETAIL_STEP) {
          vec3 pos = get_bezier_point(t, i);
          vec3 segment_pos = get_bezier_point(t + DETAIL_STEP * 0.01f, i);
          vec3 tan = segment_pos - pos;
          vec3 norm = tan.cross(vec3(0, 0, 1)); // Get normal from tangent.

          double n = (float)perlin_noise.noise((double)pos[0], (double)pos[1], 0.0) * height_scale;

          norm = norm.normalize() * TRACK_WIDTH * 0.5f; // Create track radius

          vec3 p1 = pos - norm; // Calculate border vertex locations
          vec3 p2 = pos + norm;

          vertBuff.push_back(p1[0]); // Add vertex data (3 Floats (x, y and y)) to the buffer
          vertBuff.push_back(p1[1]); // The buffer is used by opengl to render the triangles
          vertBuff.push_back(n); // Use the perlin height at the center of the track for this point along the track.
          vertBuff.push_back(p2[0]);
          vertBuff.push_back(p2[1]);
          vertBuff.push_back(n);

          if (vertPair > 0) {
            faceBuff.push_back(vertPair * 2 - 2);
            faceBuff.push_back(vertPair * 2 - 1);
            faceBuff.push_back(vertPair * 2);

            faceBuff.push_back(vertPair * 2 - 1);
            faceBuff.push_back(vertPair * 2 + 1);
            faceBuff.push_back(vertPair * 2);
          }
          vertPair++;

          debugBezBuff.push_back(pos);
        }
      }
      /*
          float TRACK_WIDTH = 0.1f;
    float DETAIL_STEP = 0.01f;
    float height_scale = 0.5f;
    int track_length = 10;
    int curve_step;
      */
      printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
      printf("RACE TRACK\n_____________________\nTrack width: %f\nMesh Detail: %f\nHeight Scale: %f\nTrack Length: %d\n_____________________\n", TRACK_WIDTH, DETAIL_STEP, height_scale, track_length);
      printf("Mesh with %d vertices\n", (int)vertBuff.size() / 3);
      printf("%d total faces\n", (int)faceBuff.size() / 3);
    }

    vec3 get_bezier_point(float t, int iter) {
      vec3 point(0, 0, 0);

      // Glitch fix
      if (t > 1.0f) {
        //printf("Tangent calculation glitching over into next points group\n"); 
        t = t - 1.0f;
        iter += curve_step;
      }

      //sorted some variables 
      float tt = t * t;
      float ttt = t * t * t;
      float u = (1 - t);
      float uu = u * u;
      float uuu = u * u * u;

      // Repoints to the front of the waypoints list if iter + n exceeds vector bounds
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
      else if ((waypoints.size() <= (iter + 1))) {
        idx1 = 0;
        idx2 = 1;
        idx3 = 2;
      }
      else if ((waypoints.size() <= (iter + 2))) {
        idx2 = 0;
        idx3 = 1;
      }
      else if ((waypoints.size() <= (iter + 3))) {
        idx3 = 0;
      }

      switch (current_curve) {

      case QUADRATIC_BEZIER:

        point[0] = uu * waypoints[idx][0] + 2 * u * t * waypoints[idx1][0] + tt * waypoints[idx2][0];
        point[1] = uu * waypoints[idx][1] + 2 * u * t * waypoints[idx1][1] + tt * waypoints[idx2][1];
        
        break;

      case CUBIC_BEZIER:

        point[0] = uuu * waypoints[idx][0] + 3 * uu * t * waypoints[idx1][0] + 3 * u * tt* waypoints[idx2][0] + ttt * waypoints[idx3][0];
        point[1] = uuu * waypoints[idx][1] + 3 * uu * t * waypoints[idx1][1] + 3 * u * tt* waypoints[idx2][1] + ttt * waypoints[idx3][1];

        break;

      case CATMULL_ROM:

        point[0] = 0.5f * ((-t) * uu * waypoints[idx][0] + (2 - 5 * tt + 3 * ttt) * waypoints[idx1][0] + t * (1 + 4 * t - 3 * tt) * waypoints[idx2][0] - tt * u * waypoints[idx3][0]);
        point[1] = 0.5f * ((-t) * uu * waypoints[idx][1] + (2 - 5 * tt + 3 * ttt) * waypoints[idx1][1] + t * (1 + 4 * t - 3 * tt) * waypoints[idx2][1] - tt * u * waypoints[idx3][1]);

        break;
      }

      return point;
    }

  public:
    /// this is called when we construct the class before everything is initialised.
    example_box(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      perlin_noise = perlin();
      pg = points_generator();

      current_curve = CATMULL_ROM;
      track_length = 10;

      glGenBuffers(1, &vertex_buffer); // Sets up our vertex array buffer for rendering
      road_shader.init(load_file("shaders/road.vert").c_str(), load_file("shaders/road.frag").c_str()); // loads, compiles and links our shader programs

      refresh_curve();
    }

    void file_create() {
      float scale_mult = 20.0f / TRACK_WIDTH;
      std::vector<float> vertexData = vertBuff;
      for (int i = 0; i < vertexData.size(); i++) {
        vertexData[i] *= scale_mult;
      }

      std::ofstream raceTrack;
      raceTrack.open("raceTrack.ply");

      raceTrack << "ply\n";
      raceTrack << "format ascii 1.0\n";
      raceTrack << "element vertex " << (int)vertexData.size() / 3 << "\n";
      raceTrack << "property float x\n";
      raceTrack << "property float y\n";
      raceTrack << "property float z\n";
      raceTrack << "element face " << (int)faceBuff.size() / 3 << "\n";
      raceTrack << "property list uint8 int32 vertex_indices\n";
      raceTrack << "end_header\n";

      //vertices
      for (int i = 0; i < vertexData.size(); i++) {
        raceTrack << vertexData[i] << " ";
        if ((i + 1) % 3 == 0) {
          raceTrack << "\n";
        }
      }

      //faces
      for (int j = 0; j < faceBuff.size(); j++) {

        if ((j) % 3 == 0) {
          raceTrack << "3 ";
        }

        raceTrack << faceBuff[j] << " ";
        if ((j + 1) % 3 == 0) {
          raceTrack << "\n";
        }
      }
      raceTrack.close();
      

    }


    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      if (is_key_going_up(key_f5)) {
        refresh_curve();
      }

      if (is_key_going_down(key_f6)) {
        file_create();
      }

      if (is_key_going_up(key_f1)) {
        current_curve = QUADRATIC_BEZIER;
        refresh_curve();
      }
      if (is_key_going_up(key_f2)) {
        current_curve = CUBIC_BEZIER;
        refresh_curve();
      }
      if (is_key_going_up(key_f3)) {
        prev_curve = current_curve;
        current_curve = CATMULL_ROM;
        refresh_curve();
      }

      if (is_key_going_up(key_space)) {
        debug_mode = !debug_mode;
      }

      if (is_key_going_up(key_up)) {
        track_length++;
        refresh_curve();
      }
      if (is_key_going_up(key_down)) {
        track_length--;
        refresh_curve();
      }

      if (is_key_going_up(key_right)) {
        TRACK_WIDTH += 0.05f;
        refresh_curve();
      }
      if (is_key_going_up(key_left)) {
        TRACK_WIDTH -= 0.05f;
        refresh_curve();
      }

      if (is_key_going_up(key_f8)) {
        height_scale += 0.1f;
        refresh_curve();
      }
      if (is_key_going_up(key_f7)) {
        height_scale -= 0.1f;
        refresh_curve();
      }

      if (is_key_going_up(key_f10)) {
        DETAIL_STEP -= 0.01f;
        refresh_curve();
      }
      if (is_key_going_up(key_f9)) {
        DETAIL_STEP += 0.01f;
        refresh_curve();
      }

      if (debug_mode) {
        glClearColor(0.5f, 0.5f, 0.5f, 1); // Grey colour
        draw_debug();
      }
      else {
        glClearColor(0.3f, 0.67f, 0.28f, 1); // Grass green colour
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
        glBufferData(GL_ARRAY_BUFFER, vertBuff.size() * sizeof(GLfloat), &vertBuff[0], GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(attribute_pos);
        glUseProgram(road_shader.get_program());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertBuff.size() / 3);
        glBindVertexArray(attribute_pos);
      }
    }

    void draw_debug() {
      /* https://en.wikibooks.org/wiki/OpenGL_Programming/GLStart/Tut3 */

      // Draw the start and end waypoints in yellow
      glUseProgram(0);
      glColor3f(1.0f, 1.0f, 0.0f); //yellow colour
      glPointSize(5.0f);//set point size to 10 pixels
      glBegin(GL_POINTS); //starts drawing of points
      glVertex3f(waypoints[0][0], waypoints[0][1], waypoints[0][2]);
      glVertex3f(waypoints[waypoints.size() - 1][0], waypoints[waypoints.size() - 1][1], waypoints[waypoints.size() - 1][2]);
      glEnd();

      // Draw the waypoints
      glColor3f(1.0f, 0.0f, 0.0f); //red colour

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
        glVertex3f(debugBezBuff[i + 1][0], debugBezBuff[i + 1][1], debugBezBuff[i + 1][2]);
      }
      glEnd();//end drawing of Line_strip


      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER, vertBuff.size() * sizeof(GLfloat), &vertBuff[0], GL_DYNAMIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(attribute_pos);
      glUseProgram(road_shader.get_program());
      glDrawArrays(GL_LINE_STRIP, 0, vertBuff.size() / 3);
      glBindVertexArray(attribute_pos);

    }
  };
}
