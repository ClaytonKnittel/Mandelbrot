import java.io.*;
import java.nio.ByteBuffer;

final int its = 2000;
final int P = 2;
final float N = 1e5;
final float logP = log(P);
final float logN = log(N);

int nframes;
double[][] temp;
color[][] frames;
int frame;

final int density = 2;

final String infile = "/users/claytonknittel/VSCode/algorithms/test.dat";

public static double[] toDoubleArray(byte[] byteArray){
    int times = Double.SIZE / Byte.SIZE;
    double[] doubles = new double[byteArray.length / times];
    for(int i=0;i<doubles.length;i++){
        doubles[i] = ByteBuffer.wrap(byteArray, i*times, times).getDouble();
    }
    return doubles;
}

void settings() {
  try {
    DataInputStream in = new DataInputStream(new FileInputStream(infile));
    int w, h;
        
    //for (byte b : bytes) {
    //  println(String.format("0x%02X", b));
    //}
    
    w = in.readInt();
    h = in.readInt();
    nframes = (int) in.readLong();
    temp = new double[nframes][w * h];
    frames = new color[nframes][w * h];
    int frame = 0, idx = 0;
    int scale = 8;
    byte[] b = new byte[w * h * scale];
    while(frame < nframes) {
      in.read(b);
      for(int i=0; i<b.length / scale; i++){
        temp[frame][i] = ByteBuffer.wrap(b, i*scale, scale).getDouble();
      }
      frame++;
    }
    in.close();
    
    size(w / density, h / density);
    //fullScreen();
    pixelDensity(density);
  } catch (IOException ex) {
    ex.printStackTrace();
    System.exit(-1);
  }
}

double div(Complex c) {
  Complex z = new Complex(0, 0);
  
  for (int i = 0; i < its; i++) {
    z = z.square().add(c);
    double mag = z.mag();
    if (mag >= N) {
      return i - Math.log(Math.log(mag) / logN) / logP;
    }
  }
  return -1;
}

color transform(float i) {
  if (i < 0) {
    return color(0, 0, 0);
  }
  float decay = 1 - exp(-i / 20.f);
  decay *= decay;
  float angle = (360 * (float) i / 180 + 180) % 360;
  return color(angle, 100, decay * 100);
}

double dlerp(double l, double r, double x) {
  return x * (r - l) + l;
}

void setup() {
  frameRate(30);
  colorMode(HSB, 360, 100, 100);
  
  for (int frame = 0; frame < nframes; ++frame) {
    for (int idx = 0; idx < width * height * density * density; ++idx) {
      frames[frame][idx] = transform((float) temp[frame][idx]);
    }
  }
  temp = null;
}

void draw() {
  loadPixels();
  for (int i = 0; i < density * density * width * height; i++) {
    pixels[i] = frames[frame][i];
  }
  updatePixels();
  frame = (frame + 1) % frames.length;
}


/*

void setup() {
  frameRate(30);
  colorMode(HSB, 360, 100, 100);
  frame = 0;
  double scale = .95;
  
  frames = new color[nframes][density * density * width * height];
  //double xc = -.77568377f, yc = .13646737, rx = .0015;
  double xc = -.77568377, yc = .13646737, rx = .3;
 
  //loadPixels();
  int w = width * density;
  int h = height * density;
  for (int i = 0; i < nframes; i++) {
    println(i);
    double ry = rx * height / width;
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        Complex c = new Complex(dlerp(xc - rx, xc + rx, (double) x / w),
                                dlerp(yc - ry, yc + ry, (double) y / h));
        color col;
        double d = div(c);
        //println(c.r + "\t" + c.i + "\t" + d);
        if (d >= 0) {
          col = transform((float) d);
        }
        else {
          col = color(0, 0, 0);
        }
        frames[i][x + y * w] = col;
      }
    }
    rx = scale * rx;
  }
  //updatePixels();
  //save("big_mandel.jpg");
}
*/
