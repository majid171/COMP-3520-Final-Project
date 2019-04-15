import processing.opengl.*;

final int N=4, M=5, f=20, RESOLUTION=10;

class Point{
  
  float x, y, z;
  Point(float _x, float _y, float _z) {
    x=_x; y=_y; z=_z; 
  }
}

Point[][] controls;
Point[][] surface;

// camera controls
float angle = 0,
      camera_x = 100,
      camera_y = 0,
      camera_z = 0,
      target_x = (f*N)/2,
      target_y = (f*M)/2;

void setup() {
  size(600, 600, OPENGL);
  setupSurface();
}

void setupSurface() {
  controls = new Point[N+1][M+1];
  for(int i=0; i<=N; i++) {
    for(int j=0; j<=M; j++) {
      controls[i][j] = new Point(f*i, f*j, random(-f,f));
    }
  }
  surface = new Point[N*RESOLUTION][M*RESOLUTION];
  fillSurface();
}

// boilerplate
void draw() {
  // lights!
  background(100,100,100);
  directionalLight(0, 0, 255, -1, 1, 1);
  directionalLight(100, 120, 40, 1, -1, -1);

  // camera!
  camera(target_x + camera_x, target_y + camera_y, camera_z,  // camera coordinate
         target_x, target_y, 0,                               // target coordinate
         0.0, 0.0, 1.0);                                      // zenith direction
  
  // action!
  stroke(0,0,150);
  drawSurfaceShape(surface);

  stroke(255);
  drawSurface(controls);
}


/**
 * draw surface as mesh
 */
void drawSurface(Point[][] points) {
  int N = points.length, M = points[0].length;
  for(int i=0; i<N; i++) {
    for(int j=0; j<M; j++) {

      // this point
      Point current = points[i][j];
      point(current.x, current.y, current.z);

      // "right" point
      if(i+1<N) {
        Point neighbour1 = points[i+1][j];
        line(current.x, current.y, current.z,
             neighbour1.x, neighbour1.y, neighbour1.z); }

      // "upper" point
      if(j+1<M) {
        Point neighbour2 = points[i][j+1];
        line(current.x, current.y, current.z,
             neighbour2.x, neighbour2.y, neighbour2.z); }
    }
  }
}

/**
 * Draw surface as quads
 */
void drawSurfaceShape(Point[][] points) {
  //noStroke();
  int N = points.length, M = points[0].length;
  for(int i=0; i<N; i++) {
    for(int j=0; j<M; j++) {
      if(i+1<N && j+1<M) {
        Point p1 = points[i][j];
        Point p2 = points[i+1][j];
        Point p3 = points[i+1][j+1];
        Point p4 = points[i][j+1];
        
        //noFill();
        stroke(255, 0, 0);
        beginShape(QUADS);
        vertex(p1.x, p1.y, p1.z);
        vertex(p2.x, p2.y, p2.z);
        vertex(p3.x, p3.y, p3.z);
        vertex(p4.x, p4.y, p4.z);
        endShape();
      }
    }
  }
}

/**
 * move the mouse to change the camera
 */
void mouseMoved() {
  angle = - map(mouseX, 0, width, 0, 2*PI);
  camera_x = (100 * cos(angle));
  camera_y = (100 * sin(angle));
  camera_z = (height/2 - mouseY)/2;
  redraw();
}


// ==== SUPPORT FUNCTIONS ====


float mui, muj, bi, bj, bv;

/**
 * Perform surface point construction
 */
void fillSurface() {
  for(int i=0, ei=N*RESOLUTION; i<ei; i++) {
    mui = i/float(ei-1);
    for(int j=0, ej=M*RESOLUTION; j<ej; j++) {
      muj = j/float(ej-1);
      surface[i][j] = new Point(0,0,0);
      interpolate(surface, i, j, mui, muj, N, M);
    }
  }
}


/**
 * Perform bezier surface point interpolation
 */
void interpolate(Point[][] surface, int i, int j, float mui, float muj, int N, int M) {
  for(int ki=0; ki<=N; ki++) {
    bi = bezierBlend(ki, mui, N);
    for(int kj=0; kj<=M; kj++) {
      bj = bezierBlend(kj, muj, M);
      surface[i][j].x += controls[ki][kj].x * bi * bj;
      surface[i][j].y += controls[ki][kj].y * bi * bj;
      surface[i][j].z += controls[ki][kj].z * bi * bj;
    }
  }
}


/**
 * Bezier interpolation function
 */
float bezierBlend(float k, float mu, int n) {
  float kn = k,
        nkn = n-k;
  float blend_value = 1;

  for(int nn=n; nn >= 1; nn--) {
    blend_value *= nn;
    if (kn > 1) { blend_value /= kn--; }
    if (nkn > 1) { blend_value /= nkn--; }
  }

  if (k > 0) { blend_value *= pow(mu, k); }
  if (n-k > 0) { blend_value *= pow(1-mu, n-k); }
    
  return blend_value;
}
